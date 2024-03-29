#ifndef _HA_STAR_HPP_
#define _HA_STAR_HPP_


//#define HIERARCHICAL_A_STAR_CACHE_H_STAR
#define HIERARCHICAL_A_STAR_CACHE_P_MINUS_G
#define HIERARCHICAL_A_STAR_CACHE_OPTIMAL_PATHS
#define HIERARCHICAL_A_STAR_REEXPANSION_COUNTING


#ifdef HIERARCHICAL_A_STAR_CACHE_OPTIMAL_PATHS
#ifndef HIERARCHICAL_A_STAR_CACHE_H_STAR
#ifndef HIERARCHICAL_A_STAR_CACHE_P_MINUS_G
#error "Optimal path caching can only be used if either h* caching or P-g caching is enabled"
#endif
#endif
#endif

#ifdef HIERARCHICAL_A_STAR_CACHE_H_STAR
#ifdef HIERARCHICAL_A_STAR_CACHE_P_MINUS_G
#error "h* caching and P-g caching are mutually exclusive"
#endif
#endif


#include <cassert>
#include <functional>
#include <iostream>
#include <vector>

#include <boost/array.hpp>
#include <boost/none.hpp>
#include <boost/optional.hpp>
#include <boost/pool/pool.hpp>
#include <boost/pool/pool_alloc.hpp>
#include <boost/unordered_map.hpp>
#include <boost/utility.hpp>

#include "search/BucketPriorityQueue.hpp"
#include "util/PointerOps.hpp"


template <
  class DomainT,
  class NodeT
  >
class HAStar : boost::noncopyable
{
public:
  typedef DomainT Domain;
  typedef NodeT Node;


private:
  typedef typename Node::Cost Cost;
  typedef typename Node::State State;

  typedef BucketPriorityQueue<Node> Open;
  typedef boost::optional<typename Open::ItemPointer> MaybeItemPointer;

  typedef boost::unordered_map<
    Node *,
    MaybeItemPointer,
    PointerHash<Node>,
    PointerEq<Node>,
    boost::fast_pool_allocator< std::pair<Node * const, MaybeItemPointer > >
    > Closed;
  typedef typename Closed::iterator ClosedIterator;
  typedef typename Closed::const_iterator ClosedConstIterator;


#ifdef HIERARCHICAL_A_STAR_REEXPANSION_COUNTING
  typedef boost::unordered_map<State, unsigned> ExpansionCount;
#endif


#ifdef HIERARCHICAL_A_STAR_CACHE_OPTIMAL_PATHS
  typedef boost::unordered_map<
    State,
    std::pair<bool, Cost>
    > Cache;

  inline static Cost get_cost(const std::pair<bool, Cost> &p)
  {
    return p.second;
  }

  inline static void set_cost(std::pair<bool, Cost> &p, Cost c)
  {
    p.second = c;
  }

  inline static bool is_exact_cost(const std::pair<bool, Cost> &p)
  {
    return p.first;
  }

  inline static void set_exact(std::pair<bool, Cost> &p)
  {
    p.first = true;
  }
#else
  typedef boost::unordered_map<
    State,
    Cost
    // , boost::hash<State>,
    // std::is_equal<State>,
    // boost::fast_pool_allocator< std::pair<State const, Cost> >
    > Cache;

  static Cost get_cost(Cost c)
  {
    return c;
  }

  static void set_cost(Cost &p, Cost c)
  {
    p = c;
  }
#endif

  typedef typename Cache::iterator CacheIterator;
  typedef typename Cache::const_iterator CacheConstIterator;


private:
  const static unsigned hierarchy_height = Domain::num_abstraction_levels + 1;

  const Node * goal;
  bool searched;

  Domain &domain;

  boost::array<unsigned, hierarchy_height> num_expanded;
  boost::array<unsigned, hierarchy_height> num_generated;

  boost::array<unsigned, hierarchy_height> cache_lookups;
  boost::array<unsigned, hierarchy_height> cache_hits;

  boost::array<Open, hierarchy_height> open;
  boost::array<Closed, hierarchy_height> closed;

  Cache cache;

  // The abstractions of the goal node at each level.  It makes sense
  // to compute these once up front, rather than repeatedly computing
  // them.
  boost::array<State, hierarchy_height> goal_abstractions;

  boost::array<boost::pool<> *, hierarchy_height> node_pool;

#ifdef HIERARCHICAL_A_STAR_CACHE_P_MINUS_G
  // A per-level vector of nodes that were expanded during a search,
  // to speed up the P-g caching at the end.
  //
  // When a node is expanded, a pointer to it is appended to this
  // vector.  To do P-g caching, only the nodes with pointers in this
  // vector need to be scanned, rather than the entire closed list.
  boost::array<std::vector<Node *>, hierarchy_height> expanded_nodes;
#endif


#ifdef HIERARCHICAL_A_STAR_REEXPANSION_COUNTING
  ExpansionCount expansion_count;
#endif


public:
  HAStar(Domain &domain)
    : goal(NULL)
    , searched(false)
    , domain(domain)
    , num_expanded()
    , num_generated()
    , cache_lookups()
    , cache_hits()
    , open()
    , closed()
    , cache()
    , goal_abstractions()
    , node_pool()
#ifdef HIERARCHICAL_A_STAR_CACHE_P_MINUS_G
    , expanded_nodes()
#endif
#ifdef HIERARCHICAL_A_STAR_REEXPANSION_COUNTING
    , expansion_count()
#endif
  {
    num_expanded.assign(0);
    num_generated.assign(0);

    for (unsigned i = 0; i < hierarchy_height; i += 1)
      goal_abstractions[i] = domain.abstract(i, domain.get_goal_state());

    for (unsigned i = 0; i < hierarchy_height; i += 1)
      node_pool[i] = new boost::pool<>(sizeof(Node));
  }

  ~HAStar()
  {
    for (unsigned i = 0; i < hierarchy_height; i += 1)
      delete node_pool[i];
  }

  void search()
  {
    if (searched)
      return;
    searched = true;

    goal = search_at_level(0, domain.get_start_state());
  }


  const Node * get_goal() const
  {
    return goal;
  }

  const Domain & get_domain() const
  {
    return domain;
  }

  unsigned get_num_generated() const
  {
    unsigned sum = 0;
    for (unsigned i = 0; i < hierarchy_height; i += 1)
      sum += num_generated[i];

    return sum;
  }

  unsigned get_num_generated(const unsigned level) const
  {
    assert(domain.is_valid_level(level));
    return num_generated[level];
  }

  unsigned get_num_expanded() const
  {
    unsigned sum = 0;
    for (unsigned i = 0; i < hierarchy_height; i += 1)
      sum += num_expanded[i];

    return sum;
  }

  unsigned get_num_expanded(const unsigned level) const
  {
    assert(domain.is_valid_level(level));
    return num_expanded[level];
  }


  void output_statistics(std::ostream &o) const
  {
    assert(searched);

    o << "final open/closed sizes:" << std::endl;
    dump_open_sizes(o);
    dump_closed_sizes(o);

    o << "nodes expanded/generated per level:" << std::endl;
    for (unsigned level = 0; level < hierarchy_height; level += 1) {
      o << "  " << level << ": " << num_expanded[level]
        << " / " << num_generated[level] << std::endl;
    }

    dump_cache_size(o);
    dump_cache_information(o);

#ifdef HIERARCHICAL_A_STAR_REEXPANSION_COUNTING
    dump_reexpansion_information(o);
#endif
  }


private:
  Node * search_at_level(const unsigned level, const State &start_state)
  {
    assert(domain.is_valid_level(level));
    assert(open[level].empty());
    assert(closed[level].empty());
#ifdef HIERARCHICAL_A_STAR_CACHE_P_MINUS_G
    assert(expanded_nodes[level].empty());
#endif

    Node *start_node =
      new (node_pool[level]->malloc()) Node(start_state,
                                            0,
                                            0,
                                            NULL);
    closed[level][start_node] = open[level].push(start_node);
    assert(closed[level].find(start_node) != closed[level].end());
    assert(open[level].size() == 1);
    assert(closed[level].size() == 1);

    Node *goal_node = NULL;

    std::vector<Node *> succs;
    while (!open[level].empty()) {
#ifdef OUTPUT_SEARCH_PROGRESS
      if (get_num_expanded() % 1000000 == 0) {
        std::cerr << get_num_expanded() << " total nodes expanded" << std::endl
                  << get_num_generated() << " total nodes generated" << std::endl;
        dump_open_sizes(std::cerr);
        dump_closed_sizes(std::cerr);
        dump_cache_size(std::cerr);
      }
#endif

      Node *n = open[level].top();
      open[level].pop();

      assert(closed[level].find(n) != closed[level].end());
      closed[level][n] = boost::none;

      if (n->get_state() == goal_abstractions[level]) {
        assert(n->get_h() == 0);
        goal_node = n;
        break;
      }

#ifdef HIERARCHICAL_A_STAR_CACHE_P_MINUS_G
      expanded_nodes[level].push_back(n);
#endif

#ifdef HIERARCHICAL_A_STAR_CACHE_OPTIMAL_PATHS
      {
        // cache_lookups[level] += 1;
        CacheConstIterator cache_it = cache.find(n->get_state());
        if (cache_it != cache.end() && is_exact_cost(cache_it->second)) {
          // cache_hits[level] += 1;
          
          // Create a goal node to insert into the open list.
          // 
          // Note that the parent pointer for this goal node is
          // somewhat bogus: the nodes along the optimal path from n
          // to the ``real'' goal node are lost.  To be really proper,
          // we would fix up the parent pointers for this synthesized
          // goal node.  However, because optimal path caching will
          // only occur at abstract levels (and not base levels), we
          // only really care about having the actual cost of the goal
          // node.
          //
          // We have the parent pointer of synthesized_goal point to
          // n.  This is necessary for further optimal path caching.
          assert(level != 0);
          Node *synthesized_goal =
            new (node_pool[level]->malloc()) Node(goal_abstractions[level],
                                                  n->get_g() + get_cost(cache_it->second),
                                                  0,
                                                  n);
          assert(closed[level].find(synthesized_goal) == closed[level].end());
          closed[level][synthesized_goal] = open[level].push(synthesized_goal);
          continue;
        }
      }
#endif      

      domain.compute_successors(*n, succs, *node_pool[level]);
      num_expanded[level] += 1;
      num_generated[level] += succs.size();
#ifdef HIERARCHICAL_A_STAR_REEXPANSION_COUNTING
      expansion_count[n->get_state()] += 1;
#endif

      for (unsigned succ_i = 0; succ_i < succs.size(); succ_i += 1)
        process_child(level, succs[succ_i]);
    } /* end while */

    if (goal_node == NULL) {
      std::cerr << "no goal found!" << std::endl;
      assert(false);
    }

    assert(goal_node->get_h() == 0);
    return goal_node;
  }


  void process_child(const unsigned level, Node *child)
  {
    assert(domain.is_valid_level(level));
    assert(open[level].size() <= closed[level].size());

    compute_heuristic(level, child);
    assert(child->get_state() != goal_abstractions[level] || child->get_h() == 0);

    ClosedIterator closed_it = closed[level].find(child);
    if (closed_it == closed[level].end()) {
      // The child has not been generated before.
      closed[level][child] = open[level].push(child);
    }
    else if (closed_it->second && child->get_f() < closed_it->first->get_f()) {
      // A worse version of the child is in the open list.
      open[level].erase(*closed_it->second);  // knock out the old one from
                                       // the open list

      // free the old, worse copy
      node_pool[level]->free(closed_it->first);
      closed[level].erase(closed_it);

      closed[level][child] = open[level].push(child);  // insert better version of
                                         // child
    }
    else {
      // The child has either already been expanded, or is worse
      // than the version in the open list.
      node_pool[level]->free(child);
    }
  }


  void compute_heuristic (const unsigned level, Node *start_node)
  {
    assert(domain.is_valid_level(level));
    assert(start_node != NULL);

    const State &goal_state = goal_abstractions[level];
    assert(goal_state == domain.abstract(level, domain.get_goal_state()));

    const State &start_state = start_node->get_state();

    // This conditional shouldn't have to be here, I think!
    if (start_state == goal_state) {
      start_node->set_h(0);
      return;
    }

    // The following code checks the cache for the given node.  This
    // is different from the other hierarchical heuristic search
    // algorithms, which check the cache for an abstraction of the
    // given node.
    //
    // I thought this was a bug when I first noticed it.  However,
    // changing it to match the other algorithms breaks things
    // horribly, causing HA* to not find solutions at all.  Hence, I
    // leave things as they are.  This implementation seems to find
    // correct solutions for 15-puzzles and Macro 15-puzzles.
    //
    // I don't understand my code.  :-(
    cache_lookups[level] += 1;
    CacheIterator cache_it = cache.find(start_state);
    if (cache_it != cache.end()) {
      cache_hits[level] += 1;
      start_node->set_h(get_cost(cache_it->second));
      return;
    }

    const Cost epsilon = domain.get_epsilon(start_state);

    if (level == Domain::num_abstraction_levels) {
      start_node->set_h(start_state == goal_state ? 0 : epsilon);
      return;
    }

    const unsigned next_level = level + 1u;
    const State abstract_start = domain.abstract(next_level,
                                                 start_state);
    const State abstract_goal = domain.abstract(next_level,
                                                goal_state);

    Node *result = search_at_level(next_level, abstract_start);
    if (result == NULL) {
      std::cerr << "whoops, infinite heuristic estimate!" << std::endl;
      assert(false);  // for the domains I am running on, there should
                      // never be an infinite heuristic estimate.
    }
    assert(result->get_state() == abstract_goal);
    assert(result->get_h() == 0);
    assert(abstract_start != abstract_goal || result->get_h() == 0);

    Cost hval = std::max(epsilon, result->get_g());
    set_cost(cache[start_state], hval);
    assert(cache.find(start_state) != cache.end());
    assert(get_cost(cache.find(start_state)->second) == hval);

#ifdef HIERARCHICAL_A_STAR_CACHE_H_STAR
    cache_h_star(next_level, result);
#endif

#ifdef HIERARCHICAL_A_STAR_CACHE_P_MINUS_G
    cache_p_minus_g(next_level, result);
#endif

    // It is rather dirty, that all the search_at_level cleanup code
    // has to go here, outside of search_at_level...

#ifdef HIERARCHICAL_A_STAR_CACHE_P_MINUS_G
    expanded_nodes[next_level].clear();
#endif

    // TODO: rather than calling purge_memory here, if closed at each
    // level is itself allocated by a memory pool, the pool could be
    // cleared, and the expensive closed[next_level].clear() operation
    // could be avoided.  ~25% of the time is spent on that!
    node_pool[next_level]->purge_memory();
    closed[next_level].clear();
    assert(closed[next_level].empty());
    open[next_level].reset();
    assert(open[next_level].empty());

    start_node->set_h(hval);
  }


#ifdef HIERARCHICAL_A_STAR_CACHE_H_STAR
  void cache_h_star(const unsigned level, const Node *goal_node)
  {
    assert(goal_node != NULL);
    assert(goal_node->get_h() == 0);
    assert(open[level].empty() || goal_node->get_f() <= open[level].top()->get_f());

    const Cost cost_to_goal = goal_node->get_g();
    const Node *parent = goal_node->get_parent();

    while (parent != NULL) {
      assert(cost_to_goal >= parent->get_g());
      const Cost hval = cost_to_goal - parent->get_g();

      CacheIterator cache_it = cache.find(parent->get_state());
      if (cache_it != cache.end()) {
        assert(get_cost(cache_it->second) <= hval);
        set_cost(cache_it->second, hval);
#ifdef HIERARCHICAL_A_STAR_CACHE_OPTIMAL_PATHS
        set_exact(cache_it->second);
#endif
      }
      else {
        set_cost(cache[parent->get_state()], hval);
#ifdef HIERARCHICAL_A_STAR_CACHE_OPTIMAL_PATHS
        set_exact(cache[parent->get_state()]);
#endif
      }

      parent = parent->get_parent();
    } /* end while */
  }
#endif


#ifdef HIERARCHICAL_A_STAR_CACHE_P_MINUS_G
  void cache_p_minus_g(const unsigned level, const Node *goal_node)
  {
    assert(goal_node != NULL);
    assert(goal_node->get_h() == 0);
    assert(open[level].empty() || goal_node->get_f() <= open[level].top()->get_f());

    const Cost p = goal_node->get_g();

    for (unsigned i = 0; i < expanded_nodes[level].size(); i += 1) {
      const Node *node = expanded_nodes[level][i];
      const Cost g = node->get_g();
      const Cost p_minus_g = p - g;
      assert(g <= p);

      CacheIterator cache_it = cache.find(node->get_state());
      if (cache_it != cache.end()) {
        const Cost cached_cost = get_cost(cache_it->second);
        set_cost(cache_it->second, std::max(cached_cost, p_minus_g));
      }
      else
        set_cost(cache[node->get_state()], p_minus_g);
    } /* end for */

#ifdef HIERARCHICAL_A_STAR_CACHE_OPTIMAL_PATHS
#ifndef NDEBUG
    const Cost cost_to_goal = goal_node->get_g();
#endif
    const Node *parent = goal_node->get_parent();

    while (parent != NULL) {
      assert(cost_to_goal >= parent->get_g());
      CacheIterator cache_it = cache.find(parent->get_state());
      assert(cache_it != cache.end());
      assert(get_cost(cache_it->second) == cost_to_goal - parent->get_g());
      set_exact(cache_it->second);

      parent = parent->get_parent();
    } /* end while */
#endif
  }
#endif


  void dump_open_sizes(std::ostream &o) const
  {
    o << "open sizes: " << std::endl;
    for (unsigned level = 0; level < hierarchy_height; level += 1)
      o << "  " << level << ": " << open[level].size() << std::endl;
  }

  void dump_closed_sizes(std::ostream &o) const
  {
    o << "closed sizes: " << std::endl;
    for (unsigned level = 0; level < hierarchy_height; level += 1)
      o << "  " << level << ": " << closed[level].size() << std::endl;
  }

  void dump_cache_size(std::ostream &o) const
  {
    o << "cache size: " << cache.size() << std::endl;
  }

  void dump_cache_information(std::ostream &o) const
  {
    o << "cache information:" << std::endl;
    for (unsigned level = 0; level < hierarchy_height; level += 1) {
      const float hit_ratio =
        static_cast<float>(cache_hits[level]) / cache_lookups[level];

      o << "  " << level << ": "
        << cache_lookups[level] << " lookups, "
        << cache_hits[level] << " hits (" << hit_ratio << ")" << std::endl;
    }
  }

#ifdef HIERARCHICAL_A_STAR_REEXPANSION_COUNTING
  void dump_reexpansion_information(std::ostream &o) const
  {
    unsigned total_num_expanded = 0;
    unsigned num_reexpanded = 0;

    for (typename ExpansionCount::const_iterator it = expansion_count.begin();
         it != expansion_count.end();
         it++)
    {
      assert(it->second > 0);
      total_num_expanded += it->second;
      if (it->second > 1)
        num_reexpanded += it->second - 1;
    }

    o << total_num_expanded << " total expansions" << std::endl
      << num_reexpanded << " total reexpansions" << std::endl;
  }
#endif
};


#endif /* !_HA_STAR_HPP_ */

#ifndef _HA_STAR_HPP_
#define _HA_STAR_HPP_


//#define HIERARCHICAL_A_STAR_CACHE_H_STAR
//#define HIERARCHICAL_A_STAR_CACHE_P_MINUS_G
//#define HIERARCHICAL_A_STAR_CACHE_OPTIMAL_PATHS


#include <iostream>
#include <vector>

#include <boost/array.hpp>
#include <boost/none.hpp>
#include <boost/optional.hpp>
#include <boost/pool/pool_alloc.hpp>
#include <boost/unordered_map.hpp>
#include <boost/utility.hpp>

#include "search/BucketPriorityQueue.hpp"
#include "util/PointerOps.hpp"


template <
  class Domain,
  class Node
  >
class HAStar : boost::noncopyable
{
private:
  typedef typename Node::Cost Cost;
  typedef typename Node::State State;

  typedef BucketPriorityQueue<Node> Open;
  typedef boost::optional<typename Open::ItemPointer> MaybeItemPointer;

  typedef boost::unordered_map<
    Node *,
    MaybeItemPointer,
    PointerHash<Node>,
    PointerEq<Node>
    , boost::fast_pool_allocator< std::pair<Node * const, MaybeItemPointer > >
    > Closed;
  typedef typename Closed::iterator ClosedIterator;
  typedef typename Closed::const_iterator ClosedConstIterator;

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

  boost::array<Open, hierarchy_height> open;
  boost::array<Closed, hierarchy_height> closed;

  Cache cache;

public:
  HAStar(Domain &domain)
    : goal(NULL)
    , searched(false)
    , domain(domain)
    , num_expanded()
    , num_generated()
    , open()
    , closed()
    , cache()
  {
    num_expanded.assign(0);
    num_generated.assign(0);
  }

  ~HAStar()
  {
  }

  void search()
  {
    if (searched)
      return;
    searched = true;

    goal = search_at_level(0,
                           domain.get_start_state(),
                           domain.get_goal_state());
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


private:
  Node * search_at_level(const unsigned level,
                         const State &start_state,
                         const State &goal_state)
  {
    assert(domain.is_valid_level(level));
    assert(open[level].empty());
    assert(closed[level].empty());
    assert(goal_state == domain.abstract(level, domain.get_goal_state()));

    Node *start_node = domain.create_node(start_state,
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
      if (get_num_expanded() % 1000000 == 0) {
        std::cerr << get_num_expanded() << " total nodes expanded" << std::endl
                  << get_num_generated() << " total nodes generated" << std::endl;
        dump_open_sizes(std::cerr);
        dump_closed_sizes(std::cerr);
        dump_cache_size(std::cerr);
      }

      Node *n = open[level].top();
      open[level].pop();
      assert(closed[level].find(n) != closed[level].end());
      closed[level][n] = boost::none;

      if (n->get_state() == goal_state) {
        assert(n->get_h() == 0);
        goal_node = n;
        break;
      }

      domain.compute_successors(*n, succs);
      num_expanded[level] += 1;
      num_generated[level] += succs.size();

      for (unsigned succ_i = 0; succ_i < succs.size(); succ_i += 1)
        process_child(level, succs[succ_i], goal_state);
    } /* end while */

    if (goal_node == NULL) {
      std::cerr << "no goal found!" << std::endl;
      assert(false);
    }

    // std::cerr << "solution at level " << level << " has length "
    //           << goal_node->num_nodes_to_start() << std::endl;

    assert(goal_node->get_h() == 0);
    return goal_node;
  }


  void process_child(const unsigned level,
                     Node *child,
                     const State &goal_state)
  {
    assert(domain.is_valid_level(level));
    // assert(open[level].invariants_satisfied());
    assert(open[level].size() <= closed[level].size());
    assert(goal_state == domain.abstract(level, domain.get_goal_state()));

    compute_heuristic(level, child, goal_state);
    assert(child->get_state() != goal_state || child->get_h() == 0);

    ClosedIterator closed_it = closed[level].find(child);
    if (closed_it == closed[level].end()) {
      // The child has not been generated before.
      closed[level][child] = open[level].push(child);
    }
    else if (closed_it->second && child->get_f() < closed_it->first->get_f()) {
      // A worse version of the child is in the open list.
      open[level].erase(*closed_it->second);  // knock out the old one from
                                       // the open list

      domain.free_node(closed_it->first);     // free the old, worse
                                              // copy
      closed[level].erase(closed_it);

      closed[level][child] = open[level].push(child);  // insert better version of
                                         // child
    }
    else {
      // The child has either already been expanded, or is worse
      // than the version in the open list.
      domain.free_node(child);
    }

    // assert(open[level].invariants_satisfied());
  }


  void compute_heuristic (const unsigned level,
                          Node *start_node,
                          const State &goal_state)
  {
    assert(domain.is_valid_level(level));
    assert(start_node != NULL);
    assert(goal_state == domain.abstract(level, domain.get_goal_state()));

    const State &start_state = start_node->get_state();

    // This conditional shouldn't have to be here, I think!
    if (start_state == goal_state) {
      start_node->set_h(0);
      return;
    }

    CacheIterator cache_it = cache.find(start_state);
    if (cache_it != cache.end()) {
      start_node->set_h(get_cost(cache_it->second));
      return;
    }

    Cost epsilon = domain.get_epsilon(start_state);
    assert(epsilon == 1);

    if (level == Domain::num_abstraction_levels) {
      start_node->set_h(start_state == goal_state ? 0 : epsilon);
      return;
    }

    const unsigned next_level = level + 1u;
    const State abstract_start = domain.abstract(next_level,
                                                 start_state);
    const State abstract_goal = domain.abstract(next_level,
                                                goal_state);

    Node *result = search_at_level(next_level, abstract_start, abstract_goal);
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

    free_all_nodes(closed[next_level]);
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
      }
      else
        set_cost(cache[parent->get_state()], hval);

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

    unsigned num_increased = 0;
    unsigned num_potential = 0;

    for(ClosedConstIterator closed_it = closed[level].begin();
        closed_it != closed[level].end();
        ++closed_it) {
      if (!closed_it->second) {
        num_potential += 1;
        const Cost g = closed_it->first->get_g();
        const Cost h = closed_it->first->get_h();
        const Cost p_minus_g = p - g;

        if (g > p) {
          std::cerr << "p_minus_g is " << p_minus_g << std::endl
                    << "p is " << p << std::endl
                    << "g is " << g << std::endl
                    << "h is " << h << std::endl;
        }
        assert(g <= p);

        CacheIterator cache_it = cache.find(closed_it->first->get_state());
        if (cache_it != cache.end()) {
          if (get_cost(cache_it->second) < p_minus_g)
            num_increased += 1;
          set_cost(cache_it->second, std::max(cache_it->second, p_minus_g));
        }
        else
          set_cost(cache[closed_it->first->get_state()], p_minus_g);
      }
    } /* end for */

    // std::cerr << "P-g caching increased the cached heuristic value for "
    //           << num_increased << " out of " << num_potential << " nodes" << std::endl;
  }
#endif


  void free_all_nodes(Closed &closed_level)
  {
    for (ClosedIterator closed_it = closed_level.begin();
         closed_it != closed_level.end();
         ++closed_it)
      domain.free_node(closed_it->first);
  }



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
};


#endif /* !_HA_STAR_HPP_ */

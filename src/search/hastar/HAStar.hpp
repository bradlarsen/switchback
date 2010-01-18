#ifndef _HA_STAR_HPP_
#define _HA_STAR_HPP_


//#define HIERARCHICAL_A_STAR_CACHE_H_STAR
//#define HIERARCHICAL_A_STAR_CACHE_P_MINUS_G
//#define HIERARCHICAL_A_STAR_CACHE_OPTIMAL_PATHS

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


#include <functional>
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


  struct CacheEntry
  {
    // A pointer into the open list...
    MaybeItemPointer open_ptr;

    // ...valid during the indicated search iteration.
    //
    // This counter is necessary to use a single data structure as
    // both closed list and heuristic cache.
    unsigned search_iteration;

#ifdef HIERARCHICAL_A_STAR_CACHE_OPTIMAL_PATHS
    boost::optional< std::pair<bool, Cost> > cached_heuristic;
#else
    boost::optional<Cost> cached_heuristic;
#endif

    // The default constructor, kind of a dummy constructor, but
    // necessary for use in the unordered_map.
    //
    // The open_ptr and cached_heuristic fields are initialized to
    // safe values.  search_iteration is initialized to 0.  0 is not a
    // valid search iteration!
    CacheEntry()
      : open_ptr(boost::none)
      , search_iteration(0)
      , cached_heuristic(boost::none)
    {
    }


    CacheEntry(const MaybeItemPointer &open_ptr,
               unsigned search_iteration,
#ifdef HIERARCHICAL_A_STAR_CACHE_OPTIMAL_PATHS
               const boost::optional< std::pair<bool, Cost> > &cached_heuristic
#else
               const boost::optional<Cost> &cached_heuristic
#endif
               )
      : open_ptr(open_ptr)
      , search_iteration(search_iteration)
      , cached_heuristic(cached_heuristic)
    {
    }

    const MaybeItemPointer & get_open_ptr() const
    {
      return open_ptr;
    }

    void set_open_ptr(const MaybeItemPointer &ptr)
    {
      open_ptr = ptr;
    }

    unsigned get_search_iteration() const
    {
      return search_iteration;
    }

    void set_search_iteration (unsigned iter)
    {
      assert(iter > search_iteration);
      search_iteration = iter;
    }

#ifdef HIERARCHICAL_A_STAR_CACHE_OPTIMAL_PATHS
    bool is_exact_estimate() const
    {
      return cached_heuristic && cached_heuristic->first;
    }

    void set_exact_estimate()
    {
      assert(cached_heuristic);
      cached_heuristic->first = true;
    }
#endif

    Cost get_cached_heuristic() const
    {
      assert(cached_heuristic);
#ifdef HIERARCHICAL_A_STAR_CACHE_OPTIMAL_PATHS      
      return cached_heuristic->second;
#else
      return *cached_heuristic;
#endif
    }

    void set_cached_heuristic(Cost h)
    {
#ifdef HIERARCHICAL_A_STAR_CACHE_OPTIMAL_PATHS
      if (cached_heuristic) {
        assert(!cached_heuristic->first || cached_heuristic->second >= h);
        cached_heuristic->second = h;
      }
      else
        cached_heuristic = std::make_pair(false, h);
#else
      if (cached_heuristic){
        assert(*cached_heuristic <= h);
        cached_heuristic = h;
      }
      else
        cached_heuristic = h;
#endif
    }
  };


  // Cache is both the closed list and the heuristic estimate cache.
  // ``Closed list'' here is a misnomer, as this contains a pointer to
  // one copy of every state every generated, and so contains pointers
  // to all nodes on open, as well as all nodes that are truly closed.
  typedef boost::unordered_map<
    Node *,
    CacheEntry,
    PointerHash<Node>,
    PointerEq<Node>,
    boost::fast_pool_allocator< std::pair<Node * const, CacheEntry > >
    > Cache;
  typedef typename Cache::iterator CacheIterator;
  typedef typename Cache::const_iterator CacheConstIterator;


private:
  const static unsigned hierarchy_height = Domain::num_abstraction_levels + 1;

  const Node * goal;
  bool searched;

  Domain &domain;

  boost::array<unsigned, hierarchy_height> num_expanded;
  boost::array<unsigned, hierarchy_height> num_generated;
  boost::array<unsigned, hierarchy_height> num_searches;

  // An open list for each level.
  boost::array<Open, hierarchy_height> open;

  // The cache of heuristic values, and the closed list, for all levels.
  Cache cache;

  // The abstractions of the goal node at each level.  It makes sense
  // to compute these once up front, rather than repeatedly computing
  // them.
  boost::array<State, hierarchy_height> goal_abstractions;

#ifdef HIERARCHICAL_A_STAR_CACHE_P_MINUS_G
  // A per-level vector of nodes that were expanded during a search,
  // to speed up the P-g caching at the end.
  //
  // When a node is expanded, a pointer to it is appended to this
  // vector.  To do P-g caching, only the nodes with pointers in this
  // vector need to be scanned, rather than the entire closed list.
  boost::array<std::vector<Node *>, hierarchy_height> expanded_nodes;
#endif


public:
  HAStar(Domain &domain)
    : goal(NULL)
    , searched(false)
    , domain(domain)
    , num_expanded()
    , num_generated()
    , num_searches()
    , open()
    , cache()
    , goal_abstractions()
#ifdef HIERARCHICAL_A_STAR_CACHE_P_MINUS_G
    , expanded_nodes()
#endif
  {
    num_expanded.assign(0);
    num_generated.assign(0);
    num_searches.assign(0);
    for (unsigned i = 0; i < hierarchy_height; i += 1)
      goal_abstractions[i] = domain.abstract(i, domain.get_goal_state());
  }


  ~HAStar()
  {
    for (CacheIterator cache_it = cache.begin();
         cache_it != cache.end();
         ++cache_it) {
      domain.free_node(cache_it->first);
    }
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


private:
  Node * search_at_level(const unsigned level,
                         const State &start_state)
  {
    assert(domain.is_valid_level(level));
    assert(open[level].empty());
#ifdef HIERARCHICAL_A_STAR_CACHE_P_MINUS_G
    assert(expanded_nodes[level].empty());
#endif

    num_searches[level] += 1;

    Node *start_node = domain.create_node(start_state,
                                          0,
                                          0,
                                          NULL);
    insert_node(level, start_node);
    assert(open[level].size() == 1);

    Node *goal_node = NULL;

    std::vector<Node *> succs;
    while (!open[level].empty()) {
#ifdef OUTPUT_SEARCH_PROGRESS
      if (get_num_expanded() % 1000000 == 0) {
        std::cerr << get_num_expanded() << " total nodes expanded" << std::endl
                  << get_num_generated() << " total nodes generated" << std::endl;
        dump_open_sizes(std::cerr);
        dump_cache_size(std::cerr);
      }
#endif

      Node *n = open[level].top();
      open[level].pop();
      assert(cache.find(n) != cache.end());
      assert(cache.find(n)->second.get_search_iteration() == num_searches[level]);
      cache.find(n)->second.set_open_ptr(boost::none);

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
        CacheConstIterator cache_it = cache.find(n);
        if (cache_it != cache.end() && cache_it->second.is_exact_estimate()) {
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
            domain.create_node(goal_abstractions[level],
                               n->get_g() + cache_it->second.get_cached_heuristic(),
                               0,
                               n);
          assert(!node_has_been_expanded(level, synthesized_goal));
          insert_node(level, synthesized_goal);
          continue;
        }
      }
#endif      

      domain.compute_successors(*n, succs);
      num_expanded[level] += 1;
      num_generated[level] += succs.size();

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


  void process_child(const unsigned level,
                     Node *child)
  {
    assert(domain.is_valid_level(level));

    compute_heuristic(level, child);
    assert(child->get_state() != goal_abstractions[level] || child->get_h() == 0);

    CacheIterator cache_it = cache.find(child);
    if (cache_it == cache.end()) {
      // The child has not been generated before.
      insert_node(level, child);
    }
    else if (node_has_been_expanded(level, cache_it->first)) {
      assert(!node_is_open(level, child));
      // The child has already been expanded.  Drop duplicates!
      domain.free_node(child);
    }
    else if (!node_is_open(level, cache_it->second)) {
      assert(!node_has_been_expanded(level, child));
      // The child has not been generated before.
      insert_node(level, child);
    }
    else if (child->get_f() < cache_it->first->get_f()) {
      assert(!node_has_been_expanded(level, child));
      assert(node_is_open(level, child));
      // A worse version of the child is in the open list.

      // knock out the old one from the open list
      open[level].erase(*cache_it->second.get_open_ptr());

      // free the old, worse copy
      domain.free_node(cache_it->first);

      // ******* THE NEXT LINE IS PROBABLY BAD ******* SHOULD I EVEN
      // HAVE THIS ADJUSTMENT OF NODES IN THE OPEN LIST, SINCE THE
      // HEURISTIC USED IS ADMISSIBLE?
      cache.erase(cache_it);

      // insert better version of child
      insert_node(level, child);
    }
    else {
      assert(!node_has_been_expanded(level, child));
      assert(node_is_open(level, child));
      assert(child->get_f() >= cache_it->first->get_f());
      // The child has already been generated, but is worse
      // than the version in the open list.
      domain.free_node(child);
    }
  }


  void insert_node (const unsigned level,
                    Node *node)
  {
    assert(!node_is_open(level, node));
    assert(!node_has_been_expanded(level, node));

    typename Open::ItemPointer open_ptr = open[level].push(node);
    CacheIterator cache_it = cache.find(node);
    if (cache_it == cache.end()) {
      CacheEntry entry(open_ptr,
                       num_searches[level],
#ifdef HIERARCHICAL_A_STAR_CACHE_OPTIMAL_PATHS
                       std::make_pair(false, node->get_h())
#else
                       node->get_h()
#endif
                      );
      cache.insert(std::make_pair(node, entry));
    }
    else {
      cache_it->second.set_open_ptr(open_ptr);
      cache_it->second.set_search_iteration(num_searches[level]);
    }

    assert(node_is_open(level, node));
    assert(!node_has_been_expanded(level, node));
  }


  bool node_has_been_expanded(const unsigned level,
                              Node *node) const
  {
    CacheConstIterator cache_it = cache.find(node);

    if (cache_it == cache.end())
      return false;
    if (cache_it->second.get_open_ptr())
      return false;
    assert(num_searches[level] > 0);
    if (cache_it->second.get_search_iteration() != num_searches[level])
      return false;

    return true;
  }


  bool node_is_open (const unsigned level,
                     Node *node) const
  {
    CacheConstIterator cache_it = cache.find(node);
    return cache_it != cache.end() && node_is_open(level, cache_it->second);
  }


  bool node_is_open (const unsigned level,
                     const CacheEntry &entry) const
  {
    return
      entry.get_open_ptr() &&
      entry.get_search_iteration() == num_searches[level];
  }


  void compute_heuristic (const unsigned level,
                          Node *start_node)
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

    CacheIterator cache_it = cache.find(start_node);
    if (cache_it != cache.end()) {
      start_node->set_h(cache_it->second.get_cached_heuristic());
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

    const Cost hval = std::max(epsilon, result->get_g());
    assert(cache.find(start_node) == cache.end());
    cache[start_node].set_cached_heuristic(hval);
    assert(cache.find(start_node) != cache.end());
    assert(cache.find(start_node)->second.get_cached_heuristic() == hval);

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

    open[next_level].reset();
    assert(open[next_level].empty());

    start_node->set_h(hval);
  }


#ifdef HIERARCHICAL_A_STAR_CACHE_H_STAR
  void cache_h_star(const unsigned level, Node *goal_node)
  {
    assert(goal_node != NULL);
    assert(goal_node->get_h() == 0);
    assert(open[level].empty() || goal_node->get_f() <= open[level].top()->get_f());

    const Cost cost_to_goal = goal_node->get_g();
    Node *parent = goal_node->get_parent();

    while (parent != NULL) {
      assert(cost_to_goal >= parent->get_g());
      const Cost hval = cost_to_goal - parent->get_g();

      CacheIterator cache_it = cache.find(parent);
      assert(cache_it != cache.end());
      assert(cache_it->second.get_search_iteration() == num_searches[level]);
      assert(!node_is_open(level, cache_it->second));
      const Cost old_hval = cache_it->second.get_cached_heuristic();
      assert(old_hval <= hval);
      cache_it->second.set_cached_heuristic(hval);
#ifdef HIERARCHICAL_A_STAR_CACHE_OPTIMAL_PATHS
      cache_it->second.set_exact_estimate();
#endif
      parent = parent->get_parent();
    } /* end while */
  }
#endif


#ifdef HIERARCHICAL_A_STAR_CACHE_P_MINUS_G
  void cache_p_minus_g(const unsigned level, Node *goal_node)
  {
    assert(goal_node != NULL);
    assert(goal_node->get_h() == 0);
    assert(open[level].empty() || goal_node->get_f() <= open[level].top()->get_f());

    const Cost p = goal_node->get_g();

    for (unsigned i = 0; i < expanded_nodes[level].size(); i += 1) {
      Node *node = expanded_nodes[level][i];
      const Cost g = node->get_g();
      const Cost p_minus_g = p - g;
      assert(g <= p);

      CacheIterator cache_it = cache.find(node);
      assert (cache_it != cache.end());
      const Cost cached_cost = cache_it->second.get_cached_heuristic();
      cache_it->second.set_cached_heuristic(std::max(cached_cost, p_minus_g));
    } /* end for */

#ifdef HIERARCHICAL_A_STAR_CACHE_OPTIMAL_PATHS
#ifndef NDEBUG
    const Cost cost_to_goal = goal_node->get_g();
#endif
    Node *parent = goal_node->get_parent();

    while (parent != NULL) {
      assert(cost_to_goal >= parent->get_g());
      CacheIterator cache_it = cache.find(parent);
      assert(cache_it != cache.end());
      assert(cache_it->second.get_cached_heuristic() == cost_to_goal - parent->get_g());
      cache_it->second.set_exact_estimate();
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

  void dump_cache_size(std::ostream &o) const
  {
    o << "cache size: " << cache.size() << std::endl;
  }
};


#endif /* !_HA_STAR_HPP_ */

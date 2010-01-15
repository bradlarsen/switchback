#ifndef _HA_STAR_HPP_
#define _HA_STAR_HPP_


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

  typedef boost::unordered_map<
    State,
    Cost
    // , boost::hash<State>,
    // std::is_equal<State>,
    // boost::fast_pool_allocator< std::pair<State const, Cost> >
    > Cache;

  typedef typename Cache::iterator CacheIterator;


private:
  const static unsigned hierarchy_height = Domain::num_abstraction_levels + 1;

  const Node * goal;
  bool searched;

  const Domain &domain;

  boost::array<unsigned, hierarchy_height> num_expanded;
  boost::array<unsigned, hierarchy_height> num_generated;

  boost::array<Open, hierarchy_height> open;
  boost::array<Closed, hierarchy_height> closed;

  Cache cache;

public:
  HAStar(const Domain &domain)
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
        goal_node = n;
        break;
      }

      domain.compute_successors(*n, succs);
      num_expanded[level] += 1;
      num_generated[level] += succs.size();

      for (unsigned succ_i = 0; succ_i < succs.size(); succ_i += 1)
        process_child(level, n, succs[succ_i]);
    } /* end while */

    // The following line causes things to break.

    // free_all_nodes(closed[level]);
    closed[level].clear();
    open[level].reset();
    
    if (goal_node == NULL) {
      std::cerr << "no goal found!" << std::endl;
      assert(false);
    }

    // std::cerr << "solution at level " << level << " has length "
    //           << goal_node->num_nodes_to_start() << std::endl;

    return goal_node;
  }


  void process_child(const unsigned level,
                     const Node *parent,
                     Node *child)
  {
    assert(domain.is_valid_level(level));
    assert(open[level].invariants_satisfied());
    assert(open[level].size() <= closed[level].size());

    child->set_h(heuristic(level, child->get_state(), parent->get_state()));

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

    assert(open[level].invariants_satisfied());
  }


  Cost heuristic (const unsigned level,
                  const State &start_state,
                  const State &goal_state)
  {
    assert(domain.is_valid_level(level));

    CacheIterator cache_it = cache.find(start_state);
    if (cache_it != cache.end())
      return cache_it->second;

    Cost epsilon = domain.get_epsilon(start_state);
    assert(epsilon == 1);

    if (level == Domain::num_abstraction_levels) {
      return
        start_state == goal_state
        ? epsilon
        : 0;
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

    Cost hval = std::max(epsilon, result->get_g());
    cache[start_state] = hval;
    cache_h_star(result);
    assert(cache.find(start_state) != cache.end());
    assert(cache.find(start_state)->second = hval);

    return hval;
  }


  void cache_h_star(const Node *goal_node)
  {
    assert(goal_node != NULL);

    const Cost cost_to_goal = goal_node->get_g();

    unsigned num_cached = 0;
    const Node *parent = goal_node->get_parent();
    while (parent != NULL) {
      assert(cost_to_goal > parent->get_g());
      const Cost hval = cost_to_goal - parent->get_g();

      CacheIterator cache_it = cache.find(parent->get_state());
      if (cache_it != cache.end()) {
        assert(cache_it->second <= hval);
        cache_it->second = hval;
      }
      else
        cache[parent->get_state()] = hval;

      parent = parent->get_parent();
      num_cached += 1;
    }

    //    std::cerr << "cached h* values for " << num_cached << " nodes" << std::endl;
  }


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

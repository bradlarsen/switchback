#ifndef _HIDA_STAR_HPP_
#define _HIDA_STAR_HPP_


#include <cassert>
#include <vector>

#include <boost/array.hpp>
#include <boost/optional.hpp>
#include <boost/pool/pool_alloc.hpp>
#include <boost/unordered_map.hpp>
#include <boost/utility.hpp>

#include "search/Constants.hpp"
#include "util/PointerOps.hpp"


template <
  class Domain,
  class Node
  >
class HIDAStar : boost::noncopyable
{
private:
  typedef typename Node::Cost Cost;
  typedef typename Node::State State;

  typedef boost::unordered_map<
    State,
    std::pair<Cost, bool>
    // ,boost::hash<State>,
    // std::equal_to<State>,
    // boost::fast_pool_allocator< std::pair<State const, std::pair<Cost, bool> > >
    > Cache;

  typedef typename Cache::iterator CacheIterator;
  typedef typename Cache::const_iterator CacheConstIterator;


private:
  const static unsigned hierarchy_height = Domain::num_abstraction_levels + 1;

  const Node *goal;
  bool searched;

  Domain &domain;

  boost::array<unsigned, hierarchy_height> num_expanded;
  boost::array<unsigned, hierarchy_height> num_generated;

  boost::array<State, hierarchy_height> abstract_goals;

  Cache cache;

  boost::pool<> node_pool;


public:
  HIDAStar(Domain &domain)
    : goal(NULL)
    , searched(false)
    , domain(domain)
    , num_expanded()
    , num_generated()
    , abstract_goals()
    , cache()
    , node_pool(sizeof(Node))
  {
    num_expanded.assign(0);
    num_generated.assign(0);
    for (unsigned level = 0; level < hierarchy_height; level += 1)
      abstract_goals[level] = domain.abstract(level, domain.get_start_state());
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
    return num_expanded[level];
  }


  void search()
  {
    if (searched)
      return;
    searched = true;

    Node start_node(domain.get_start_state(), 0, 0);
    hidastar_search(0, start_node);
  }


private:
  Node * hidastar_search(const unsigned level, const Node &start_node)
  {
    boost::optional<Node> goal_node;
    Cost bound = heuristic(level, start_node);

    bool more_to_examine(true);

    while ( !goal && more_to_examine ) {
      boost::optional<Cost> newbound =
        cost_bounded_search(level,
                            start_node,
                            bound,
                            goal_node);

      if (newbound && newbound > bound) {
        more_to_examine = true;
        bound = *newbound;
      }
      else
        more_to_examine = false;
    }

    if (goal_node) {
      cache_optimal_path(level, *goal_node);
      return new (node_pool.malloc()) Node(*goal_node);
    }
    else {
      std::cerr << "no solution found at level " << level << "!" << std::endl;
      assert(false);
      return NULL;
    }
  }

  
  boost::optional<Cost>
  cost_bounded_search(const unsigned level,
                      const Node &start_node,
                      const Cost bound,
                      boost::optional<Node> &goal_node)
  {
    if (start_node.get_state() == abstract_goals[level]) {
      assert(start_node.get_h() == 0);
      goal_node = start_node;
      return boost::none;
    }

    bool is_exact_cost(false);
    boost::optional<Cost> newbound;
    std::vector<Node *> succs;
    domain.compute_successors(start_node, succs, node_pool);
    for (unsigned i = 0; i < succs.size(); i += 1) {
      Node &succ = *succs[i];

      const Cost hval = heuristic(level, *succs[i]);
      const Cost bound_minus_g = bound - succ.get_g();
      const Cost pre_new_hval = std::max(bound_minus_g, hval);
      
      // P-g caching
      CacheIterator cache_it = cache.find(succ.get_state());
      if (cache_it != cache.end()) {
        const Cost new_hval = std::max(cache_it->second.first, pre_new_hval);
        cache_it->second.first = new_hval;
        succ.set_h(new_hval);
        is_exact_cost = cache_it->second.second;
      }
      else {
        const Cost new_hval = pre_new_hval;
        cache.insert(cache_it, std::make_pair(succ.get_state(),
                                              std::make_pair(new_hval, false)));
      }

      // Optimal path caching
      if (succ.get_f() == bound && is_exact_cost) {
        Node synthesized_goal(abstract_goals[level],
                              bound,
                              0,
                              &succ);
        goal_node = synthesized_goal;
        return boost::none;
      }

      // Normal IDA* stuff
      if (succ.get_g() <= bound) {
        boost::optional<Cost> maybe_new_bound =
          cost_bounded_search(level,
                              succ,
                              bound,
                              goal_node);
        if (!newbound || (maybe_new_bound && *maybe_new_bound < *newbound))
          newbound = maybe_new_bound;
      }
    } /* end for */

    return newbound;
  }


  void cache_optimal_path(const unsigned level, const Node &goal_node)
  {
    assert(goal_node.get_state() == abstract_goals[level]);

    const Node *parent = goal_node.get_parent();
    while (parent != NULL) {
      assert(goal_node.get_g() > parent->get_g());
      const Cost distance = goal_node.get_g() - parent->get_g();

      CacheIterator cache_it = cache.find(parent->get_state());
      cache.insert(cache_it, std::make_pair(parent->get_state(),
                                            std::make_pair(distance, true)));
    }
  }


  Cost heuristic(const unsigned level, const Node &node)
  {
    if (node.get_state() == abstract_goals[level])
      return 0;

    if (level == Domain::num_abstraction_levels)
      return domain.get_epsilon(node.get_state());

    const unsigned next_level = level + 1;
    Node node_abstraction = Node(domain.abstract(next_level, node.get_state()),
                                 0,
                                 0);
    CacheConstIterator cache_it = cache.find(node_abstraction.get_state());
    if (cache_it != cache.end() && cache_it->second.second) {
      hidastar_search(next_level, node_abstraction);
    }

    assert(cache.find(node_abstraction.get_state()) != cache.end());
    return cache.find(node_abstraction.get_state())->second.first;
  }
};


#endif /* !_HIDA_STAR_HPP_ */

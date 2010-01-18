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


  struct BoundedResult
  {
    union Result
    {
      Cost cutoff;
      Node *goal;
    } result;

    typedef enum {Cutoff, Goal, Failure} ResultType;

    ResultType result_type;


    BoundedResult(Cost cutoff)
    {
      result.cutoff = cutoff;
      result_type = Cutoff;
    }

    BoundedResult(Node *goal)
    {
      assert(goal != NULL);
      result.goal = goal;
      result_type = Goal;
    }

    BoundedResult()
    {
      result_type = Failure;
    }


    bool is_failure() const
    {
      return result_type == Failure;
    }

    bool is_goal() const
    {
      return result_type == Goal;
    }

    bool is_cutoff() const
    {
      return result_type == Cutoff;
    }

    Node * get_goal() const
    {
      assert(is_goal());
      return result.goal;
    }

    Cost get_cutoff() const
    {
      assert(is_cutoff());
      return result.cutoff;
    }
  };


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
      abstract_goals[level] = domain.abstract(level, domain.get_goal_state());
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

    Node *start_node = new (node_pool.malloc()) Node(domain.get_start_state(), 0, 0);
    goal = hidastar_search(0, start_node);
  }


private:
  Node * hidastar_search(const unsigned level, Node *start_node)
  {
    Cost bound = heuristic(level, start_node);
    bool failed = false;

    Node *goal_node = NULL;

    while ( goal_node == NULL && !failed ) {
#ifdef OUTPUT_SEARCH_PROGRESS
    std::cerr << "hidastar_search at level " << level << std::endl;
    std::cerr << "doing cost-bounded search with cutoff " << bound << std::endl;
    std::cerr << get_num_expanded() << " total nodes expanded" << std::endl
              << get_num_generated() << " total nodes generated" << std::endl;
#endif
      BoundedResult res = cost_bounded_search(level, start_node, bound);

      if (res.is_failure()) {
        assert(!res.is_goal());
        assert(!res.is_cutoff());
        failed = true;
      }
      else if (res.is_goal()) {
        assert(!res.is_cutoff());
        assert(!res.is_failure());
        goal_node = res.get_goal();
        assert(goal_node != NULL);
      }
      else if (res.is_cutoff()) {
        assert(!res.is_goal());
        assert(!res.is_failure());
        bound = res.get_cutoff();
      }
      else {
        assert(false);
      }
    }

    if (goal_node != NULL) {
      cache_optimal_path(level, *goal_node);
      return goal_node;
    }
    else {
      std::cerr << "no solution found at level " << level << "!" << std::endl;
      assert(false);
      return NULL;
    }
  }

  
  BoundedResult
  cost_bounded_search(const unsigned level,
                      Node *start_node,
                      const Cost bound)
  {
    if (start_node->get_state() == abstract_goals[level]) {
      BoundedResult res(start_node);
      assert(res.is_goal());
      return res;
    }

    std::vector<Node *> succs;
    domain.compute_successors(*start_node, succs, node_pool);

    num_expanded[level] += 1;
    num_generated[level] += succs.size();


#ifdef OUTPUT_SEARCH_PROGRESS
      if (get_num_expanded() % 1000000 == 0) {
        std::cerr << "progress update:" << std::endl;
        std::cerr << get_num_expanded() << " total nodes expanded" << std::endl
                  << get_num_generated() << " total nodes generated" << std::endl;
        dump_cache_size(std::cerr);
      }
#endif


    boost::optional<Cost> new_cutoff;
    
    for (unsigned i = 0; i < succs.size(); i += 1) {
      Node *succ = succs[i];

      Cost hval = heuristic(level, succ);


      // P-g caching
      const Cost p_minus_g = bound - succ->get_g();
      hval = std::max(hval, p_minus_g);
      CacheIterator cache_it = cache.find(succ->get_state());
      if (cache_it != cache.end()) {
        hval = std::max(hval, cache_it->second.first);
        cache_it->second.first = hval;
      }
      else {
        cache[succ->get_state()] = std::make_pair(hval, false);
      }
      // end P-g caching


      succ->set_h(hval);


      // Optimal path caching
      {
        CacheConstIterator cache_it = cache.find(succ->get_state());
        bool is_exact_cost = cache_it != cache.end() && cache_it->second.second;
        if (succ->get_f() == bound && is_exact_cost) {
          assert(level > 0);
          Node *synthesized_goal = new (node_pool.malloc())
            Node(abstract_goals[level],
                 succ->get_g() + cache_it->second.first,
                 0,
                 succ);
          BoundedResult res(synthesized_goal);
          assert(res.is_goal());
          return res;
        }
      }
      // end Optimal path caching
      

      // Normal IDA* stuff
      if (succ->get_f() <= bound) {
        BoundedResult res = cost_bounded_search(level, succ, bound);
        if (res.is_goal()) {
          assert(!res.is_cutoff());
          assert(!res.is_failure());
          return res;
        }
        else if (res.is_cutoff()) {
          assert(!res.is_goal());
          assert(!res.is_failure());

          if (new_cutoff)
            new_cutoff = std::min(*new_cutoff, res.get_cutoff());
          else
            new_cutoff = res.get_cutoff();
        }
      }
      else {
        if (new_cutoff)
          new_cutoff = std::min(*new_cutoff, succ->get_f());
        else
          new_cutoff = succ->get_f();
      }
      // end Normal IDA* stuff

      node_pool.free(succ);
    } /* end for */

    if (new_cutoff) {
      BoundedResult res(*new_cutoff);
      assert(res.is_cutoff());
      assert(!res.is_goal());
      assert(!res.is_failure());
      return res;
    }
    else {
      BoundedResult res;
      assert(res.is_failure());
      assert(!res.is_cutoff());
      assert(!res.is_failure());
      return res;
    }
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


  Cost heuristic(const unsigned level, Node *node)
  {
    if (node->get_state() == abstract_goals[level])
      return 0;

    if (level == Domain::num_abstraction_levels)
      return domain.get_epsilon(node->get_state());

    const unsigned next_level = level + 1;
    Node *node_abstraction = new (node_pool.malloc())
      Node(domain.abstract(next_level, node->get_state()),
           0,
           0);
    CacheConstIterator cache_it = cache.find(node_abstraction->get_state());
    if (cache_it == cache.end() || !cache_it->second.second) {
      Node *res = hidastar_search(next_level, node_abstraction);
      cache[node_abstraction->get_state()] = std::make_pair(res->get_h(), false);
      node_pool.free(res);
    }

    node_pool.free(node_abstraction);

    assert(cache.find(node_abstraction->get_state()) != cache.end());
    return cache.find(node_abstraction->get_state())->second.first;
  }


  void dump_cache_size(std::ostream &o) const
  {
    o << "cache size: " << cache.size() << std::endl;
  }
};


#endif /* !_HIDA_STAR_HPP_ */

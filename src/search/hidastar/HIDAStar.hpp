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


#define HIDA_STAR_CYCLE_CHECKING


template <
  class DomainT,
  class NodeT
  >
class HIDAStar : boost::noncopyable
{
public:
  typedef DomainT Domain;
  typedef NodeT Node;


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


  typedef boost::unordered_map<
    State,
    std::pair<Cost, unsigned>
    > GCache;

  typedef typename GCache::iterator GCacheIterator;
  typedef typename GCache::const_iterator GCacheConstIterator;


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

  Node goal;
  bool searched;

  Domain &domain;

  boost::array<unsigned, hierarchy_height> num_expanded;
  boost::array<unsigned, hierarchy_height> num_generated;

  boost::array<unsigned, hierarchy_height> num_iterations;

  boost::array<unsigned, hierarchy_height> cache_lookups;
  boost::array<unsigned, hierarchy_height> cache_hits;

  boost::array<State, hierarchy_height> abstract_goals;

  Cache cache;
  GCache gcache;

  // One node pool for each level of the hierarchy.
  boost::array<boost::pool<> *, hierarchy_height> node_pool;


public:
  HIDAStar(Domain &domain)
    : goal(domain.get_goal_state(), 0, 0)
    , searched(false)
    , domain(domain)
    , num_expanded()
    , num_generated()
    , num_iterations()
    , cache_lookups()
    , cache_hits()
    , abstract_goals()
    , cache()
    , gcache()
    , node_pool()
  {
    num_expanded.assign(0);
    num_generated.assign(0);
    num_iterations.assign(0);
    cache_lookups.assign(0);
    cache_hits.assign(0);
    for (unsigned level = 0; level < hierarchy_height; level += 1) {
      abstract_goals[level] = domain.abstract(level, domain.get_goal_state());
      node_pool[level] = new boost::pool<>(sizeof(Node));
    }
  }

  ~HIDAStar()
  {
    for (unsigned i = 0; i < hierarchy_height; i += 1)
      delete node_pool[i];
  }

  const Node * get_goal() const
  {
    return &goal;
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

    Node *start_node = new (node_pool[0]->malloc())
      Node(domain.get_start_state(),
           0,
           0);

    Node goal_node(domain.get_goal_state(), 0, 0);
    hidastar_search(0, start_node, &goal_node);

    goal = goal_node;
  }


  void output_statistics(std::ostream &o) const
  {
    assert(searched);
    o << "iterations:" << std::endl;
    for (unsigned level = 0; level < hierarchy_height; level += 1) {
      o << "  " << level << ": " << num_iterations[level] << std::endl;
    }

    o << "nodes expanded/generated per level:" << std::endl;
    for (unsigned level = 0; level < hierarchy_height; level += 1) {
      o << "  " << level << ": " << num_expanded[level]
        << " / " << num_generated[level] << std::endl;
    }

    dump_cache_size(o);
    dump_cache_information(o);
  }


private:
  // start_node should be const, but that didn't work out.
  // goal_node is modified, if a goal is found.
  // returns true if a goal was found.
  bool hidastar_search(const unsigned level, Node *start_node, Node *goal_node)
  {
    assert(start_node != NULL);
    assert(goal_node != NULL);
    assert(start_node->num_nodes_to_start() == 1);
    Cost bound = start_node->get_h();
    bool failed = false;
    bool goal_found = false;

    gcache.clear();

    while ( !goal_found && !failed ) {
#ifdef OUTPUT_SEARCH_PROGRESS
      if (level == 0) {
        std::cerr << "hidastar_search at level " << level << std::endl;
        std::cerr << "doing cost-bounded search with cutoff " << bound << std::endl;
        std::cerr << get_num_expanded() << " total nodes expanded" << std::endl
                  << get_num_generated() << " total nodes generated" << std::endl;
      }
#endif
      num_iterations[level] += 1;

      BoundedResult res = cost_bounded_search(level, start_node, bound);

      if (res.is_failure()) {
        assert(!res.is_goal());
        assert(!res.is_cutoff());
        failed = true;
      }
      else if (res.is_goal()) {
        assert(!res.is_cutoff());
        assert(!res.is_failure());
        goal_found = true;
        *goal_node = *res.get_goal();
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

    if (goal_found) {
      cache_optimal_path(level, goal_node);
      return true;
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
    domain.compute_successors(*start_node, succs, *node_pool[level]);

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

#ifdef HIDA_STAR_CYCLE_CHECKING
      if (start_node->is_descendent_of(succ)) {
        node_pool[level]->free(succ);
        continue;
      }
#endif

      // GCACHE STUFF GOES HERE!
      GCacheIterator gcache_it = gcache.find(succ->get_state());
      if (gcache_it != gcache.end()) {
        assert(gcache_it->second.second <= num_iterations[level]);

        if (gcache_it->second.second == num_iterations[level] &&
            succ->get_g() >= gcache_it->second.first) {
		// We have seen this node at this iteration via either
		// an equally as expensive or a cheaper path.
          node_pool[level]->free(succ);
          continue;
        }
        else if (gcache_it->second.second < num_iterations[level] &&
                 succ->get_g() > gcache_it->second.first) {
		// There is a better way to get to this node (we know
		// this from previous search iterations).  We will get
		// to it thru another path on this iteration.
          node_pool[level]->free(succ);
          continue;
        }
      }
      // At this point, we have 3 cases:
      //
      // 1) We have never seen this node before and need to add it to
      //    the cache
      //
      // 2) We have seen this node before with a worse g-value and we
      //    need to update the g-value in the cache.
      //
      // 3) We have seen this node before with the same g-value, but
      //    on a previous iteration and we need to update the
      //    iteration number.
      //
      // In all three cases, we can just set the cache entry for this
      // node to be the current g-value (which is either equal to or
      // better than the cached value) and the current iteration
      // number.

      gcache[succ->get_state()] = std::make_pair(succ->get_g(), num_iterations[level]);


      assert(succ->num_nodes_to_start() == start_node->num_nodes_to_start() + 1u);

      Cost hval = heuristic(level, succ);

      // P-g caching
      const Cost p_minus_g = bound >= succ->get_g() ? bound - succ->get_g() : 0;
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
          num_generated[level] += 1;
          assert(level > 0);
          Node *synthesized_goal = new (node_pool[level]->malloc())
            Node(abstract_goals[level],
                 succ->get_g() + cache_it->second.first,
                 0,
                 succ);
          BoundedResult res(synthesized_goal);
          assert(res.is_goal());

          // cleanup remaining successors
          for (unsigned j = i + 1; j < succs.size(); j += 1)
            node_pool[level]->free(succs[j]);

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

          // cleanup remaining successors
          for (unsigned j = i + 1; j < succs.size(); j += 1)
            node_pool[level]->free(succs[j]);

          return res;
        }
        else if (res.is_cutoff()) {
          assert(!res.is_goal());
          assert(!res.is_failure());

          if (new_cutoff)
            new_cutoff = std::min(*new_cutoff, res.get_cutoff());
          else
            new_cutoff = res.get_cutoff();

          node_pool[level]->free(succ);
        }
      }
      else {
        if (new_cutoff)
          new_cutoff = std::min(*new_cutoff, succ->get_f());
        else
          new_cutoff = succ->get_f();

        node_pool[level]->free(succ);
      }
      // end Normal IDA* stuff
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
      return res;
    }
  }


  void cache_optimal_path(const unsigned level, const Node *goal_node)
  {
    assert(goal_node != NULL);
    assert(goal_node->get_state() == abstract_goals[level]);

    const Node *parent = goal_node->get_parent();
    while (parent != NULL) {
      assert(goal_node->get_g() >= parent->get_g());
      const Cost distance = goal_node->get_g() - parent->get_g();

      cache[parent->get_state()].first = distance;
      cache[parent->get_state()].second = distance;

      parent = parent->get_parent();
    }
  }


  Cost heuristic(const unsigned level, Node *node)
  {
    if (node->get_state() == abstract_goals[level])
      return 0;

    if (level == Domain::num_abstraction_levels)
      return domain.get_epsilon(node->get_state());

    const unsigned next_level = level + 1;
    Node node_abstraction(domain.abstract(next_level, node->get_state()),
                          0,
                          0);

    cache_lookups[level] += 1;
    Cost hval;
    CacheConstIterator cache_it = cache.find(node_abstraction.get_state());
    if (cache_it == cache.end() || !cache_it->second.second) {
      cache_hits[level] += 1;
      cache[node_abstraction.get_state()] = std::make_pair(0, true);
      Node goal_abstraction(abstract_goals[next_level], 0, 0);
      bool goal_found = hidastar_search(next_level, &node_abstraction, &goal_abstraction);
      if (!goal_found) {
        std::cerr << "infinite heuristic estimate!" << std::endl;
        assert(false);
      }
      assert(cache.find(node_abstraction.get_state())->second.second);
      assert(goal_abstraction.get_h() == 0);
      assert(goal_abstraction.get_state() == abstract_goals[next_level]);
      cache[node_abstraction.get_state()].first = goal_abstraction.get_g();
      hval = goal_abstraction.get_g();

      // TODO: I think this code leaks all the nodes along the goal
      // path.  However, all attempts to put cleanup code for
      // goal_abstraction's parents here has resulted in memory
      // corruption...
    }
    else {
      hval = cache_it->second.first;
    }

    assert(cache.find(node_abstraction.get_state()) != cache.end());

    node_pool[next_level]->purge_memory();
    return hval;
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
};


#endif /* !_HIDA_STAR_HPP_ */

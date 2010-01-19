#ifndef _SWITCHBACK_HPP_
#define _SWITCHBACK_HPP_


#include <cassert>
#include <vector>

#include <boost/array.hpp>
#include <boost/none.hpp>
#include <boost/optional.hpp>
#include <boost/pool/pool_alloc.hpp>
#include <boost/unordered_map.hpp>
#include <boost/utility.hpp>

#include "search/BucketPriorityQueue.hpp"
#include "search/Constants.hpp"
#include "util/PointerOps.hpp"


template <
  class DomainT,
  class NodeT
  >
class Switchback : boost::noncopyable
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
    boost::fast_pool_allocator< std::pair<Node * const, MaybeItemPointer> >
    > Closed;

  typedef typename Closed::iterator ClosedIterator;
  typedef typename Closed::const_iterator ClosedConstIterator;


private:
  const static unsigned hierarchy_height = Domain::num_abstraction_levels + 1;

  const Node *goal;
  bool searched;

  Domain &domain;

  boost::array<unsigned, hierarchy_height> num_expanded;
  boost::array<unsigned, hierarchy_height> num_generated;

  boost::array<unsigned, hierarchy_height> num_expanded_on_first_search_at_level;
  boost::array<unsigned, hierarchy_height> num_generated_on_first_search_at_level;
  boost::array<unsigned, hierarchy_height> num_searches;

  boost::array<unsigned, hierarchy_height> cache_lookups;
  boost::array<unsigned, hierarchy_height> cache_hits;

  boost::array<Open, hierarchy_height> open;
  Closed closed;

  boost::array<State, hierarchy_height> abstract_goals;

  boost::pool<> node_pool;


public:
  Switchback(Domain &domain)
    : goal(NULL)
    , searched(false)
    , domain(domain)
    , num_expanded()
    , num_generated()
    , num_expanded_on_first_search_at_level()
    , num_generated_on_first_search_at_level()
    , num_searches()
    , cache_lookups()
    , cache_hits()
    , open()
    , closed(INITIAL_CLOSED_SET_SIZE)
    , abstract_goals()
    , node_pool(sizeof(Node))
  {
    num_expanded.assign(0);
    num_generated.assign(0);
    num_expanded_on_first_search_at_level.assign(0);
    num_generated_on_first_search_at_level.assign(0);
    cache_lookups.assign(0);
    cache_hits.assign(0);
    initialize();
  }

  ~Switchback()
  {
  }

  void search()
  {
    if (searched)
      return;
    searched = true;

    goal = resume_search(0, domain.get_goal_state());
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


  void output_statistics(std::ostream &o) const
  {
    assert(searched);

    o << "final statistics:" << std::endl;

    o << "nodes expanded/generated per level:" << std::endl;
    for (unsigned level = 0; level < hierarchy_height; level += 1) {
      o << "  " << level << ": " << num_expanded[level]
        << " / " << num_generated[level] << std::endl;
    }

    dump_open_sizes(o);
    dump_closed_sizes(o);

    dump_cache_information(o);
    dump_first_searches_information(o);
  }


private:
  Cost heuristic(const unsigned level, const State &goal_state)
  {
    assert(domain.is_valid_level(level));

    if (goal_state == abstract_goals[level])
      return 0;

    const Cost epsilon = domain.get_epsilon(goal_state);

    if (level == Domain::num_abstraction_levels)
      return epsilon;

    // Need to create a dummy goal node to look up in the hash table.
    // This smells of bad design!
    const unsigned next_level = level + 1;
    const State abstract_goal_state = domain.abstract(next_level, goal_state);
    Node abstract_goal_node(abstract_goal_state, 0, 0);

    cache_lookups[level] += 1;
    ClosedIterator closed_it = closed.find(&abstract_goal_node);
    if (closed_it != closed.end() && !closed_it->second) {
      cache_hits[level] += 1;
      return std::max(closed_it->first->get_g(), epsilon);
    }

    Node *result = resume_search(next_level, abstract_goal_state);
    if (result == NULL) {
      std::cerr << "whoops, infinite heuristic estimate!" << std::endl;
      assert(false);  // for the domains I am running on, there should
                      // never be an infinite heuristic estimate.
    }
    assert(closed.find(&abstract_goal_node) != closed.end());
    assert(!closed.find(&abstract_goal_node)->second);

    return std::max(result->get_g(), epsilon);
  }

  
  Node * resume_search(const unsigned level, const State &goal_state)
  {
    assert(domain.is_valid_level(level));

    num_searches[level] += 1;

    // Dummy goal node, for hash table lookup.
    Node goal_node(goal_state, 0, 0);
    ClosedIterator closed_it = closed.find(&goal_node);

    if (closed_it != closed.end() && !closed_it->second) {
      return closed_it->first;
    }

    std::vector<Node *> children;

    // A*-ish code ahead
    while (!open[level].empty()) {
#ifdef OUTPUT_SEARCH_PROGRESS
      if (get_num_expanded() % 1000000 == 0) {
        std::cerr << get_num_expanded() << " total nodes expanded" << std::endl
                  << get_num_generated() << " total nodes generated" << std::endl;
        dump_open_sizes(std::cerr);
        dump_closed_sizes(std::cerr);
      }
#endif

      Node *n = open[level].top();
      assert(closed.find(n) != closed.end());
      assert(closed.find(n)->second);
      assert(open[level].valid_item_pointer(*closed.find(n)->second));
      open[level].pop();

      closed[n] = boost::none;

      if (level % 2 == 0)
        domain.compute_successors(*n, children, node_pool);
      else
        domain.compute_predecessors(*n, children, node_pool);
      num_expanded[level] += 1;
      num_generated[level] += children.size();

      if (num_searches[level] == 1) {
        num_expanded_on_first_search_at_level[level] += 1;
        num_generated_on_first_search_at_level[level] += children.size();
      }
      
      for (unsigned child_idx = 0; child_idx < children.size(); child_idx += 1) {
        process_child(level, children[child_idx]);
      }

      if (n->get_state() == goal_state)
        return n;
    } /* end while */

    return NULL;
  }

  void process_child(const unsigned level, Node *child)
  {
    assert(open[level].size() <= closed.size());

    child->set_h(heuristic(level, child->get_state()));

    ClosedIterator closed_it = closed.find(child);
    if (closed_it == closed.end()) {
      // The child has not been generated before.
      closed[child] = open[level].push(child);
    }
    else if (closed_it->second && child->get_f() < closed_it->first->get_f()) {
      // A worse version of the child is in the open list.
      open[level].erase(*closed_it->second);  // knock out the old
                                              // one from the open
                                              // list

     // free the old, worse copy
      node_pool.free(closed_it->first);
      closed.erase(closed_it);

      closed[child] = open[level].push(child);  // insert better version of child
    }
    else {
      // The child has either already been expanded, or is worse
      // than the version in the open list.
      node_pool.free(child);
    }
  }

  void initialize()
  {
    for (unsigned level = 0; level <= Domain::num_abstraction_levels; level += 1) {
      num_generated[level] += 1;
      num_generated_on_first_search_at_level[level] += 1;
      State start = level % 2 == 0
                      ? domain.get_start_state()
                      : domain.get_goal_state();
      State goal = level % 2 == 0
                      ? domain.get_goal_state()
                      : domain.get_start_state();
      Node *start_node = new (node_pool.malloc()) Node(domain.abstract(level, start),
                                                       0,
                                                       0,
                                                       NULL);
      closed[start_node] = open[level].push(start_node);
      abstract_goals[level] = goal;
    }
  }

  void dump_open_sizes(std::ostream &o) const
  {
    o << "open sizes:" << std::endl;
    for (unsigned level = 0; level < hierarchy_height; level += 1)
      o << "  " << level << ": " << open[level].size() << std::endl;
  }

  void dump_closed_sizes(std::ostream &o) const
  {
    o << "closed size: " << closed.size() << std::endl;
  }


  void dump_cache_information(std::ostream &o) const
  {
    o << "cache information:" << std::endl;
    for (unsigned level = 0; level < hierarchy_height; level += 1) {
      const float hit_ratio =
        static_cast<float>(cache_hits[level]) / static_cast<float>(cache_lookups[level]);

      o << "  " << level << ": "
        << cache_lookups[level] << " lookups, "
        << cache_hits[level] << " hits (" << hit_ratio << ")" << std::endl;
    }
  }


  void dump_first_searches_information(std::ostream &o) const
  {
    o << "nodes expanded/generated during first search:" << std::endl;
    for (unsigned level = 0; level < hierarchy_height; level += 1) {
      o << "  " << level << ": " << num_expanded_on_first_search_at_level[level]
        << " / " << num_generated_on_first_search_at_level[level] << std::endl;
    }
  }
};


#endif /* !_SWITCHBACK_HPP_ */

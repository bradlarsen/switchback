#ifndef _SWITCHBACK_HPP_
#define _SWITCHBACK_HPP_


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
class Switchback : boost::noncopyable
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
    , boost::fast_pool_allocator< std::pair<Node * const, MaybeItemPointer> >
    > Closed;

  typedef typename Closed::iterator ClosedIterator;
  typedef typename Closed::const_iterator ClosedConstIterator;

private:
  const Node *goal;
  bool searched;

  const Domain &domain;

  const static unsigned hierarchy_height = Domain::num_abstraction_levels + 1;

  boost::array<unsigned, hierarchy_height> num_expanded;
  boost::array<unsigned, hierarchy_height> num_generated;

  boost::array<Open, hierarchy_height> open;
  boost::array<Closed, hierarchy_height> closed;


public:
  Switchback(const Domain &domain)
    : goal(NULL)
    , searched(false)
    , domain(domain)
  {
    num_expanded.assign(0);
    num_generated.assign(0);
  }

  ~Switchback()
  {
  }

  void search()
  {
    if (searched)
      return;
    searched = true;

    init_open_and_closed();
    goal = resume_search(0, domain.get_goal_state());
  }

  const Node * get_goal() const
  {
    std::cerr << "final open/closed sizes:" << std::endl;
    dump_open_sizes(std::cerr);
    dump_closed_sizes(std::cerr);

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


private:
  Cost heuristic(const unsigned level, const State &goal_state)
  {
    assert(is_valid_level(level));

    if (level == Domain::num_abstraction_levels)
      return 0;

    // Need to create a dummy goal node to look up in the hash table.
    // This smells of bad design!
    const unsigned next_level = level + 1;
    const State abstract_goal_state = domain.abstract(next_level, goal_state);
    Node abstract_goal_node(abstract_goal_state, 0, 0);

    ClosedIterator closed_it = closed[next_level].find(&abstract_goal_node);
    if (closed_it != closed[next_level].end() && !closed_it->second) {
      return closed_it->first->get_g();
    }

    Node *result = resume_search(next_level, abstract_goal_state);
    if (result == NULL) {
      std::cerr << "whoops, infinite heuristic estimate!" << std::endl;
      assert(false);  // for the domains I am running on, there should
                      // never be an infinite heuristic estimate.
    }
    assert(closed[next_level].find(&abstract_goal_node) != closed[next_level].end());
    assert(!closed[next_level].find(&abstract_goal_node)->second);

    return result->get_g();
  }
  
  Node * resume_search(const unsigned level, const State &goal_state)
  {
    assert(is_valid_level(level));

    // Dummy goal node, for hash table lookup.
    Node goal_node(goal_state, 0, 0);
    ClosedIterator closed_it = closed[level].find(&goal_node);

    if (closed_it != closed[level].end() &&!closed_it->second)
      return closed_it->first;

    std::vector<Node *> children;

    // A*-ish code ahead
    while (!open[level].empty()) {
      if (get_num_expanded() % 500000 == 0) {
        std::cerr << get_num_expanded() << " total nodes expanded" << std::endl
                  << get_num_generated() << " total nodes generated" << std::endl;
        dump_open_sizes(std::cerr);
        dump_closed_sizes(std::cerr);
      }

      Node *n = open[level].top();
      assert(closed[level].find(n) != closed[level].end());
      assert(closed[level].find(n)->second);
      assert(open[level].valid_item_pointer(*closed[level].find(n)->second));
      open[level].pop();

      closed[level][n] = boost::none;
      assert(all_closed_item_ptrs_valid(level));

      if (level % 2 == 0)
        domain.compute_successors(*n, children);
      else
        domain.compute_predecessors(*n, children);
      num_expanded[level] += 1;
      num_generated[level] += children.size();
      
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
    assert(open[level].invariants_satisfied());
    assert(open[level].size() <= closed[level].size());
    assert(all_closed_item_ptrs_valid(level));

    child->set_h(heuristic(level, child->get_state()));

    ClosedIterator closed_it = closed[level].find(child);
    if (closed_it == closed[level].end()) {
      // The child has not been generated before.
      closed[level][child] = open[level].push(child);
    }
    else if (closed_it->second && child->get_f() < closed_it->first->get_f()) {
      // A worse version of the child is in the open list.
      open[level].erase(*closed_it->second);  // knock out the old
                                              // one from the open
                                              // list

      domain.free_node(closed_it->first);     // free the old,
                                              // worse copy
      closed[level].erase(closed_it);

      closed[level][child] = open[level].push(child);  // insert better version of child
    }
    else {
      // The child has either already been expanded, or is worse
      // than the version in the open list.
      domain.free_node(child);
    }

    assert(all_closed_item_ptrs_valid(level));
    assert(open[level].invariants_satisfied());
  }

  void init_open_and_closed()
  {
    for (unsigned level = 0; level <= Domain::num_abstraction_levels; level += 1) {
      std::cerr << "initializing level " << level << std::endl;

      num_generated[level] += 1;
      State start = level % 2 == 0
                      ? domain.get_start_state()
                      : domain.get_goal_state();
      Node *start_node = domain.create_node(domain.abstract(level, start),
                                            0,
                                            0,
                                            NULL
                                            );
      closed[level][start_node] = open[level].push(start_node);
    }

    std::cerr << "###### INITIALIZATION COMPLETE ######" << std::endl;
  }


  bool all_closed_item_ptrs_valid(const unsigned level) const
  {
#ifdef CHECK_ALL_CLOSED_ITEM_PTRS_VALID
    std::cerr << "checking if item pointers are valid" << std::endl
              << "  " << closed.size() << " pointers to check" << std::endl;
    for (ClosedConstIterator closed_it = closed[level].begin();
         closed_it != closed[level].end();
         ++closed_it) {
      if ( closed_it->second && !open[level].valid_item_pointer(*closed_it->second) )
        return false;
    }
    return true;
#else
    return true;
#endif
  }

  static bool is_valid_level(const unsigned level)
  {
    return level <= Domain::num_abstraction_levels;
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
};


#endif /* !_SWITCHBACK_HPP_ */

#ifndef _SWITCHBACK_HPP_
#define _SWITCHBACK_HPP_


#include <vector>

#include <boost/array.hpp>
#include <boost/none.hpp>
#include <boost/optional.hpp>
#include <boost/pool/pool_alloc.hpp>
#include <boost/unordered_map.hpp>

#include "search/BucketPriorityQueue.hpp"
#include "util/PointerOps.hpp"


template <
  class Domain,
  class Node
  >
class Switchback
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

private:
  Switchback(const Switchback<Domain, Node> &);
  Switchback<Domain, Node> & operator =(const Switchback<Domain, Node> &);

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
    if (searched || goal != NULL)
      return;
    searched = true;

    init_open_and_closed();
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


private:
  Cost heuristic(const unsigned level, const State &goal_state)
  {
    assert(is_valid_level(level));

    if (level == Domain::num_abstraction_levels)
      return 0;

    // Need to create a dummy goal node to look up in the hash table.
    // This smells of bad design!
    Node goal_node(goal_state, 0, 0);
    ClosedIterator closed_it = closed[level].find(&goal_node);
    if (closed_it != closed[level].end())
      return closed_it->first->get_g();

    const State abstracted_goal_state = domain.abstract(level + 1, goal_state);

    Node *result = resume_search(level + 1, abstracted_goal_state);
    if (result == NULL) {
      std::cerr << "whoops, infinite heuristic estimate!" << std::endl;
      assert(false);  // for the domains I am running on, there should
                      // never be an infinite heuristic estimate.
    }

    return result->get_g();
  }
  
  Node * resume_search(const unsigned level, const State &goal_state)
  {
    assert(is_valid_level(level));

    std::cerr << "conducting hierarchical search at level " << level
              << " for the following state:" << std::endl
              << goal_state << std::endl;

    dump_open_sizes(std::cerr);
    dump_closed_sizes(std::cerr);

    // Dummy goal node, for hash table lookup.
    Node goal_node(goal_state, 0, 0);
    ClosedIterator closed_it = closed[level].find(&goal_node);

    if (closed_it != closed[level].end())
      return closed_it->first;

    std::vector<Node *> children;

    // A*-ish code goes here
    while (!open[level].empty()) {
      if (get_num_expanded() % 500000 == 0) {
        std::cerr << get_num_expanded() << " total nodes expanded" << std::endl
                  << get_num_generated() << " total nodes generated" << std::endl;
        dump_open_sizes(std::cerr);
        dump_closed_sizes(std::cerr);
      }

      Node *n = open[level].top();
      open[level].pop();
      assert(closed[level].find(n) != closed[level].end());
      closed[level][n] = boost::none;
      assert(all_closed_item_ptrs_valid(level));

      if (level % 2 == 0)
        domain.compute_successors(*n, children);
      else
        domain.compute_predecessors(*n, children);
      num_expanded[level] += 1;
      num_generated[level] += children.size();
      
      for (unsigned child_idx = 0; child_idx < children.size(); child_idx += 1) {
        Node *child = children[child_idx];
        assert(open[level].size() <= closed[level].size());
        assert(all_closed_item_ptrs_valid(level));
        ClosedIterator closed_it = closed[level].find(child);
        if (closed_it != closed[level].end()) {
          MaybeItemPointer &open_ptr = closed_it->second;

          if (open_ptr) {
            Node *old_child = open[level].lookup(*open_ptr);

            if ( child->get_f() < old_child->get_f() ) {
              // A worse copy of succ is in the open list.  Replace it!
              assert(old_child->get_state() == child->get_state());
              assert(*old_child == *child);

              domain.free_node(old_child);    // get rid of the old copy
              open[level].erase(*open_ptr);
              closed[level].erase(child);

              open_ptr = open[level].push(child);    // insert the new copy
              closed[level][child] = open_ptr;

              assert(all_closed_item_ptrs_valid(level));
            }
          }
          else {
            // child is not in the open list, but is closed.  Drop it!
            domain.free_node(child);
          }
        }
        else {
          // child has not been generated before.
          MaybeItemPointer open_ptr = open[level].push(child);
          closed[level][child] = open_ptr;
          assert(closed[level].find(open[level].lookup(*open_ptr)) != closed[level].end());
          assert(all_closed_item_ptrs_valid(level));
        }
      } /* end for */

      if (n->get_state() == goal_state)
        return n;
    } /* end while */

    return NULL;
  }

  void init_open_and_closed()
  {
    for (int level = Domain::num_abstraction_levels; level >= 0; level -= 1) {
      std::cerr << "initializing level " << level << std::endl;
      State state = level % 2 == 0
                      ? domain.get_start_state()
                      : domain.get_goal_state();

      State abstract_state = domain.abstract(level, state);
      Node *start = domain.create_node(abstract_state,
                                       0,
                                       heuristic(level, abstract_state),
                                       NULL
                                       );
      MaybeItemPointer open_ptr = open[level].push(start);
      closed[level][start] = open_ptr;
    }
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

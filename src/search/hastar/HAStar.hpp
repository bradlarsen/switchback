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


private:
  const Node * goal;
  bool searched;

  const Domain &domain;

  const static unsigned hierarchy_height = Domain::num_abstraction_levels + 1;

  boost::array<unsigned, hierarchy_height> num_expanded;
  boost::array<unsigned, hierarchy_height> num_generated;

  boost::array<Open, hierarchy_height> open;
  boost::array<Closed, hierarchy_height> closed;


public:
  HAStar(const Domain &domain)
    : goal(NULL)
    , searched(false)
    , domain(domain)
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
  Node * search_at_level(const unsigned level,
                         const State &start_state,
                         const State &goal_state)
  {
    Open open;
    Closed closed;

    Node *start_node = domain.create_node(start_state,
                                          0,
                                          0,
                                          NULL);
    closed[start_node] = open.push(start_node);
    assert(closed.find(start_node) != closed.end());
    assert(open.size() == 1);
    assert(closed.size() == 1);

    Node *goal_node = NULL;

    std::vector<Node *> succs;
    while (!open.empty())
    {
      Node *n = open.top();
      open.pop();
      assert(closed.find(n) != closed.end());
      closed[n] = boost::none;

      if (n->get_state() == goal_state) {
        goal_node = n;
        break;
      }

      domain.compute_successors(*n, succs);
      num_expanded[level] += 1;
      num_generated[level] += succs.size();

      for (unsigned succ_i = 0; succ_i < succs.size(); succ_i += 1)
        process_child(level, open, closed, n, succs[succ_i]);
    } /* end while */

    //    free_all_nodes(closed);
    
    if (goal_node == NULL) {
      std::cerr << "no goal found!" << std::endl;
      assert(false);
    }

    return goal_node;
  }


  void process_child(const unsigned level,
                     Open &open,
                     Closed &closed,
                     const Node *parent,
                     Node *child)
  {
    domain.compute_heuristic(*parent, *child);
    assert(open.size() <= closed.size());

    ClosedIterator closed_it = closed.find(child);
    if (closed_it == closed.end()) {
      // The child has not been generated before.
      closed[child] = open.push(child);
    }
    else if (closed_it->second && child->get_f() < closed_it->first->get_f()) {
      // A worse version of the child is in the open list.
      open.erase(*closed_it->second);  // knock out the old one from
                                       // the open list

      domain.free_node(closed_it->first);     // free the old, worse
                                              // copy
      closed.erase(closed_it);

      closed[child] = open.push(child);  // insert better version of
                                         // child
    }
    else {
      // The child has either already been expanded, or is worse
      // than the version in the open list.
      domain.free_node(child);
    }

    assert(open.invariants_satisfied());
  }


  void free_all_nodes(Closed &closed)
  {
    for (ClosedIterator closed_it = closed.begin();
         closed_it != closed.end();
         ++closed_it)
      domain.free_node(closed_it->first);
  }
};


#endif /* !_HA_STAR_HPP_ */

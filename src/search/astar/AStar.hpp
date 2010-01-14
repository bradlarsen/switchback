#ifndef _A_STAR_HPP_
#define _A_STAR_HPP_


#include <vector>

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
class AStar
{
private:
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

public:
  AStar(const Domain &domain)
    : open(*new Open())
    , closed(*new Closed(50000000))  // requested number of hash buckets
    , goal(NULL)
    , searched(false)
    , domain(domain)
    , num_expanded(0)
    , num_generated(0)
  {
  }

  ~AStar()
  {
    // I don't delete open or closed.  This appears to be acceptable,
    // due to my use of memory pools in key places.  Check for
    // yourself, with valgrind: no memory is leaked.
  }

  void search()
  {
    if (searched)
      return;
    searched = true;

    std::vector<Node *> succs;    // re-use a stack-allocated vector
                                  // for successor nodes, thus
                                  // avoiding repeated heap
                                  // allocation.

    {
      assert(all_closed_item_ptrs_valid());
      Node *start_node = domain.create_node(domain.get_start_state(), 0, 0, NULL);
      domain.compute_heuristic(*start_node);
      MaybeItemPointer open_ptr = open.push(start_node);
      assert(open_ptr);
      closed[start_node] = open_ptr;
      assert(closed.find(start_node) != closed.end());
      assert(open.size() == 1);
      assert(closed.size() == 1);
      assert(all_closed_item_ptrs_valid());
    }

    while (!open.empty())
    {
      Node *n = open.top();
      open.pop();
      assert(closed.find(n) != closed.end());
      closed[n] = boost::none;
      assert(all_closed_item_ptrs_valid());

      if (domain.is_goal(n->get_state())) {
        goal = n;
        return;
      }

      domain.compute_successors(*n, succs);
      num_expanded += 1;
      num_generated += succs.size();

      for (unsigned succ_i = 0; succ_i < succs.size(); succ_i += 1)
      {
        process_child(n, succs[succ_i]);
      } /* end for */
    } /* end while */
  }


  const Node * get_goal() const
  {
#ifndef NDEBUG
    std::cerr << open.size() << " nodes in open at end of search" << std::endl
              << closed.size() << " nodes in closed at end of search" << std::endl;
#endif
    return goal;
  }

  const Domain & get_domain() const
  {
    return domain;
  }

  unsigned get_num_generated() const
  {
    return num_generated;
  }

  unsigned get_num_expanded() const
  {
    return num_expanded;
  }

private:
  void process_child(Node *parent, Node *child)
  {
    domain.compute_heuristic(*parent, *child);
    assert(open.size() <= closed.size());
    assert(all_closed_item_ptrs_valid());

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

      closed[child] = open.push(child);  // insert better version of child
    }
    else {
      // The child has either already been expanded, or is worse
      // than the version in the open list.
      domain.free_node(child);
    }

    assert(all_closed_item_ptrs_valid());
    assert(open.invariants_satisfied());
  }


  bool all_closed_item_ptrs_valid() const
  {
#ifdef CHECK_ALL_CLOSED_ITEM_PTRS_VALID
    std::cerr << "checking if item pointers are valid" << std::endl
              << "  " << closed.size() << " pointers to check" << std::endl;
    for (ClosedConstIterator closed_it = closed.begin();
         closed_it != closed.end();
         ++closed_it) {
      if ( closed_it->second && !open.valid_item_pointer(*closed_it->second) )
        return false;
    }
    return true;
#else
    return true;
#endif
  }

private:
  Open &open;
  Closed &closed;

  const Node * goal;
  bool searched;

  const Domain &domain;

  unsigned num_expanded;
  unsigned num_generated;

private:
  AStar(const AStar<Domain, Node> &);
  AStar<Domain, Node> & operator =(const AStar<Domain, Node> &);
};


#endif /* !_A_STAR_HPP_ */

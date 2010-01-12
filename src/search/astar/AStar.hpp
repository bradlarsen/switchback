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
  typedef boost::unordered_map<
    Node *,
    boost::optional<typename Open::ItemPointer>,
    PointerHash<Node>,
    PointerEq<Node>
    , boost::fast_pool_allocator< std::pair<Node * const,
                                            boost::optional<typename Open::ItemPointer> > >
    > Closed;

public:
  AStar(Domain &domain)
    : closed(50000000)  // requested number of hash buckets
    , goal(NULL)
    , domain(domain)
    , num_expanded(0)
    , num_generated(0)
  {
  }

  ~AStar()
  {
  }

  void search()
  {
    // TODO: the following check is buggy.  What if there is no goal,
    // and the search has been completed?
    if (goal != NULL)
      return;

    typename std::vector<Node *> succs;    // re-use a stack-allocated
                                           // vector for successor
                                           // nodes, thus avoiding
                                           // repeated heap
                                           // allocation.

    {
      assert(all_closed_item_ptrs_valid());
      Node *start_node = domain.create_start_node();
      boost::optional<typename Open::ItemPointer> open_ptr = open.push(start_node);
      assert(open_ptr);
      closed[start_node] = open_ptr;
      assert(closed.find(start_node) != closed.end());
      assert(open.size() == 1);
      assert(closed.size() == 1);
      assert(all_closed_item_ptrs_valid());
    }

    while (!open.empty())
    {
// #ifndef NDEBUG
//       std::cerr << "open size is " << open.size() << std::endl
//                 << "closed size is " << closed.size() << std::endl;
// #endif

      Node *n = open.top();
      open.pop();
      closed[n] = boost::none;
      assert(all_closed_item_ptrs_valid());

      if (domain.is_goal(n->get_state())) {
        goal = n;
        return;
      }

      domain.expand(*n, succs);
      num_expanded += 1;
      num_generated += succs.size();

      for (unsigned succ_i = 0; succ_i < succs.size(); succ_i += 1)
      {
        Node *succ = succs[succ_i];
        assert(open.size() <= closed.size());
        assert(all_closed_item_ptrs_valid());
        typename Closed::iterator closed_it = closed.find(succ);
        if (closed_it != closed.end()) {
          boost::optional<typename Open::ItemPointer> &open_ptr = closed_it->second;

          if ( !open_ptr ) {
            // node is not in the open list, but is closed.  Drop it!
          }
          else if (succ->get_f() < open.lookup(*open_ptr)->get_f()) {
            // node is in the open list, but we found a better path
            // to its state.
            assert(open.lookup(*open_ptr)->get_state() == succ->get_state());
            assert(*open.lookup(*open_ptr) == *succ);
            open.erase(*open_ptr);
            open_ptr = open.push(succ);
            assert(open_ptr);
            closed_it->second = open_ptr;
            assert(all_closed_item_ptrs_valid());
          }
        }
        else {
          boost::optional<typename Open::ItemPointer> open_ptr = open.push(succ);
          assert(open_ptr);
          closed[succ] = open_ptr;
          assert(closed.find(open.lookup(*open_ptr)) != closed.end());
          assert(all_closed_item_ptrs_valid());
        }
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
  bool all_closed_item_ptrs_valid() const
  {
    return true;
    std::cerr << "checking if item pointers are valid" << std::endl
              << "  " << closed.size() << " pointers to check" << std::endl;
    for (typename Closed::const_iterator closed_it = closed.begin();
         closed_it != closed.end();
         ++closed_it) {
      if ( closed_it->second && !open.valid_item_pointer(*closed_it->second) )
        return false;
    }
    return true;
  }

private:
  Open open;
  Closed closed;

  const Node * goal;
  Domain &domain;

  unsigned num_expanded;
  unsigned num_generated;

private:
  AStar(const AStar<Domain, Node> &);
  AStar<Domain, Node> & operator =(const AStar<Domain, Node> &);
};


#endif /* !_A_STAR_HPP_ */

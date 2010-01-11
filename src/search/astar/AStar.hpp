#ifndef _A_STAR_HPP_
#define _A_STAR_HPP_


#include <vector>

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
    typename BucketPriorityQueue<Node>::ItemPointer *,
    PointerHash<Node>,
    PointerEq<Node>
    > Closed;

public:
  AStar(Domain &domain)
    : closed(50000000)  // requested number of hash buckets
    , goal(NULL)
    , domain(domain)
    , num_expanded(0)
    , num_generated(0)
  {
    closed.max_load_factor(0.70);
    check_all_closed_found();
  }

  ~AStar()
  {
  }

  void search()
  {
    if (goal != NULL)
      return;

    typename std::vector<Node *> succs;    // re-use a stack-allocated
                                           // vector for successor
                                           // nodes, thus avoiding
                                           // repeated heap
                                           // allocation.

    {
      typename Open::ItemPointer *open_ptr = open.push(domain.create_start_node());
      assert(open_ptr != NULL);
      assert(open.list_found(open_ptr->ptr.it));
      closed.insert(std::make_pair(open_ptr->get_item(), open_ptr));
      assert(closed.find(open_ptr->get_item()) != closed.end());
      assert(open.size() == 1);
      assert(closed.size() == 1);
      check_all_closed_found();
    }

    unsigned closed_size = closed.size();

    while (!open.empty()) {
      check_all_closed_found();

      Node *n = open.top();
      check_all_closed_found();
      open.pop();
      delete closed[n];
      closed[n] = NULL;
      check_all_closed_found();

      if (domain.is_goal(n->get_state())) {
        goal = n;
        break;
      }

      check_all_closed_found();
      domain.expand(*n, succs);
      num_expanded += 1;
      num_generated += succs.size();
      check_all_closed_found();

      for (typename std::vector<Node *>::const_iterator succs_it = succs.begin();
           succs_it != succs.end();
           ++succs_it)
      {
        assert(open.size() <= closed.size());
        check_all_closed_found();
        typename Closed::iterator closed_it = closed.find(*succs_it);
        check_all_closed_found();
        if (closed_it != closed.end()) {
          typename Open::ItemPointer *open_ptr = closed_it->second;
          check_all_closed_found();
          if (open_ptr != NULL)
            assert(open.list_found(open_ptr->ptr.it));

          if (open_ptr == NULL) {
            check_all_closed_found();
            // node is not in the open list, but is closed.  Drop it!
          }
          else if ((*succs_it)->get_f() < open_ptr->get_item()->get_f()) {
            // node is in the open list, but we found a better path
            // to its state.
            assert(open_ptr->get_item()->get_state() == (*succs_it)->get_state());
            assert(*open_ptr->get_item() == (**succs_it));
            check_all_closed_found();
            open.erase(*open_ptr);
            check_all_closed_found();
            open_ptr = open.push(*succs_it);
            check_all_closed_found();
            assert(open.list_found(open_ptr->ptr.it));
            assert(open_ptr != NULL);
            check_all_closed_found();
            closed_it->second = open_ptr;
            check_all_closed_found();
            assert(closed.size() == closed_size);
          }
        }
        else {
          std::cerr << "fawk!" << std::endl;
          check_all_closed_found();
          typename Open::ItemPointer *open_ptr = open.push(*succs_it);
          assert(open_ptr != NULL);
          assert(open.list_found(open_ptr->ptr.it));
          closed[*succs_it] = open_ptr;
          check_all_closed_found();
          assert(closed_size + 1 == closed.size());
          closed_size += 1;
          assert(closed.find(open_ptr->get_item()) != closed.end());
        }
      }
    }
  }

  void check_all_closed_found() const
  {
    unsigned trip_count = 0;
    for (typename Closed::const_iterator it = closed.begin();
         it != closed.end();
         ++it)
      {
        trip_count += 1;
        if (it->second != NULL)
          assert(open.list_found(it->second->ptr.it));
      }

    assert(trip_count == closed.size());
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
    return num_generated;
  }

  unsigned get_num_expanded() const
  {
    return num_expanded;
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

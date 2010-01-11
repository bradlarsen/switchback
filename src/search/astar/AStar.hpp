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
    assert(all_closed_found());
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
      typename Open::ItemPointer *open_ptr = open.push(domain.create_start_node());
      assert(open_ptr != NULL);
      assert(open.list_found(open_ptr->ptr->it));
      closed.insert(std::make_pair(open_ptr->get_item(), open_ptr));
      assert(closed.find(open_ptr->get_item()) != closed.end());
      assert(open.size() == 1);
      assert(closed.size() == 1);
      assert(all_closed_found());
    }

    unsigned closed_size = closed.size();

    while (!open.empty())
    {
      std::cerr << "open size is " << open.size() << std::endl
                << "closed size is " << closed.size() << std::endl;

      assert(all_closed_found());

      Node *n = open.top();
      assert(all_closed_found());
      open.pop();
      delete closed[n];
      closed[n] = NULL;
      assert(all_closed_found());

      if (domain.is_goal(n->get_state())) {
        goal = n;
        return;
      }

      assert(all_closed_found());
      domain.expand(*n, succs);
      num_expanded += 1;
      num_generated += succs.size();
      assert(all_closed_found());

      for (typename std::vector<Node *>::const_iterator succs_it = succs.begin();
           succs_it != succs.end();
           ++succs_it)
      {
        assert(open.size() <= closed.size());
        assert(all_closed_found());
        typename Closed::iterator closed_it = closed.find(*succs_it);
        assert(all_closed_found());
        if (closed_it != closed.end()) {
          typename Open::ItemPointer *open_ptr = closed_it->second;
          assert(all_closed_found());
          if (open_ptr != NULL)
            assert(open.list_found(open_ptr->ptr->it));

          if (open_ptr == NULL) {
            assert(all_closed_found());
            // node is not in the open list, but is closed.  Drop it!
          }
          else if ((*succs_it)->get_f() < open_ptr->get_item()->get_f()) {
            // node is in the open list, but we found a better path
            // to its state.
            assert(open_ptr->get_item()->get_state() == (*succs_it)->get_state());
            assert(*open_ptr->get_item() == (**succs_it));
            assert(all_closed_found());
            open.erase(open_ptr);
            assert(all_closed_found());
            open_ptr = open.push(*succs_it);
            assert(all_closed_found());
            assert(open.list_found(open_ptr->ptr->it));
            assert(open_ptr != NULL);
            assert(all_closed_found());
            closed_it->second = open_ptr;
            assert(all_closed_found());
            assert(closed.size() == closed_size);
          }
        }
        else {
          assert(all_closed_found());
          typename Open::ItemPointer *open_ptr = open.push(*succs_it);
          assert(open_ptr != NULL);
          assert(open.list_found(open_ptr->ptr->it));
          closed[*succs_it] = open_ptr;
          assert(all_closed_found());
          assert(closed_size + 1 == closed.size());
          closed_size += 1;
          assert(closed.find(open_ptr->get_item()) != closed.end());
        }
      }
    }
  }

#ifndef NDEBUG
  bool all_closed_found() const
  {
    unsigned trip_count = 0;
    for (typename Closed::const_iterator it = closed.begin();
         it != closed.end();
         ++it)
      {
        trip_count += 1;
        if (it->second != NULL && !open.list_found(it->second->ptr->it))
          return false;
      }

    assert(trip_count == closed.size());
    return true;
  }
#endif

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

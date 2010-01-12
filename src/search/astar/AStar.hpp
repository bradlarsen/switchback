#ifndef _A_STAR_HPP_
#define _A_STAR_HPP_


#include <vector>

#include <boost/none.hpp>
#include <boost/optional.hpp>
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
      Node *start_node = domain.create_start_node();
      boost::optional<typename Open::ItemPointer> open_ptr = open.push(start_node);
      assert(open_ptr);
      closed[start_node] = open_ptr;
      assert(closed.find(start_node) != closed.end());
      assert(open.size() == 1);
      assert(closed.size() == 1);
    }

    while (!open.empty())
    {
#ifndef NDEBUG
      std::cerr << "open size is " << open.size() << std::endl
                << "closed size is " << closed.size() << std::endl;
#endif

      Node *n = open.top();
      open.pop();
      closed[n] = boost::none;

      if (domain.is_goal(n->get_state())) {
        goal = n;
        return;
      }

      domain.expand(*n, succs);
      num_expanded += 1;
      num_generated += succs.size();

      for (typename std::vector<Node *>::const_iterator succs_it = succs.begin();
           succs_it != succs.end();
           ++succs_it)
      {
        assert(open.size() <= closed.size());
        typename Closed::iterator closed_it = closed.find(*succs_it);
        if (closed_it != closed.end()) {
          boost::optional<typename Open::ItemPointer> &open_ptr = closed_it->second;

          if ( !open_ptr) {
            // node is not in the open list, but is closed.  Drop it!
          }
          else if ((*succs_it)->get_f() < open.lookup(*open_ptr)->get_f()) {
            // node is in the open list, but we found a better path
            // to its state.
            assert(open.lookup(*open_ptr)->get_state() == (*succs_it)->get_state());
            assert(*open.lookup(*open_ptr) == (**succs_it));
            open.erase(*open_ptr);
            open_ptr = open.push(*succs_it);
            assert(open_ptr);
            closed_it->second = open_ptr;
          }
        }
        else {
          boost::optional<typename Open::ItemPointer> open_ptr = open.push(*succs_it);
          assert(open_ptr);
          closed[*succs_it] = open_ptr;
          assert(closed.find(open.lookup(*open_ptr)) != closed.end());
        }
      }
    }
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

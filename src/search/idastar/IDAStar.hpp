#ifndef _IDA_STAR_HPP_
#define _IDA_STAR_HPP_


#include <cassert>
#include <vector>

#include <boost/array.hpp>
#include <boost/optional.hpp>
#include <boost/pool/pool_alloc.hpp>
#include <boost/utility.hpp>

#include "search/Constants.hpp"
#include "util/PointerOps.hpp"


template <
  class DomainT,
  class NodeT
  >
class IDAStar : boost::noncopyable
{
public:
  typedef DomainT Domain;
  typedef NodeT Node;


private:
  typedef typename Node::Cost Cost;
  typedef typename Node::State State;


  struct BoundedResult
  {
    union Result
    {
      Cost cutoff;
      const Node *goal;
    } result;

    typedef enum {Cutoff, Goal, Failure} ResultType;

    ResultType result_type;


    BoundedResult(Cost cutoff)
    {
      result.cutoff = cutoff;
      result_type = Cutoff;
    }

    BoundedResult(const Node *goal)
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

    const Node * get_goal() const
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
  const Node *goal;
  bool searched;

  Domain &domain;

  unsigned num_expanded;
  unsigned num_generated;

  boost::pool<> node_pool;


public:
  IDAStar(Domain &domain)
    : goal(NULL)
    , searched(false)
    , domain(domain)
    , num_expanded(0)
    , num_generated(0)
    , node_pool(sizeof(Node))
  {
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


  void search()
  {
    if (searched)
      return;
    searched = true;

    Node *start_node = new (node_pool.malloc()) Node(domain.get_start_state(),
                                                     0,
                                                     0);
    domain.compute_heuristic(*start_node);
    goal = idastar_search(start_node);
  }


private:
  const Node * idastar_search(Node *start_node)
  {
    Cost bound = start_node->get_h();
    bool failed = false;

    const Node *goal_node = NULL;

    while ( goal_node == NULL && !failed ) {
#ifdef OUTPUT_SEARCH_PROGRESS
    std::cerr << "doing cost-bounded search with cutoff " << bound << std::endl;
    std::cerr << get_num_expanded() << " total nodes expanded" << std::endl
              << get_num_generated() << " total nodes generated" << std::endl;
#endif
      BoundedResult res = cost_bounded_search(start_node, bound);

      if (res.is_failure()) {
        assert(!res.is_goal());
        assert(!res.is_cutoff());
        failed = true;
      }
      else if (res.is_goal()) {
        assert(!res.is_cutoff());
        assert(!res.is_failure());
        goal_node = res.get_goal();
        assert(goal_node != NULL);
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

    return goal_node;
  }

  
  BoundedResult
  cost_bounded_search(Node *start_node,
                      const Cost bound)
  {
    if (start_node->get_state() == domain.get_goal_state()) {
      BoundedResult res(start_node);
      assert(res.is_goal());
      return res;
    }

    std::vector<Node *> succs;
    domain.compute_successors(*start_node, succs, node_pool);

    num_expanded += 1;
    num_generated += succs.size();


#ifdef OUTPUT_SEARCH_PROGRESS
      if (get_num_expanded() % 1000000 == 0) {
        std::cerr << "progress update:" << std::endl;
        std::cerr << get_num_expanded() << " total nodes expanded" << std::endl
                  << get_num_generated() << " total nodes generated" << std::endl;
      }
#endif


    boost::optional<Cost> new_cutoff;
    
    for (unsigned i = 0; i < succs.size(); i += 1) {
      Node *succ = succs[i];
      domain.compute_heuristic(*start_node, *succ);

      if (succ->get_f() <= bound) {
        BoundedResult res = cost_bounded_search(succ, bound);
        if (res.is_goal()) {
          assert(!res.is_cutoff());
          assert(!res.is_failure());
          return res;
        }
        else if (res.is_cutoff()) {
          assert(!res.is_goal());
          assert(!res.is_failure());

          if (new_cutoff)
            new_cutoff = std::min(*new_cutoff, res.get_cutoff());
          else
            new_cutoff = res.get_cutoff();
        }
      }
      else {
        if (new_cutoff)
          new_cutoff = std::min(*new_cutoff, succ->get_f());
        else
          new_cutoff = succ->get_f();
      }

      node_pool.free(succ);
    } /* end for */

    if (new_cutoff) {
      BoundedResult res(*new_cutoff);
      assert(res.is_cutoff());
      assert(!res.is_goal());
      assert(!res.is_failure());
      return res;
    }
    else {
      // std::cerr << "returning failure!" << std::endl;
      BoundedResult res;
      assert(res.is_failure());
      assert(!res.is_cutoff());
      assert(!res.is_failure());
      return res;
    }
  }
};


#endif /* !_IDA_STAR_HPP_ */

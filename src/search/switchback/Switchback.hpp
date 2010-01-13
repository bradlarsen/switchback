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
    State *,
    MaybeItemPointer,
    PointerHash<State>,
    PointerEq<State>
    , boost::fast_pool_allocator< std::pair<State * const, MaybeItemPointer> >
    > Closed;

private:
  const Node *goal;
  bool searched;

  const Domain &domain;

  unsigned num_expanded;
  unsigned num_generated;

  boost::array<Open, Domain::num_abstraction_levels> opened;
  boost::array<Closed, Domain::num_abstraction_levels> closed;

private:
  Switchback(const Switchback<Domain, Node> &);
  Switchback<Domain, Node> & operator =(const Switchback<Domain, Node> &);

public:
  Switchback(const Domain &domain)
    : goal(NULL)
    , searched(false)
    , domain(domain)
    , num_expanded(0)
    , num_generated(0)
  {
  }

  ~Switchback()
  {
  }

  void search()
  {
    if (searched || goal != NULL)
      return;
    searched = true;

    init_opened_and_closed();
    goal = resume_search(0, &domain.get_goal_state());
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
  Cost heuristic(const unsigned level, const State *goal_state)
  {
    assert(false);
    return 0;
  }
  
  Node * resume_search(const unsigned level, const State *goal_state)
  {
    assert(false);
    return NULL;
  }

  void init_opened_and_closed()
  {
    assert(false);
  }
};


#endif /* !_SWITCHBACK_HPP_ */

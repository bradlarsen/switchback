#ifndef _SWITCHBACK_HPP_
#define _SWITCHBACK_HPP_


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
class Switchback
{
private:
  typedef BucketPriorityQueue<Node> Open;
  typedef boost::unordered_map<
    Node *,
    boost::optional<typename Open::ItemPointer>,
    PointerHash<Node>,
    PointerEq<Node>
    , boost::fast_pool_allocator<
        std::pair<Node * const,
                  boost::optional<typename Open::ItemPointer> > >
    > Closed;

private:
  const Node * goal;
  const Domain &domain;

  unsigned num_expanded;
  unsigned num_generated;

private:
  Switchback(const Switchback<Domain, Node> &);
  Switchback<Domain, Node> & operator =(const Switchback<Domain, Node> &);

public:
  Switchback(const Domain &domain)
    : goal(NULL)
    , domain(domain)
    , num_expanded(0)
    , num_generated(0)
  {
    assert(false);
  }

  ~Switchback()
  {
    assert(false);
  }

  void search()
  {
    assert(false);
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
};


#endif /* !_SWITCHBACK_HPP_ */

#ifndef _A_STAR_HPP_
#define _A_STAR_HPP_


#include <vector>

#include <boost/unordered_set.hpp>

#include "util/BucketPriorityQueue.hpp"
#include "util/PointerOps.hpp"


template <
  class Domain,
  class Node
  >
class AStar
{
private:
  typedef BucketPriorityQueue<Node *, bucket<Node *>, PointerGeq<Node> > Open;
  typedef boost::unordered_set<Node *, PointerHash<Node>, PointerEq<Node> > Closed;

public:
  AStar(Domain &domain)
    : open(Open(domain.get_num_buckets()))
    , closed(50000000)  // requested number of buckets
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
    if (goal != NULL)
      return;

    typename std::vector<Node *> succs;    // re-use a stack-allocated
                                           // vector for successor
                                           // nodes, thus avoiding
                                           // repeated heap
                                           // allocation.

    open.push(domain.create_start_node());

    while (!open.empty()) {
      Node *n = open.top();
      open.pop();

      if (domain.is_goal(n->get_state())) {
        goal = n;
        break;
      }

      // drop duplicates
      if (closed.find(n) != closed.end()) {
        continue;
      }

      closed.insert(n);
      domain.expand(*n, succs);
      num_expanded += 1;
      num_generated += succs.size();
      for (typename std::vector<Node *>::const_iterator succs_it = succs.begin();
           succs_it != succs.end();
           ++succs_it)
      {
        // TODO: should only insert nodes that are not present in
        // open, or that are better than their existing copies in
        // open.  In the latter case, the worse copies should be
        // deleted.
        //
        // Wheeler says I should keep a ``closed'' set that actually
        // contains everything in open and everything in closed.  I
        // should check this closed set at node generation time, not
        // node expansion time.  If a node is known to be in the open
        // list, and the generated copy is better, the old version in
        // the open list should be deleted, and the new one inserted.
        assert(open.size() == open.debug_slow_size());
        open.push(*succs_it);
        assert(open.size() == open.debug_slow_size());
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

#ifndef _A_STAR_HPP_
#define _A_STAR_HPP_


#include <queue>
#include <vector>

#include <boost/unordered_set.hpp>
#include "BucketPriorityQueue.hpp"
#include "PointerHash.hpp"


template <class T>
struct PointerGeq
{
  inline bool operator ()(const T *t1, const T *t2)
  {
    return *t1 >= *t2;
  }
};


template <class T>
struct PointerEq
{
  inline bool operator ()(const T *t1, const T *t2) const
  {
    return *t1 == *t2;
  }
};


template <
  class Domain,
  class Node
  >
class AStar
{
private:
  //  typedef BucketPriorityQueue<Node *> Open;
  typedef std::priority_queue<Node *, std::vector<Node *>, PointerGeq<Node> > Open;
  typedef boost::unordered_set<Node *, PointerHash<Node>, PointerEq<Node> > Closed;

public:
  AStar(const Domain &domain)
    : closed(Closed())
      //    , open(Open(domain.get_num_buckets()))
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
    if (goal != NULL)
      return;

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
      typename std::vector<Node *> *succs = domain.expand(*n);
      num_expanded += 1;
      num_generated += succs->size();
      for (typename std::vector<Node *>::const_iterator succs_it = succs->begin();
           succs_it != succs->end();
           ++succs_it)
      {
        open.push(*succs_it);
      }
      delete succs;
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
  Closed closed;
  Open open;

  const Node * goal;
  const Domain &domain;

  unsigned num_expanded;
  unsigned num_generated;

private:
  AStar(const AStar<Domain, Node> &);
  AStar<Domain, Node> & operator =(const AStar<Domain, Node> &);
};


#endif /* !_A_STAR_HPP_ */

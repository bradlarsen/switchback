#ifndef _BUCKET_PRIORITY_QUEUE_HPP_
#define _BUCKET_PRIORITY_QUEUE_HPP_


#include <cassert>
#include <list>
#include <vector>

#include <boost/integer_traits.hpp>


namespace
{
  template <class Node>
  class Bucket
  {
  public:
    Bucket()
      : last_bin(0)
      , num_elems(0)
    {
      assert(empty());
      assert(invariants_hold());
    }

    ~Bucket()
    {
      assert(invariants_hold());
    }

    void push(Node *n)
    {
      assert(invariants_hold());

      num_elems += 1;

      const unsigned bucket_num = n->get_g();
      if (bucket_num >= store.size()) {
        store.resize(bucket_num+1);
      }
      assert(bucket_num < store.size());

      if (bucket_num > last_bin)
        last_bin = bucket_num;
      store[bucket_num].push_front(n);

      assert(invariants_hold());
    }

    void pop()
    {
      assert(invariants_hold());
      assert(!empty());

      num_elems -= 1;
      store[last_bin].pop_front();

      if (empty()) {
        last_bin = 0;
      }
      else if (store[last_bin].empty() && last_bin > 0) {
        for (unsigned i = last_bin - 1; i >= 0; i -= 1) {
          if (!store[i].empty()) {
            last_bin = i;
            break;
          }
        }
      }

      assert(invariants_hold());
    }

    Node * top() const
    {
      assert(invariants_hold());
      assert(!empty());
      return store[last_bin].front();
    }

    bool empty() const
    {
      return num_elems == 0;
    }

    unsigned size() const
    {
      return num_elems;
    }

#ifndef NDEBUG
    unsigned debug_slow_size() const
    {
      unsigned slow_size = 0;
      for (unsigned i = 0; i < store.size(); i += 1) {
        if (store[i].size() > 0)
          slow_size += store[i].size();
      }
      return slow_size;
    }
#endif

  private:
    unsigned last_bin;
    unsigned num_elems;
    std::vector< std::list<Node *> > store;


#ifndef NDEBUG
    bool invariants_hold() const
    {
      return
        size() == debug_slow_size() &&
        ((empty() && size() == 0) || (!empty() && size() > 0)) &&
        (empty() || last_bin < store.size());
    }
#endif
  };
}


template <class Node>
class BucketPriorityQueue
{
public:
  BucketPriorityQueue()
    : first_bucket(boost::integer_traits<unsigned>::const_max)
    , num_elems(0)
  {
    assert(empty());
    assert(invariants_hold());
  }

  ~BucketPriorityQueue()
  {
    assert(invariants_hold());
  }

  void push(Node *n)
  {
    assert(invariants_hold());

    num_elems += 1;

    const unsigned bucket_num = n->get_f();
    if (bucket_num >= store.size()) {
      store.resize(bucket_num+1);
    }
    assert(bucket_num < store.size());

    if (bucket_num < first_bucket)
      first_bucket = bucket_num;
    store[bucket_num].push(n);

    assert(invariants_hold());
  }

  void pop()
  {
    assert(invariants_hold());
    assert(!empty());

    num_elems -= 1;
    store[first_bucket].pop();

    if (empty()) {
      first_bucket = boost::integer_traits<unsigned>::const_max;
    }
    else if (store[first_bucket].empty()) {
      for (unsigned i = first_bucket + 1; i < store.size(); i += 1) {
        if (!store[i].empty()) {
          first_bucket = i;
          break;
        }
      }
    }

    assert(invariants_hold());
  }

  Node * top() const
  {
    assert(!empty());
    return store[first_bucket].top();
  }

  bool empty() const
  {
    return num_elems == 0;
  }

  unsigned size() const
  {
    return num_elems;
  }

#ifndef NDEBUG
  unsigned debug_slow_size() const
  {
    unsigned slow_size = 0;
    for (unsigned i = 0; i < store.size(); i += 1) {
      if (store[i].size() > 0)
      slow_size += store[i].debug_slow_size();
    }
    return slow_size;
  }
#endif

private:
  std::vector< Bucket<Node> > store;

  unsigned first_bucket;
  unsigned num_elems;

#ifndef NDEBUG
  bool invariants_hold() const
  {
    return
      size() == debug_slow_size() &&
      ((empty() && size() == 0) || (!empty() && size() > 0)) &&
      (empty() || first_bucket < store.size());
  }
#endif
};


#endif /* !_BUCKET_PRIORITY_QUEUE_HPP_ */

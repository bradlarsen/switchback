#ifndef _BUCKET_PRIORITY_QUEUE_HPP_
#define _BUCKET_PRIORITY_QUEUE_HPP_


#include <algorithm>
#include <cassert>
#include <functional>
#include <list>
#include <vector>
#include <boost/integer_traits.hpp>


// TODO: BucketPriorityQueue needs an iterator or some such.


template <class T>
struct bucket
{
  inline unsigned operator ()(const T &t) const
  {
    return get_bucket(t);
  }
};


template<class T>
inline unsigned get_bucket(const T *t)
{
  return get_bucket(*t);
}


template <
  class T,
  class BucketFun=bucket<T>,
  class Comparator=std::less<T>
  >
class BucketPriorityQueue
{
public:
  typedef std::vector<T> BucketType;

  BucketPriorityQueue(unsigned num_buckets)
    : store(std::vector<BucketType>(num_buckets, BucketType()))
    , first_bucket(initial_first_bucket_value)
    , num_elems(0)
    , bucket(BucketFun())
    , comparator(Comparator())
  {
    assert(empty());
    assert(size() == debug_slow_size());
  }

  ~BucketPriorityQueue()
  {
    assert(size() == debug_slow_size());
  }

  inline void push(const T &e)
  {
    assert(size() == debug_slow_size());
    num_elems += 1;

    unsigned bucket_num = bucket(e);
    assert(bucket_num < store.size());

    if (bucket_num < first_bucket)
      first_bucket = bucket_num;
    insert_bucket(store[bucket_num], e);
    assert(size() == debug_slow_size());
  }

  void pop()
  {
    assert(size() == debug_slow_size());
    assert(!empty());

    num_elems -= 1;
    BucketType &b = store[first_bucket];
    std::pop_heap(b.begin(), b.end(), comparator);
    b.pop_back();

    if (empty()) {
      first_bucket = initial_first_bucket_value;
    }
    else if (b.empty()) {
      for (unsigned i = first_bucket + 1; i < store.size(); i += 1) {
        if (!store[i].empty()) {
          first_bucket = i;
          break;
        }
      }
    }

    assert( !empty() || first_bucket == initial_first_bucket_value );
    assert(size() == debug_slow_size());
  }

  const T & top() const
  {
    assert(!empty());

    return store[first_bucket].front();
  }

  // inline void reset()
  // {
  //   store.clear();
  //   first_bucket = 0;
  //   num_elems = 0;

  //   assert(empty());
  // }
  
  inline bool empty() const
  {
    assert(size() == debug_slow_size());
    return num_elems == 0;
  }

  inline unsigned size() const
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
  inline void insert_bucket(BucketType &b, const T &e)
  {
    b.push_back(e);
    std::push_heap(b.begin(), b.end(), comparator);
  }

  const static unsigned initial_first_bucket_value =
    boost::integer_traits<unsigned>::const_max;

  std::vector<BucketType> store;

  unsigned first_bucket;

  unsigned num_elems;
  BucketFun bucket;
  Comparator comparator;
};


#endif /* !_BUCKET_PRIORITY_QUEUE_HPP_ */

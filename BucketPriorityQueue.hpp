#ifndef _BUCKET_PRIORITY_QUEUE_HPP_
#define _BUCKET_PRIORITY_QUEUE_HPP_


#include <cassert>
#include <functional>
#include <vector>
#include <list>
#include <boost/integer_traits.hpp>


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
  typedef std::list<T> BucketType;

  BucketPriorityQueue(unsigned num_buckets)
    : store(std::vector<BucketType>(num_buckets, BucketType()))
    , first_bucket(initial_first_bucket_value)
    , num_elems(0)
    , bucket(BucketFun())
  {
    assert(empty());
  }

  ~BucketPriorityQueue()
  {
  }

  inline void insert(const T &e)
  {
    num_elems += 1;
    unsigned bucket_num = bucket(e);
    assert(bucket_num < store.size());
    if (bucket_num < first_bucket)
      first_bucket = bucket_num;
    BucketType &b = store[bucket_num];
    typename BucketType::iterator it = b.begin();
    while (it != b.end() && *it < e) {
      ++it;
    }
    b.insert(it, e);
  }

  void pop()
  {
    assert(!empty());

    num_elems -= 1;
    BucketType &b = store[first_bucket];
    b.pop_front();

    if (empty()) {
      first_bucket = initial_first_bucket_value;
    }
    else if (b.empty() && num_elems != 0) {
      for (unsigned i = first_bucket + 1; i < store.size(); i += 1) {
        if (!store[i].empty()) {
          first_bucket = i;
          break;
        }
      }
    }

    assert( !empty() || first_bucket == initial_first_bucket_value );
  }

  const T & top() const
  {
    assert(!empty());

    return store[first_bucket].front();
  }

  inline void reset()
  {
    store.clear();
    first_bucket = 0;
    num_elems = 0;

    assert(empty());
  }
  
  inline bool empty() const
  {
    return num_elems == 0;
  }

  inline unsigned size() const
  {
    return num_elems;
  }


private:
  const static unsigned initial_first_bucket_value =
    boost::integer_traits<unsigned>::const_max;

  std::vector<BucketType> store;

  unsigned first_bucket;

  unsigned num_elems;
  BucketFun bucket;
};


#endif /* !_BUCKET_PRIORITY_QUEUE_HPP_ */

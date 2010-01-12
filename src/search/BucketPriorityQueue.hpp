#ifndef _BUCKET_PRIORITY_QUEUE_HPP_
#define _BUCKET_PRIORITY_QUEUE_HPP_


#include <cassert>

#include <list>
#include <vector>

#include <boost/integer_traits.hpp>


template <class Node>
class BucketPriorityQueue
{
private:
  unsigned num_elems;
  unsigned first_bucket;

  typedef std::vector<Node *> Bin;
  typedef std::vector<Bin> Bucket;

  std::vector<Bucket> store;


public:
    struct ItemPointer
    {
      ItemPointer(unsigned bucket_num, unsigned bin_num, unsigned idx)
        : bucket_num(bucket_num)
        , bin_num(bin_num)
        , idx(idx)
      {
      }

      unsigned bucket_num;
      unsigned bin_num;
      unsigned idx;
    };


public:
  BucketPriorityQueue()
    : num_elems(0)
    , first_bucket(boost::integer_traits<unsigned>::const_max)
  {
    assert(empty());
  }

  ~BucketPriorityQueue()
  {
  }

  ItemPointer push(Node *n)
  {
#ifndef NDEBUG
    std::cerr << "push!" << std::endl;
#endif

    num_elems += 1;

    const unsigned bucket_num = n->get_f();
    if (bucket_num >= store.size()) {
#ifndef NDEBUG
      std::cerr << "resizing store from " << store.size() << " to " << bucket_num + 1 << std::endl;
#endif
      store.resize(bucket_num + 1);
    }
    assert(bucket_num < store.size());
 
    const unsigned bin_num = n->get_g();
    if (bin_num >= store[bucket_num].size()) {
#ifndef NDEBUG
      std::cerr << "resizing bucket " << bucket_num << " from " << store[bucket_num].size() << " to " << bin_num + 1 << std::endl;
#endif
      store[bucket_num].resize(bin_num + 1);
    }
    assert(bin_num < store[bucket_num].size());

    store[bucket_num][bin_num].push_back(n);
    
    const unsigned idx = store[bucket_num][bin_num].size() - 1;

    if (bucket_num < first_bucket) {
#ifndef NDEBUG
      std::cerr << "changing first_bucket from " << first_bucket << " to " << bucket_num << std::endl;
#endif
      first_bucket = bucket_num;
    }

    ItemPointer item_ptr = ItemPointer(bucket_num, bin_num, idx);
    return item_ptr;
  }

  void pop()
  {
#ifndef NDEBUG
    std::cerr << "pop!" << std::endl;
#endif
    assert(!empty());

    num_elems -= 1;

    if (empty()) {
      // entire priority queue has been exhausted
#ifndef NDEBUG
      std::cerr << "priority queue exhausted" << std::endl;
#endif
      first_bucket = boost::integer_traits<unsigned>::const_max;
      store.clear();
      return;
    }

    assert(!store[first_bucket].empty());
    assert(!store[first_bucket].back().empty());
    store[first_bucket].back().pop_back();

    while (!store[first_bucket].empty() && store[first_bucket].back().empty()) {
#ifndef NDEBUG
      std::cerr << "popping a bin from bucket " << first_bucket << std::endl
                << "  bin has size " << store[first_bucket].back().size() << std::endl
                << "  bucket " << first_bucket << " has size " << store[first_bucket].size() << std::endl;
#endif
      store[first_bucket].pop_back();
    }

    while (store[first_bucket].empty() && first_bucket < store.size() - 1) {
#ifndef NDEBUG
      std::cerr << "incrementing first_bucket from " << first_bucket << std::endl;
#endif
      first_bucket += 1;
    }
  }

  Node * top() const
  {
    assert(!empty());
    assert(first_bucket < store.size());
    assert(!store[first_bucket].empty());
    assert(!store[first_bucket].back().empty());
    Node *ret = store[first_bucket].back().back();
    assert(ret != NULL);
    return ret;
  }

  void erase(const ItemPointer &ptr)
  {
    assert(false);
    assert(!empty());
    store[ptr.bucket_num][ptr.bin_num][ptr.idx] = NULL;
  }

  Node * lookup(const ItemPointer &ptr)
  {
    assert(!empty());
    assert(ptr.bucket_num < store.size());
    Node * ret = store[ptr.bucket_num][ptr.bin_num][ptr.idx];
    assert(ret != NULL);
    return ret;
  }

  bool empty() const
  {
    return num_elems == 0;
  }

  unsigned size() const
  {
    return num_elems;
  }
};


#endif /* !_BUCKET_PRIORITY_QUEUE_HPP_ */

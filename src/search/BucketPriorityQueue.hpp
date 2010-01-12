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
    num_elems += 1;

    const unsigned bucket_num = n->get_f();
    if (bucket_num >= store.size())
      store.resize(bucket_num + 1);
    assert(bucket_num < store.size());
 
    const unsigned bin_num = n->get_g();
    if (bin_num >= store[bucket_num].size())
      store[bucket_num].resize(bin_num + 1);
    assert(bin_num < store[bucket_num].size());

    store[bucket_num][bin_num].push_back(n);
    
    const unsigned idx = store[bucket_num][bin_num].size() - 1;

    if (bucket_num < first_bucket)
      first_bucket = bucket_num;

    ItemPointer item_ptr = ItemPointer(bucket_num, bin_num, idx);
    assert(valid_item_pointer(item_ptr));
    return item_ptr;
  }

  void pop()
  {
    assert(!empty());

    num_elems -= 1;

    if (empty()) {
      // entire priority queue has been exhausted
      reset();
      return;
    }

    assert(!store[first_bucket].empty());
    assert(!store[first_bucket].back().empty());
    store[first_bucket].back().pop_back();

    while (!store[first_bucket].empty() && store[first_bucket].back().empty())
      store[first_bucket].pop_back();

    while (store[first_bucket].empty() && first_bucket < store.size() - 1)
      first_bucket += 1;
  }

  Node * top() const
  {
    assert(!empty());
    assert(first_bucket < store.size());
    assert(!store[first_bucket].empty());
    assert(!store[first_bucket].back().empty());

    // Because of the way element deletions are implemented in this
    // priority queue, we may have to scan an entire bin to find a
    // non-deleted element.
    const Bin &bin = store[first_bucket].back();
    for (typename Bin::const_reverse_iterator bin_it = bin.rbegin();
         bin_it != bin.rend();
         ++bin_it) {
      if (*bin_it != NULL)
        return *bin_it;
    }

    assert(false);
    return NULL;
  }

  void erase(const ItemPointer &ptr)
  {
    std::cerr << "erase!" << std::endl;
    assert(!empty());
    assert(valid_item_pointer(ptr));

    num_elems -= 1;

    if (empty()) {
      reset();
      return;
    }

    Bucket &bucket = store[ptr.bucket_num];
    Bin &bin = store[ptr.bucket_num][ptr.bin_num];
    bin[ptr.idx] = NULL;

    bool all_null = bin_vals_all_null(bin);
    if (all_null && bucket.size() == ptr.bin_num + 1) {
      std::cerr << "bin at end to be popped" << std::endl;
      // The bin at the end of the bucket has been entirely deleted.
      // Get rid of it.
      //
      // An invariant is that at most one bin needs to be popped when
      // erasing an element.
      bucket.pop_back();

      if (bucket.empty() && ptr.bucket_num == first_bucket) {
        std::cerr << "first bucket to be updated" << std::endl;
        // The first bucket has been emptied.  We need to find the new
        // first bucket.
        assert(!empty());
        while (store[first_bucket].empty())
          first_bucket += 1;
      }
    }
    else if (all_null) {
      std::cerr << "non-last bin to be cleared" << std::endl;
      // A bin somewhere in the bucket, but not at the end, has been
      // entirely deleted.  Clear it out, but don't delete it, as
      // deletion would invalidate any ItemPointers that have been
      // handed out.
      bin.clear();
    }
  }

  Node * lookup(const ItemPointer &ptr)
  {
    assert(!empty());
    assert(valid_item_pointer(ptr));
    Node *ret = store[ptr.bucket_num][ptr.bin_num][ptr.idx];
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

private:
  bool bin_vals_all_null(const Bin &bin) const
  {
    for (unsigned i = 0; i < bin.size(); ++i)
      if (bin[i] != NULL) {
        return false;
      }
    return true;
  }

  void reset()
  {
    first_bucket = boost::integer_traits<unsigned>::const_max;
    store.clear();
  }


public:
  bool valid_item_pointer(const ItemPointer &ptr) const
  {
    return
      ptr.bucket_num < store.size() &&
      ptr.bin_num < store[ptr.bucket_num].size() &&
      ptr.idx < store[ptr.bucket_num][ptr.bin_num].size();
  }
};


#endif /* !_BUCKET_PRIORITY_QUEUE_HPP_ */

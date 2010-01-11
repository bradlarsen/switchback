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
    struct ItemPointer
    {
      ItemPointer(unsigned bin_idx, typename std::list<Node *>::iterator it)
        : bin_idx(bin_idx)
        , it(it)
      {
      }

      Node * get_item() const
      {
        return *it;
      }

      const unsigned bin_idx;
      typename std::list<Node *>::iterator it;
    };

  public:
    Bucket()
      : last_bin(0)
    {
      assert(empty());
      assert(invariants_hold());
    }

    ~Bucket()
    {
      assert(invariants_hold());
    }

    ItemPointer * push(Node *n)
    {
      assert(invariants_hold());

      const unsigned bucket_num = n->get_g();
      while (bucket_num >= store.size()) {
        store.push_back(new std::list<Node *>());
      }
      assert(bucket_num < store.size());

      if (bucket_num > last_bin)
        last_bin = bucket_num;
      store[bucket_num]->push_front(n);

      assert(invariants_hold());

      assert(bucket_num < store.size());
      assert(!store[bucket_num]->empty());
      return new ItemPointer(bucket_num, store[bucket_num]->begin());
    }

    void erase(const ItemPointer *ptr)
    {
      assert(invariants_hold());
      assert(!empty());

      assert(ptr->bin_idx < store.size());
      assert(!store[ptr->bin_idx]->empty());

      store[ptr->bin_idx]->erase(ptr->it);
      
      if (empty()) {
        last_bin = 0;
      }
      else if (ptr->bin_idx == last_bin && store[last_bin]->empty() && last_bin > 0) {
        for (unsigned i = last_bin - 1; i >= 0; i -= 1) {
          if (!store[i]->empty()) {
            last_bin = i;
            break;
          }
        }
      }
    }

    void pop()
    {
      assert(invariants_hold());
      assert(!empty());

      assert(last_bin < store.size());
      store[last_bin]->pop_front();

      if (store[last_bin]->empty() && last_bin > 0) {
        assert(last_bin > 0);
        while (last_bin > 0) {
          if (!store[last_bin]->empty())
            break;
          last_bin -= 1;
        }
      }
    }

    Node * top() const
    {
      assert(invariants_hold());
      assert(!empty());
      return store[last_bin]->front();
    }

    bool empty() const
    {
      return store.empty() || (last_bin == 0 && store[last_bin]->empty());
    }

    unsigned size() const
    {
      unsigned size_sum = 0;
      for (unsigned i = 0; i < store.size(); i += 1) {
        assert(store[i] != NULL);
        size_sum += store[i]->size();
      }
      return size_sum;
    }

    //  private:
  public:
    unsigned last_bin;
    std::vector< std::list<Node *> * > store;


#ifndef NDEBUG
    bool invariants_hold() const
    {
      return
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
    struct ItemPointer
    {
      ItemPointer(unsigned bin_idx, typename Bucket<Node>::ItemPointer *ptr)
        : bin_idx(bin_idx)
        , ptr(ptr)
      {
      }

      Node * get_item() const
      {
        return ptr->get_item();
      }

      unsigned bin_idx;
      typename Bucket<Node>::ItemPointer *ptr;
    };


public:
  BucketPriorityQueue()
    : first_bucket(boost::integer_traits<unsigned>::const_max)
  {
    assert(empty());
    assert(invariants_hold());
  }

  ~BucketPriorityQueue()
  {
    assert(invariants_hold());
  }

  ItemPointer * push(Node *n)
  {
    assert(invariants_hold());

    const unsigned bucket_num = n->get_f();
    while (bucket_num >= store.size()) {
      store.push_back(new Bucket<Node>());
    }
    assert(bucket_num < store.size());

    if (bucket_num < first_bucket)
      first_bucket = bucket_num;
    typename Bucket<Node>::ItemPointer *ptr = store[bucket_num]->push(n);
    assert(list_found(ptr->it));

    assert(invariants_hold());
    ItemPointer *item_ptr = new ItemPointer(bucket_num, ptr);
    assert(list_found(item_ptr->ptr->it));
    return item_ptr;
  }

  void erase(const ItemPointer *ptr)
  {
    assert(invariants_hold());

    assert(ptr->bin_idx < store.size());
    assert(!store[ptr->bin_idx]->empty());

    assert(list_found(ptr->ptr->it));
    store[ptr->bin_idx]->erase(ptr->ptr);
    assert(store[ptr->bin_idx]->size() <= size());

    if (empty()) {
      first_bucket = boost::integer_traits<unsigned>::const_max;
    }
    else if (ptr->bin_idx == first_bucket && store[first_bucket]->empty()) {
      for (unsigned i = first_bucket + 1; i < store.size(); i += 1) {
        if (!store[i]->empty()) {
          first_bucket = i;
          break;
        }
      }
    }

    assert(invariants_hold());
  }


#ifndef NDEBUG
  bool list_found(const typename std::list<Node *>::const_iterator &clit) const
  {
    for (typename std::vector< Bucket<Node> * >::const_iterator cit = store.begin();
         cit != store.end();
         ++cit)
      {
        for (typename std::vector< std::list<Node *> * >::const_iterator cit2 = (*cit)->store.begin();
             cit2 != (*cit)->store.end();
             ++cit2)
          {
            for (typename std::list<Node *>::const_iterator list_it = (*cit2)->begin();
                 list_it != (*cit2)->end();
                 ++list_it)
              {
                if (list_it == clit) {
                  return true;
                }
              }
          }
      }

    return false;
  }
#endif

  void pop()
  {
    assert(invariants_hold());
    assert(!empty());
    assert(first_bucket < store.size());

    assert(!store[first_bucket]->empty());
    store[first_bucket]->pop();

    if (store[first_bucket]->empty()) {
      bool empty = true;
      for (unsigned i = first_bucket + 1; i < store.size(); i += 1) {
        if (!store[i]->empty()) {
          empty = false;
          first_bucket = i;
          break;
        }
      }

      if (empty) {
        first_bucket = boost::integer_traits<unsigned>::const_max;
      }
    }

    assert(((empty() && size() == 0) || (!empty() && size() > 0)));
    assert((empty() || first_bucket < store.size()));
    assert(invariants_hold());
  }

  Node * top() const
  {
    assert(!empty());
    assert(first_bucket < store.size());
    return store[first_bucket]->top();
  }

  bool empty() const
  {
    return store.empty() || first_bucket == boost::integer_traits<unsigned>::const_max;
  }

  unsigned size() const
  {
    unsigned size_sum = 0;
    for (unsigned i = 0; i < store.size(); i += 1)
      size_sum += store[i]->size();
    return size_sum;
  }

private:
  std::vector< Bucket<Node> * > store;

  unsigned first_bucket;

#ifndef NDEBUG
  bool invariants_hold() const
  {
    return
      ((empty() && size() == 0) || (!empty() && size() > 0)) &&
      (empty() || first_bucket < store.size()) &&
      (!empty() || first_bucket == boost::integer_traits<unsigned>::const_max);
  }
#endif
};


#endif /* !_BUCKET_PRIORITY_QUEUE_HPP_ */

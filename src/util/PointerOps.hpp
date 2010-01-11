#ifndef _POINTER_OPS_HPP_
#define _POINTER_OPS_HPP_


#include <boost/functional/hash.hpp>


template <class T>
struct PointerHash
{
  inline std::size_t operator ()(const T *ptr) const
  {
    return hash_value(*ptr);
  }
};


template <class T>
struct PointerLt
{
  inline bool operator ()(const T *t1, const T *t2)
  {
    return *t1 < *t2;
  }
};


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


#endif /* !_POINTER_OPS_HPP_ */

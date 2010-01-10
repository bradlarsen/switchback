#ifndef _POINTER_HASH_HPP_
#define _POINTER_HASH_HPP_


#include <boost/functional/hash.hpp>


template <class T>
struct PointerHash
{
  inline std::size_t operator ()(const T *ptr) const
  {
    return hash_value(*ptr);
  }
};


#endif /* !_POINTER_HASH_HPP_ */

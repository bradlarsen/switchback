#ifndef _TILES_NODE_HPP_
#define _TILES_NODE_HPP_


#include "TilesTypes.hpp"
#include "TilesState.hpp"


class TilesNode15
{
public:
  TilesNode15(const TilesState15 &s,
              Cost g,
              Cost h,
              const TilesNode15 *p = 0)
    : g(g)
    , h(h)
    , state(s)
    , parent(p)
  {
  }

  inline Cost get_f() const
  {
    return g + h;
  }

  inline Cost get_g() const
  {
    return g;
  }

  inline Cost get_h() const
  {
    return h;
  }

  inline const TilesState15 & get_state() const
  {
    return state;
  }

  const TilesNode15 * get_parent() const
  {
    return parent;
  }

  /**
   * Compare nodes by f-value, breaking ties in favor of high g-value.
   */
  inline bool operator <(const TilesNode15 &other) const
  {
    Cost my_f = get_f();
    Cost other_f = other.get_f();

    return my_f < other_f || (my_f == other_f && get_g() > other.get_g());
  }

  inline bool operator >=(const TilesNode15 &other) const
  {
    return !(*this < other);
  }

  /**
   * Equality is determined by the enclosed tile puzzle states.
   */
  inline bool operator ==(const TilesNode15 &other) const
  {
    return state == other.state;
  }

private:
  Cost g;
  Cost h;
  TilesState15 state;
  const TilesNode15 *parent;
};


/**
 * To hash a TilesNode15, simply use the hash value of the enclosed
 * tile puzzle state.
 */
inline std::size_t hash_value(TilesNode15 const &node)
{
  return hash_value(node.get_state());
}


/**
 * This procedure is needed for the bucket-based priority queue, which
 * has constant time operations.
 *
 * The bucket for a TilesNode15 is its f-value.
 */
inline unsigned get_bucket(const TilesNode15 &n)
{
  return n.get_f();
}


std::ostream & operator <<(std::ostream &o, const TilesNode15 &n);

#endif /* !_TILES_NODE_HPP_ */

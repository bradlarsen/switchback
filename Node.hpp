#ifndef _NODE_HPP_
#define _NODE_HPP_


template <
  class State,
  class Cost
  >
class Node
{
public:
  Node(const State &s,
       Cost g,
       Cost h,
       const Node<State, Cost> *p = NULL)
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

  inline const State & get_state() const
  {
    return state;
  }

  const Node<State, Cost> * get_parent() const
  {
    return parent;
  }

  /**
   * Compare nodes by f-value, breaking ties in favor of high g-value.
   */
  inline bool operator <(const Node<State, Cost> &other) const
  {
    Cost my_f = get_f();
    Cost other_f = other.get_f();

    return my_f < other_f || (my_f == other_f && get_g() > other.get_g());
  }

  inline bool operator >=(const Node<State, Cost> &other) const
  {
    return !(*this < other);
  }

  /**
   * Equality is determined by the enclosed tile puzzle states.
   */
  inline bool operator ==(const Node<State, Cost> &other) const
  {
    return state == other.state;
  }

private:
  Cost g;
  Cost h;
  State state;
  const Node<State, Cost> *parent;
};


/**
 * To hash a TilesNode15, simply use the hash value of the enclosed
 * tile puzzle state.
 */
template <
  class State,
  class Cost
  >
inline std::size_t hash_value(Node<State, Cost> const &node)
{
  return hash_value(node.get_state());
}


/**
 * This procedure is needed for the bucket-based priority queue, which
 * has constant time operations.
 *
 * The bucket for a node is its f-value.
 */
template <
  class State,
  class Cost
  >
inline unsigned get_bucket(const Node<State, Cost> &n)
{
  return n.get_f();
}


template <
  class State,
  class Cost
  >
std::ostream & operator <<(std::ostream &o, const Node<State, Cost> &n)
{
  o << n.get_state() << std::endl
    << "f: " << n.get_f() << std::endl
    << "g: " << n.get_g() << std::endl
    << "h: " << n.get_h() << std::endl;
  return o;
}


#endif /* !_NODE_HPP_ */

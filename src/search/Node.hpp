#ifndef _NODE_HPP_
#define _NODE_HPP_


template <
  class StateT,
  class CostT
  >
class Node
{
public:
  typedef StateT State;
  typedef CostT Cost;

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

  Cost get_f() const
  {
    return g + h;
  }

  Cost get_g() const
  {
    return g;
  }

  void set_g(Cost new_g)
  {
    g = new_g;
  }

  Cost get_h() const
  {
    return h;
  }

  void set_h(Cost new_h)
  {
    h = new_h;
  }

  const State & get_state() const
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
  bool operator <(const Node<State, Cost> &other) const
  {
    Cost my_f = get_f();
    Cost other_f = other.get_f();

    return my_f < other_f || (my_f == other_f && get_g() > other.get_g());
  }

  bool operator >=(const Node<State, Cost> &other) const
  {
    return !(*this < other);
  }

  /**
   * Equality is determined by the enclosed states.
   *
   * This is kind of horrible.
   */
  bool operator ==(const Node<State, Cost> &other) const
  {
    return state == other.state;
  }

private:
  Cost g;
  Cost h;
  const State state;
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
std::size_t hash_value(Node<State, Cost> const &node)
{
  return hash_value(node.get_state());
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

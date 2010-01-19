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


private:
  const Node<State, Cost> *parent;
  State state;
  Cost g;
  Cost h;
#ifdef CACHE_NODE_F_VALUE
  Cost f;
#endif

public:
  Node(const State &s,
       Cost g,
       Cost h,
       const Node<State, Cost> *p = NULL)
    : parent(p)
    , state(s)
    , g(g)
    , h(h)
#ifdef CACHE_NODE_F_VALUE
    , f(g + h)
#endif
  {
  }

  Cost get_f() const
  {
#ifdef CACHE_NODE_F_VALUE
    return f;
#else
    return g + h;
#endif
  }

  Cost get_g() const
  {
    return g;
  }

  void set_g(Cost new_g)
  {
    g = new_g;
#ifdef CACHE_NODE_F_VALUE
    f = g + h;
#endif
  }

  Cost get_h() const
  {
    return h;
  }

  void set_h(Cost new_h)
  {
    h = new_h;
#ifdef CACHE_NODE_F_VALUE
    f = g + h;
#endif
  }

  const State & get_state() const
  {
    return state;
  }

  const Node<State, Cost> * get_parent() const
  {
    return parent;
  }

  unsigned num_nodes_to_start() const
  {
    unsigned num_nodes = 1;
    const Node<State, Cost> *parent_ptr = get_parent();
    while (parent_ptr != NULL) {
      num_nodes += 1;
      parent_ptr = parent_ptr->get_parent();
    }
    return num_nodes;
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
  /* This will print the solution path.
  if (n.get_parent())
    o << *n.get_parent();
  */

  o << n.get_state() << std::endl
    << "f: " << n.get_f() << std::endl
    << "g: " << n.get_g() << std::endl
    << "h: " << n.get_h() << std::endl;
  return o;
}


#endif /* !_NODE_HPP_ */

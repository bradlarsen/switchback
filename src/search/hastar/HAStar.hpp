#ifndef _HA_STAR_HPP_
#define _HA_STAR_HPP_


#include <vector>

#include <boost/array.hpp>
#include <boost/none.hpp>
#include <boost/optional.hpp>
#include <boost/pool/pool_alloc.hpp>
#include <boost/unordered_map.hpp>
#include <boost/utility.hpp>

#include "search/BucketPriorityQueue.hpp"
#include "util/PointerOps.hpp"


template <
  class Domain,
  class Node
  >
class HAStar : boost::noncopyable
{
private:
  typedef typename Node::Cost Cost;
  typedef typename Node::State State;

  typedef BucketPriorityQueue<Node> Open;
  typedef boost::optional<typename Open::ItemPointer> MaybeItemPointer;

  typedef boost::unordered_map<
    Node *,
    MaybeItemPointer,
    PointerHash<Node>,
    PointerEq<Node>
    , boost::fast_pool_allocator< std::pair<Node * const, MaybeItemPointer > >
    > Closed;

  typedef typename Closed::iterator ClosedIterator;
  typedef typename Closed::const_iterator ClosedConstIterator;


private:
  const Node * goal;
  bool searched;

  const Domain &domain;

  const static unsigned hierarchy_height = Domain::num_abstraction_levels + 1;

  boost::array<unsigned, hierarchy_height> num_expanded;
  boost::array<unsigned, hierarchy_height> num_generated;

  boost::array<Open, hierarchy_height> open;
  boost::array<Closed, hierarchy_height> closed;


public:
  HAStar(const Domain &domain)
    : goal(NULL)
    , searched(false)
    , domain(domain)
  {
    num_expanded.assign(0);
    num_generated.assign(0);
  }

  ~HAStar()
  {
  }

  void search()
  {
    if (searched)
      return;
    searched = true;

    init_open_and_closed();
    search_at_level(0);
  }


  const Node * get_goal() const
  {
#ifndef NDEBUG
    std::cerr << open[level].size()
              << " nodes in open at end of search" << std::endl
              << closed[level].size()
              << " nodes in closed at end of search" << std::endl;
#endif
    return goal;
  }

  const Domain & get_domain() const
  {
    return domain;
  }

  unsigned get_num_generated() const
  {
    return num_generated;
  }

  unsigned get_num_expanded() const
  {
    return num_expanded;
  }


private:
  void init_open_and_closed()
  {
    Node *start_node = domain.create_node(domain.get_start_state(),
                                          0,
                                          0,
                                          NULL);
    closed[0][start_node] = open[0].push(start_node);
    assert(closed[0].find(start_node) != closed[0].end());
    assert(open[0].size() == 1);
    assert(closed[0].size() == 1);
  }


  void search_at_level(const unsigned level)
  {
    std::vector<Node *> succs;

    while (!open[level].empty())
    {
      Node *n = open[level].top();
      open[level].pop();
      assert(closed[level].find(n) != closed[level].end());
      closed[level][n] = boost::none;
      assert(all_closed_item_ptrs_valid(level));

      if (domain.is_goal(n->get_state())) {
        goal = n;
        return;
      }

      domain.compute_successors(*n, succs);
      num_expanded[level] += 1;
      num_generated[level] += succs.size();

      for (unsigned succ_i = 0; succ_i < succs.size(); succ_i += 1)
      {
        process_child(n, succs[succ_i]);
      } /* end for */
    } /* end while */
  }


  void process_child(const unsigned level, const Node *parent, Node *child)
  {
    domain.compute_heuristic(*parent, *child);
    assert(open[level].size() <= closed[level].size());
    assert(all_closed_item_ptrs_valid(level));

    ClosedIterator closed_it = closed[level].find(child);
    if (closed_it == closed[level].end()) {
      // The child has not been generated before.
      closed[level][child] = open[level].push(child);
    }
    else if (closed_it->second && child->get_f() < closed_it->first->get_f()) {
      // A worse version of the child is in the open list.
      open[level].erase(*closed_it->second);  // knock out the old one
                                              // from the open list

      domain.free_node(closed_it->first);     // free the old, worse
                                              // copy
      closed[level].erase(closed_it);

      closed[level][child] = open[level].push(child);  // insert
                                                       // better
                                                       // version of
                                                       // child
    }
    else {
      // The child has either already been expanded, or is worse
      // than the version in the open list.
      domain.free_node(child);
    }

    assert(all_closed_item_ptrs_valid(level));
    assert(open[level].invariants_satisfied());
  }


  bool all_closed_item_ptrs_valid(const unsigned level) const
  {
#ifdef CHECK_ALL_CLOSED_ITEM_PTRS_VALID
    std::cerr << "checking if item pointers are valid" << std::endl
              << "  " << closed.size() << " pointers to check" << std::endl;
    for (ClosedConstIterator closed_it = closed[level].begin();
         closed_it != closed[level].end();
         ++closed_it) {
      if ( closed_it->second && !open[level].valid_item_pointer(*closed_it->second) )
        return false;
    }
    return true;
#else
    return true;
#endif
  }
};


#endif /* !_HA_STAR_HPP_ */

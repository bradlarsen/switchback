#ifndef BOUNDED_SEARCH_RESULT_HPP
#define BOUNDED_SEARCH_RESULT_HPP

#include <cassert>

/*!
\brief A discriminated union:  Cutoff Cost + Goal (Node *) + Failure

\tparam Cost   The type of the cost of a move
\tparam Node   The type of the search node
*/
template <typename Cost, typename Node>
class BoundedSearchResult
{
private:
    union Result
    {
        Cost cutoff;
        Node *goal;
    } result;

    typedef enum {Cutoff, Goal, Failure} ResultType;

    ResultType result_type;

public:
    BoundedSearchResult(Cost cutoff)
    {
        result.cutoff = cutoff;
        result_type = Cutoff;
    }

    BoundedSearchResult(Node *goal)
    {
        assert(goal != NULL);
        result.goal = goal;
        result_type = Goal;
    }

    BoundedSearchResult()
    {
        result_type = Failure;
    }


    bool is_failure() const
    {
        return result_type == Failure;
    }

    bool is_goal() const
    {
        return result_type == Goal;
    }

    bool is_cutoff() const
    {
        return result_type == Cutoff;
    }

    Node * get_goal() const
    {
        assert(is_goal());
        return result.goal;
    }

    Cost get_cutoff() const
    {
        assert(is_cutoff());
        return result.cutoff;
    }
};

#endif /* BOUNDED_SEARCH_RESULT_HPP */

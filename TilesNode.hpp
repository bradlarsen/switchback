#ifndef _TILES_NODE_HPP_
#define _TILES_NODE_HPP_


#include "Node.hpp"
#include "TilesTypes.hpp"
#include "TilesState.hpp"


template class Node<TilesState15, Cost>;
typedef Node<TilesState15, Cost> TilesNode15;


#endif /* !_TILES_NODE_HPP_ */

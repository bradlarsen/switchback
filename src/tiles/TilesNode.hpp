#ifndef _TILES_NODE_HPP_
#define _TILES_NODE_HPP_


#include "search/Node.hpp"
#include "tiles/TilesTypes.hpp"
#include "tiles/TilesState.hpp"


template class Node<TilesState15, Cost>;
typedef Node<TilesState15, Cost> TilesNode15;


#endif /* !_TILES_NODE_HPP_ */

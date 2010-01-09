#include "TilesNode.hpp"


std::ostream & operator <<(std::ostream &o, const TilesNode15 &n)
{
  o << n.get_state() << std::endl
    << "f: " << static_cast<unsigned>(n.get_f()) << std::endl
    << "g: " << static_cast<unsigned>(n.get_g()) << std::endl
    << "h: " << static_cast<unsigned>(n.get_h()) << std::endl;
  return o;
}

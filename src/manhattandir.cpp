#include "manhattandir.h"

u8 ManhattanDir::FromPoint(Point<int>* offset)
{
  int xdir = offset->GetX() > 0 ? -1 : 1;
  int ydir = offset->GetY() > 0 ? -1 : 1;

  int offX = offset->GetX();
  int offY = offset->GetY();

  if(offset->GetManhattanDistance() > 4)
  {
    /*TODO there is probably a better way to do this*/
    throw "Manhattan direction too large.";
  }

  u8 m_bitRep = 0;

  while(offX != 0)
  {
    m_bitRep <<= 2;

    m_bitRep |= (xdir < 0) ? 0 : 1;

    offX += xdir;
  }
  
  while(offY != 0)
  {
    m_bitRep <<= 2;

    m_bitRep |= (ydir < 0) ? 2 : 3;

    offY += ydir;
  }

  return m_bitRep;
}

#include "tile.h"

Tile::Tile(Point<int>* location)
{
  m_location.SetX(location->GetX());
  m_location.SetY(location->GetY());
}

Tile::Tile(int x, int y)
{
  m_location.SetX(x);
  m_location.SetY(y);
}

Tile::~Tile()
{

}

Atom* Tile::GetAtom(Point<int>* pt)
{
  return m_atoms[pt->GetX() + pt->GetY() * TILE_WIDTH];
}

Atom* Tile::GetAtom(int x, int y)
{
  return m_atoms[x + y * TILE_WIDTH];
}

Atom* Tile::GetAtom(int i)
{
  return m_atoms[i];
}

EventWindow* Tile::CreateWindow(Point<int>* ctr)
{
  return new EventWindow(ctr, m_atoms);
}

EventWindow* Tile::CreateRandomWindow()
{
  
}

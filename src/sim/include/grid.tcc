#include "eucliddir.h" /* -*- C++ -*- */
#include "grid.h"

template <class T,u32 R>
Grid<T,R>::Grid(int width, int height, ElementTable<T,R>* elementTable)
{
  m_width = width;
  m_height = height;

  m_tiles = new Tile<T,R>[m_width * m_height];

  u8 neighbors;
  for(u32 x = 0; x < m_width; x++)
  {
    for(u32 y = 0; y < m_height; y++)
    {
      neighbors = 0;
      if(x > 0)
      {
	neighbors |= 0x8; 
      }
      if(y > 0)
      {
	neighbors |= 0x1;
      }
      if(x < m_width - 1)
      {
	neighbors |= 0x2;
      }
      if(y < m_height - 1)
      {
	neighbors |= 0x4;
      }

      GetTile(x, y).SetNeighbors(neighbors);
    }
  }
 
  m_elementTable = elementTable;
}

template <class T,u32 R>
void Grid<T,R>::SetStateFunc(u32 (*stateFunc)(T* atom))
{
  for(u32 i = 0; i < m_width * m_height; i++)
  {
    m_tiles[i].SetStateFunc(stateFunc);
  }
}

template <class T, u32 R>
Grid<T,R>::~Grid()
{
  delete[] m_tiles;
}

template <class T, u32 R>
u32 Grid<T,R>::GetHeight()
{
  return m_height;
}

template <class T, u32 R>
u32 Grid<T,R>::GetWidth()
{
  return m_width;
}

template <class T, u32 R>
void Grid<T,R>::PlaceAtom(T& atom, Point<int>& loc)
{
  u32 ovlapLen = TILE_WIDTH - R * 2;

  /* Account for overlapping caches  vvvvv */
  u32 x = loc.GetX() / ovlapLen;
  u32 y = loc.GetY() / ovlapLen;

  /* How many tiles is this going to be placed in? */
  bool xOverlap = (u32)(loc.GetX() / TILE_WIDTH) != x;
  bool yOverlap = (u32)(loc.GetY() / TILE_WIDTH) != y;

  Point<int> local(loc.GetX() % ovlapLen,
		   loc.GetY() % ovlapLen);

  if(xOverlap)
  {
    if(x - 1 >= 0)
    {
      Point<int> remotePt(local.GetX() + ovlapLen,
			  local.GetY());

      GetTile(x - 1, y).PlaceAtom(atom, remotePt);
    }
  }
  if(yOverlap)
  {
    if(y - 1 >= 0)
    {
      Point<int> remotePt(local.GetX(),
			  local.GetY() + ovlapLen);

      GetTile(x, y - 1).PlaceAtom(atom, remotePt);
    }
  }

  if(xOverlap && yOverlap)
  {
    if(y - 1 >= 0 && y - 1 >= 0)
    {
      Point<int> remotePt(local.GetX() + ovlapLen,
			  local.GetY() + ovlapLen);

      GetTile(x - 1, y - 1).PlaceAtom(atom, remotePt);
    }
  }

  GetTile(x,y).PlaceAtom(atom, local);
}

template <class T, u32 R>
T* Grid<T,R>::GetAtom(Point<int>& loc)
{
  /* Account for overlapping caches  vvvvv */
  u32 x = loc.GetX() / (TILE_WIDTH - R * 2);
  u32 y = loc.GetY() / (TILE_WIDTH - R * 2);

  Point<int> local(loc.GetX() % TILE_WIDTH,
		   loc.GetY() % TILE_WIDTH);

  return GetTile(x,y).GetAtom(&local);
}

template <class T, u32 R>
void Grid<T,R>::Expand(int extraW, int extraH)
{
  Resize(m_width + extraW, m_height + extraH);
}

template <class T, u32 R>
void Grid<T,R>::Resize(int newWidth, int newHeight)
{
  delete m_tiles;

  m_width = newWidth;
  m_height = newHeight;

  m_tiles = new Tile<T,R>[m_width * m_height];
}

template <class T, u32 R>
void Grid<T,R>::TriggerEvent()
{
  /*Change to 0 if aiming a window at a certian tile.*/
#if 1
  Point<int> windowTile(true, m_width, m_height);
#else
  Point<int> windowTile(0, 1);
#endif

  Tile<T,R>& execTile = m_tiles[windowTile.GetX() + 
				windowTile.GetY() * m_width];

  execTile.Execute(*m_elementTable);

  Packet<T>* readPack;
  Point<int> nOffset;
  while((readPack = execTile.NextPacket()))
  {
    EuDir::FillEuclidDir(nOffset, readPack->GetReceivingNeighbor());
    nOffset.Add(windowTile);
    if(nOffset.GetX() >= 0 && nOffset.GetY() >= 0 &&
       nOffset.GetX() < (s32)m_width && nOffset.GetY() < (s32)m_height)
    {
      /* Tile's there! Give it the packet. */
      Tile<T,R>& writeTile = GetTile((u32)nOffset.GetX(), (u32)nOffset.GetY());

      writeTile.ReceivePacket(*readPack);
    }
  }

  m_lastEventTile.Set(windowTile.GetX(), windowTile.GetY());
}

template <class T, u32 R>
void Grid<T,R>::FillLastEventTile(Point<int>& out)
{
  out.Set(m_lastEventTile.GetX(),
	  m_lastEventTile.GetY());
}

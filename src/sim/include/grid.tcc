#include "eucliddir.h" /* -*- C++ -*- */
#include "grid.h"

namespace MFM {

template <class T,u32 R>
Grid<T,R>::Grid(int width, int height)
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
	neighbors |= 0x8;  // XXX TODO: Should be (1<<EUDIR_WEST) or something, etc
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

      // XXX Here, shouldn't there also be like:
      // if (neighbors&NORTH_BIT && neighbors&WEST_BIT) neighbors |= NORTH_WEST_BIT;
      // etc?  What are these neighbors used for?
      GetTile(x, y).SetNeighbors(neighbors);
    }
  }
}

template <class T,u32 R>
void Grid<T,R>::SetSeed(u32 seed) 
{
  m_random.SetSeed(seed);
  for(u32 i = 0; i < m_width * m_height; i++)
  {
    m_tiles[i].GetRandom().SetSeed(m_random.Create());
  }
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
bool Grid<T,R>::IsLegalTileIndex(const SPoint & tileInGrid) const
{
  if (tileInGrid.GetX() < 0 || tileInGrid.GetY() < 0)
    return false;
  if (tileInGrid.GetX() >= (s32) m_width || tileInGrid.GetY() >= (s32) m_height)
    return false;
  return true;
}

template <class T, u32 R>
bool Grid<T,R>::MapGridToTile(const SPoint & siteInGrid, SPoint & tileInGrid, SPoint & siteInTile) const
{
  if (siteInGrid.GetX() < 0 || siteInGrid.GetY() < 0)
    return false;

  SPoint t = siteInGrid/Tile<T,R>::OWNED_SIDE;

  if (t.GetX() >= (s32) m_width || t.GetY() >= (s32) m_height)
    return false;

  // Set up return values
  tileInGrid = t;
  siteInTile = 
    siteInGrid % Tile<T,R>::OWNED_SIDE  // get index into just 'owned' sites
    + SPoint(R,R);                      // and adjust to full Tile indexing
  return true;
}

template <class T, u32 R>
void Grid<T,R>::PlaceAtom(T& atom, const SPoint& siteInGrid)
{
  SPoint tileInGrid, siteInTile;
  if (!MapGridToTile(siteInGrid, tileInGrid, siteInTile)) 
    FAIL(ILLEGAL_ARGUMENT);  // XXX Change to return bool?

  Tile<T,R> & owner = GetTile(tileInGrid);
  owner.PlaceAtom(atom, siteInTile);

  EuclidDir startDir = owner.CacheAt(siteInTile); 

  if ((s32) startDir < 0)       // Doesn't hit cache, we're done
    return;

  EuclidDir stopDir = EuDir::CWDir(startDir);

  if (EuDir::IsCorner(startDir)) {
    startDir = EuDir::CCWDir(startDir);
    stopDir = EuDir::CWDir(stopDir);
  }

  for (EuclidDir dir = startDir; dir != stopDir; dir = EuDir::CWDir(dir)) {
    SPoint tileOffset;
    EuDir::FillEuclidDir(tileOffset,dir);

    SPoint otherTileIndex = tileInGrid+tileOffset;
    
    if (!IsLegalTileIndex(otherTileIndex)) continue;  // edge of grid

    Tile<T,R> & other = GetTile(otherTileIndex);
    SPoint otherIndex = siteInTile + tileOffset * Tile<T,R>::OWNED_SIDE;
    
    other.PlaceAtom(atom,otherIndex);
    FAIL(INCOMPLETE_CODE);
  }

}

#if 0
template <class T, u32 R>
void Grid<T,R>::PlaceAtom(T& atom, SPoint& loc)
{
  u32 ovlapLen = TILE_WIDTH - R * 2;

  /* Account for overlapping caches  vvvvv */
  u32 x = loc.GetX() / ovlapLen;
  u32 y = loc.GetY() / ovlapLen;

  /* How many tiles is this going to be placed in? */
  bool xOverlap = (u32)(loc.GetX() / TILE_WIDTH) != x;
  bool yOverlap = (u32)(loc.GetY() / TILE_WIDTH) != y;

  SPoint local(loc.GetX() % ovlapLen,
               loc.GetY() % ovlapLen);

  if(xOverlap)
  {
    if(x - 1 >= 0)
    {
      SPoint remotePt(local.GetX() + ovlapLen,
                      local.GetY());

      GetTile(x - 1, y).PlaceAtom(atom, remotePt);
    }
  }
  if(yOverlap)
  {
    if(y - 1 >= 0)
    {
      SPoint remotePt(local.GetX(),
                      local.GetY() + ovlapLen);

      GetTile(x, y - 1).PlaceAtom(atom, remotePt);
    }
  }

  if(xOverlap && yOverlap)
  {
    if(y - 1 >= 0 && y - 1 >= 0)
    {
      SPoint remotePt(local.GetX() + ovlapLen,
                      local.GetY() + ovlapLen);

      GetTile(x - 1, y - 1).PlaceAtom(atom, remotePt);
    }
  }

  GetTile(x,y).PlaceAtom(atom, local);
}
#endif

template <class T, u32 R>
T* Grid<T,R>::GetAtom(SPoint& loc)
{
  /* Account for overlapping caches  vvvvv */
  u32 x = loc.GetX() / (TILE_WIDTH - R * 2);
  u32 y = loc.GetY() / (TILE_WIDTH - R * 2);

  SPoint local(loc.GetX() % TILE_WIDTH,
               loc.GetY() % TILE_WIDTH);

  return GetTile(x,y).GetAtom(local);
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
  SPoint windowTile(GetRandom(), m_width, m_height);
#else
  SPoint windowTile(0, 1);
#endif

  Tile<T,R>& execTile = m_tiles[windowTile.GetX() + 
				windowTile.GetY() * m_width];

  execTile.Execute(ElementTable<T,R>::get());

  Packet<T>* readPack;
  SPoint nOffset;
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
void Grid<T,R>::FillLastEventTile(SPoint& out)
{
  out.Set(m_lastEventTile.GetX(),
	  m_lastEventTile.GetY());
}

template <class T, u32 R>
u32 Grid<T,R>::GetAtomCount(ElementType atomType)
{
  u32 total = 0;
  for(u32 i = 0; i < m_width * m_height; i++)
  {
    total += m_tiles[i].GetAtomCount(atomType);
  }

  return total;
}
} /* namespace MFM */


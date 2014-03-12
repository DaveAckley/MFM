#include "eucliddir.h" /* -*- C++ -*- */
#include "grid.h"

namespace MFM {

  template <class T,u32 R,u32 W, u32 H>
  Grid<T,R,W,H>::Grid() : m_width(W), m_height(H)
  {

    /* Set the neighbors flags of each tile. This lets the tiles know */
    /* if any of its caches are dead and should not be written to.    */
    u8 neighbors;
    for(u32 x = 0; x < m_width; x++)
    {
      for(u32 y = 0; y < m_height; y++)
      {
	Tile<T,R>& ctile = GetTile(x, y);
	neighbors = 0;
	if(x > 0)
        {
	  ctile.Connect(GetTile(x - 1, y), EUDIR_WEST);
	  neighbors |= (1<<EUDIR_WEST);
	}
	if(y > 0)
	{
	  ctile.Connect(GetTile(x, y - 1), EUDIR_NORTH);
	  neighbors |= (1<<EUDIR_NORTH);
	}
	if(x < m_width - 1)
        {
	  ctile.Connect(GetTile(x + 1, y), EUDIR_EAST);
	  neighbors |= (1<<EUDIR_EAST);
	}
	if(y < m_height - 1)
        {
	  ctile.Connect(GetTile(x, y + 1), EUDIR_SOUTH);
	  neighbors |= (1<<EUDIR_SOUTH);
	}
	if((neighbors & (1<<EUDIR_SOUTH)) &&
	   (neighbors & (1<<EUDIR_WEST)))
        {
	  ctile.Connect(GetTile(x - 1, y + 1), EUDIR_SOUTHWEST);
	}
	if((neighbors & (1<<EUDIR_NORTH)) &&
	   (neighbors & (1<<EUDIR_WEST)))
        {
	  ctile.Connect(GetTile(x - 1, y - 1), EUDIR_NORTHWEST);
	}
	if((neighbors & (1<<EUDIR_SOUTH)) &&
	   (neighbors & (1<<EUDIR_EAST)))
        {
	  ctile.Connect(GetTile(x + 1, y + 1), EUDIR_SOUTHEAST);
	}
	if((neighbors & (1<<EUDIR_NORTH)) &&
	   (neighbors & (1<<EUDIR_EAST)))
        {
	  ctile.Connect(GetTile(x + 1, y - 1), EUDIR_NORTHEAST);
	}
      }
    }
  }

  template <class T,u32 R,u32 W, u32 H>
  void Grid<T,R,W,H>::SetSeed(u32 seed)
  {
    m_random.SetSeed(seed);
    for(u32 i = 0; i < W; i++)
      for(u32 j = 0; j < H; j++)
        {
          m_tiles[i][j].GetRandom().SetSeed(m_random.Create());
        }
  }

  template <class T, u32 R,u32 W, u32 H>
  Grid<T,R,W,H>::~Grid()
  {
  }

  template <class T, u32 R,u32 W, u32 H>
  u32 Grid<T,R,W, H>::GetHeight()
  {
    return H;
  }

  template <class T, u32 R,u32 W, u32 H>
  u32 Grid<T,R,W,H>::GetWidth()
  {
    return W;
  }

  template <class T, u32 R,u32 W, u32 H>
  bool Grid<T,R,W,H>::IsLegalTileIndex(const SPoint & tileInGrid) const
  {
    if (tileInGrid.GetX() < 0 || tileInGrid.GetY() < 0)
      return false;
    if (tileInGrid.GetX() >= (s32) W || tileInGrid.GetY() >= (s32) H)
      return false;
    return true;
  }

  template <class T, u32 R,u32 W, u32 H>
  bool Grid<T,R,W,H>::MapGridToTile(const SPoint & siteInGrid, SPoint & tileInGrid, SPoint & siteInTile) const
  {
    if (siteInGrid.GetX() < 0 || siteInGrid.GetY() < 0)
      return false;

    SPoint t = siteInGrid/Tile<T,R>::OWNED_SIDE;

    if (!IsLegalTileIndex(t))
      return false;

    // Set up return values
    tileInGrid = t;
    siteInTile =
      siteInGrid % Tile<T,R>::OWNED_SIDE  // get index into just 'owned' sites
      + SPoint(R,R);                      // and adjust to full Tile indexing
    return true;
  }

  template <class T, u32 R,u32 W, u32 H>
  void Grid<T,R,W,H>::PlaceAtom(T& atom, const SPoint& siteInGrid)
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

  template <class T, u32 R,u32 W, u32 H>
  const T* Grid<T,R,W,H>::GetAtom(SPoint& loc)
  {
    SPoint tileInGrid, siteInTile;
    if (!MapGridToTile(loc, tileInGrid, siteInTile))
      FAIL(ILLEGAL_ARGUMENT);  // XXX Change to return bool?

    return GetTile(tileInGrid).GetAtom(siteInTile);
  }

  template <class T, u32 R,u32 W, u32 H>
  void Grid<T,R,W,H>::Pause()
  {
    for(u32 x = 0; x < W; x++)
    {
      for(u32 y = 0; y < H; y++)
      {
	GetTile(x, y).Pause();
      }
    }
  }

  template <class T, u32 R,u32 W, u32 H>
  void Grid<T,R,W,H>::Unpause()
  {
    for(u32 x = 0; x < W; x++)
    {
      for(u32 y = 0; y < H; y++)
      {
	GetTile(x, y).Start();
      }
    }
  }

  template <class T, u32 R,u32 W, u32 H>
  void Grid<T,R,W,H>::TriggerEvent()
  {
    /*Change to 0 if aiming a window at a certian tile.*/
#if 1
    SPoint windowTile(GetRandom(), m_width, m_height);
#else
    SPoint windowTile(0, 1);
#endif

    Tile<T,R>& execTile = m_tiles[windowTile.GetX()][windowTile.GetY()];

    //  execTile.Execute(ElementTable<T,R>::get());
    execTile.Execute();

    m_lastEventTile.Set(windowTile.GetX(), windowTile.GetY());
  }

  template <class T, u32 R,u32 W, u32 H>
  void Grid<T,R,W,H>::FillLastEventTile(SPoint& out)
  {
    out.Set(m_lastEventTile.GetX(),
            m_lastEventTile.GetY());
  }

  template <class T, u32 R, u32 W, u32 H>
  u64 Grid<T,R,W,H>::GetTotalEventsExecuted()
  {
    u64 total = 0;
    for(u32 x = 0; x < W; x++)
    {
      for(u32 y = 0; y < H; y++)
      {
	total += m_tiles[x][y].GetEventsExecuted();
      }
    }
    return total;
  }

  template <class T, u32 R, u32 W, u32 H>
  u64 Grid<T,R,W,H>::WriteEPSRaster(FILE* outstrm)
  {
    u64 max = 0;
    for(u32 y = 0; y < H; y++)
    {
      for(u32 i = 0; i < TILE_WIDTH - R * 2; i++)
      {
	for(u32 x = 0; x < W; x++)
	{
	  max = MAX(max, GetTile(x, y).WriteEPSRasterLine(outstrm, i));
	}
	fputc('\n', outstrm);
      }
    }
    return max;
  }

  template <class T, u32 R,u32 W, u32 H>
  u32 Grid<T,R,W,H>::GetAtomCount(ElementType atomType)
  {
    u32 total = 0;
    for(u32 i = 0; i < W; i++)
      for(u32 j = 0; j < H; j++)
        total += m_tiles[i][j].GetAtomCount(atomType);

    return total;
  }

  template <class T, u32 R,u32 W, u32 H>
  void Grid<T,R,W,H>::Needed(const Element<T,R> & anElement)
  {
    for(u32 i = 0; i < W; i++)
      for(u32 j = 0; j < H; j++)
        m_tiles[i][j].RegisterElement(anElement);
  }
} /* namespace MFM */


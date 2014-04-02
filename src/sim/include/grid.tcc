/* -*- C++ -*- */

#include "Dirs.h"
#include "grid.h"

namespace MFM {

  template <class GC>
  void Grid<GC>::Reinit() {

    /* Reseed grid PRNG and push seeds to the tile PRNGs */
    ReinitSeed();

    /* Reinit all the tiles */

    /* Set the neighbors flags of each tile. This lets the tiles know */
    /* if any of its caches are dead and should not be written to.    */
    u8 neighbors;
    for(u32 x = 0; x < m_width; x++)
    {
      for(u32 y = 0; y < m_height; y++)
      {
	Tile<CC>& ctile = GetTile(x, y);

        ctile.Reinit();

	neighbors = 0;
	if(x > 0)
        {
	  ctile.Connect(GetTile(x - 1, y), Dirs::WEST);
	  neighbors |= (1<<Dirs::WEST);
	}
	if(y > 0)
	{
	  ctile.Connect(GetTile(x, y - 1), Dirs::NORTH);
	  neighbors |= (1<<Dirs::NORTH);
	}
	if(x < m_width - 1)
        {
	  ctile.Connect(GetTile(x + 1, y), Dirs::EAST);
	  neighbors |= (1<<Dirs::EAST);
	}
	if(y < m_height - 1)
        {
	  ctile.Connect(GetTile(x, y + 1), Dirs::SOUTH);
	  neighbors |= (1<<Dirs::SOUTH);
	}
	if((neighbors & (1<<Dirs::SOUTH)) &&
	   (neighbors & (1<<Dirs::WEST)))
        {
	  ctile.Connect(GetTile(x - 1, y + 1), Dirs::SOUTHWEST);
	}
	if((neighbors & (1<<Dirs::NORTH)) &&
	   (neighbors & (1<<Dirs::WEST)))
        {
	  ctile.Connect(GetTile(x - 1, y - 1), Dirs::NORTHWEST);
	}
	if((neighbors & (1<<Dirs::SOUTH)) &&
	   (neighbors & (1<<Dirs::EAST)))
        {
	  ctile.Connect(GetTile(x + 1, y + 1), Dirs::SOUTHEAST);
	}
	if((neighbors & (1<<Dirs::NORTH)) &&
	   (neighbors & (1<<Dirs::EAST)))
        {
	  ctile.Connect(GetTile(x + 1, y - 1), Dirs::NORTHEAST);
	}
      }
    }
  }

  template <class GC>
  void Grid<GC>::SetSeed(u32 seed)
  {
    m_seed = seed;
  }

  template <class GC>
  void Grid<GC>::ReinitSeed()
  {
    if (m_seed==0)  // SetSeed must have been called by now!
      FAIL(ILLEGAL_STATE);

    m_random.SetSeed(m_seed);
    for(u32 i = 0; i < W; i++)
      for(u32 j = 0; j < H; j++)
        {
          m_tiles[i][j].GetRandom().SetSeed(m_random.Create());
        }
  }

  template <class GC>
  void Grid<GC>::SetTileToExecuteOnly(const SPoint& tileLoc, bool value)
  {
    GetTile(tileLoc).SetExecuteOwnEvents(value);
  }

  template <class GC>
  bool Grid<GC>::GetTileExecutionStatus(const SPoint& tileLoc)
  {
    return GetTile(tileLoc).GetExecutingOwnEvents();
  }

  template <class GC>
  bool Grid<GC>::IsLegalTileIndex(const SPoint & tileInGrid) const
  {
    if (tileInGrid.GetX() < 0 || tileInGrid.GetY() < 0)
      return false;
    if (tileInGrid.GetX() >= (s32) W || tileInGrid.GetY() >= (s32) H)
      return false;
    return true;
  }

  template <class GC>
  bool Grid<GC>::MapGridToTile(const SPoint & siteInGrid, SPoint & tileInGrid, SPoint & siteInTile) const
  {
    SPoint myTile, mySite;
    if (!MapGridToUncachedTile(siteInGrid, myTile, mySite)) return false;
    tileInGrid = myTile;
    siteInTile = mySite+SPoint(R,R);      // adjust to full Tile indexing
    return true;
  }

  template <class GC>
  bool Grid<GC>::MapGridToUncachedTile(const SPoint & siteInGrid, SPoint & tileInGrid, SPoint & siteInTile) const
  {
    if (siteInGrid.GetX() < 0 || siteInGrid.GetY() < 0)
      return false;

    SPoint t = siteInGrid/Tile<CC>::OWNED_SIDE;

    if (!IsLegalTileIndex(t))
      return false;

    // Set up return values
    tileInGrid = t;
    siteInTile =
      siteInGrid % Tile<CC>::OWNED_SIDE;  // get index into just 'owned' sites
    return true;
  }

  template <class GC>
  void Grid<GC>::PlaceAtom(T& atom, const SPoint& siteInGrid)
  {
    SPoint tileInGrid, siteInTile;
    if (!MapGridToTile(siteInGrid, tileInGrid, siteInTile))
      FAIL(ILLEGAL_ARGUMENT);  // XXX Change to return bool?

    Tile<CC> & owner = GetTile(tileInGrid);
    owner.PlaceAtom(atom, siteInTile);

    Dir startDir = owner.CacheAt(siteInTile);

    if ((s32) startDir < 0)       // Doesn't hit cache, we're done
      return;

    Dir stopDir = Dirs::CWDir(startDir);

    if (Dirs::IsCorner(startDir)) {
      startDir = Dirs::CCWDir(startDir);
      stopDir = Dirs::CWDir(stopDir);
    }

    for (Dir dir = startDir; dir != stopDir; dir = Dirs::CWDir(dir)) {
      SPoint tileOffset;
      Dirs::FillDir(tileOffset,dir);

      SPoint otherTileIndex = tileInGrid+tileOffset;

      if (!IsLegalTileIndex(otherTileIndex)) continue;  // edge of grid

      Tile<CC> & other = GetTile(otherTileIndex);
      SPoint otherIndex = siteInTile + tileOffset * Tile<CC>::OWNED_SIDE;

      other.PlaceAtom(atom,otherIndex);
      FAIL(INCOMPLETE_CODE);
    }

  }

  template <class GC>
  void Grid<GC>::Pause()
  {
    for(u32 x = 0; x < W; x++)
    {
      for(u32 y = 0; y < H; y++)
      {
	GetTile(x, y).Pause();
      }
    }
  }

  template <class GC>
  void Grid<GC>::Unpause()
  {
    for(u32 x = 0; x < W; x++)
    {
      for(u32 y = 0; y < H; y++)
      {
	GetTile(x, y).Start();
      }
    }
  }

  template <class GC>
  void Grid<GC>::FillLastEventTile(SPoint& out)
  {
    out.Set(m_lastEventTile.GetX(),
            m_lastEventTile.GetY());
  }

  template <class GC>
  u64 Grid<GC>::GetTotalEventsExecuted() const
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

  template <class GC>
  void Grid<GC>::WriteEPSImage(FILE* outstrm) const
  {
    u64 max = 0;
    const u32 swidth = GetWidthSites();
    const u32 sheight = GetHeightSites();

    for(u32 pass = 0; pass < 2; ++pass) {
      if (pass==1)
        fprintf(outstrm,"P5\n # Max site events = %ld\n%d %d 255\n",max,swidth,sheight);
      for(u32 y = 0; y < sheight; y++) {
	for(u32 x = 0; x < swidth; x++) {
          SPoint siteInGrid(x,y), tileInGrid, siteInTile;
          if (!MapGridToUncachedTile(siteInGrid, tileInGrid, siteInTile))
            FAIL(ILLEGAL_STATE);
          u64 events = GetTile(tileInGrid).GetUncachedSiteEvents(siteInTile);
          if (pass==0)
            max = MAX(max, events);
          else
            fputc((u8) (events*255/max), outstrm);
        }
      }
    }
  }

  template <class GC>
  u32 Grid<GC>::GetAtomCount(ElementType atomType) const
  {
    u32 total = 0;
    for(u32 i = 0; i < W; i++)
      for(u32 j = 0; j < H; j++)
        total += m_tiles[i][j].GetAtomCount(atomType);

    return total;
  }

  template <class GC>
  void Grid<GC>::SurroundRectangleWithWall(s32 sx, s32 sy, s32 w, s32 h, s32 thickness)
  {
    if(thickness > 0)
    {
      SPoint aloc;
      T atom(Element_Wall<CC>::THE_INSTANCE.GetDefaultAtom());
      /* Draw out one rectangle */
      for(s32 x = sx; x <= sx + w; x++)
      {
	aloc.SetX(x);
	aloc.SetY(sy);

	PlaceAtom(atom, aloc);

	aloc.SetY(sy + h);
	PlaceAtom(atom, aloc);
      }

      for(s32 y = sy; y <= sy + h; y++)
      {
	aloc.SetY(y);
	aloc.SetX(sx);

	PlaceAtom(atom, aloc);

	aloc.SetX(sx + w);
	PlaceAtom(atom, aloc);
      }
      
      /* Recursively create a larger one around us */
      SurroundRectangleWithWall(sx - 1, sy - 1, w + 2, h + 2, thickness - 1);
    }
  }

  template <class GC>
  u32 Grid<GC>::CountActiveSites()
  {
    u32 acc   = 0,
        sides = GetTile(0,0).GetSites();
    
    for(u32 x = 0; x < W; x++)
    {
      for(u32 y = 0; y < H; y++)
      {
	acc += GetTile(x,y).GetExecutingOwnEvents() ? sides : 0;
      }
    }
    return acc;
  }
    
} /* namespace MFM */


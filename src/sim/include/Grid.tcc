/* -*- C++ -*- */
#include "Dirs.h"
#include "Grid.h"
#include "Utils.h"   /* For Sleep */
#include "FileByteSink.h"

#define XRAY_BIT_ODDS 100

namespace MFM {

  template <class GC>
  void Grid<GC>::Reinit() {

    /* Reseed grid PRNG and push seeds to the tile PRNGs */
    ReinitSeed();

    m_backgroundRadiationEnabled = false;

    /* Reinit all the tiles */

    /* Set the neighbors flags of each tile. This lets the tiles know */
    /* if any of its caches are dead and should not be written to.    */
    u8 neighbors;
    for(u32 x = 0; x < m_width; x++)
    {
      for(u32 y = 0; y < m_height; y++)
      {
        Tile<CC>& ctile = GetTile(x, y);

        OString16 & tbs = ctile.GetLabelPrinter();
        tbs.Reset();
        tbs.Printf("[%d,%d]", x, y);

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
    {
      FAIL(ILLEGAL_STATE);
    }

    m_random.SetSeed(m_seed);
    for(u32 i = 0; i < W; i++)
    {
      for(u32 j = 0; j < H; j++)
        {
          m_tiles[i][j].GetRandom().SetSeed(m_random.Create());
        }
    }
  }

  template <class GC>
  void Grid<GC>::SetTileToExecuteOnly(const SPoint& tileLoc, bool value)
  {
    if(tileLoc.GetX() >= 0 && tileLoc.GetY() >= 0 &&
       tileLoc.GetX() < W  && tileLoc.GetY() < H)
    {
      GetTile(tileLoc).SetExecuteOwnEvents(value);
    }
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
  void Grid<GC>::RecountAtoms()
  {
    for(u32 i = 0; i < W; i++)
      for(u32 j = 0; j < H; j++)
        m_tiles[i][j].RecountAtoms();
  }

  template <class GC>
  void Grid<GC>::PlaceAtom(const T& atom, const SPoint& siteInGrid)
  {
    SPoint tileInGrid, siteInTile;
    if (!MapGridToTile(siteInGrid, tileInGrid, siteInTile))
    {
      printf("Can't place at (%d,%d)\n", siteInGrid.GetX(), siteInGrid.GetY());
      FAIL(ILLEGAL_ARGUMENT);  // XXX Change to return bool?
    }

    Tile<CC> & owner = GetTile(tileInGrid);
    owner.PlaceAtom(atom, siteInTile);

    Dir startDir = owner.SharedAt(siteInTile);

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

      // siteInTile is in tileInGrid's shared region, indexed with
      // including-cache coords.  Offsetting by the owned size
      // (excluding caches) maps into including-cache coords on their
      // side.  Hmm.

      SPoint otherIndex = siteInTile - tileOffset * Tile<CC>::OWNED_SIDE;

      other.PlaceAtom(atom,otherIndex);
    }
  }

  template <class GC>
  void Grid<GC>::MaybeXRayAtom(const SPoint& siteInGrid)
  {
    if (m_random.OneIn(10))
    {
      XRayAtom(siteInGrid);
    }
  }

  template <class GC>
  void Grid<GC>::XRayAtom(const SPoint& siteInGrid)
  {
    SPoint tileInGrid, siteInTile;
    if (!MapGridToTile(siteInGrid, tileInGrid, siteInTile))
    {
      printf("Can't xray at (%d,%d)\n", siteInGrid.GetX(), siteInGrid.GetY());
      FAIL(ILLEGAL_ARGUMENT);  // XXX Change to return bool?
    }

    Tile<CC> & owner = GetTile(tileInGrid);
    owner.SingleXRay(siteInTile.GetX(), siteInTile.GetY());

    /* This doesn't focus on xraying across caches, which I suppose is
     * the correct behavior. */
  }

  template <class GC>
  void Grid<GC>::ReportGridStatus(Logger::Level level)
  {
    LOG.Log(level,"===GRID STATUS REPORT===");
    LOG.Log(level," Object location + size: %p + %d", (void*) this, sizeof(*this));
    LOG.Log(level," Size: (%d, %d)", m_width, m_height);
    LOG.Log(level," Generation: %d", m_gridGeneration);
    LOG.Log(level," Last event tile: (%d, %d)", m_lastEventTile.GetX(), m_lastEventTile.GetY());
    LOG.Log(level," Background radiation: %s", m_backgroundRadiationEnabled?"true":"false");
    LOG.Log(level," Xray odds: %d", m_xraySiteOdds);

    for(u32 x = 0; x < W; x++)
    {
      for(u32 y = 0; y < H; y++)
      {
        Tile<CC> & tile = GetTile(x,y);
        LOG.Log(level,"--Grid(%d,%d)=Tile %s (%p)--",
                x,  y, tile.GetLabel(), (void *) &tile);
        tile.ReportTileStatus(level);
      }
    }
  }

  template <class GC>
  void Grid<GC>::DoTileControl(TileControl & tc)
  {
    // Issue request to all
    for(u32 x = 0; x < W; x++)
    {
      for(u32 y = 0; y < H; y++)
      {
        tc.MakeRequest(GetTile(x, y));
      }
    }

    // Wait until all acknowledge
    u32 loops = 0;
    u32 notReady = 0;
    s32 sleepTimer = 0;
    do
    {
      if (++loops >= 25000)
      {
        if(m_ignoreThreadingProblems)
        {
          LOG.Error("%s:%d: THREADING PROBLEM ENCOUNTERED! Grid is configured to ignore"
                    " this problem and will continue execution.", __FILE__, __LINE__);
          return;
        }
        LOG.Error("%s control looped %d times, but %d still not ready, killing",
                  tc.GetName(), loops, notReady);
        ReportGridStatus(Logger::ERROR);
        FAIL(ILLEGAL_STATE);
      }

      if (--sleepTimer < 0)
      {
        Sleep(0, loops);  // Actually sleeping seems to avoid rare livelock?
        sleepTimer = m_random.Create(1000);
      }
      else
      {
        pthread_yield();
      }

      notReady = 0;

      for(u32 x = 0; x < W; x++)
      {
        for(u32 y = 0; y < H; y++)
        {
          if (!tc.CheckIfReady(GetTile(x, y)))
          {
            ++notReady;
            pthread_yield();
          }
        }
      }

    } while (notReady > 0);

    if (loops > 1000)
    {
      LOG.Debug("%s control looped %d times",
                tc.GetName(), loops);
    }

    // Release the hounds
    for(u32 x = 0; x < W; x++)
    {
      for(u32 y = 0; y < H; y++)
      {
        tc.Execute(GetTile(x, y));
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
  void Grid<GC>::WriteEPSImage(ByteSink & outstrm) const
  {
    u64 max = 0;
    const u32 swidth = GetWidthSites();
    const u32 sheight = GetHeightSites();

    for(u32 pass = 0; pass < 2; ++pass) {
      if (pass==1)
        outstrm.Printf("P5\n # Max site events = %d\n%d %d 255\n",(u32) max,swidth,sheight);
      for(u32 y = 0; y < sheight; y++) {
	for(u32 x = 0; x < swidth; x++) {
          SPoint siteInGrid(x,y), tileInGrid, siteInTile;
          if (!MapGridToUncachedTile(siteInGrid, tileInGrid, siteInTile))
            FAIL(ILLEGAL_STATE);
          u64 events = GetTile(tileInGrid).GetUncachedSiteEvents(siteInTile);
          if (pass==0)
            max = MAX(max, events);
          else
            outstrm.WriteByte((u8) (events*255/max));
        }
      }
    }
  }

  template <class GC>
  void Grid<GC>::WriteEPSAverageImage(ByteSink & outstrm) const
  {
    u64 max = 0;
    const u32 swidth = Tile<CC>::OWNED_SIDE;
    const u32 sheight = Tile<CC>::OWNED_SIDE;
    const u32 tileCt = GetHeight() * GetWidth();

    for(u32 pass = 0; pass < 2; pass++)
    {
      if(pass == 1)
      {
	outstrm.Printf("P5\n #Max site events = %d\n%d %d 255\n", (u32) max, swidth, sheight);
      }
      for(u32 y = 0; y < sheight; y++)
      {
	for(u32 x = 0; x < swidth; x++)
	{
	  u64 events = 0;
	  for(u32 tx = 0; tx < GetWidth(); tx++)
	  {
	    for(u32 ty = 0; ty < GetHeight(); ty++)
	    {
	      SPoint siteInGrid(x + swidth * tx, y + sheight * ty), tileInGrid, siteInTile;
	      if (!MapGridToUncachedTile(siteInGrid, tileInGrid, siteInTile))
	      {
		FAIL(ILLEGAL_STATE);
	      }
	      events += GetTile(tileInGrid).GetUncachedSiteEvents(siteInTile);
	    }
	  }
	  events /= tileCt;
	  if (pass==0)
	  {
	    max = MAX(max, events);
	  }
	  else
	  {
	    outstrm.WriteByte((u8) (events*255/max));
	  }
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
  void Grid<GC>::RandomNuke()
  {
    Random& rand = m_random;

    SPoint center(rand.Create(W * CC::PARAM_CONFIG::TILE_WIDTH),
		  rand.Create(H * CC::PARAM_CONFIG::TILE_WIDTH));

    u32 radius = rand.Between(5, CC::PARAM_CONFIG::TILE_WIDTH);
    T atom(Element_Empty<CC>::THE_INSTANCE.GetDefaultAtom());

    SPoint siteInGrid, tileInGrid, siteInTile;
    for(s32 x = center.GetX() - radius; x < (s32)(center.GetX() + radius); x++)
    {
      for(s32 y = center.GetY() - radius; y < (s32)(center.GetY() + radius); y++)
      {
	siteInGrid.Set(x, y);
	if(DISTANCE(x, y, center.GetX(), center.GetY()) < radius)
	{
	  if(MapGridToTile(siteInGrid, tileInGrid, siteInTile))
	  {
	    PlaceAtom(atom, siteInGrid);
	  }
	}
      }
    }
  }

  template <class GC>
  void Grid<GC>::Clear()
  {
    ++m_gridGeneration;
    for(u32 x = 0; x < W; x++)
    {
      for(u32 y = 0; y < H; y++)
      {
	EmptyTile(SPoint(x, y));

      }
    }
  }

  template <class GC>
  void Grid<GC>::CheckCaches()
  {
    for(u32 x = 0; x < W; x++)
    {
      for(u32 y = 0; y < H; y++)
      {
        const SPoint usp(x,y);

        for (Dir dir = Dirs::NORTH; dir <= Dirs::NORTHWEST; ++dir)
        {
          SPoint offset;
          Dirs::FillDir(offset, dir);
          const SPoint themp(usp + offset);

          if (IsLegalTileIndex(themp))
          {
            GetTile(usp).CheckCacheFromDir(dir, GetTile(themp));
          }
        }
      }
    }
  }

  template <class GC>
  void Grid<GC>::SetBackgroundRadiation(bool value)
  {
    for(u32 x = 0; x < W; x++)
    {
      for(u32 y = 0; y < H; y++)
      {
	GetTile(x, y).SetBackgroundRadiation(value);
      }
    }
    m_backgroundRadiationEnabled = value;
  }

  template <class GC>
  void Grid<GC>::XRay()
  {
    for(u32 x = 0; x < W; x++)
    {
      for(u32 y = 0; y < H; y++)
      {
	GetTile(x,y).XRay(m_xraySiteOdds,
			  XRAY_BIT_ODDS);
      }
    }
  }

  template <class GC>
  u32 Grid<GC>::CountActiveSites() const
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

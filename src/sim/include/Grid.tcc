/* -*- C++ -*- */
#include "Dirs.h"
#include "Grid.h"
#include "Utils.h"   /* For Sleep */
#include "FileByteSink.h"

#define XRAY_BIT_ODDS 100

namespace MFM {

  template <class GC>
  LonglivedLock & Grid<GC>::GetIntertileLock(u32 x, u32 y, Dir dir)
  {
    switch (dir)
    {
    case Dirs::NORTH:
      MFM_API_ASSERT_ARG(y > 0);
      --y;
      dir = Dirs::SOUTH;
      break;

    case Dirs::NORTHEAST:
      MFM_API_ASSERT_ARG(y > 0);
      --y;
      dir = Dirs::SOUTHEAST;
      break;

    case Dirs::EAST:
    case Dirs::SOUTHEAST:
    case Dirs::SOUTH:
      // Ready to rock
      break;

    case Dirs::SOUTHWEST:
      MFM_API_ASSERT_ARG(x > 0);
      --x;
      dir = Dirs::SOUTHEAST;
      break;

    case Dirs::WEST:
      MFM_API_ASSERT_ARG(x > 0);
      --x;
      dir = Dirs::EAST;
      break;

    case Dirs::NORTHWEST:
      MFM_API_ASSERT_ARG(x > 0 && y > 0);
      --x;
      --y;
      dir = Dirs::SOUTHEAST;
      break;

    default:
      FAIL(ILLEGAL_STATE);
    }
    return m_intertileLocks[x][y][dir - Dirs::EAST];
  }

  template <class GC>
  void Grid<GC>::Init() {

    /* Reseed grid PRNG and push seeds to the tile PRNGs */
    InitSeed();

    m_backgroundRadiationEnabled = false;

    /* Init the tiles */

    for(u32 x = 0; x < m_width; x++)
    {
      for(u32 y = 0; y < m_height; y++)
      {
        SPoint tpt(x,y);
        Tile<CC>& ctile = GetTile(tpt);

        ctile.Init();

        OString16 tbs;
        tbs.Printf("[%d,%d]", x, y);
        ctile.SetLabel(tbs.GetZString());

        if (m_warpFactor >= 0)
        {
          ctile.SetWarpFactor((u32) m_warpFactor);
        }
      }
    }

    // Connect them up
    for(u32 x = 0; x < m_width; x++)
    {
      for(u32 y = 0; y < m_height; y++)
      {
        for (Dir d = Dirs::NORTHEAST; d <= Dirs::SOUTH; ++d)
        {
          SPoint tpt(x,y);
          Tile<CC>& ctile = GetTile(tpt);

          SPoint npt = tpt + Dirs::GetOffset(d);

          if (!IsLegalTileIndex(npt))
          {
            continue;
          }

          TileDriver & td = m_tileDrivers[x][y];
          GridTransceiver & gt = td.m_channels[d - Dirs::NORTHEAST];
          LonglivedLock & ctl = GetIntertileLock(x,y,d);

          Tile<CC>& otile = GetTile(npt);
          Dir odir = Dirs::OppositeDir(d);
          LonglivedLock & otl = GetIntertileLock(npt.GetX(),npt.GetY(),odir);

          ctile.Connect(gt, ctl, d);
          otile.Connect(gt, otl, odir);

          gt.SetEnabled(true);
          gt.SetDataRate(100000000);
          gt.SetMaxInFlight(0);
        }
      }
    }
  }

  template <class GC>
  double Grid<GC>::GetAverageCacheRedundancy() const
  {
    u32 count = 0;
    double sum = 0.0;
    for(u32 x = 0; x < m_width; x++)
    {
      for(u32 y = 0; y < m_height; y++)
      {
        const Tile<CC> & tile = GetTile(x,y);
        double red = tile.GetAverageCacheRedundancy();
        if (red >= 0)
        {
          sum += red;
          count++;
        }
      }
    }
    if (count == 0)
    {
      return -1.0;
    }
    return sum / count;
  }

  template <class GC>
  void Grid<GC>::SetCacheRedundancy(u32 redundancyOddsType)
  {
    for(u32 x = 0; x < m_width; x++)
    {
      for(u32 y = 0; y < m_height; y++)
      {
        Tile<CC> & tile = GetTile(x,y);
        tile.SetCacheRedundancy(redundancyOddsType);
      }
    }
  }

  template <class GC>
  void Grid<GC>::InitThreads()
  {
    if (m_threadsInitted)
    {
      FAIL(ILLEGAL_STATE);
    }

    /* Init the tile thread drivers */
    for(u32 x = 0; x < m_width; x++)
    {
      for(u32 y = 0; y < m_height; y++)
      {
        SPoint tpt(x,y);
        TileDriver & td = m_tileDrivers[x][y];
        td.m_loc = tpt;
        td.m_gridPtr = this;
        td.SetState(TileDriver::PAUSED);
        if (pthread_create(&td.m_threadId, NULL, TileDriverRunner, &td))
        {
          FAIL(ILLEGAL_STATE);
        }
      }
    }

    m_threadsInitted = true;
  }

  template <class GC>
  void Grid<GC>::SetGridRunning(bool running)
  {
    //    /* Notify the transceivers */
    //    m_gtDriver.SetState(running ? GTDriver::ADVANCING : GTDriver::PAUSED);

    /* Notify the Tiles */
    for(u32 x = 0; x < m_width; x++)
    {
      for(u32 y = 0; y < m_height; y++)
      {
        TileDriver & td = m_tileDrivers[x][y];
        td.SetState(running? TileDriver::ADVANCING : TileDriver::PAUSED);
      }
    }
  }

  template <class GC>
  void* Grid<GC>::TileDriverRunner(void * arg)
  {
    TileDriver * td = (TileDriver*) arg;
    Tile<CC> & ctile = td->GetTile();

    // Init error stack pointer (for this thread only)
    MFMPtrToErrEnvStackPtr = ctile.GetErrorEnvironmentStackTop();

    LOG.Debug("TileDriver %p init: (%d,%d) == %s",
              (void*) td,
              td->m_loc.GetX(),
              td->m_loc.GetY(),
              ctile.GetLabel());

    ctile.RequestStatePassive();

    while (true)
    {
      switch (td->GetState())
      {
      case TileDriver::EXIT_REQUEST:
        break;

      case TileDriver::ADVANCING:
      {
        // Drive this tile's transceivers
        timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        for (u32 c = 0; c < 4; ++c)
        {
          td->m_channels[c].AdvanceToTime(now);
        }

        // Drive the tile itself
        if (!ctile.Advance())
        {
          // We accomplished nothing.  Let somebody else try
          pthread_yield();
        }
        break;
      }

      case TileDriver::PAUSED:
        // Sleep a little
        SleepUsec(ctile.GetRandom().Between(10,100));  // 0.01ms..1ms
        break;

      default:
        FAIL(ILLEGAL_STATE);
      }
    }

    return NULL;
  }

  template <class GC>
  void Grid<GC>::SetSeed(u32 seed)
  {
    m_seed = seed;
  }

  template <class GC>
  void Grid<GC>::InitSeed()
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
  void Grid<GC>::SetTileEnabled(const SPoint& tileLoc, bool isEnabled)
  {
    Tile<CC> & tile = GetTile(tileLoc);
    if (isEnabled)
    {
      tile.SetEnabled();
    }
    else
    {
      tile.SetDisabled();
    }

  }

  template <class GC>
  bool Grid<GC>::IsTileEnabled(const SPoint& tileLoc)
  {
    return GetTile(tileLoc).IsEnabled();
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

    FAIL(INCOMPLETE_CODE);
    /*
    Tile<CC> & owner = GetTile(tileInGrid);
    owner.SingleXRay(siteInTile.GetX(), siteInTile.GetY());
    */
    /* This doesn't focus on xraying across caches, which I suppose is
     * the correct behavior. */
  }

  template <class GC>
  void Grid<GC>::ReportGridStatus(Logger::Level level)
  {
    LOG.Log(level,"===GRID STATUS REPORT===");
    LOG.Log(level," Object location + size: %p + %d", (void*) this, sizeof(*this));
    LOG.Log(level," Size: (%d, %d)", m_width, m_height);
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
  void Grid<GC>::DoTileDriverControl(TileDriverControl & tc)
  {
    // Initial grid changes
    tc.PreGridControl(*this);

    // Ensure everybody is ready for the request
    for(u32 x = 0; x < W; x++)
    {
      for(u32 y = 0; y < H; y++)
      {
        TileDriver & td = m_tileDrivers[x][y];
        if (!tc.CheckPrecondition(td))
        {
          LOG.Error("%s control precondition failed at (%d,%d)=Tile %s (%p)--",
                    tc.GetName(), x,  y, td.GetTile().GetLabel(), (void *) &td);
          ReportGridStatus(Logger::ERROR);
          FAIL(ILLEGAL_STATE);
        }
      }
    }

    // Issue request to all
    for(u32 x = 0; x < W; x++)
    {
      for(u32 y = 0; y < H; y++)
      {
        TileDriver & td = m_tileDrivers[x][y];
        tc.MakeRequest(td);
      }
    }

    // Wait until all acknowledge
    u32 loops = 0;
    u32 notReady = 0;
    s32 sleepTimer = 100000000; // XXX m_random.Between(1000,10000);
    do
    {
      if (++loops >= 1000000)
      {
        LOG.Error("%s control looped %d times, but %d still not ready, killing",
                  tc.GetName(), loops, notReady);
        ReportGridStatus(Logger::ERROR);
        LOG.Error("%s control: Sleeping", tc.GetName());
        SleepUsec(60*1000000);  // 1 minute
        LOG.Error("%s control: Resetting", tc.GetName());
        loops = 0;
      }

      if (--sleepTimer < 0)
      {
        SleepUsec(m_random.Between(10,1000));  // 0.01ms..1ms
        sleepTimer = m_random.Create(10000);
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
          TileDriver & td = m_tileDrivers[x][y];
          if (!tc.CheckIfReady(td))
          {
            ++notReady;
            pthread_yield();
          }
        }
      }

    } while (notReady > 0);

    if (loops > 5000)
    {
      LOG.Debug("%s control looped %d times",
                tc.GetName(), loops);
    }

    // Release the hounds
    for(u32 x = 0; x < W; x++)
    {
      for(u32 y = 0; y < H; y++)
      {
        TileDriver & td = m_tileDrivers[x][y];
        tc.Execute(td);
      }
    }

    // Final grid changes
    tc.PostGridControl(*this);
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

#if 0
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
#endif

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
        const Tile<CC> & tile = GetTile(usp);

        FAIL(INCOMPLETE_CODE);
      }
    }
  }

#if 0
  template <class CC>
  void Tile<CC>::CheckCaches()
  {
    // XXX assert(IsPausedOrOwner());

    for(u32 x = 0; x < TILE_WIDTH; x++)
    {
      for(u32 y = 0; y < TILE_WIDTH; y++)
      {
        const SPoint sp(x,y);
        if (!IsInCache(sp)) continue;

        Dir dir = CacheAt(sp);
        if (dir != direction)
        {
          continue;
        }
        if (!IsConnected(dir))
        {
          continue;
        }

        const SPoint rp(GetNeighborLoc(dir, sp));

        T otherAtom = *otherTile.GetAtom(rp);
        if (m_atoms[x][y] != otherAtom)
        {
          AtomSerializer<CC> uss(m_atoms[x][y]), thems(otherAtom);
          LOG.Debug("%s: Mismatch at (%d,%d) dir %d, us: %@, them: %@",
                    this->GetLabel(), x, y, dir, &uss, &thems);
        }

      }
    }
  }
#endif

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
	acc += GetTile(x,y).IsOff() ? 0 : sides;
      }
    }
    return acc;
  }

} /* namespace MFM */

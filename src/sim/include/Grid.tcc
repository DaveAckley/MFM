/* -*- C++ -*- */
#include "Dirs.h"
#include "Grid.h"
#include "Utils.h"   /* For Sleep */
#include "FileByteSink.h"

#define XRAY_BIT_ODDS 100

namespace MFM {

  template <class GC>
  LonglivedLock & Grid<GC>::GetIntertileLock(u32 x, u32 y, Dir dir, bool isStaggered)
  {
    if(isStaggered)
      return GetIntertileLockStaggered(x,y, dir);
    return GetIntertileLockCheckerboard(x,y, dir);
  }



  template <class GC>
  LonglivedLock & Grid<GC>::GetIntertileLockCheckerboard(u32 x, u32 y, Dir dir)
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
    return _getIntertileLock(x,y,dir - Dirs::EAST);
  }

  template <class GC>
  LonglivedLock & Grid<GC>::GetIntertileLockStaggered(u32 x, u32 y, Dir dir)
  {
    switch (dir)
      {
      case Dirs::NORTHEAST:
      case Dirs::EAST:
      case Dirs::SOUTHEAST:
	// Ready to rock
	break;
      default:
	FAIL(ILLEGAL_STATE);
      }
    return _getIntertileLock(x,y,dir - Dirs::NORTHEAST);
  }

  template <class GC>
  void Grid<GC>::Init() {

    bool isStaggered = IsGridLayoutStaggered();
#if 0
    //no longer using dummy tile concept, maybe repurpose later
    // for irregular grids.
    if(isStaggered)
      {
	InitDummyTiles();
	ReinitGridRandomIterator();
      }
#endif

    /* Reseed grid PRNG and push seeds to the tile PRNGs */
    InitSeed();

    /* Give the tile iterator an initial shuffle */
    m_rgi.Shuffle(m_random);

    m_backgroundRadiationEnabled = false;

    /* Init the (non-dummy) tiles */
    for (m_rgi.ShuffleOrReset(m_random); m_rgi.HasNext(); )
    {
      SPoint tpt = IteratorIndexToCoord(m_rgi.Next());
      Tile<EC> & ctile = GetTile(tpt);

      MFM_API_ASSERT_ARG(!ctile.IsDummyTile());
      ctile.Init(); //again

      OString16 tbs;
      tbs.Printf("[%d,%d]", tpt.GetX(), tpt.GetY());
      LOG.Message("[%d,%d]", tpt.GetX(), tpt.GetY());
      ctile.SetLabel(tbs.GetZString());

      ctile.CopyHero(m_heroTile); //copies hero to ctile
    }

    // Connect up (non-dummy) tiles
    for (iterator_type i = begin(); i != end(); ++i)
      {
	MFM_LOG_DBG4(("Tile[%d][%d] @ %p", i.GetX(), i.GetY(), &(*i)));
	Tile<EC> & ctile = *i;
	SPoint tpt = i.At();

	MFM_API_ASSERT_STATE(!ctile.IsDummyTile());

        for (Dir d = Dirs::NORTHEAST; d <= Dirs::SOUTH; ++d)
	  {
	    if(! Dirs::IsValidDir(d, isStaggered))
	      continue; //staggered skips South/North

	    SPoint gridoffset;
	    Dirs::ToNeighborTileInGrid(gridoffset, d, isStaggered, tpt);
	    SPoint npt = tpt + gridoffset;

	    if(!IsLegalTileIndex(npt))
	      {
		MFM_LOG_DBG4(("Grid::Skip Init tile at illegal npt(%d, %d), tpt(%d, %d), direction %d", npt.GetX(), npt.GetY(), tpt.GetX(), tpt.GetY(), d));
		continue;
	      }

	    Tile<EC>& otile = GetTile(npt);

	    if(otile.IsDummyTile())
	      {
		MFM_LOG_DBG4(("Grid::Skip Init DUMMY tile at npt(%d, %d), tpt(%d, %d), direction %d", npt.GetX(), npt.GetY(), tpt.GetX(), tpt.GetY(), d));
		continue;
	      }

	    TileDriver & td = _getTileDriver(tpt.GetX(),tpt.GetY());
	    GridTransceiver & gt = td.m_channels[d - Dirs::NORTHEAST];
	    LonglivedLock & ctl = GetIntertileLock(tpt.GetX(),tpt.GetY(),d, isStaggered);

	    Dir odir = Dirs::OppositeDir(d);
	    MFM_API_ASSERT_STATE(Dirs::IsValidDir(odir, isStaggered));

	    LonglivedLock & otl = isStaggered ? ctl : GetIntertileLock(npt.GetX(),npt.GetY(),odir, false); //simpler for staggered, refs not changed

	    ctile.Connect(gt, ctl, d);
	    otile.Connect(gt, otl, odir);

	    gt.SetEnabled(true);
	    gt.SetDataRate(100000000);
	    gt.SetMaxInFlight(0);
	  } //direction loop
      } //tile loop
  } //Init

  template <class GC>
  double Grid<GC>::GetAverageCacheRedundancy() const
  {
    u32 count = 0;
    double sum = 0.0;
    for(u32 x = 0; x < m_width; x++)
    {
      for(u32 y = 0; y < m_height; y++)
      {
	if(!IsLegalTileIndex(x,y))
	  continue;

        const Tile<EC> & tile = GetTile(x,y);

	if(tile.IsDummyTile())
	  continue;

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
	if(!IsLegalTileIndex(x,y))
	  continue;

        const Tile<EC> & tile = GetTile(x,y);

	if(tile.IsDummyTile())
	  continue;

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
    for (m_rgi.ShuffleOrReset(m_random); m_rgi.HasNext(); )
    {
      SPoint tpt = IteratorIndexToCoord(m_rgi.Next());
      MFM_API_ASSERT_STATE(IsLegalTileIndex(tpt));

      TileDriver & td = _getTileDriver(tpt.GetX(),tpt.GetY());
      td.m_loc = tpt; //init m_loc before a GetTile call
      td.m_gridPtr = this;
      td.SetState(TileDriver::PAUSED);
      MFM_API_ASSERT_STATE(!td.GetTile().IsDummyTile());

      if (pthread_create(&td.m_threadId, NULL, TileDriverRunner, &td))
      {
        FAIL(ILLEGAL_STATE);
      }
    }

    m_threadsInitted = true;
  }

  template <class GC>
  void Grid<GC>::SetGridRunning(bool running)
  {
    /* Notify the Tiles */
    for (m_rgi.ShuffleOrReset(m_random); m_rgi.HasNext(); )
    {
      SPoint tpt = IteratorIndexToCoord(m_rgi.Next());
      MFM_API_ASSERT_STATE(IsLegalTileIndex(tpt));

      TileDriver & td = _getTileDriver(tpt.GetX(),tpt.GetY());
      MFM_API_ASSERT_STATE(!td.GetTile().IsDummyTile());

      td.SetState(running? TileDriver::ADVANCING : TileDriver::PAUSED);
    }
  }

  template <class GC>
  void* Grid<GC>::TileDriverRunner(void * arg)
  {
    TileDriver * td = (TileDriver*) arg;
    MFM_API_ASSERT_STATE(td->m_gridPtr->IsLegalTileIndex(td->m_loc));

    Tile<EC> & ctile = td->GetTile();

    MFM_API_ASSERT_ARG(!ctile.IsDummyTile()); //sanity

    // Init error stack pointer (for this thread only)
    MFMPtrToErrEnvStackPtr = ctile.GetErrorEnvironmentStackTop();

    MFM_LOG_DBG4(("TileDriver %p init: (%d,%d) == %s",
		  (void*) td,
		  td->m_loc.GetX(),
		  td->m_loc.GetY(),
		  ctile.GetLabel()));

    ctile.RequestStatePassive();

    bool running = true;
    u32 pauseUsec = 0;
    while (running)
    {
      switch (td->GetState())
      {
      case TileDriver::EXIT_REQUEST:
        running = false;
        break;

      case TileDriver::ADVANCING:
      {
        // Drive this tile's transceivers
        timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        for (u32 c = 0; c < 4; ++c)
        {
	  if(td->m_channels[c].IsEnabled()) //esa
	    td->m_channels[c].AdvanceToTime(now);
        }

        // Drive the tile itself
        if (!ctile.Advance())
        {
          // We accomplished nothing.  Let somebody else try
          pthread_yield();
        }
        pauseUsec = 0;
        break;
      }

      case TileDriver::PAUSED:
        // Sleep a little
        if (pauseUsec < 100000)
          pauseUsec += ctile.GetRandom().Between(10,100);
        SleepUsec(pauseUsec);
        break;

      default:
        FAIL(ILLEGAL_STATE);
      }
    }
    MFM_LOG_DBG4(("Tile %s thread exiting", ctile.GetLabel()));
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
    for (iterator_type i = begin(); i != end(); ++i)
      i->GetRandom().SetSeed(m_random.Create());
  }


  template <class GC>
  void Grid<GC>::InitDummyTiles()
  {
    FAIL(INCOMPLETE_CODE);
    for(u32 j=0; j < m_height; j++)
      {
	for(u32 i=0; i < m_width; i++)
	  {
	    if(IsDummyTileCoord(i, j))
	      {
		MFM_LOG_DBG3(("Tile[%d][%d] setting to DUMMY", i, j));

		Tile<EC> & tile = GetTile(SPoint(i,j));
		tile.SetDummyTile();
	      }
	  }
      }
  }

  template <class GC>
  void Grid<GC>::ReinitGridRandomIterator()
  {
    FAIL(INCOMPLETE_CODE);

    if(!IsGridLayoutStaggered())
      return;

    static u32 staggeredindexes[MAX_TILES_SUPPORTED];
    MFM_API_ASSERT_STATE(m_height * m_width <= MAX_TILES_SUPPORTED);

    u32 counter = 0;
    for(u32 j=0; j < m_height; j++)
      {
	for(u32 i=0; i < m_width; i++)
	  {
	    SPoint tpt(i,j);
	    if(!IsLegalTileIndex(tpt))
	      continue;

	    Tile<EC> & ntile = GetTile(tpt);
	    if(! ntile.IsDummyTile())
	      {
		staggeredindexes[counter] = j*m_width + i;
		MFM_LOG_DBG3(("Tile[%d][%d] in random iterator at counter %d, index %d", i, j, counter, staggeredindexes[counter]));
		counter++;
	      }
	    //else skip this tile.
	  }
      }
    m_rgi.Reinit(counter, staggeredindexes);
    MFM_API_ASSERT_STATE(m_rgi.GetLimit() == counter);
  }


  template <class GC>
  void Grid<GC>::SetTileEnabled(const SPoint& tileLoc, bool isEnabled)
  {
    if(!IsLegalTileIndex(tileLoc))
      return;

    Tile<EC> & tile = GetTile(tileLoc);
    if(tile.IsDummyTile())
      {
	tile.SetDisabled(); //?
	return;
      }

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
    MFM_API_ASSERT_ARG(!IsLegalTileIndex(tileLoc));


    Tile<EC> & tile = GetTile(tileLoc);
    MFM_API_ASSERT_ARG(!tile.IsDummyTile());
    return tile.IsEnabled();
  }

  template <class GC>
  bool Grid<GC>::IsLegalTileIndex(const SPoint & tileInGrid) const
  {
    if (tileInGrid.GetX() < 0 || tileInGrid.GetY() < 0)
      return false;
    if (tileInGrid.GetX() >= (s32) m_width || tileInGrid.GetY() >= (s32) m_height)
      return false;
    return true;
  }

  template <class GC>
  bool Grid<GC>::IsDummyTileCoord(s32 tileingridX, s32 tileingridY) const
  {
    //called once, only to init the tiles in the grid;
    //hence forth use Tile<EC>::IsDummyTile() for generality.
    if(IsGridLayoutStaggered())
      {
	//staggered grid width + 1,
	//even rows don't use last spot in row,
	//odd rows don't use first spot in row
	if(tileingridY % 2 == 0)
	  {
	    if(tileingridX >= ((s32) m_width - 1))
	      return true;
	  }
	else
	  {
	    if(tileingridX == 0)
	      return true;
	  }
      }
    return false;
  }

  template <class GC>
  void Grid<GC>::SenseTouchAround(const GridTouchEvent & gte)
  {
    const MDist<R> & md = MDist<R>::get();
    u32 lim = (u32) m_random.Between(0,R);
    for (u32 radius = 0; radius <= lim; ++radius)
    {
      u32 sqv = (radius + 3)*(radius + 3);
      for (u32 sn = md.GetFirstIndex(radius); sn <= md.GetLastIndex(radius); ++sn)
      {
        if (m_random.OddsOf(8,sqv))
          this->SenseTouchAt(gte.m_gridAtomCoord + md.GetPoint(sn), gte.m_touchType);
      }
    }
  }

  template <class GC>
  void Grid<GC>::SenseTouchAt(const SPoint gridCoord, SiteTouchType touch)
  {
    SPoint tileInGrid, siteInTile;
    if (!MapGridToTile(gridCoord, tileInGrid, siteInTile))
    {
      return;  // ain't no touch
    }

    Tile<EC> & owner = GetTile(tileInGrid);
    Site<AC> & site = owner.GetSite(siteInTile);

    //////// NOTE WE ARE RACING AGAINST THE TILE THREADS HERE!
    //
    // This code is (typically expected to be) running on the driver
    // thread, while events in the tile threads can be accesssing the
    // same Site data.  We are DELIBERATELY LETTING THIS HAPPEN woah.
    //
    site.Sense(touch);
  }


  template <class GC>
  bool Grid<GC>::RunEventIfPausedAt(const SPoint & gridCoord)
  {
    SPoint tileInGrid, siteInTile;
    if (!MapGridToTile(gridCoord, tileInGrid, siteInTile))
    {
      return false;  // ain't no touch
    }

    Tile<EC> & owner = GetTile(tileInGrid);

    MFM_API_ASSERT_ARG(!owner.IsDummyTile());

    if (owner.IsActive()) return false;  // Not paused?  Is this how we check?

    if (owner.RegionIn(siteInTile) != owner.REGION_HIDDEN)
      return false;           // Cache involvment not allowed

    EventWindow<EC> & ew = owner.GetEventWindow();
    bool attempt = ew.TryForceEventAt(siteInTile);

    return attempt;
  }


  template <class GC>
  bool Grid<GC>::MapGridToTile(const SPoint & siteInGrid, SPoint & tileInGrid, SPoint & siteInTile) const
  {
    SPoint myTile, mySite;
    if (!MapGridToUncachedTile(siteInGrid, myTile, mySite)) return false;

    MFM_API_ASSERT_ARG(!GetTile(myTile).IsDummyTile());

    tileInGrid = myTile;
    siteInTile = mySite+SPoint(R,R);      // adjust to full Tile indexing
    return true;
  }

  template <class GC>
  bool Grid<GC>::IsGridCoord(const SPoint & siteInGrid) const
  {
    //Cache coords are not distinct in terms of the grid (only known by
    //gridpanel); OWNED_ is used here, instead of TILE_ dimensions;
    if (siteInGrid.GetX() < 0 || siteInGrid.GetY() < 0)
      return false;

    SPoint offset;
    if(IsGridRowStaggered(siteInGrid))
      offset.Set(-OWNED_WIDTH/2,0);
    const SPoint ownedp(OWNED_WIDTH, OWNED_HEIGHT);

    const SPoint t = siteInGrid + offset;
    if(t.GetX() < 0 || t.GetX() > (OWNED_WIDTH * (s32) GetWidth()))
      return false;

    SPoint tileCoord = t / ownedp;
    if(!IsLegalTileIndex(tileCoord))
      return false;

    const Tile<EC>& tile = GetTile(tileCoord);
    if(tile.IsDummyTile())
      return false;
    return true;
  }

  template <class GC>
  bool Grid<GC>::MapGridToUncachedTile(const SPoint & siteInGrid, SPoint & tileInGrid, SPoint & siteInTile) const
  {
    if(!IsGridCoord(siteInGrid))
      return false;

    SPoint offset;
    if(IsGridRowStaggered(siteInGrid))
      offset.Set(-OWNED_WIDTH/2,0);
    const SPoint ownedp(OWNED_WIDTH, OWNED_HEIGHT);

    // Set up return values
    tileInGrid = (siteInGrid + offset)/ownedp;;
    siteInTile = (siteInGrid + offset) % ownedp;  // get index into just 'owned' sites
    return true;
  }

  template <class GC>
  void Grid<GC>::RecountAtoms()
  {
    for (iterator_type i = begin(); i != end(); ++i)
      i->NeedAtomRecount();
  }

  template <class GC>
  void Grid<GC>:: CheckAtom(const T& atom, const SPoint& siteInGrid)
  {
    this->PlaceAtomInSite(false, atom, siteInGrid, true);
  }

  template <class GC>
  void Grid<GC>:: PlaceAtom(const T& atom, const SPoint& siteInGrid)
  {
    this->PlaceAtomInSite(false, atom, siteInGrid);
  }

  template <class GC>
  void Grid<GC>:: PlaceAtomInSite(bool placeInBase, const T& atom, const SPoint& siteInGrid, bool checkOnly)
  {
    SPoint tileInGrid, siteInTile;
    if (!MapGridToTile(siteInGrid, tileInGrid, siteInTile))
    {
      if(!IsGridLayoutStaggered())
	{
	  printf("Can't place at (%d,%d)\n", siteInGrid.GetX(), siteInGrid.GetY());
	  FAIL(ILLEGAL_ARGUMENT);  // XXX Change to return bool?
	}
      return;
    }

    Tile<EC> & owner = GetTile(tileInGrid);
    MFM_API_ASSERT_ARG(!owner.IsDummyTile());

    owner.PlaceAtomInSite(placeInBase, atom, siteInTile, checkOnly);

    THREEDIR connectedDirs;
    u32 dircount = owner.SharedAt(siteInTile, connectedDirs, YESCHKCONNECT);

    bool isStaggered = IsGridLayoutStaggered();
    for (u32 d = 0; d < dircount; d++) {
      SPoint tileOffset;
      Dir dir = connectedDirs[d];

      Dirs::ToNeighborTileInGrid(tileOffset,dir, isStaggered, tileInGrid);

      SPoint otherTileIndex = tileInGrid+tileOffset;
      MFM_API_ASSERT_ARG(IsLegalTileIndex(otherTileIndex));

      Tile<EC> & other = GetTile(otherTileIndex);

      if (other.IsDummyTile()) continue;  // edge of grid

      // siteInTile is in tileInGrid's shared region, indexed with
      // including-cache coords.  Offsetting by the owned size
      // (excluding caches) maps into including-cache coords on their
      // side.  Hmm.
      SPoint siteOffset;
      Dirs::FillDir(siteOffset,dir, isStaggered);
      const SPoint ownedph(OWNED_WIDTH/2, OWNED_HEIGHT/2);
      SPoint otherIndex = siteInTile - siteOffset * ownedph;

      other.PlaceAtomInSite(placeInBase, atom, otherIndex, checkOnly);
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
      if(!IsGridLayoutStaggered())
	{
	  printf("Can't xray at (%d,%d)\n", siteInGrid.GetX(), siteInGrid.GetY());
	  FAIL(ILLEGAL_ARGUMENT);  // XXX Change to return bool?
	}
      return;
    }

    Tile<EC> & owner = GetTile(tileInGrid);
    owner.SingleXRay(siteInTile, 100);
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

    for (iterator_type i = begin(); i != end(); ++i)
    {
      Tile<EC> & tile = *i;
      LOG.Log(level,"--Grid(%d,%d)=Tile %s (%p)--",
              i.GetX(),  i.GetY(), tile.GetLabel(), (void *) &tile);
        tile.ReportTileStatus(level);
    }
  }

  template <class GC>
  void Grid<GC>::DoTileDriverControl(TileDriverControl & tc)
  {
    // Initial grid changes
    tc.PreGridControl(*this);

    // Ensure everybody is ready for the request
    for (m_rgi.ShuffleOrReset(m_random); m_rgi.HasNext(); )
    {
      SPoint i = IteratorIndexToCoord(m_rgi.Next());
      MFM_API_ASSERT_STATE(IsLegalTileIndex(i));

      u32 x = i.GetX();
      u32 y = i.GetY();
      TileDriver & td = _getTileDriver(x,y);
      MFM_API_ASSERT_STATE(!td.GetTile().IsDummyTile());

      if (!tc.CheckPrecondition(td))
      {
        LOG.Error("%s control precondition failed at (%d,%d)=Tile %s (%p)--",
                  tc.GetName(), x,  y, td.GetTile().GetLabel(), (void *) &td);
        ReportGridStatus(Logger::ERROR);
        FAIL(ILLEGAL_STATE);
      }
    }

    // Issue request to all
    for (m_rgi.ShuffleOrReset(m_random); m_rgi.HasNext(); )
    {
      SPoint i = IteratorIndexToCoord(m_rgi.Next());
      MFM_API_ASSERT_STATE(IsLegalTileIndex(i));

      u32 x = i.GetX();
      u32 y = i.GetY();
      TileDriver & td = _getTileDriver(x,y);
      MFM_API_ASSERT_STATE(!td.GetTile().IsDummyTile());
      tc.MakeRequest(td);
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

      for (m_rgi.ShuffleOrReset(m_random); m_rgi.HasNext(); )
      {
        SPoint i = IteratorIndexToCoord(m_rgi.Next());
	MFM_API_ASSERT_STATE(IsLegalTileIndex(i));

        u32 x = i.GetX();
        u32 y = i.GetY();
        TileDriver & td = _getTileDriver(x,y);
	MFM_API_ASSERT_STATE(!td.GetTile().IsDummyTile());

        if (!tc.CheckIfReady(td))
        {
          ++notReady;
          pthread_yield();
        }
      }

    } while (notReady > 0);

    if (loops > 5000)
    {
      LOG.Debug("%s control looped %d times",
                tc.GetName(), loops);
    }

    // Release the hounds
    for (m_rgi.ShuffleOrReset(m_random); m_rgi.HasNext(); )
    {
      SPoint i = IteratorIndexToCoord(m_rgi.Next());
      MFM_API_ASSERT_STATE(IsLegalTileIndex(i));

      u32 x = i.GetX();
      u32 y = i.GetY();
      TileDriver & td = _getTileDriver(x,y);
      MFM_API_ASSERT_STATE(!td.GetTile().IsDummyTile());
      tc.Execute(td);
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
    for (const_iterator_type i = begin(); i != end(); ++i)
      total += i->GetEventsExecuted();

    return total;
  }

  template <class GC>
  u64 Grid<GC>::GetTotalSitesAccessed() const
  {
    u64 total = 0;
    for (const_iterator_type i = begin(); i != end(); ++i)
      total += i->GetSitesAccessed();

    return total;
  }

  template <class GC>
  void Grid<GC>::WriteEPSImage(ByteSink & outstrm) const
  {
    u64 max = 1; //avoid division by 0
    const u32 swidth = GetWidthSites();
    const u32 sheight = GetHeightSites();

    for(u32 pass = 0; pass < 2; ++pass) {
      if (pass==1)
        outstrm.Printf("P5\n # Max site events = %d\n%d %d 255\n",(u32) max,swidth,sheight);
      for(u32 y = 0; y < sheight; y++) {
	for(u32 x = 0; x < swidth; x++) {
          SPoint siteInGrid(x,y), tileInGrid, siteInTile;
          bool inGrid = MapGridToUncachedTile(siteInGrid, tileInGrid, siteInTile); //false when undef in staggered grid
	  //continue; //FAIL(ILLEGAL_STATE); possible dummy tile in staggered grid.
          u64 events = inGrid ? GetTile(tileInGrid).GetUncachedSiteEvents(siteInTile) : 0;
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
    u64 max = 1; //avoid division by zero
    const u32 swidth = OWNED_WIDTH;
    const u32 sheight = OWNED_HEIGHT;
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
    for (const_iterator_type i = begin(); i != end(); ++i)
      total += i->GetAtomCount(atomType);

    return total;
  }

  template <class GC>
  s32 Grid<GC>::GetAtomCountFromSymbol(const u8 * symbol) const
  {
    const Element<EC> * elt = this->LookupElementFromSymbol(symbol);
    if (!elt) return -1;
    return (s32) this->GetAtomCount(elt->GetType());
  }

  template <class GC>
  void Grid<GC>::RandomNuke()
  {
    Random& rand = m_random;

    u32 gw = m_width * TILE_WIDTH;
    u32 gh = m_height * TILE_HEIGHT;
    SPoint center(rand.Create(gw), rand.Create(gh));

    u32 radius = rand.Between(5, MIN(gw,gh)/3); //up to a third of smaller grid dim
    T atom(Element_Empty<EC>::THE_INSTANCE.GetDefaultAtom());

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
    for (iterator_type i = begin(); i != end(); ++i)
      EmptyTile(i.At());
  }

  template <class GC>
  void Grid<GC>::CheckCaches()
  {
    const u32 gridWidth = this->GetWidthSites(); //includes dummy tiles in scattered grid
    const u32 gridHeight = this->GetHeightSites();

    for(u32 y = 0; y < gridHeight; y++)
    {
      for(u32 x = 0; x < gridWidth; x++)
      {
        SPoint siteInGrid(x, y);
	if(this->IsGridCoord(siteInGrid))
	  {
	    T atom = *this->GetAtom(siteInGrid);
	    this->CheckAtom(atom, siteInGrid);  // This checks caches
	  }
      }
    }
  } //CheckCaches

  template <class GC>
  void Grid<GC>::RefreshAllCaches()
  {
    const u32 gridWidth = this->GetWidthSites(); //includes dummy tiles in scattered grid
    const u32 gridHeight = this->GetHeightSites();

    for(u32 y = 0; y < gridHeight; y++)
    {
      for(u32 x = 0; x < gridWidth; x++)
      {
        SPoint siteInGrid(x, y);
	if(this->IsGridCoord(siteInGrid))
	  {
	    T atom = *this->GetAtom(siteInGrid);
	    this->PlaceAtom(atom, siteInGrid);  // This updates caches
	  }
      }
    }
  }

  template <class GC>
  void Grid<GC>::SetBackgroundRadiationEnabled(bool value)
  {
    for (iterator_type i = begin(); i != end(); ++i)
      i->SetBackgroundRadiationEnabled(value);

    m_backgroundRadiationEnabled = value;
  }

  template <class GC>
  void Grid<GC>::SetForegroundRadiationEnabled(bool value)
  {
    for (iterator_type i = begin(); i != end(); ++i)
      i->SetForegroundRadiationEnabled(value);

    m_foregroundRadiationEnabled = value;
  }

  template <class GC>
  void Grid<GC>::XRay()
  {
    for (iterator_type i = begin(); i != end(); ++i)
      i->XRay(m_xraySiteOdds, XRAY_BIT_ODDS);
  }

  template <class GC>
  void Grid<GC>::Thin()
  {
    for (iterator_type i = begin(); i != end(); ++i)
      i->Thin(m_xraySiteOdds);
  }

  template <class GC>
  u32 Grid<GC>::CountActiveSites() const
  {
    u32 acc   = 0,
        sides = GetTile(0,0).GetSites();

    for (const_iterator_type i = begin(); i != end(); ++i)
      //      acc += i->IsOff() ? 0 : sides;
      acc +=  sides;

    return acc;
  }

} /* namespace MFM */

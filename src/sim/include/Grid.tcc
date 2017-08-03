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
    return _getIntertileLock(x,y,dir - Dirs::EAST);
  }

  template <class GC>
  void Grid<GC>::Init() {

    /* Reseed grid PRNG and push seeds to the tile PRNGs */
    InitSeed();

    /* Give the tile iterator an initial shuffle */
    m_rgi.Shuffle(m_random);

    m_backgroundRadiationEnabled = false;

    /* Init the tiles */

    for (m_rgi.ShuffleOrReset(m_random); m_rgi.HasNext(); )
    {
      SPoint tpt = IteratorIndexToCoord(m_rgi.Next());
      Tile<EC> & ctile = GetTile(tpt);

      ctile.Init();

      OString16 tbs;
      tbs.Printf("[%d,%d]", tpt.GetX(), tpt.GetY());
      ctile.SetLabel(tbs.GetZString());

      ctile.CopyHero(m_heroTile);
    }

    // Connect them up
    for(u32 x = 0; x < m_width; x++)
    {
      for(u32 y = 0; y < m_height; y++)
      {
        for (Dir d = Dirs::NORTHEAST; d <= Dirs::SOUTH; ++d)
        {
          SPoint tpt(x,y);
          Tile<EC>& ctile = GetTile(tpt);

          SPoint npt = tpt + Dirs::GetOffset(d);

          if (!IsLegalTileIndex(npt))
          {
            continue;
          }

          TileDriver & td = _getTileDriver(x,y);
          GridTransceiver & gt = td.m_channels[d - Dirs::NORTHEAST];
          LonglivedLock & ctl = GetIntertileLock(x,y,d);

          Tile<EC>& otile = GetTile(npt);
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
        const Tile<EC> & tile = GetTile(x,y);
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
        Tile<EC> & tile = GetTile(x,y);
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
      TileDriver & td = _getTileDriver(tpt.GetX(),tpt.GetY());
      td.m_loc = tpt;
      td.m_gridPtr = this;
      td.SetState(TileDriver::PAUSED);
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
      TileDriver & td = _getTileDriver(tpt.GetX(),tpt.GetY());
      td.SetState(running? TileDriver::ADVANCING : TileDriver::PAUSED);
    }
  }

  template <class GC>
  void* Grid<GC>::TileDriverRunner(void * arg)
  {
    TileDriver * td = (TileDriver*) arg;
    Tile<EC> & ctile = td->GetTile();

    // Init error stack pointer (for this thread only)
    MFMPtrToErrEnvStackPtr = ctile.GetErrorEnvironmentStackTop();

    LOG.Debug("TileDriver %p init: (%d,%d) == %s",
              (void*) td,
              td->m_loc.GetX(),
              td->m_loc.GetY(),
              ctile.GetLabel());

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
    LOG.Debug("Tile %s thread exiting", ctile.GetLabel());
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
  void Grid<GC>::SetTileEnabled(const SPoint& tileLoc, bool isEnabled)
  {
    Tile<EC> & tile = GetTile(tileLoc);
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
    if (tileInGrid.GetX() >= (s32) m_width || tileInGrid.GetY() >= (s32) m_height)
      return false;
    return true;
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
    tileInGrid = myTile;
    siteInTile = mySite+SPoint(R,R);      // adjust to full Tile indexing
    return true;
  }

  template <class GC>
  bool Grid<GC>::IsGridCoord(const SPoint & siteInGrid) const
  {
    if (siteInGrid.GetX() < 0 || siteInGrid.GetY() < 0)
      return false;

    return IsLegalTileIndex(siteInGrid/OWNED_SIDE);
  }

  template <class GC>
  bool Grid<GC>::MapGridToUncachedTile(const SPoint & siteInGrid, SPoint & tileInGrid, SPoint & siteInTile) const
  {
    if (siteInGrid.GetX() < 0 || siteInGrid.GetY() < 0)
      return false;

    SPoint t = siteInGrid/OWNED_SIDE;

    if (!IsLegalTileIndex(t))
      return false;

    // Set up return values
    tileInGrid = t;
    siteInTile =
      siteInGrid % OWNED_SIDE;  // get index into just 'owned' sites
    return true;
  }

  template <class GC>
  void Grid<GC>::RecountAtoms()
  {
    for (iterator_type i = begin(); i != end(); ++i)
      i->NeedAtomRecount();
  }

  template <class GC>
  void Grid<GC>:: PlaceAtom(const T& atom, const SPoint& siteInGrid)
  {
    this->PlaceAtomInSite(false, atom, siteInGrid);
  }

  template <class GC>
  void Grid<GC>:: PlaceAtomInSite(bool placeInBase, const T& atom, const SPoint& siteInGrid)
  {
    SPoint tileInGrid, siteInTile;
    if (!MapGridToTile(siteInGrid, tileInGrid, siteInTile))
    {
      printf("Can't place at (%d,%d)\n", siteInGrid.GetX(), siteInGrid.GetY());
      FAIL(ILLEGAL_ARGUMENT);  // XXX Change to return bool?
    }

    Tile<EC> & owner = GetTile(tileInGrid);
    owner.PlaceAtomInSite(placeInBase, atom, siteInTile);

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

      Tile<EC> & other = GetTile(otherTileIndex);

      // siteInTile is in tileInGrid's shared region, indexed with
      // including-cache coords.  Offsetting by the owned size
      // (excluding caches) maps into including-cache coords on their
      // side.  Hmm.

      SPoint otherIndex = siteInTile - tileOffset * OWNED_SIDE;

      other.PlaceAtomInSite(placeInBase, atom, otherIndex);
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
      u32 x = i.GetX();
      u32 y = i.GetY();
      TileDriver & td = _getTileDriver(x,y);
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
      u32 x = i.GetX();
      u32 y = i.GetY();
      TileDriver & td = _getTileDriver(x,y);
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
        u32 x = i.GetX();
        u32 y = i.GetY();
        TileDriver & td = _getTileDriver(x,y);
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
      u32 x = i.GetX();
      u32 y = i.GetY();
      TileDriver & td = _getTileDriver(x,y);
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
    const u32 swidth = OWNED_SIDE;
    const u32 sheight = OWNED_SIDE;
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

    SPoint center(rand.Create(m_width * TILE_SIDE),
		  rand.Create(m_height * TILE_SIDE));

    u32 radius = rand.Between(5, TILE_SIDE);
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
    for (iterator_type i = begin(); i != end(); ++i)
    {
      const SPoint usp = i.At();
      const Tile<EC> & tile = GetTile(usp);

      FAIL(INCOMPLETE_CODE);
    }
  }

  template <class GC>
  void Grid<GC>::RefreshAllCaches()
  {
    const u32 gridWidth = this->GetWidthSites();
    const u32 gridHeight = this->GetHeightSites();

    for(u32 y = 0; y < gridHeight; y++)
    {
      for(u32 x = 0; x < gridWidth; x++)
      {
        SPoint currentPt(x, y);
        T atom = *this->GetAtom(currentPt);
        this->PlaceAtom(atom, currentPt);  // This updates caches
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

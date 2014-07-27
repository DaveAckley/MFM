/* -*- C++ -*- */
#include "MDist.h"
#include "Element_Empty.h"
#include "Logger.h"
#include "Util.h"
#include <time.h>

namespace MFM
{
  template <class CC>
  Tile<CC>::Tile() : m_executingWindow(*this)
  {
    Reinit();
  }

  template <class CC>
  void Tile<CC>::Reinit()
  {
    elementTable.Reinit();

    Element_Empty<CC>::THE_INSTANCE.AllocateType();
    RegisterElement(Element_Empty<CC>::THE_INSTANCE);

    ClearAtoms();

    m_writeFailureOdds = 0;  // Default is reliable

    m_eventsExecuted = 0;

    m_onlyWaitOnBuffers = false;

    m_backgroundRadiationEnabled = false;

    /* Set up our connection pointers. Some of these may remain NULL, */
    /* symbolizing a dead edge.       */
    u32 edges = 0;
    for(Dir i = Dirs::NORTH; edges < Dirs::DIR_COUNT; i = Dirs::CWDir(i), edges++)
    {
      if(IS_OWNED_CONNECTION(i))
      {
        /* We own this one! Hook it up. */
        m_connections[i] = m_ownedConnections + i - Dirs::EAST;
      }
      else
      {
        /* We will rely on the grid to hook these up when the time
           comes. */
        m_connections[i] = NULL;
      }
    }

    /* Zero out all of the counting fields */
    for(u32 i = 0; i < REGION_COUNT; i++)
    {
      m_regionEvents[i] = 0;
    }
    for(u32 i = 0; i < LOCKTYPE_COUNT; i++)
    {
      m_lockEvents[i] = 0;
    }
    for(u32 x = 0; x < OWNED_SIDE; x++)
    {
      for(u32 y = 0; y < OWNED_SIDE; y++)
      {
        m_siteEvents[x][y] = 0;
      }
    }

    m_needRecount = false;
    m_threadInitialized = false;
    m_threadPaused = false;
  }

  /* Definitely not thread safe. Make sure to pause and join this Tile
     before calling this from the outside. */
  template <class CC>
  void Tile<CC>::ClearAtoms()
  {
    for(u32 i = 0; i < ELEMENT_TABLE_SIZE; i++)
    {
      m_atomCount[i] = 0;
    }

    m_illegalAtomCount = 0;

    for(u32 x = 0; x < TILE_WIDTH; x++)
    {
      for(u32 y = 0; y < TILE_WIDTH; y++)
      {
        m_atoms[x][y] = Element_Empty<CC>::THE_INSTANCE.GetDefaultAtom();
      }
    }

    RecountAtoms();
  }

  template <class CC>
  void Tile<CC>::Connect(Tile<CC>& other, Dir toCache)
  {
    if(IS_OWNED_CONNECTION(toCache))
    {
      m_connections[toCache]->SetConnected(true);
    }
    else
    {
      m_connections[toCache] = other.GetConnection(Dirs::OppositeDir(toCache));
      m_connections[toCache]->SetConnected(true);
      other.Connect(*this, Dirs::OppositeDir(toCache));
    }
  }

  template <class CC>
  Connection* Tile<CC>::GetConnection(Dir cache)
  {
    return m_connections[cache];
  }

  template <class CC>
  Random& Tile<CC>::GetRandom()
  {
    return m_random;
  }

  template <class CC>
  u64 Tile<CC>::GetUncachedSiteEvents(const SPoint site) const
  {
    if (!IsInUncachedTile(site))
    {
      FAIL(ILLEGAL_ARGUMENT);
    }
    return m_siteEvents[site.GetX()][site.GetY()];
  }

  template <class CC>
  void Tile<CC>::InternalPutAtom(const T & atom, s32 x, s32 y)
  {
    if (((u32) x) >= TILE_WIDTH || ((u32) y) >= TILE_WIDTH)
    {
      FAIL(ARRAY_INDEX_OUT_OF_BOUNDS);
    }
    m_atoms[x][y] = atom;
  }

  template <class CC>
  void Tile<CC>::SendAcknowledgmentPacket(Packet<T>& packet)
  {
    Dir from = Dirs::OppositeDir(packet.GetReceivingNeighbor());
    Packet<T> sendout(PACKET_EVENT_ACKNOWLEDGE);
    sendout.SetReceivingNeighbor(from);

    m_connections[from]->Write(!IS_OWNED_CONNECTION(from),
                               (u8*)&sendout,
                               sizeof(Packet<T>));
  }

  template <class CC>
  void Tile<CC>::ReceivePacket(Packet<T>& packet)
  {
    switch(packet.GetType())
    {
    case PACKET_WRITE:
      if(packet.GetAtom().IsSane())
      {
        PlaceAtom(packet.GetAtom(), packet.GetLocation());
      }
      else
      {
        PlaceAtom(Element_Empty<CC>::THE_INSTANCE.GetDefaultAtom(), packet.GetLocation());
      }
      break;
    case PACKET_EVENT_COMPLETE:
      SendAcknowledgmentPacket(packet);
      break;
    case PACKET_EVENT_ACKNOWLEDGE:
      break;
    default:
      FAIL(INCOMPLETE_CODE);
      break;
    }
  }

  template <class CC>
  void Tile<CC>::FillLastExecutedAtom(SPoint& out)
  {
    out.Set(m_lastExecutedAtom.GetX(),
            m_lastExecutedAtom.GetY());
  }

  template <class CC>
  void Tile<CC>::CreateRandomWindow()
  {
    /* Make sure not to be created in the cache */
    int maxval = TILE_WIDTH - (EVENT_WINDOW_RADIUS << 1);
    SPoint pt(GetRandom(), maxval, maxval);
    pt.Add(EVENT_WINDOW_RADIUS, EVENT_WINDOW_RADIUS);

    m_executingWindow.SetCenterInTile(pt);
  }

  template <class CC>
  void Tile<CC>::CreateWindowAt(const SPoint& pt)
  {
    m_executingWindow.SetCenter(pt);
  }

  template <class CC>
  Dir Tile<CC>::RegionAt(const SPoint& sp, u32 reach) const
  {
    UPoint pt = MakeUnsigned(sp);

    if(pt.GetX() < reach)
    {
      if(pt.GetY() < reach)
      {
        return Dirs::NORTHWEST;
      }
      else if(pt.GetY() >= TILE_WIDTH - reach)
      {
        return Dirs::SOUTHWEST;
      }
      return Dirs::WEST;
    }
    else if(pt.GetX() >= TILE_WIDTH - reach)
    {
      if(pt.GetY() < reach)
      {
        return Dirs::NORTHEAST;
      }
      else if(pt.GetY() >= TILE_WIDTH - reach)
      {
        return Dirs::SOUTHEAST;
      }
      return Dirs::EAST;
    }

    if(pt.GetY() < reach)
    {
      return Dirs::NORTH;
    }
    else if(pt.GetY() >= TILE_WIDTH - reach)
    {
      return Dirs::SOUTH;
    }

    return (Dir)-1;
  }

  template <class CC>
  Dir Tile<CC>::CacheAt(const SPoint& pt) const
  {
    return RegionAt(pt, R);
  }

  template <class CC>
  Dir Tile<CC>::SharedAt(const SPoint& pt) const
  {
    return RegionAt(pt, R * 2);
  }

  template <class CC>
  Dir Tile<CC>::VisibleAt(const SPoint& pt) const
  {
    return RegionAt(pt, R * 3);
  }

  template <class CC>
  void Tile<CC>::PlaceAtom(const T& atom, const SPoint& pt)
  {
    const T* oldAtom = GetAtom(pt);
    u32 oldType = 0;
    unwind_protect(
    {
      InternalPutAtom(Element_Empty<CC>::THE_INSTANCE.GetDefaultAtom(),
                      pt.GetX(), pt.GetY());
      RecountAtoms();
    },
    {
      oldType = oldAtom->GetType();

      if(m_backgroundRadiationEnabled &&
         m_random.OneIn(BACKGROUND_RADIATION_SITE_ODDS))
      {
        // Maybe zap oldAtom
        SingleXRay(pt.GetX(), pt.GetY());

        if (!oldAtom->IsSane())
        {
          // This is actually more like bogus control flow, rather a
          // 'true' failure :(.  We just want to empty the site and
          // recount, the same as if an inconsistency had been
          // detected elsewhere in the code.
          FAIL(INCONSISTENT_ATOM);
        }

        u32 newOldType = oldAtom->GetType();

        if(newOldType != oldType && !IsInCache(pt))
        {
          // Here an xray has changed an atom from a
          // legal oldType into a legal newOldType
          IncrAtomCount(oldType, -1);
          IncrAtomCount(newOldType, 1);

        }

        // So this is (now) the 'old' atom's type
        oldType = newOldType;

      }

      u32 newType = atom.GetType();
      if(newType != oldType && !IsInCache(pt))
      {
        // Here we're just displacing an oldType
        // atom with a newType atom
        IncrAtomCount(oldType, -1);
        IncrAtomCount(newType, 1);
      }

      InternalPutAtom(atom,pt.GetX(),pt.GetY());
    });
  }

  template <class CC>
  void Tile<CC>::SendAtom(Dir neighbor, SPoint& atomLoc)
  {
    if(IsConnected(neighbor))
    {
      SPoint remoteLoc(atomLoc);

      u32 tileDiff = TILE_WIDTH - 2 * R;

      /* The neighbor will think this atom is in a different location. */
      switch(neighbor)
        {
        case Dirs::NORTH: remoteLoc.Add(0, tileDiff); break;
        case Dirs::SOUTH: remoteLoc.Add(0, -tileDiff); break;
        case Dirs::WEST:  remoteLoc.Add(tileDiff, 0); break;
        case Dirs::EAST:  remoteLoc.Add(-tileDiff, 0); break;
        case Dirs::NORTHEAST:
          remoteLoc.Add(-tileDiff, tileDiff); break;
        case Dirs::SOUTHEAST:
          remoteLoc.Add(-tileDiff, -tileDiff); break;
        case Dirs::SOUTHWEST:
          remoteLoc.Add(tileDiff, -tileDiff); break;
        case Dirs::NORTHWEST:
          remoteLoc.Add(tileDiff, tileDiff); break;
        default:
          FAIL(INCOMPLETE_CODE); break;
        }

      Packet<T> sendout(PACKET_WRITE);

      /* Did this atom get corrupted? Destroy it! */
      if(!m_atoms[atomLoc.GetX()][atomLoc.GetY()].IsSane())
      {
        PlaceAtom(Element_Empty<CC>::THE_INSTANCE.GetDefaultAtom(), atomLoc);
      }

      sendout.SetLocation(remoteLoc);
      sendout.SetAtom(m_atoms[atomLoc.GetX()][atomLoc.GetY()]);
      sendout.SetReceivingNeighbor(neighbor);

      /* Send out the serialized version of the packet */
      m_connections[neighbor]->Write(!IS_OWNED_CONNECTION(neighbor),
                                     (u8*)&sendout,
                                     sizeof(Packet<T>));
    }
  }

  template <class CC>
  bool Tile<CC>::IsConnected(Dir dir) const
  {
    return m_connections[dir] != NULL &&
      m_connections[dir]->IsConnected();
  }

  template <class CC>
  bool Tile<CC>::IsLiveSite(const SPoint & location) const
  {
    return IsInTile(location) &&
      (!IsInCache(location) || IsConnected(CacheAt(location)));
  }

  template <class CC>
  bool Tile<CC>::IsOwnedSite(const SPoint & location)
  {
    return IsInTile(location) && !IsInCache(location);
  }

  template <class CC>
  bool Tile<CC>::IsInCache(const SPoint& pt)
  {
    s32 upbnd = TILE_WIDTH - R;
    return (u32)pt.GetX() < R || (u32)pt.GetY() < R ||
      pt.GetX() >= upbnd || pt.GetY() >= upbnd;
  }

  template <class CC>
  bool Tile<CC>::IsInTile(const SPoint& pt)
  {
    return ((u32) pt.GetX()) < TILE_WIDTH && ((u32) pt.GetY() < TILE_WIDTH);
  }

  template <class CC>
  bool Tile<CC>::IsInUncachedTile(const SPoint& pt)
  {
    return ((u32) pt.GetX()) < OWNED_SIDE && ((u32) pt.GetY() < OWNED_SIDE);
  }

  template <class CC>
  void Tile<CC>::SendEndEventPackets(u32 dirWaitWord)
  {
    Dir dir = Dirs::NORTH;
    do
    {
      if(IsConnected(dir) && (dirWaitWord & (1 << dir)))
      {
        Packet<T> sendout(PACKET_EVENT_COMPLETE);
        sendout.SetReceivingNeighbor(dir);

        /* We don't care about what other kind of stuff is in the Packet */
        m_connections[dir]->Write(!IS_OWNED_CONNECTION(dir),
                                  (u8*)&sendout, sizeof(Packet<T>));
      }

      dir = Dirs::CWDir(dir);
    } while (dir != Dirs::NORTH);
  }

  template <class CC>
  u32 Tile<CC>::SendRelevantAtoms()
  {
    // Extract short names for parameter types
    typedef typename CC::PARAM_CONFIG P;

    SPoint localLoc;
    SPoint ewCenter;

    u32 dirBitfield = 0;

    s32 r2 = R * 2;
    ewCenter = m_executingWindow.GetCenterInTile();

    for(u32 i = 0; i < m_executingWindow.GetAtomCount(); i++)
    {
      MDist<R>::get().FillFromBits(localLoc, i, R);
      localLoc.Add(ewCenter);

      /* Send to West neighbor? */
      if(IsConnected(Dirs::WEST) && localLoc.GetX() < r2)
      {
        SendAtom(Dirs::WEST, localLoc);
        dirBitfield = Dirs::AddDirToMask(dirBitfield, Dirs::WEST);
        if(IsConnected(Dirs::NORTH) && localLoc.GetY() < r2)
        {
          SendAtom(Dirs::NORTHWEST, localLoc);
          SendAtom(Dirs::NORTH, localLoc);
          dirBitfield = Dirs::AddDirToMask(dirBitfield, Dirs::NORTHWEST);
          dirBitfield = Dirs::AddDirToMask(dirBitfield, Dirs::NORTH);
        }
        else if(IsConnected(Dirs::SOUTH) && localLoc.GetY() >= P::TILE_WIDTH - r2)
        {
          SendAtom(Dirs::SOUTHWEST, localLoc);
          SendAtom(Dirs::SOUTH, localLoc);
          dirBitfield = Dirs::AddDirToMask(dirBitfield, Dirs::SOUTHWEST);
          dirBitfield = Dirs::AddDirToMask(dirBitfield, Dirs::SOUTH);
        }
      }
      /*East neighbor?*/
      else if(IsConnected(Dirs::EAST) && localLoc.GetX() >= P::TILE_WIDTH - r2)
      {
        SendAtom(Dirs::EAST, localLoc);
        dirBitfield = Dirs::AddDirToMask(dirBitfield, Dirs::EAST);
        if(IsConnected(Dirs::NORTH) && localLoc.GetY() < r2)
        {
          SendAtom(Dirs::NORTHEAST, localLoc);
          SendAtom(Dirs::NORTH, localLoc);
          dirBitfield = Dirs::AddDirToMask(dirBitfield, Dirs::NORTHEAST);
          dirBitfield = Dirs::AddDirToMask(dirBitfield, Dirs::NORTH);
        }
        if(IsConnected(Dirs::SOUTH) && localLoc.GetY() >= P::TILE_WIDTH - r2)
        {
          SendAtom(Dirs::SOUTHEAST, localLoc);
          SendAtom(Dirs::SOUTH, localLoc);
          dirBitfield = Dirs::AddDirToMask(dirBitfield, Dirs::SOUTHEAST);
          dirBitfield = Dirs::AddDirToMask(dirBitfield, Dirs::SOUTH);
        }
      }
      else if(IsConnected(Dirs::NORTH) && localLoc.GetY() < r2)
      {
        SendAtom(Dirs::NORTH, localLoc);
        dirBitfield = Dirs::AddDirToMask(dirBitfield, Dirs::NORTH);
      }
      else if(IsConnected(Dirs::SOUTH) && localLoc.GetY() >= P::TILE_WIDTH - r2)
      {
        SendAtom(Dirs::SOUTH, localLoc);
        dirBitfield = Dirs::AddDirToMask(dirBitfield, Dirs::SOUTH);
      }
    }
    return dirBitfield;
  }

  template <class CC>
  bool Tile<CC>::TryLock(Dir connectionDir)
  {
    return IsConnected(connectionDir) &&
      m_connections[connectionDir]->Lock();
  }

  template <class CC>
  bool Tile<CC>::TryLockCorner(Dir cornerDir)
  {
    u32 locked = 0;

    /* Go back one, then wind until we lock all three. */
    cornerDir = Dirs::CCWDir(cornerDir);
    for(u32 i = 0; i < 3; i++)
    {
      if(TryLock(cornerDir))
      {
        locked++;
        cornerDir = Dirs::CWDir(cornerDir);
      }
      /* If we can't hit one, rewind, unlocking all held locks. */
      else
      {
        for(u32 j = 0; j < locked; j++)
        {
          cornerDir = Dirs::CCWDir(cornerDir);
          m_connections[cornerDir]->Unlock();
        }
        return false;
      }
    }
    return true;
  }

  template <class CC>
  bool Tile<CC>::LockRegion(Dir regionDir)
  {
    switch(regionDir)
    {
    case Dirs::NORTH:
    case Dirs::EAST:
    case Dirs::SOUTH:
    case Dirs::WEST:
      return TryLock(regionDir);

    case Dirs::NORTHWEST:
    case Dirs::NORTHEAST:
    case Dirs::SOUTHEAST:
    case Dirs::SOUTHWEST:
      return TryLockCorner(regionDir);

    default:
      FAIL(ILLEGAL_ARGUMENT);
    }
  }

  template <class CC>
  void Tile<CC>::UnlockCorner(Dir corner)
  {
    corner = Dirs::CCWDir(corner);
    for(u32 i = 0; i < 3; i++)
    {
      m_connections[corner]->Unlock();
      corner = Dirs::CWDir(corner);
    }
  }

  template <class CC>
  void Tile<CC>::UnlockRegion(Dir regionDir)
  {
    switch(regionDir)
    {
    case Dirs::NORTH:
    case Dirs::EAST:
    case Dirs::SOUTH:
    case Dirs::WEST:
      m_connections[regionDir]->Unlock();
      return;

    case Dirs::NORTHWEST:
    case Dirs::NORTHEAST:
    case Dirs::SOUTHEAST:
    case Dirs::SOUTHWEST:
      return UnlockCorner(regionDir);

    default:
      FAIL(ILLEGAL_ARGUMENT);
    }
  }

  template <class CC>
  bool Tile<CC>::IsInHidden(const SPoint& pt)
  {
    // Extract short names for parameter types
    typedef typename CC::PARAM_CONFIG P;

    return pt.GetX() >= (s32)R * 3 && pt.GetX() < P::TILE_WIDTH - (s32)R * 3 &&
      pt.GetY() >= (s32)R * 3 && pt.GetY() < P::TILE_WIDTH - (s32)R * 3;
  }

  template <class CC>
  TileRegion Tile<CC>::RegionFromIndex(const u32 index)
  {
    // Extract short names for parameter types
    typedef typename CC::PARAM_CONFIG P;
    enum { R = P::EVENT_WINDOW_RADIUS };

    if(index > P::TILE_WIDTH)
    {
      FAIL(ARRAY_INDEX_OUT_OF_BOUNDS); /* Index out of Tile bounds */
    }

    const u32 hiddenWidth = P::TILE_WIDTH - R * 6;

    if(index < R * REGION_HIDDEN)
    {
      return (TileRegion)(index / R);
    }
    else if(index >= R * REGION_HIDDEN + hiddenWidth)
    {
      return (TileRegion)((index - (R * REGION_HIDDEN) - hiddenWidth) / R);
    }
    else
    {
      return REGION_HIDDEN;
    }
  }

  template <class CC>
  TileRegion Tile<CC>::RegionIn(const SPoint& pt)
  {
    return MIN(RegionFromIndex((u32)pt.GetX()),
               RegionFromIndex((u32)pt.GetY()));
  }

  template <class CC>
  void Tile<CC>::FlushAndWaitOnAllBuffers(u32 dirWaitWord)
  {
    Packet<T> readPack(PACKET_WRITE);
    u32 readBytes;
    do
    {
      /* Flush out all packet buffers */
      for(Dir dir = Dirs::NORTH; dir < Dirs::DIR_COUNT; ++dir)
      {
        if(IsConnected(dir))
        {
          while((readBytes = m_connections[dir]->Read(!IS_OWNED_CONNECTION(dir),
                                                      (u8*)&readPack, sizeof(Packet<T>))))
          {
            if(readBytes != sizeof(Packet<T>))
            {
              FAIL(ILLEGAL_STATE);  /* Didn't read enough for a full packet! */
            }
            if(dirWaitWord & (1 << dir))
            {
              if(readPack.GetType() == PACKET_EVENT_ACKNOWLEDGE)
              {
                /* FAIL(ILLEGAL_STATE);  Didn't get an acknowledgment right away */
                dirWaitWord &= (~(1 << dir));
              }
            }
            ReceivePacket(readPack);
          }
        }
        /* Have we waited long enough without a response? Let's disconnect that tile. */

      }
      pthread_yield();
      m_threadPauser.WaitIfPaused();
    } while(dirWaitWord);
  }

  template <class CC>
  void Tile<CC>::Execute()
  {
    while(m_threadInitialized)
    {
      RecountAtomsIfNeeded();

      CreateRandomWindow();

      bool locked = false;
      Dir lockRegion = Dirs::NORTH;
      u32 dirWaitWord = 0;
      if((!m_onlyWaitOnBuffers) &&
         (
           IsInHidden(m_executingWindow.GetCenterInTile()) ||
           !IsConnected(lockRegion = VisibleAt(m_executingWindow.GetCenterInTile())) ||
           (locked = LockRegion(lockRegion))
           ))
      {
        unwind_protect({
            ++m_eventsFailed;
            ++m_failuresErased;

            if(!m_executingWindow.GetCenterAtom().IsSane())
            {
              LOG.Debug("FE(INSANE)");
            }
            else
            {
              LOG.Debug("FE(%x) (SANE)",m_executingWindow.GetCenterAtom().GetType());
            }


            m_executingWindow.SetCenterAtom(Element_Empty<CC>::THE_INSTANCE.GetDefaultAtom());
          },{
            elementTable.Execute(m_executingWindow);
          });

        // XXX INSANE SLOWDOWN FOR DEBUG: AssertValidAtomCounts();

        m_lastExecutedAtom = m_executingWindow.GetCenterInTile();

        dirWaitWord = SendRelevantAtoms();

        SendEndEventPackets(dirWaitWord);

        FlushAndWaitOnAllBuffers(dirWaitWord);


        ++m_eventsExecuted;
        ++m_regionEvents[RegionIn(m_executingWindow.GetCenterInTile())];

        ++m_siteEvents[m_executingWindow.GetCenterInTile().GetX() - R]
          [m_executingWindow.GetCenterInTile().GetY() - R];

        if(locked)
        {
          UnlockRegion(lockRegion);

          switch(lockRegion)
          {
          case Dirs::NORTH: case Dirs::SOUTH:
          case Dirs::EAST:  case Dirs::WEST:
            ++m_regionEvents[LOCKTYPE_SINGLE]; break;
          default: /* UnlockRegion would have caught a bad argument. */
            ++m_regionEvents[LOCKTYPE_TRIPLE]; break;
          }
        }
        else
        {
          ++m_regionEvents[LOCKTYPE_NONE];
        }
      }
      else
      {
        FlushAndWaitOnAllBuffers(dirWaitWord);
      }
    }
  }

  template <class CC>
  void* Tile<CC>::ExecuteThreadHelper(void* arg)
  {
    Tile* tilePtr = (Tile*) arg;
    MFMPtrToErrEnvStackPtr = &(tilePtr->m_errorEnvironmentStackTop);
    tilePtr->Execute();
    return NULL;
  }

  template <class CC>
  u32 Tile<CC>::GetAtomCount(ElementType atomType) const
  {
    s32 idx = elementTable.GetIndex(atomType);
    if (idx < 0) return 0;
    return m_atomCount[idx];
  }

  template <class CC>
  void Tile<CC>::SetAtomCount(ElementType atomType, s32 count)
  {
    s32 idx = elementTable.GetIndex(atomType);
    if (idx < 0) {
      if (count > 0) FAIL(ILLEGAL_STATE);
      return;
    }
    m_atomCount[idx] = count;
  }
  template <class CC>
  void Tile<CC>::Start()
  {
    if(!m_threadInitialized)
    {
      // Possible xrays before start means we can't assert even here
      //      AssertValidAtomCounts();
      RecountAtoms();
      m_threadInitialized = true;
      pthread_create(&m_thread, NULL, ExecuteThreadHelper, this);
    }
    else
    {
      m_threadPauser.Unpause();
    }
  }

  template <class CC>
  void Tile<CC>::Pause()
  {
    m_threadPauser.Pause();
  }

  template <class CC>
  void Tile<CC>::IncrAtomCount(ElementType atomType, s32 delta)
  {
    s32 idx = elementTable.GetIndex(atomType);
    if (idx < 0)
    {
      m_illegalAtomCount += delta;
      return;
    }
    if (delta < 0 && -delta > m_atomCount[idx])
    {
      LOG.Warning("LOST ATOMS %x %d %d (Tile %p) - requesting recount",
                  (int) atomType,delta,m_atomCount[idx],(void*) this);
      m_needRecount = true;
      return;
    }

    if (delta < 0)
    {
      m_atomCount[idx] -= -delta;
    }
    else
    {
      m_atomCount[idx] += delta;
    }

  }

  template <class CC>
  void Tile<CC>::AssertValidAtomCounts() const
  {
    s32 counts[ELEMENT_TABLE_SIZE];
    for (u32 i = 0; i < ELEMENT_TABLE_SIZE; ++i)
    {
      counts[i] = 0;
    }
    for (u32 x = 0; x < TILE_WIDTH; ++x)
    {
      for (u32 y = 0; y < TILE_WIDTH; ++y)
      {
        const SPoint pt(x, y);

        if(IsInCache(pt))
        {
          continue;
        }

        const T * atom = GetAtom(x,y);
        s32 type = elementTable.GetIndex(atom->GetType());
        if (type < 0)
        {
          FAIL(UNKNOWN_ELEMENT);
        }
        counts[type]++;
      }
    }
    for (u32 i = 0; i < ELEMENT_TABLE_SIZE; ++i)
    {
      if (counts[i] != m_atomCount[i])
      {
        FAIL(ILLEGAL_STATE);
      }
    }
  }

  template <class CC>
  void Tile<CC>::RecountAtomsIfNeeded()
  {
    if (m_needRecount)
    {
      LOG.Message("Recounting atoms (Tile %p)", this);
      RecountAtoms();
      m_needRecount = false;
    }
  }

  template <class CC>
  void Tile<CC>::RecountAtoms()
  {
    for(u32 i = 0; i < ELEMENT_TABLE_SIZE; i++)
    {
      m_atomCount[i] = 0;
    }

    // Not clear that anybody cares about this, but
    m_illegalAtomCount = 0;

    for(u32 x = 0; x < TILE_WIDTH; x++)
    {
      for(u32 y = 0; y < TILE_WIDTH; y++)
      {
        const SPoint pt(x,y);

        if (IsInCache(pt))
        {
          continue;
        }

        IncrAtomCount(m_atoms[x][y].GetType(), 1);
      }
    }
  }

  template <class CC>
  void Tile<CC>::SingleXRay()
  {
    SingleXRay(m_random.Create(W), m_random.Create(W));
  }

  template <class CC>
  void Tile<CC>::SingleXRay(u32 x, u32 y)
  {
    m_atoms[x][y].XRay(m_random, BACKGROUND_RADIATION_BIT_ODDS);
  }

  template <class CC>
  void Tile<CC>::XRay(u32 siteOdds, u32 bitOdds)
  {
    for(u32 x = 0; x < W; x++)
    {
      for(u32 y = 0; y < W; y++)
      {
        if(m_random.OneIn(siteOdds))
        {
          m_atoms[x][y].XRay(m_random, bitOdds);
        }
      }
    }
  }
} /* namespace MFM */

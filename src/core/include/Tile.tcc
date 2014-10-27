/* -*- C++ -*- */
#include "MDist.h"
#include "Element_Empty.h"
#include "Logger.h"
#include "AtomSerializer.h"
#include "PacketSerializer.h"
#include "Util.h"

namespace MFM
{
  template <class CC>
  Tile<CC>::Tile() :
    m_ignoreThreadingProblems(false),
    m_executingWindow(*this),
    m_generation(0)
  {
    m_lockAttempts = m_lockAttemptsSucceeded = 0;
    Reinit();
  }

  template <class CC>
  void Tile<CC>::SetIgnoreThreadingProblems(bool value)
  {
    m_ignoreThreadingProblems = value;
    m_threadPauser.SetIgnoreThreadingProblems(value);
  }

  template <class CC>
  void Tile<CC>::Reinit()
  {
    elementTable.Reinit();

    Element_Empty<CC>::THE_INSTANCE.AllocateType();
    RegisterElement(Element_Empty<CC>::THE_INSTANCE);

    ClearAtoms();

#if 0
    m_writeFailureOdds = 0;  // Default is reliable
#endif

    m_isFnWing = false;
    m_curFnWingDirWaitWord = 0;
    m_origFnWingDirWaitWord = 0;
    m_isA2PRed = false;
    m_isGSBIsw = false;

    m_eventsExecuted = 0;

    m_executeOwnEvents = true;

    m_backgroundRadiationEnabled = false;

    /* Set up our connection pointers. Some of these may remain NULL, */
    /* symbolizing a dead edge.       */
    u32 edges = 0;
    for(Dir i = Dirs::NORTH; edges < Dirs::DIR_COUNT; i = Dirs::CWDir(i), edges++)
    {

      /* We have nothing locked */
      m_iLocked[i] = false;

      if(IS_OWNED_CONNECTION(i))
      {
        /* We own this one! Hook it up. */
        m_connections[i] = &m_ownedConnections[i - Dirs::EAST];
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
        m_lastChangedEventNumber[x][y] = 0;
      }
    }

    m_needRecount = false;
    m_threadInitialized = false;
    //    m_threadPaused = false;
  }

  /* Definitely not thread safe. Make sure to pause and join this Tile
     before calling this from the outside. */
  template <class CC>
  void Tile<CC>::ClearAtoms()
  {

    assert(IsPausedOrOwner());

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

  /* Definitely not thread safe. Make sure to pause and join this Tile
     before calling this from the outside. */
  template <class CC>
  void Tile<CC>::CheckCacheFromDir(Dir direction, const Tile & otherTile)
  {
    if(m_ignoreThreadingProblems)
    {
      if(!IsPausedOrOwner())
      {
        LOG.Error("%s:%d: THREADING PROBLEM ENCOUNTERED! Tile is configured to ignore "
                  "this problem and will continue execution.", __FILE__, __LINE__);
      }
    }
    else
    {
      assert(IsPausedOrOwner());
    }

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
  u32 Tile<CC>::GetUncachedWriteAge(const SPoint site) const
  {
    if (!IsInUncachedTile(site))
    {
      FAIL(ILLEGAL_ARGUMENT);
    }
    return (u32) (m_eventsExecuted -
                  m_lastChangedEventNumber[site.GetX()][site.GetY()]);
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
    // Acknowledge on whatever generation they said they were
    Packet<T> sendout(PACKET_EVENT_ACKNOWLEDGE, packet.GetGeneration());
    sendout.SetReceivingNeighbor(from);

    m_connections[from]->Write(!IS_OWNED_CONNECTION(from),
                               (u8*)&sendout,
                               sizeof(Packet<T>));
  }

  template <class CC>
  void Tile<CC>::ReceivePacket(Packet<T>& packet)
  {
    //bool isObsolete = packet.IsObsolete(m_generation);
    bool isObsolete = packet.GetGeneration() != m_generation;
    if (isObsolete)
    {
      LOG.Debug("Received obsolete packet in %d", m_generation);
    }

    switch(packet.GetType())
    {
    case PACKET_WRITE:
      if (!isObsolete)
      {
        if(packet.GetAtom().IsSane())
        {
          PlaceAtom(packet.GetAtom(), packet.GetLocation());
        }
        else
        {
          LOG.Debug("%s received insane atom for (%d,%d), discarding",
                    this->GetLabel(),
                    packet.GetLocation().GetX(),
                    packet.GetLocation().GetY());
          PlaceAtom(Element_Empty<CC>::THE_INSTANCE.GetDefaultAtom(), packet.GetLocation());
        }
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
    if (!IsLiveSite(pt))
    {
      if (atom.GetType() != Element_Empty<CC>::THE_INSTANCE.GetType())
      {
        LOG.Debug("Not placing type %04x at (%2d,%2d) of %s",
                  atom.GetType(), pt.GetX(), pt.GetY(), this->GetLabel());
      }
      return;
    }

    T newAtom = atom;
    unwind_protect(
    {
      InternalPutAtom(Element_Empty<CC>::THE_INSTANCE.GetDefaultAtom(),
                      pt.GetX(), pt.GetY());
      RecountAtoms();
      LOG.Warning("Failure during PlaceAtom, erased (%2d,%2d) of %s",
                  pt.GetX(), pt.GetY(), this->GetLabel());
    },
    {
      if(m_backgroundRadiationEnabled &&
         m_random.OneIn(BACKGROUND_RADIATION_SITE_ODDS))
      {
        // Write fault!
        newAtom.XRay(m_random, BACKGROUND_RADIATION_BIT_ODDS);

        if (!newAtom.IsSane())
        {
          // This is actually more like bogus control flow, rather a
          // 'true' failure :(.  We just want to empty the site and
          // recount, the same as if an inconsistency had been
          // detected elsewhere in the code.
          FAIL(INCONSISTENT_ATOM);
        }
      }

      const T& oldAtom = *GetAtom(pt);
      bool owned = IsOwnedSite(pt);
      if (oldAtom != newAtom)
      {
        if (owned)
        {
          const SPoint opt = pt - SPoint(R,R); // Really no routine to map into owned coords?
          m_lastChangedEventNumber[opt.GetX()][opt.GetY()] = m_eventsExecuted;
        }

        u32 oldType = oldAtom.GetType();
        u32 newType = newAtom.GetType();
        if(newType != oldType && owned)
        {
          // Here we're displacing an oldType
          // atom with a newType atom
          IncrAtomCount(oldType, -1);
          IncrAtomCount(newType, 1);
        }

        InternalPutAtom(newAtom,pt.GetX(),pt.GetY());
      }
    });
  }

  template <class CC>
  SPoint Tile<CC>::GetNeighborLoc(Dir neighbor, const SPoint& atomLoc)
  {
    u32 tileDiff = TILE_WIDTH - 2 * R;
    SPoint remoteLoc(atomLoc);

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
    return remoteLoc;
  }

  template <class CC>
  void Tile<CC>::SendAtom(Dir neighbor, SPoint& atomLoc)
  {
    if(IsConnected(neighbor))
    {
      SPoint remoteLoc(GetNeighborLoc(neighbor,atomLoc));

      Packet<T> sendout(PACKET_WRITE, m_generation);

      /* Did this atom get corrupted? Destroy it! */
      if(!GetAtom(atomLoc)->IsSane())
      {
        PlaceAtom(Element_Empty<CC>::THE_INSTANCE.GetDefaultAtom(), atomLoc);
      }

      sendout.SetLocation(remoteLoc);
      sendout.SetAtom(*GetAtom(atomLoc));
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
        Packet<T> sendout(PACKET_EVENT_COMPLETE, m_generation);
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

    const s32 r2 = R * 2;
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
    assert(!m_iLocked[connectionDir]);

    if (!IsConnected(connectionDir))
    {
      return true;
    }
    if (!m_connections[connectionDir]->Lock())
    {
      return false;
    }
    m_iLocked[connectionDir] = true;
    return true;
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

          UnlockDir(cornerDir);
        }
        return false;
      }
    }
    return true;
  }

  template <class CC>
  bool Tile<CC>::HasAnyConnections(Dir regionDir) const
  {
    switch(regionDir)
    {
    case Dirs::NORTH:
    case Dirs::EAST:
    case Dirs::SOUTH:
    case Dirs::WEST:
      return IsConnected(regionDir);

    case Dirs::NORTHWEST:
    case Dirs::NORTHEAST:
    case Dirs::SOUTHEAST:
    case Dirs::SOUTHWEST:
      if (IsConnected(regionDir)) return true;
      if (IsConnected(Dirs::CCWDir(regionDir))) return true;
      if (IsConnected(Dirs::CWDir(regionDir))) return true;
      return false;

    default:
      FAIL(ILLEGAL_ARGUMENT);
    }
  }

  template <class CC>
  bool Tile<CC>::LockRegion(Dir regionDir)
  {
    ++m_lockAttempts;

    bool success = false;
    switch(regionDir)
    {
    case Dirs::NORTH:
    case Dirs::EAST:
    case Dirs::SOUTH:
    case Dirs::WEST:
      success = TryLock(regionDir);
      break;

    case Dirs::NORTHWEST:
    case Dirs::NORTHEAST:
    case Dirs::SOUTHEAST:
    case Dirs::SOUTHWEST:
      success = TryLockCorner(regionDir);
      break;

    default:
      FAIL(ILLEGAL_ARGUMENT);
    }
    if (success)
    {
      ++m_lockAttemptsSucceeded;
    }
    const u32 MILLION = 1000000;
    if ((m_lockAttempts % (1*MILLION)) == 0)
    {
      LOG.Debug("Locks %dM of %dM (%d%%) for %s",
                (u32) (m_lockAttemptsSucceeded / MILLION),
                (u32) (m_lockAttempts / MILLION),
                (u32) (100 * m_lockAttemptsSucceeded / m_lockAttempts),
                this->GetLabel());
    }
    return success;
  }

  template <class CC>
  void Tile<CC>::UnlockCorner(Dir corner)
  {
    corner = Dirs::CCWDir(corner);
    for(u32 i = 0; i < 3; i++)
    {
      UnlockDir(corner);
      corner = Dirs::CWDir(corner);
    }
  }

  template <class CC>
  void Tile<CC>::UnlockDir(Dir dir)
  {
    if (IsConnected(dir))
    {
      assert(m_iLocked[dir]);

      m_connections[dir]->Unlock();

      m_iLocked[dir] = false;
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
      UnlockDir(regionDir);
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
  void Tile<CC>::ReportIfBuffersAreNonEmpty()
  {
    for(Dir dir = Dirs::NORTH; dir < Dirs::DIR_COUNT; ++dir)
    {
      if(IsConnected(dir))
      {
        u32 threadTag = (((u32) pthread_self())>>8)&0xffff;
        if (m_connections[dir]->InputByteCount() != 0)
        {
          LOG.Warning("NON-EMPTY INPUT BUFFER (%d bytes) IN %s%04x. Packets:",
                      m_connections[dir]->InputByteCount(),
                      this->GetLabel(),
                      threadTag);

          for(u32 i = 0; i + sizeof(Packet<T>) <= m_connections[dir]->InputByteCount(); i += sizeof(Packet<T>))
          {
            Packet<T> buffer((PacketType) 0xff, 0xff);  // Deliberately invalid initialization
            m_connections[dir]->PeekRead(false, (u8*) &buffer, i, sizeof(Packet<T>));

            PacketSerializer<CC> serializer(buffer);

            LOG.Warning(" %3d%s%04x: %@",
                        i,
                        this->GetLabel(),
                        threadTag,
                        &serializer);
          }
        }
        if (m_connections[dir]->OutputByteCount() != 0)
        {
          LOG.Warning("NON-EMPTY OUTPUT BUFFER (%d bytes) IN %s%04x",
                      m_connections[dir]->OutputByteCount(),
                      this->GetLabel(),
                      threadTag);
        }
      }
    }
  }


  template <class CC>
  bool Tile<CC>::FlushAndWaitOnAllBuffers(u32 dirWaitWord)
  {
    m_curFnWingDirWaitWord = m_origFnWingDirWaitWord = dirWaitWord;
    m_isFnWing = true;  // Track where we are

    Packet<T> readPack(PACKET_WRITE, m_generation);
    u32 readBytes;
    u32 locksStillHeld = 0;
    u32 loops = 0;
    s32 sleepTimer = m_random.Create(10000);
    do
    {
      locksStillHeld = 0; // Assume this

      /* Flush out all packet buffers */
      for(Dir dir = Dirs::NORTH; dir < Dirs::DIR_COUNT; ++dir)
      {
        if(IsConnected(dir))
        {

          if (m_iLocked[dir] || m_connections[dir]->IsLockedByAnother())
          {
            ++locksStillHeld;
          }

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
                dirWaitWord &= (~(1 << dir));
                m_curFnWingDirWaitWord = dirWaitWord;
              }
              else
              {
                FAIL(ILLEGAL_STATE);  /* Didn't get an acknowledgment right away */
              }
            }
            ReceivePacket(readPack);
          }

        }
        /* Have we waited long enough without a response? Let's disconnect that tile. */

      }
      if (++loops >= 100000000)
      {
        LOG.Error("Tile %s flush looped %d times, but dirWaitWord (0x%x) still not 0, and %d locks held",
                  this->GetLabel(), loops, dirWaitWord, locksStillHeld);
        FAIL(LOCK_FAILURE);  // Not really, but it's a marker
      }

      if (--sleepTimer < 0)
      {
        // Try sleeping every once in a while
        Sleep(0, loops);
        sleepTimer = m_random.Create(250);
      }
      else
      {
        pthread_yield();
      }

    } while(dirWaitWord);

    m_isFnWing = false;

    return locksStillHeld > 0;
  }

  template <class CC>
  void Tile<CC>::DoEvent(bool locked, Dir lockRegion)
  {
    u32 dirWaitWord = 0;
    unwind_protect(
      {
        ++m_eventsFailed;
        ++m_failuresErased;

        if ((m_failuresErased % 100) == 0)
        {
          LOG.Debug("%d erasures tile %s", m_failuresErased, this->GetLabel());
        }

        if(!m_executingWindow.GetCenterAtom().IsSane())
        {
          LOG.Debug("FE(INSANE)");
        }
        else
        {
          LOG.Debug("FE(%x) (SANE)",m_executingWindow.GetCenterAtom().GetType());
        }


        m_executingWindow.SetCenterAtom(Element_Empty<CC>::THE_INSTANCE.GetDefaultAtom());
      },
      {
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

  template <class CC>
  void Tile<CC>::Execute()
  {
    while(m_threadInitialized)
    {
      RecountAtomsIfNeeded();

      m_isGSBIsw = true;
      ThreadState curState = m_threadPauser.GetStateBlockingInner();
      m_isGSBIsw = false;

      switch (curState)
      {
      case THREADSTATE_RUN_REQUESTED:
        m_threadPauser.AdvanceStateInner();
        break;

      case THREADSTATE_RUN_READY:
        // The pauser should be blocking us whenever this is true!
        assert(false);
        break;

      case THREADSTATE_RUNNING:
        if (m_executeOwnEvents)
        {
          // It's showtime!
          bool locked = false;
          Dir lockRegion = Dirs::NORTH;
          /*
          UsageTimer execTimer = UsageTimer::NowThread();
          */

          CreateRandomWindow();

          if (IsInHidden(m_executingWindow.GetCenterInTile()) ||
               !HasAnyConnections(lockRegion = VisibleAt(m_executingWindow.GetCenterInTile())) ||
               (locked = LockRegion(lockRegion)))
          {
            DoEvent(locked, lockRegion);
          }

          /*
          u32 ms = (UsageTimer::NowThread() - execTimer).TotalMicroseconds();
          if(ms > 1)
          {
            LOG.Debug("Atom (type 0x%x) took longer 1 ms to execute: %d ms",
                      m_executingWindow.GetCenterAtom().GetType(), ms);
          }
          */
        }
        else
        {
          FlushAndWaitOnAllBuffers(0);
        }
        break;

      case THREADSTATE_PAUSE_REQUESTED:
        // Confirm we are done with our event (including processing
        // any needed inbound ACKs, and freeing connection locks) by
        // advancing to pause ready.
        m_isA2PRed = true;
        m_threadPauser.AdvanceStateInner();
        m_isA2PRed = false;
        break;

      case THREADSTATE_PAUSE_READY:
        if (FlushAndWaitOnAllBuffers(0))   // Mop up if necessary
        {
          pthread_yield();                 // And try to hurry others
        }
        break;

      case THREADSTATE_PAUSED:
        // The pauser should be blocking us whenever this is true!
        assert(false);
        break;

      default:
        assert(false);
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
    if (idx < 0)
    {
      return 0;
    }
    return m_atomCount[idx];
  }

  template <class CC>
  void Tile<CC>::SetAtomCount(ElementType atomType, s32 count)
  {
    s32 idx = elementTable.GetIndex(atomType);
    if (idx < 0)
    {
      if (count > 0)
      {
        FAIL(ILLEGAL_STATE);
      }
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
      if (pthread_create(&m_thread, NULL, ExecuteThreadHelper, this))
        FAIL(ILLEGAL_STATE);
    }

    m_threadPauser.RequestRun();
  }

  template <class CC>
  void Tile<CC>::Pause()
  {
    // This is kind of the only place we can check this -- running
    // under the outer (grid) thread rather than the tile -- because
    // the inner (tile) thread blocks as soon as we pause it..
    ReportIfBuffersAreNonEmpty();
    m_threadPauser.Pause();
  }

  /*
  */

  template <class CC>
  bool Tile<CC>::IsPauseReady()
  {
    return m_threadPauser.IsPauseReady();
  }

  template <class CC>
  void Tile<CC>::RequestPause()
  {
    m_threadPauser.RequestPause();
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
      LOG.Warning("LOST ATOMS %x %d %d (Tile %s) - requesting recount",
                  (int) atomType,delta,m_atomCount[idx],this->GetLabel());
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
  void Tile<CC>::ReportTileStatus(Logger::Level level)
  {
    LOG.Log(level," ===TILE %s STATUS REPORT===", m_label.GetZString());

    LOG.Log(level,"  ==Tile %s Global==", m_label.GetZString());
    LOG.Log(level,"   Address: %p", (void*) this);
    LOG.Log(level,"   Thread id: %p", (void*) m_thread);
    LOG.Log(level,"   Error stack top: %p", (void*) m_errorEnvironmentStackTop);
    LOG.Log(level,"   Background radiation: %s", m_backgroundRadiationEnabled?"true":"false");

    LOG.Log(level,"   In FnWOAB: %s (cur 0x%x, orig 0x%x)",
            m_isFnWing?"true":"false", m_curFnWingDirWaitWord, m_origFnWingDirWaitWord);
    LOG.Log(level,"   In A2PRed: %s", m_isA2PRed?"true":"false");
    LOG.Log(level,"   In GSBIsw: %s", m_isGSBIsw?"true":"false");

    LOG.Log(level,"  ==Tile %s Thread==", m_label.GetZString());
    m_threadPauser.ReportThreadPauserStatus(level);

    LOG.Log(level,"  ==Tile %s Atomic==", m_label.GetZString());
    LOG.Log(level,"   Recount needed: %s", m_needRecount?"true":"false");
    LOG.Log(level,"   Illegal atom count: %d", m_illegalAtomCount);
    LOG.Log(level,"   Last executed atom at: (%d, %d)", m_lastExecutedAtom.GetX(), m_lastExecutedAtom.GetY());

    LOG.Log(level,"  ==Tile %s Events==", m_label.GetZString());
    LOG.Log(level,"   Events: %dM (total)", (u32) (m_eventsExecuted / 1000000));
    LOG.Log(level,"    Failed events: %d", m_eventsFailed);
    LOG.Log(level,"    Failures erased: %d", m_failuresErased);

    const int ONE_MILLION = 1000000;
    for (u32 r = 0; r < REGION_COUNT; ++r)
    {
      const char * lab = "";
      switch (r) {
      case REGION_CACHE: lab = "Cache"; break;
      case REGION_SHARED: lab = "Shared"; break;
      case REGION_VISIBLE: lab = "Visible"; break;
      case REGION_HIDDEN: lab = "Hidden"; break;
      }
      LOG.Log(level,"   Events: %dM (%s)", (u32) (m_regionEvents[r] / ONE_MILLION), lab);
    }
    LOG.Log(level,"   Event locks attempted: %dM", (u32) (m_lockAttempts / ONE_MILLION));
    LOG.Log(level,"   Event locks succeeded: %dM", (u32) (m_lockAttemptsSucceeded / ONE_MILLION));

    for (u32 r = 0; r < LOCKTYPE_COUNT; ++r)
    {
      const char * lab = "";
      switch (r) {
      case LOCKTYPE_NONE: lab = "No"; break;
      case LOCKTYPE_SINGLE: lab = "Single"; break;
      case LOCKTYPE_TRIPLE: lab = "Triple"; break;
      }
      LOG.Log(level,"   %s lock events: %dM", lab, (u32) (m_lockEvents[r] / ONE_MILLION));
    }

    LOG.Log(level,"  ==Tile %s Connections==", m_label.GetZString());
    LOG.Log(level,"   -Connection locks held-");
    for (u32 r = 0; r < Dirs::DIR_COUNT; ++r)
    {
      const char * lab = Dirs::GetName(r);
      LOG.Log(level,"    %s from %s: %s",
              lab, m_label.GetZString(),
              m_iLocked[r]?"LOCKED":"unlocked");
    }
    LOG.Log(level,"   -Connection details-");
    for (u32 r = 0; r < Dirs::DIR_COUNT; ++r)
    {
      const char * lab = Dirs::GetName(r);
      Connection * c = m_connections[r];
      if (!c)
      {
        LOG.Log(level,"   %s: No connection", lab);
      }
      else
      {
        bool iOwnThis = r >= Dirs::EAST && r <= Dirs::SOUTHWEST;
        LOG.Log(level,"   %s from %s%s: %p",
                lab, m_label.GetZString(), iOwnThis? " (owned)" : "", (void*) c);
        c->ReportConnectionStatus(level, iOwnThis);
      }
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
      LOG.Message("Recounting atoms (Tile %s)", this->GetLabel());
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

#include "MDist.h"      /* -*- C++ -*- */
#include "Element_Empty.h"
#include "Util.h"

namespace MFM {

  template <class T, u32 R>
  Tile<T,R>::Tile() : m_executingWindow(*this)
  {
    Reinit();
  }

  template <class T, u32 R>
  void Tile<T,R>::Reinit() 
  {
    elementTable.Reinit();

    m_eventsExecuted = 0;

    for(u32 i = 0; i < ELEMENT_TABLE_SIZE; i++)
    {
      m_atomCount[i] = 0;
    }

    RegisterElement(Element_Empty<T,R>::THE_INSTANCE);

    SetAtomCount(ELEMENT_EMPTY,OWNED_SIDE*OWNED_SIDE);

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
    for(u32 x = 0; x < TILE_WIDTH - 2 * R; x++)
    {
      for(u32 y = 0; y < TILE_WIDTH - 2 * R; y++)
      {
	m_siteEvents[x][y] = 0;
      }
    }

    m_threadInitialized = false;
    m_threadPaused = false;
  }

  template <class T, u32 R>
  void Tile<T,R>::Connect(Tile<T,R>& other, Dir toCache)
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

  template <class T, u32 R>
  Connection* Tile<T,R>::GetConnection(Dir cache)
  {
    return m_connections[cache];
  }

  template <class T, u32 R>
  Random& Tile<T,R>::GetRandom()
  {
    return m_random;
  }

  template <class T,u32 R>
  const T* Tile<T,R>::GetAtom(const SPoint& pt) const
  {
    return GetAtom(pt.GetX(), pt.GetY());
  }

  template <class T,u32 R>
  const T* Tile<T,R>::GetUncachedAtom(const SPoint& pt) const
  {
    return GetUncachedAtom(pt.GetX(), pt.GetY());
  }

  template <class T,u32 R>
  const T* Tile<T,R>::GetUncachedAtom(s32 x, s32 y) const
  {
    return GetAtom(x+R, y+R);
  }

  template <class T,u32 R>
  const T* Tile<T,R>::GetAtom(s32 x, s32 y) const
  {
    if (x < 0 || y < 0 || x >= TILE_WIDTH || y >= TILE_WIDTH)
      FAIL(ARRAY_INDEX_OUT_OF_BOUNDS);
    return GetAtom(x + y * TILE_WIDTH);
  }

  template <class T,u32 R>
  const T* Tile<T,R>::GetAtom(s32 i) const
  {
    if (i < 0 || i >= TILE_SIZE)
      FAIL(ARRAY_INDEX_OUT_OF_BOUNDS);
    return &m_atoms[i];
  }

  template <class T, u32 R>
  void Tile<T,R>::SendAcknowledgmentPacket(Packet<T>& packet)
  {
    Dir from = Dirs::OppositeDir(packet.GetReceivingNeighbor());
    Packet<T> sendout(PACKET_EVENT_ACKNOWLEDGE);
    sendout.SetReceivingNeighbor(from);

    m_connections[from]->Write(!IS_OWNED_CONNECTION(from),
			       (u8*)&sendout,
			       sizeof(Packet<T>));
  }

  template <class T, u32 R>
  void Tile<T,R>::ReceivePacket(Packet<T>& packet)
  {
    switch(packet.GetType())
    {
    case PACKET_WRITE:
      PlaceAtom(packet.GetAtom(), packet.GetLocation());
      break;
    case PACKET_EVENT_COMPLETE:
      SendAcknowledgmentPacket(packet);
      break;
    case PACKET_EVENT_ACKNOWLEDGE:

      break;
    default:
      FAIL(INCOMPLETE_CODE); break;
    }
  }

  template <class T,u32 R>
  void Tile<T,R>::FillLastExecutedAtom(SPoint& out)
  {
    out.Set(m_lastExecutedAtom.GetX(),
	    m_lastExecutedAtom.GetY());
  }

  template <class T,u32 R>
  void Tile<T,R>::CreateRandomWindow()
  {
    /* Make sure not to be created in the cache */
    int maxval = TILE_WIDTH - (EVENT_WINDOW_RADIUS << 1);
    SPoint pt(GetRandom(), maxval, maxval);
    pt.Add(EVENT_WINDOW_RADIUS, EVENT_WINDOW_RADIUS);

    m_executingWindow.SetCenter(pt);
  }

  template <class T,u32 R>
  void Tile<T,R>::CreateWindowAt(const SPoint& pt)
  {
    m_executingWindow.SetCenter(pt);
  }

  template <class T, u32 R>
  Dir Tile<T,R>::RegionAt(const SPoint& sp, u32 reach)
  {
    UPoint pt = makeUnsigned(sp);

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

  template <class T, u32 R>
  Dir Tile<T,R>::CacheAt(const SPoint& pt)
  {
    return RegionAt(pt, R);
  }

  template <class T, u32 R>
  Dir Tile<T,R>::SharedAt(const SPoint& pt)
  {
    return RegionAt(pt, R * 3);
  }

  template <class T,u32 R>
  void Tile<T,R>::PlaceAtom(const T& atom, const SPoint& pt)
  {
    const T* oldAtom = GetAtom(pt);
    u32 oldType = oldAtom->GetType();

    m_atoms[pt.GetX() +
            pt.GetY() * TILE_WIDTH] = atom;

    if(!IsInCache(pt))
    {
      IncrAtomCount(atom.GetType(),1);
      IncrAtomCount(oldType,-1);
    }
  }

  template <class T, u32 R>
  void Tile<T, R>::SendAtom(Dir neighbor, SPoint& atomLoc)
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

      sendout.SetLocation(remoteLoc);
      sendout.SetAtom(*GetAtom(atomLoc));
      sendout.SetReceivingNeighbor(neighbor);

      /* Send out the serialized version of the packet */
      m_connections[neighbor]->Write(!IS_OWNED_CONNECTION(neighbor),
				     (u8*)&sendout,
				     sizeof(Packet<T>));
    }
  }

  template <class T, u32 R>
  bool Tile<T,R>::IsConnected(Dir dir)
  {
    return m_connections[dir] != NULL &&
      m_connections[dir]->IsConnected();
  }

  template <class T, u32 R>
  bool Tile<T,R>::IsLiveSite(const SPoint & location)
  {
    return IsInTile(location) &&
      (!IsInCache(location) || IsConnected(CacheAt(location)));
  }

  template <class T, u32 R>
  bool Tile<T,R>::IsInCache(const SPoint& pt)
  {
    int upbnd = TILE_WIDTH - R;
    return (u32)pt.GetX() < R || (u32)pt.GetY() < R ||
      pt.GetX() >= upbnd || pt.GetY() >= upbnd;
  }

  template <class T, u32 R>
  bool Tile<T,R>::IsInTile(const SPoint& pt)
  {
    return ((u32) pt.GetX()) < TILE_WIDTH && ((u32) pt.GetY() < TILE_WIDTH);
  }

  template <class T, u32 R>
  void Tile<T,R>::SendEndEventPackets(u32 dirWaitWord)
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

  template <class T, u32 R>
  u32 Tile<T,R>::SendRelevantAtoms()
  {
    SPoint localLoc;
    SPoint ewCenter;

    u32 dirBitfield = 0;

    s32 r2 = R * 2;
    m_executingWindow.FillCenter(ewCenter);

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
	else if(IsConnected(Dirs::SOUTH) && localLoc.GetY() >= TILE_WIDTH - r2)
	{
	  SendAtom(Dirs::SOUTHWEST, localLoc);
	  SendAtom(Dirs::SOUTH, localLoc);
	  dirBitfield = Dirs::AddDirToMask(dirBitfield, Dirs::SOUTHWEST);
	  dirBitfield = Dirs::AddDirToMask(dirBitfield, Dirs::SOUTH);
	}
      }
      /*East neighbor?*/
      else if(IsConnected(Dirs::EAST) && localLoc.GetX() >= TILE_WIDTH - r2)
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
	if(IsConnected(Dirs::SOUTH) && localLoc.GetY() >= TILE_WIDTH - r2)
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
      else if(IsConnected(Dirs::SOUTH) && localLoc.GetY() >= TILE_WIDTH - r2)
      {
	SendAtom(Dirs::SOUTH, localLoc);
	dirBitfield = Dirs::AddDirToMask(dirBitfield, Dirs::SOUTH);
      }
    }
    return dirBitfield;
  }

  template <class T, u32 R>
  bool Tile<T,R>::TryLock(Dir connectionDir)
  {
    return IsConnected(connectionDir) &&
      m_connections[connectionDir]->Lock();
  }

  template <class T, u32 R>
  bool Tile<T,R>::TryLockCorner(Dir cornerDir)
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

  template <class T, u32 R>
  bool Tile<T,R>::LockRegion(Dir regionDir)
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

  template <class T, u32 R>
  void Tile<T,R>::UnlockCorner(Dir corner)
  {
    corner = Dirs::CCWDir(corner);
    for(u32 i = 0; i < 3; i++)
    {
      m_connections[corner]->Unlock();
      corner = Dirs::CWDir(corner);
    }
  }

  template <class T, u32 R>
  void Tile<T,R>::UnlockRegion(Dir regionDir)
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

  template <class T, u32 R>
  bool Tile<T,R>::IsInHidden(const SPoint& pt)
  {
    return pt.GetX() >= (s32)R * 3 && pt.GetX() < TILE_WIDTH - (s32)R * 3 &&
      pt.GetY() >= (s32)R * 3 && pt.GetY() < TILE_WIDTH - (s32)R * 3;
  }

  template <class T, u32 R>
  TileRegion Tile<T,R>::RegionFromIndex(const u32 index)
  {
    if(index > TILE_WIDTH)
    {
      FAIL(ARRAY_INDEX_OUT_OF_BOUNDS); /* Index out of Tile bounds */
    }

    const u32 hiddenWidth = TILE_WIDTH - R * 6;

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

  template <class T, u32 R>
  TileRegion Tile<T,R>::RegionIn(const SPoint& pt)
  {
    return MIN(RegionFromIndex((u32)pt.GetX()),
               RegionFromIndex((u32)pt.GetY()));
  }

  template <class T, u32 R>
  void Tile<T,R>::FlushAndWaitOnAllBuffers(u32 dirWaitWord)
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
	      FAIL(ILLEGAL_STATE); /* Didn't read enough for a full packet! */
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
      }
      pthread_yield();
      m_threadPauser.WaitIfPaused();
    } while(dirWaitWord);
  }

  template <class T,u32 R>
  void Tile<T,R>::Execute()
  {
    while(m_threadInitialized)
    {
      /*Change to 0 if placing a window in a certain place*/
#if 1
      CreateRandomWindow();
#else
      SPoint winCenter(R * 2 - 1, R);
      CreateWindowAt(winCenter);
#endif

      bool locked = false;
      Dir lockRegion = Dirs::NORTH;
      u32 dirWaitWord = 0;
      if(IsInHidden(m_executingWindow.GetCenter()) ||
	 !IsConnected(lockRegion = SharedAt(m_executingWindow.GetCenter())) ||
	 (locked = LockRegion(lockRegion)))
	{
	  elementTable.Execute(m_executingWindow);

	  // XXX INSANE SLOWDOWN FOR DEBUG: AssertValidAtomCounts();

	  m_executingWindow.FillCenter(m_lastExecutedAtom);

	  dirWaitWord = SendRelevantAtoms();

	  SendEndEventPackets(dirWaitWord);

	  FlushAndWaitOnAllBuffers(dirWaitWord);

	  ++m_eventsExecuted;
	  ++m_regionEvents[RegionIn(m_executingWindow.GetCenter())];

	  ++m_siteEvents[m_executingWindow.GetCenter().GetX() - R]
	                [m_executingWindow.GetCenter().GetY() - R];

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

  template <class T, u32 R>
  void* Tile<T,R>::ExecuteThreadHelper(void* tilePtr)
  {
    ((Tile*)tilePtr)->Execute();
    return NULL;
  }

  template <class T, u32 R>
  u32 Tile<T,R>::GetAtomCount(ElementType atomType)
  {
    s32 idx = elementTable.GetIndex(atomType);
    if (idx < 0) return 0;
    return m_atomCount[idx];
  }

  template <class T, u32 R>
  void Tile<T,R>::SetAtomCount(ElementType atomType, s32 count)
  {
    s32 idx = elementTable.GetIndex(atomType);
    if (idx < 0) {
      if (count > 0) FAIL(ILLEGAL_STATE);
      return;
    }
    m_atomCount[idx] = count;
  }
  template <class T, u32 R>
  void Tile<T,R>::Start()
  {
    if(!m_threadInitialized)
    {
      AssertValidAtomCounts();
      m_threadInitialized = true;
      pthread_create(&m_thread, NULL, ExecuteThreadHelper, this);
    }
    else
    {
      m_threadPauser.Unpause();
    }
  }

  template <class T, u32 R>
  void Tile<T,R>::Pause()
  {
    m_threadPauser.Pause();
  }

  template <class T, u32 R>
  void Tile<T,R>::IncrAtomCount(ElementType atomType, s32 delta)
  {
    s32 idx = elementTable.GetIndex(atomType);
    if (idx < 0) {
      if (delta != 0) FAIL(ILLEGAL_STATE);
      return;
    }
    if (delta < 0 && -delta > m_atomCount[idx]) {
      fprintf(stderr, "LOST ATOMS %x %d %d (Tile %p)\n",
              (int) atomType,delta,m_atomCount[idx],(void*) this);
      FAIL(ILLEGAL_ARGUMENT);
    }

    if (delta < 0)
      m_atomCount[idx] -= -delta;
    else
      m_atomCount[idx] += delta;

  }

  template <class T, u32 R>
  u64 Tile<T,R>::WriteEPSRasterLine(FILE* outstrm, u32 lineIdx)
  {
    u64 max = 0;
    for(u32 x = 0; x < TILE_WIDTH - 2 * R; x++)
    {
      fprintf(outstrm, "%d ", (u32)m_siteEvents[x][lineIdx]);
      max = MAX(max, m_siteEvents[x][lineIdx]);
    }
    return max;
  }

  template <class T, u32 R>
  void Tile<T,R>::AssertValidAtomCounts() const
  {
    s32 counts[ELEMENT_TABLE_SIZE];
    for (u32 i = 0; i < ELEMENT_TABLE_SIZE; ++i)
      counts[i] = 0;
    for (u32 x = 0; x < OWNED_SIDE; ++x)
      for (u32 y = 0; y < OWNED_SIDE; ++y) {
        const Atom<T,R> * atom = GetUncachedAtom(x,y);
        s32 type = elementTable.GetIndex(atom->GetType());
        if (type < 0)
          FAIL(ILLEGAL_STATE);
        counts[type]++;
    }
    for (u32 i = 0; i < ELEMENT_TABLE_SIZE; ++i)
      if (counts[i] != m_atomCount[i])
        FAIL(ILLEGAL_STATE);
  }

} /* namespace MFM */

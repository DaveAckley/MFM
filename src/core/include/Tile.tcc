#include "manhattandir.h"      /* -*- C++ -*- */
#include "element_empty.h"

namespace MFM {

/*
template <class T, u32 R>
static u32 UninitializedStateFunc(T* atom) {
  FAIL(UNINITIALIZED_VALUE);
}
*/

template <class T, u32 R>
Tile<T,R>::Tile() : m_eventsExecuted(0), m_executingWindow(*this)
{
  for(u32 i = 0; i < ELEMENT_TABLE_SIZE; i++)
  {
    m_atomCount[i] = 0;
  }

  RegisterElement(Element_Empty<T,R>::THE_INSTANCE);

  SetAtomCount(ELEMENT_NOTHING,TILE_SIZE * TILE_SIZE);

  /* Set up our connection pointers. Some of these may remain NULL, */
  /* symbolizing a dead edge.       */
  u32 edges = 0;
  for(EuclidDir i = EUDIR_NORTH; edges < EUDIR_COUNT; i = EuDir::CWDir(i), edges++)
  {
    if(IS_OWNED_CONNECTION(i))
    {
      /* We own this one! Hook it up. */
      m_connections[i] = m_ownedConnections + i - EUDIR_EAST;
    }
    else
    {
      /* We will rely on the grid to hook these up when the time
	 comes. */
      m_connections[i] = NULL;
    }
  }

  m_threadInitialized = false;
  m_threadPaused = false;
}

template <class T, u32 R>
void Tile<T,R>::Connect(Tile<T,R>& other, EuclidDir toCache)
{
  if(IS_OWNED_CONNECTION(toCache))
  {
    m_connections[toCache]->SetConnected(true);
  }
  else
  {
    m_connections[toCache] = other.GetConnection(EuDir::OppositeDir(toCache));
    m_connections[toCache]->SetConnected(true);
    other.Connect(*this, EuDir::OppositeDir(toCache));
  }
}

template <class T, u32 R>
Connection* Tile<T,R>::GetConnection(EuclidDir cache)
{
  return m_connections[cache];
}

template <class T, u32 R>
Random& Tile<T,R>::GetRandom()
{
  return m_random;
}

template <class T,u32 R>
T* Tile<T,R>::GetAtom(const SPoint& pt)
{
  return GetAtom(pt.GetX(), pt.GetY());
}

template <class T,u32 R>
T* Tile<T,R>::GetAtom(s32 x, s32 y)
{
  if (x < 0 || y < 0 || x >= TILE_WIDTH || y >= TILE_WIDTH)
    FAIL(ARRAY_INDEX_OUT_OF_BOUNDS);
  return GetAtom(x + y * TILE_WIDTH);
}

template <class T,u32 R>
T* Tile<T,R>::GetAtom(s32 i)
{
  if (i < 0 || i >= TILE_SIZE)
    FAIL(ARRAY_INDEX_OUT_OF_BOUNDS);
  return &m_atoms[i];
}

template <class T, u32 R>
void Tile<T,R>::ReceivePacket(Packet<T>& packet)
{
  switch(packet.GetType())
  {
  case PACKET_WRITE:
    PlaceAtom(packet.GetAtom(), packet.GetLocation());
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
EuclidDir Tile<T,R>::RegionAt(const SPoint& sp, u32 reach)
{
  UPoint pt = makeUnsigned(sp);

  if(pt.GetX() < reach)
  {
    if(pt.GetY() < reach)
    {
      return EUDIR_NORTHWEST;
    }
    else if(pt.GetY() >= TILE_WIDTH - reach)
    {
      return EUDIR_SOUTHWEST;
    }
    return EUDIR_WEST;
  }
  else if(pt.GetX() >= TILE_WIDTH - reach)
  {
    if(pt.GetY() < reach)
    {
      return EUDIR_NORTHEAST;
    }
    else if(pt.GetY() >= TILE_WIDTH - reach)
    {
      return EUDIR_SOUTHEAST;
    }
    return EUDIR_EAST;
  }

  if(pt.GetY() < reach)
  {
    return EUDIR_NORTH;
  }
  else if(pt.GetY() >= TILE_WIDTH - reach)
  {
    return EUDIR_SOUTH;
  }

  return (EuclidDir)-1;
}

template <class T, u32 R>
EuclidDir Tile<T,R>::CacheAt(const SPoint& pt)
{
  return RegionAt(pt, R);
}

template <class T, u32 R>
EuclidDir Tile<T,R>::SharedAt(const SPoint& pt)
{
  return RegionAt(pt, R * 3);
}

template <class T,u32 R>
void Tile<T,R>::PlaceAtom(const T& atom, const SPoint& pt)
{
  T* oldAtom = GetAtom(pt);
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
void Tile<T, R>::SendAtom(EuclidDir neighbor, SPoint& atomLoc)
{
  if(IsConnected(neighbor))
  {
    SPoint remoteLoc(atomLoc);

    u32 tileDiff = TILE_WIDTH - 2 * R;

    /* The neighbor will think this atom is in a different location. */
    switch(neighbor)
    {
    case EUDIR_NORTH: remoteLoc.Add(0, tileDiff); break;
    case EUDIR_SOUTH: remoteLoc.Add(0, -tileDiff); break;
    case EUDIR_WEST:  remoteLoc.Add(tileDiff, 0); break;
    case EUDIR_EAST:  remoteLoc.Add(-tileDiff, 0); break;
    case EUDIR_NORTHEAST:
      remoteLoc.Add(-tileDiff, tileDiff); break;
    case EUDIR_SOUTHEAST:
      remoteLoc.Add(-tileDiff, -tileDiff); break;
    case EUDIR_SOUTHWEST:
      remoteLoc.Add(tileDiff, -tileDiff); break;
    case EUDIR_NORTHWEST:
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
bool Tile<T,R>::IsConnected(EuclidDir dir)
{
  return m_connections[dir] != NULL &&
         m_connections[dir]->IsConnected();
}

template <class T, u32 R>
bool Tile<T,R>::IsInCache(const SPoint& pt)
{
  int upbnd = TILE_WIDTH - R;
  return (u32)pt.GetX() < R || (u32)pt.GetY() < R ||
    pt.GetX() >= upbnd || pt.GetY() >= upbnd;
}

/*
 * This examines the current event window and sends all atoms
 *  within this tile's shared section of memory. This ensures
 *  that any and all atoms that need to be sent are sent.
 */
template <class T, u32 R>
void Tile<T,R>::SendRelevantAtoms()
{
  SPoint localLoc;
  SPoint ewCenter;

  s32 r2 = R * 2;

  m_executingWindow.FillCenter(ewCenter);

  for(u32 i = 0; i < m_executingWindow.GetAtomCount(); i++)
  {
    ManhattanDir<R>::get().FillFromBits(localLoc, i, (TableType)R);
    localLoc.Add(ewCenter);

    /* Send to West neighbor? */
    if(localLoc.GetX() < r2)
    {
      SendAtom(EUDIR_WEST, localLoc);
      if(localLoc.GetY() < r2)
      {
	SendAtom(EUDIR_NORTHWEST, localLoc);
	SendAtom(EUDIR_NORTH, localLoc);
      }
      else if(localLoc.GetY() >= TILE_WIDTH - r2)
      {
	SendAtom(EUDIR_SOUTHWEST, localLoc);
	SendAtom(EUDIR_SOUTH, localLoc);
      }
    }
    /*East neighbor?*/
    else if(localLoc.GetX() >= TILE_WIDTH - r2)
    {
      SendAtom(EUDIR_EAST, localLoc);
      if(localLoc.GetY() < r2)
      {
	SendAtom(EUDIR_NORTHEAST, localLoc);
	SendAtom(EUDIR_NORTH, localLoc);
      }
      if(localLoc.GetY() >= TILE_WIDTH - r2)
      {
	SendAtom(EUDIR_SOUTHEAST, localLoc);
	SendAtom(EUDIR_SOUTH, localLoc);
      }
    }
    else if(localLoc.GetY() < r2)
    {
      SendAtom(EUDIR_NORTH, localLoc);
    }
    else if(localLoc.GetY() >= TILE_WIDTH - r2)
    {
      SendAtom(EUDIR_SOUTH, localLoc);
    }

  }
}

template <class T, u32 R>
bool Tile<T,R>::TryLock(EuclidDir connectionDir)
{
  return IsConnected(connectionDir) &&
    m_connections[connectionDir]->Lock();
}

template <class T, u32 R>
bool Tile<T,R>::TryLockCorner(EuclidDir cornerDir)
{
  u32 locked = 0;

  /* Go back one, then wind until we lock all three. */
  cornerDir = EuDir::CCWDir(cornerDir);
  for(u32 i = 0; i < 3; i++)
  {
    if(TryLock(cornerDir))
    {
      locked++;
      cornerDir = EuDir::CWDir(cornerDir);
    }
    /* If we can't hit one, rewind, unlocking all held locks. */
    else
    {
      for(u32 j = 0; j < locked; j++)
      {
	cornerDir = EuDir::CCWDir(cornerDir);
	m_connections[cornerDir]->Unlock();
      }
      return false;
    }
  }
  return true;
}

template <class T, u32 R>
bool Tile<T,R>::LockRegion(EuclidDir regionDir)
{
  switch(regionDir)
  {
  case EUDIR_NORTH:
  case EUDIR_EAST:
  case EUDIR_SOUTH:
  case EUDIR_WEST:
    return TryLock(regionDir);

  case EUDIR_NORTHWEST:
  case EUDIR_NORTHEAST:
  case EUDIR_SOUTHEAST:
  case EUDIR_SOUTHWEST:
    return TryLockCorner(regionDir);

  default:
    FAIL(ILLEGAL_ARGUMENT);
  }
}

template <class T, u32 R>
void Tile<T,R>::UnlockCorner(EuclidDir corner)
{
  corner = EuDir::CCWDir(corner);
  for(u32 i = 0; i < 3; i++)
  {
    m_connections[corner]->Unlock();
    corner = EuDir::CWDir(corner);
  }
}

template <class T, u32 R>
void Tile<T,R>::UnlockRegion(EuclidDir regionDir)
{
  switch(regionDir)
  {
  case EUDIR_NORTH:
  case EUDIR_EAST:
  case EUDIR_SOUTH:
  case EUDIR_WEST:
    m_connections[regionDir]->Unlock();
    return;

  case EUDIR_NORTHWEST:
  case EUDIR_NORTHEAST:
  case EUDIR_SOUTHEAST:
  case EUDIR_SOUTHWEST:
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
    EuclidDir lockRegion = EUDIR_NORTH;
    if(IsInHidden(m_executingWindow.GetCenter()) ||
       !IsConnected(lockRegion = SharedAt(m_executingWindow.GetCenter())) ||
       (locked = LockRegion(lockRegion)))
    {
      elementTable.Execute(m_executingWindow);

      m_executingWindow.FillCenter(m_lastExecutedAtom);

      SendRelevantAtoms();

      ++m_eventsExecuted;

      if(locked)
      {
	UnlockRegion(lockRegion);
      }
    }

    Packet<T> readPack(PACKET_WRITE);
    u32 readBytes;

    /* Flush out all packet buffers */
    for(EuclidDir dir = EUDIR_NORTH; dir < EUDIR_COUNT; dir = (EuclidDir)(dir + 1))
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
	  ReceivePacket(readPack);
	}
      }
    }
    /* Let's try to give someone else a chance. */
    pthread_yield();
    m_threadPauser.WaitIfPaused();
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
  if (delta < 0 && -delta > m_atomCount[idx])
    FAIL(ILLEGAL_ARGUMENT);

  if (delta < 0)
    m_atomCount[idx] -= -delta;
  else
    m_atomCount[idx] += delta;

}
} /* namespace MFM */

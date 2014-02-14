#include "manhattandir.h"      /* -*- C++ -*- */

namespace MFM {

template <class T, u32 R>
static u32 UninitializedStateFunc(T* atom) {
  FAIL(UNINITIALIZED_VALUE);
}

template <class T, u32 R>
Tile<T,R>::Tile() : m_executingWindow(*this), m_stateFunc(&UninitializedStateFunc<T,R>)
{
  m_atomCount[ELEMENT_NOTHING] = TILE_SIZE;
  
  for(s32 i = 1; i < ELEMENT_COUNT; i++)
  {
    m_atomCount[i] = 0;
  }
}

template <class T, u32 R>
Random& Tile<T,R>::GetRandom()
{
  return m_random;
}

template <class T, u32 R>
void Tile<T,R>::SetNeighbors(u8 neighbors)
{
  m_neighborConnections = neighbors;
}

template <class T,u32 R>
T* Tile<T,R>::GetAtoms()
{
  return m_atoms;
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

template <class T, u32 R>
Packet<T>* Tile<T,R>::NextPacket()
{
  if(m_outgoingPackets.PacketsHeld())
  {
    return m_outgoingPackets.PopPacket();
  }
  return NULL;
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
EuclidDir Tile<T,R>::CacheAt(SPoint& sp)
{
  UPoint pt = makeUnsigned(sp);

  if(pt.GetX() < R)
  {
    if(pt.GetY() < R)
    {
      return EUDIR_NORTHWEST;
    }
    else if(pt.GetY() >= TILE_WIDTH - R)
    {
      return EUDIR_SOUTHWEST;
    }
    return EUDIR_WEST;
  }
  else if(pt.GetX() >= TILE_WIDTH - R)
  {
    if(pt.GetY() < R)
    {
      return EUDIR_NORTHEAST;
    }
    else if(pt.GetY() >= TILE_WIDTH - R)
    {
      return EUDIR_SOUTHEAST;
    }
    return EUDIR_EAST;
  }
  
  if(pt.GetY() < R)
  {
    return EUDIR_NORTH;
  }
  else if(pt.GetY() >= TILE_WIDTH - R)
  {
    return EUDIR_SOUTH;
  }

  return (EuclidDir)-1;
}

template <class T,u32 R>
void Tile<T,R>::PlaceAtom(T& atom, const SPoint& pt)
{
  T* oldAtom = GetAtom(pt);
  u32 type = m_stateFunc(oldAtom);

  m_atomCount[type]--;

  m_atoms[pt.GetX() + 
	  pt.GetY() * TILE_WIDTH] = atom;  

  m_atomCount[m_stateFunc(&atom)]++;
}

template <class T, u32 R>
void Tile<T, R>::SendAtom(EuclidDir neighbor, SPoint& atomLoc)
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

  m_outgoingPackets.PushPacket(sendout);
}

template <class T, u32 R>
bool Tile<T,R>::IsConnected(EuclidDir dir)
{
  return m_neighborConnections & (1 << ((u8)dir >> 1));
}

template <class T, u32 R>
bool Tile<T,R>::IsInCache(SPoint& pt)
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

template <class T,u32 R>
void Tile<T,R>::Execute(ElementTable<T,R>& table)
{
  /*Change to 0 if placing a window in a certain place*/
#if 1
  CreateRandomWindow();
#else
  SPoint winCenter(R * 2 - 1, R);
  CreateWindowAt(winCenter);
#endif
  
  table.Execute(m_executingWindow);

  m_executingWindow.FillCenter(m_lastExecutedAtom);

  SendRelevantAtoms();
}

template <class T, u32 R>
u32 Tile<T,R>::GetAtomCount(ElementType atomType)
{
  return m_atomCount[(u32)atomType];
}
} /* namespace MFM */

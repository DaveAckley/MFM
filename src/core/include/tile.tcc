#include "manhattandir.h"      /* -*- C++ -*- */

template <class T, u32 R>
Tile<T,R>::Tile()
{
  m_atomCount[ELEMENT_NOTHING] = TILE_SIZE;
  
  for(s32 i = 1; i < ELEMENT_COUNT; i++)
  {
    m_atomCount[i] = 0;
  }
}

template <class T, u32 R>
void Tile<T,R>::SetNeighbors(u8 neighbors)
{
  m_neighborConnections = neighbors;
}

template <class T,u32 R>
T* Tile<T,R>::GetAtom(Point<int>* pt)
{
  return &m_atoms[pt->GetX() + 
		  pt->GetY() * TILE_WIDTH];
}

template <class T,u32 R>
T* Tile<T,R>::GetAtom(int x, int y)
{
  return &m_atoms[x + y * TILE_WIDTH];
}

template <class T,u32 R>
T* Tile<T,R>::GetAtom(int i)
{
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
void Tile<T,R>::FillLastExecutedAtom(Point<int>& out)
{
  out.Set(m_lastExecutedAtom.GetX(),
	  m_lastExecutedAtom.GetY());
}

template <class T,u32 R>
void Tile<T,R>::CreateRandomWindow()
{
  /* Make sure not to be created in the cache */
  int maxval = TILE_WIDTH - (EVENT_WINDOW_RADIUS << 1);
  Point<int> pt(true, maxval, maxval);
  pt.Add(EVENT_WINDOW_RADIUS, EVENT_WINDOW_RADIUS);

  m_executingWindow = EventWindow<T,R>(pt, m_atoms, TILE_WIDTH,
				       m_neighborConnections);
}

template <class T,u32 R>
void Tile<T,R>::CreateWindowAt(Point<int>& pt)
{
  m_executingWindow =  EventWindow<T,R>(pt, m_atoms, TILE_WIDTH,
				       m_neighborConnections);
}

template <class T, u32 R>
EuclidDir Tile<T,R>::CacheAt(Point<int>& pt)
{
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
void Tile<T,R>::PlaceAtom(T& atom, Point<int>& pt)
{
  T* oldAtom = GetAtom(&pt);
  u32 type = m_stateFunc(oldAtom);

  m_atomCount[type]--;

  m_atoms[pt.GetX() + 
	  pt.GetY() * TILE_WIDTH] = atom;  

  m_atomCount[m_stateFunc(&atom)]++;
}

template <class T,u32 R>
void Tile<T,R>::DiffuseAtom(EventWindow<T,R>& window)
{
  Point<int> neighbors[4];
  Point<int> center;
  window.FillCenter(center);

  ManhattanDir<R>::get().FillVNNeighbors(neighbors);
  u8 idx = rand() & 3;

  Point<int> current;

  for(int i = 0; i < 4; i++)
  {
    idx++;
    idx &= 3;
    
    current = center;
    current.Add(neighbors[idx]);

    /* Is this a dead cache? */
    if(IsInCache(current) && 
       !IsConnected(EuDir::FromOffset(neighbors[idx])))
    {
      continue;
    }


    T& atom = window.GetRelativeAtom(neighbors[idx]);

    /* It's empty! Move there! */
    if(m_stateFunc(&atom) == 0)
    {
      Point<int> empty(0, 0);
      window.SwapAtoms(neighbors[idx], empty);
      return;
    }
  }
}

template <class T, u32 R>
void Tile<T, R>::SendAtom(EuclidDir neighbor, Point<int>& atomLoc)
{
  Point<int> remoteLoc(atomLoc);

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
  sendout.SetAtom(*GetAtom(&atomLoc));
  sendout.SetReceivingNeighbor(neighbor);

  m_outgoingPackets.PushPacket(sendout);
}

template <class T, u32 R>
bool Tile<T,R>::IsConnected(EuclidDir dir)
{
  return m_neighborConnections & (1 << ((u8)dir >> 1));
}

template <class T, u32 R>
bool Tile<T,R>::IsInCache(Point<int>& pt)
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
  Point<int> localLoc;
  Point<int> ewCenter;

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
  Point<int> winCenter(R * 2 - 1, R);
  CreateWindowAt(winCenter);
#endif
  
  table.Execute(m_executingWindow);

  if(table.Diffusable((ElementType)
		      m_stateFunc(&m_executingWindow.GetCenterAtom())))
  {
    DiffuseAtom(m_executingWindow);
  }

  m_executingWindow.FillCenter(m_lastExecutedAtom);

  SendRelevantAtoms();
}

template <class T, u32 R>
u32 Tile<T,R>::GetAtomCount(u32 atomType)
{
  return m_atomCount[atomType];
}

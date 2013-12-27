#include <stdio.h>
#include "manhattandir.h"

u32 ManhattanDir::m_eventWindowRadius;
u32 ManhattanDir::m_shortTableSize;
u32 ManhattanDir::m_eventTableSize;
u32 ManhattanDir::m_longTableSize;

Point<int> ManhattanDir::
pointTableShort[MANHATTANDIR_SHORT_TABLE_SIZE];
Point<int> ManhattanDir::
pointTableLong[MANHATTANDIR_LONG_TABLE_SIZE];

Point<int>* ManhattanDir::pointTableEventWindow;

void ManhattanDir::AllocTables(u8 eventWindowRadius)
{
  FillTable(pointTableShort, MANHATTAN_TABLE_SHORT);
  FillTable(pointTableLong, MANHATTAN_TABLE_LONG);

  m_eventWindowRadius = eventWindowRadius;
  m_eventTableSize = ManhattanArea(m_eventWindowRadius);

  pointTableEventWindow = new 
    Point<int>[m_eventTableSize];

  FillTable(pointTableEventWindow, MANHATTAN_TABLE_EVENT);
}

u32 ManhattanDir::GetBondSize(TableType type)
{
  switch(type)
  {
  case MANHATTAN_TABLE_SHORT: return 2;
  case MANHATTAN_TABLE_LONG:  return 4;
  case MANHATTAN_TABLE_EVENT: return m_eventWindowRadius;
  default: return 0;
  }
}

u32 ManhattanDir::GetTableSize(TableType type)
{
  switch(type)
  {
  case MANHATTAN_TABLE_LONG: return m_longTableSize;
  case MANHATTAN_TABLE_SHORT:  return m_shortTableSize;
  case MANHATTAN_TABLE_EVENT: return m_eventTableSize;
  default: return 0;
  }
}

Point<int>* ManhattanDir::GetTable(TableType type)
{
  switch(type)
  {
  case MANHATTAN_TABLE_SHORT: return pointTableShort;
  case MANHATTAN_TABLE_LONG:  return pointTableLong;
  case MANHATTAN_TABLE_EVENT: return pointTableEventWindow;
  default: return NULL;
  }
}

void  ManhattanDir::FillTable(Point<int>* table,
			      TableType type)
{
  int maxDist = GetBondSize(type);
  u8 cidx = 0;
  Point<int> current;
  for(int x = -maxDist; x <= maxDist; x++)
  {
    current.SetX(x);
    for(int y = -maxDist; y <= maxDist; y++)
    {
      current.SetY(y);

      if(current.GetManhattanDistance() <= maxDist)
      {
	table[cidx].SetX(x);
	table[cidx].SetY(y);
	cidx++;
      }
    }
  }

  switch(type)
  {
  case MANHATTAN_TABLE_SHORT:
    m_shortTableSize = cidx; break;
  case MANHATTAN_TABLE_LONG:
    m_longTableSize = cidx; break;
  default:
    m_eventTableSize = cidx; break;
  }
}

void ManhattanDir::DeallocTables()
{
  delete[] pointTableEventWindow;
}

u32 ManhattanDir::ShortTableSize()
{
  return m_shortTableSize;
}

u32 ManhattanDir::LongTableSize()
{
  return m_longTableSize;
}

u8 ManhattanDir::FromPoint(Point<int>& offset, TableType type)
{
  int arrSize = GetTableSize(type);
  Point<int>* arr = GetTable(type);
  for(int i = 0; i < arrSize; i++)
  {
    if(offset.GetX() == arr[i].GetX())
    {
      if(offset.GetY() == arr[i].GetY())
      {
	return i;
      }
    }
  }
  return -1;
}

void ManhattanDir::FillFromBits(Point<int>& pt,
				u8 bits, TableType type)
{
  Point<int> bp = GetTable(type)[bits];

  pt.SetX(bp.GetX());
  pt.SetY(bp.GetY());
}

u32 ManhattanDir::ManhattanArea(u32 maxDistance)
{
  int oddSum = 0;
  int oddAcc = 1;
  for(int i = 0; i < maxDistance; i++)
  {
    oddSum += oddAcc;
    oddAcc += 2;
  }
  return (oddSum << 1) + oddAcc;
}


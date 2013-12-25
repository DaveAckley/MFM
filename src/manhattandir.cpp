#include <stdio.h>
#include "manhattandir.h"

u32 ManhattanDir::m_shortTableSize;
u32 ManhattanDir::m_longTableSize;

Point<int> ManhattanDir::
pointTableShort[MANHATTANDIR_SHORT_TABLE_SIZE];
Point<int> ManhattanDir::
pointTableLong[MANHATTANDIR_LONG_TABLE_SIZE];

void ManhattanDir::AllocTables()
{
  FillTable(pointTableShort, true);
  FillTable(pointTableLong, false);
}

void  ManhattanDir::FillTable(Point<int>* table,
			      bool sbond)
{
  int maxDist = sbond ? 2 : 4;
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
	table[cidx++].SetY(y);
      }
    }
  }

  (sbond ? m_shortTableSize : m_longTableSize) 
    = cidx;
}

u32 ManhattanDir::ShortTableSize()
{
  return m_shortTableSize;
}

u32 ManhattanDir::LongTableSize()
{
  return m_longTableSize;
}

u8 ManhattanDir::FromPoint(Point<int>* offset,
			   bool sbond)
{
  int arrSize = sbond?
    MANHATTANDIR_SHORT_TABLE_SIZE :
    MANHATTANDIR_LONG_TABLE_SIZE;
  Point<int>* arr = sbond?
    pointTableShort :
    pointTableLong;
  for(int i = 0; i < arrSize; i++)
  {
    if(offset->GetX() == arr[i].GetX())
    {
      if(offset->GetY() == arr[i].GetY())
      {
	return i;
      }
    }
  }
  return -1;
}

void ManhattanDir::FillFromBits(Point<int>* pt,
				u8 bits, bool sbond)
{
  Point<int> bp =
    (sbond ? pointTableShort : pointTableLong)[bits];

  pt->SetX(bp.GetX());
  pt->SetY(bp.GetY());
}

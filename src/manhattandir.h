#ifndef MANHATTANDIR_H
#define MANHATTANDIR_H

#include "itype.h"
#include "point.h"

/*
 * This class assumes that a Short bond is
 * maximum 2 away from the center and a
 * Long bond is maximum 4 from the center.
 */
#define MANHATTANDIR_SHORT_TABLE_SIZE 13
#define MANHATTANDIR_LONG_TABLE_SIZE 41

using namespace std;

typedef enum
{
  MANHATTAN_TABLE_SHORT,
  MANHATTAN_TABLE_LONG,
  MANHATTAN_TABLE_EVENT
}TableType;

class ManhattanDir
{
private:

  static Point<int> 
  pointTableShort[MANHATTANDIR_SHORT_TABLE_SIZE];

  static Point<int>
  pointTableLong[MANHATTANDIR_LONG_TABLE_SIZE];

  static Point<int>* pointTableEventWindow;

  static void FillTable(Point<int>* table,
			TableType type);

  static u32 m_shortTableSize;
  static u32 m_longTableSize;
  static u32 m_eventTableSize;
  static u32 m_eventWindowRadius;

  static u32 GetBondSize(TableType type);

  static Point<int>* GetTable(TableType type);

public:

  static u32 GetTableSize(TableType type);

  static u32 ShortTableSize();

  static u32 LongTableSize();
  
  /*
   * Call this before using any of the other public
   * methods.
   */
  static void AllocTables(u8 eventWindowRadius);

  /*
   * Call this after using this class to deallocate
   * any used memory.
   */
  static void DeallocTables();
  
  static u8 FromPoint(Point<int>& offset,
		      TableType type);

  /* 
   * Fills pt with the point represented by bits.
   * If this is a short bond (i.e. a 4-bit rep),
   * sbond needs to be true.
   */
  static void FillFromBits(Point<int>& pt, u8 bits,
			   TableType type);

  static u32 ManhattanArea(u32 maxDistance);
  
};

#endif /*MANHATTANDIR_H*/

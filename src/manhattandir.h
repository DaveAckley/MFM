#ifndef MANHATTANDIR_H
#define MANHATTANDIR_H

#include "itype.h"
#include "point.h"

#define MANHATTANDIR_SHORT_TABLE_SIZE 0xf
#define MANHATTANDIR_LONG_TABLE_SIZE 0xff

using namespace std;

class ManhattanDir
{
private:

  static Point<int> 
  pointTableShort[MANHATTANDIR_SHORT_TABLE_SIZE];

  static Point<int>
  pointTableLong[MANHATTANDIR_LONG_TABLE_SIZE];

  static void FillTable(Point<int>* table, bool sbond);

public:
  
  /*
   * Call this before using any of the other public
   * methods.
   */
  static void AllocTables();
  
  static u8 FromPoint(Point<int>* offset, bool sbond);

  /* 
   * Fills pt with the point represented by bits.
   * If this is a short bond (i.e. a 4-bit rep),
   * sbond needs to be true.
   */
  static void FillFromBits(Point<int>* pt, u8 bits,
			   bool sbond);
  
};

#endif /*MANHATTANDIR_H*/

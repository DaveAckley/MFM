#include "Parity2D_4x4.h"
#include "Util.h"  /* For PARITY */

namespace MFM {

  // Helper functions for slow checking and table generation

  /*
  static u32 BitOf(u32 val, u32 bitno) {
    return (val>>bitno)&1;
  }
  */

  static u32 BitAt(u32 bitno) {
    return 1<<bitno;
  }

  u32 Parity2D_4x4::Correct2DParityIfPossible(u32 allBits)
  {
    s32 failed[2];  // [row0], [col1]
    failed[0] = failed[1] = -1;

    // Check all groups for errors, crap out if multiple failures
    // detected
    for (u32 m = 0; m < H + 1 + W + 1; ++m) {  // Check all masks
      u32 rc, idx;
      if (m < H + 1) {
        rc = 0;
        idx = m;
      } else {
        rc = 1;
        idx = m - (H + 1);
      }
      u32 bits = allBits & masks[m];
      if (PARITY(bits)==0) {        // Even parity is a failure
        if (failed[rc] >= 0)        // If so, already failed in direction
          return 0;                 // So we cannot correct this
        failed[rc] = idx;           // Else remember which idx failed
      }
    }

    if (failed[0] < 0 && failed[1] < 0) // Then no failures, you dork,
      return allBits;               // so why did you call me?
    if (failed[0] < 0 || failed[1] < 0) // If apparently just one failure
      return 0;                         // Must be multiple, we're dead

    // Here we have isolated the single failed bit!  Fix it!

    u32 failedIndex = indices2D[failed[0]][failed[1]];
    return allBits^BitAt(failedIndex);

  }

  /*
    The bit indices in the 2D 9+16 encoding, [row0..4][col0..4]
   */
  const u8 Parity2D_4x4::indices2D[H + 1][W + 1] = {
    //Col0, Col1, Col2, Col3, Col4
    { 0x0f, 0x0e, 0x0d, 0x0c, 0x13 },  // Row 0
    { 0x0b, 0x0a, 0x09, 0x08, 0x12 },  // Row 1
    { 0x07, 0x06, 0x05, 0x04, 0x11 },  // Row 2
    { 0x03, 0x02, 0x01, 0x00, 0x10 },  // Row 3
    { 0x14, 0x15, 0x16, 0x17, 0x18 },  // Row 4
  };

  u32 Parity2D_4x4::ComputeParitySlow(u32 dataBits) {
    u32 res = dataBits;

    // Odd parity on rows excluding last
    for (u32 r = 0; r < H; ++r) {
      u32 bit = 0;
      for (u32 c = 0; c < W; ++c) {
        bit ^= (res >> indices2D[r][c]) & 1;
      }
      if (!bit)
        res |= 1 << indices2D[r][W];
    }
    // Odd parity on cols including last
    for (u32 c = 0; c < W + 1; ++c) {
      u32 bit = 0;
      for (u32 r = 0; r < H; ++r) {
        bit ^= (res >> indices2D[r][c]) & 1;
      }
      if (!bit)
        res |= 1 << indices2D[H][c];
    }

    return res >> DATA_BITS;
  }

#include "Parity2D_4x4_tables.inc"

}

#ifdef WRITE_PARITY_TABLES
#include <stdio.h>
#include <stdlib.h>
// g++ -I ../include -DWRITE_PARITY_TABLES Parity2D_4x4.cpp -o Parity2D_4x4.elf;./Parity2D_4x4.elf > Parity2D_4x4_tables.inc;rm -f Parity2D_4x4.elf

namespace MFM {

  static void WriteMasksTable(FILE * output) {

    fprintf(output,"%s", "  const u32 Parity2D_4x4::masks[H + 1 + W + 1] =\n    {\n");
    for (u32 r = 0; r < Parity2D_4x4::H + 1; ++r) {
      u32 bits = 0;
      for (u32 c = 0; c < Parity2D_4x4::W + 1; ++c) {
        bits |= 1 << Parity2D_4x4::indices2D[r][c];
      }
      fprintf(output,"      0x%07x, // Row %d\n", bits, r);
    }
    for (u32 c = 0; c < Parity2D_4x4::W + 1; ++c) {
      u32 bits = 0;
      for (u32 r = 0; r < Parity2D_4x4::H + 1; ++r) {
        bits |= 1 << Parity2D_4x4::indices2D[r][c];
      }
      fprintf(output,"      0x%07x%s // Col %d\n", bits, c==Parity2D_4x4::W?" ":",", c);
    }
    fprintf(output,"%s", "  };\n");
  }
  static void WriteParityTables(FILE * output) {
    fprintf(output,"%s", "  //// GENERATED TABLES: DO NOT EDIT\n");
    WriteMasksTable(output);
    fprintf(output,"%s", "  const u16 Parity2D_4x4::eccTable[TABLE_SIZE] =\n    {");
    for (u32 dataBit = 0; dataBit < Parity2D_4x4::TABLE_SIZE; ++dataBit) {
      if (dataBit%16==0)
        fprintf(output,"\n     ");
      fprintf(output,"0x%03x", Parity2D_4x4::ComputeParitySlow(dataBit));
      if (dataBit!=Parity2D_4x4::TABLE_SIZE-1)
        fprintf(output,", ");
    }
    fprintf(output,"\n    };\n");
    fprintf(output,"%s", "  //// END OF GENERATED TABLES\n");
  }
}

int main() {
  MFM::WriteParityTables(stdout);
  return 0;
}
#endif

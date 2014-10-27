#include "assert.h"
#include "Parity2D_4x4_Test.h"
#include "itype.h"
#include "Util.h"   /* For PARITY */

namespace MFM {

  void Parity2D_4x4_Test::Test_RunTests() {

    // Test all possible 9 bit data values
    for (u32 i = 0; i < 1<<9; ++i) {
      u32 wpar = Parity2D_4x4::Add2DParity(i);
      u32 tripleCases = 0;
      u32 tripleFailures = 0;

      // Parity of all 25 bits must always be odd
      assert(PARITY(wpar)==1);

      // Generated parity must always check
      assert(Parity2D_4x4::Check2DParity(wpar));

      // Removing parity must work
      u32 recovered9 = 0;
      assert(Parity2D_4x4::Remove2DParity(wpar,recovered9));

      // Value with parity removed must match
      assert(recovered9==i);

      for (u32 j = 0; j < 16; ++j) {
        u32 oneFail = wpar^(1<<j);

        // All single failures must be corrected
        u32 correctedFail = Parity2D_4x4::CheckAndCorrect2DParity(oneFail);
        assert(correctedFail==wpar);

        // All double bit failures must be detected
        for (u32 k = 0; k < 16; ++k) {
          if (k==j) continue;

          u32 twoFail = oneFail^(1<<k);
          assert(Parity2D_4x4::CheckAndCorrect2DParity(twoFail)==0);

          // Most triple bit failures must be detected
          for (u32 m = 0; m < 16; ++m) {
            if (m==j || m==k) continue;
            u32 threeFail = twoFail^(1<<m);

            ++tripleCases;
            if (Parity2D_4x4::CheckAndCorrect2DParity(threeFail)==0)
              ++tripleFailures;
          }
        }
      }

      // Let's say 'most' means 'more than 70%'
      assert(tripleFailures * 100 > tripleCases * 70);
    }
  }

} /* namespace MFM */


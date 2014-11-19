#include "Fail_Test.h"
#include <assert.h>  /* For assert */
#include <stdlib.h>  /* For rand */
#include "itype.h"

namespace MFM {

  void Fail_Test::Test_RunTests() {
    Test_NoFail();
    Test_SimpleFail();
    Test_FailAndNot();
    Test_FailWithCode();
    Test_NestedFail();
    Test_UnsafeModification();
  }

  void Fail_Test::Test_NoFail()
  {
    bool failed = false;
    int num = 0;
    unwind_protect({failed = true;},{
        num = 1;
      });
    assert(num==1);
    assert(failed==false);
  }

  void Fail_Test::Test_SimpleFail()
  {
    bool failed = false;
    int num = 0;
    unwind_protect({
        failed = true;
        num = 2;   // This assignment will definitely be visible on throw
      },{
        num = 1;  // Note: With -O99, this assignment may invisible on throw!
        FAIL(ILLEGAL_ARGUMENT);
      });
    assert(failed==true);
    assert(num==2);
  }

  void Fail_Test::Test_FailAndNot()
  {
    bool failed = false;
    int num = 0;
    for (int i = 10; i < 12; ++i) {
      unwind_protect({
          failed = true;
          num = i;   // This assignment will definitely be visible on throw
        },{
          num = 1;  // Note: With -O99, this assignment may invisible on throw!
          if (i > 10)
            FAIL(ILLEGAL_ARGUMENT);
        });
      if (i==10) {
        assert(failed==false);
        assert(num==1);
      } else {
        assert(failed==true);
        assert(num==11);
      }
    }
  }

  void Fail_Test::Test_FailWithCode()
  {
    bool failed = false;
    int num = 0;
    int codeCapture = 0;
    unwind_protect({
        failed = true;
        codeCapture = MFMThrownFailCode;
        num = 1;
      },{
        num = 2;
        FAIL(ILLEGAL_STATE);
      });
    assert(num==1);
    assert(failed==true);
    assert(codeCapture==MFM_FAIL_CODE_NUMBER(ILLEGAL_STATE));
  }

  static void failIllegalArgumentHelper(int * pnum) {
    bool failed = false;
    int num = 0;
    int codeCapture = 0;

    unwind_protect({
        failed = true;
        codeCapture = MFMThrownFailCode;
        num = 1;
      },{
        num = 2;
        FAIL(ILLEGAL_STATE);
      });
    assert(failed==true);
    assert(num==1);
    assert(codeCapture==MFM_FAIL_CODE_NUMBER(ILLEGAL_STATE));
    *pnum = 3;
    FAIL(ILLEGAL_ARGUMENT);
  }

  void Fail_Test::Test_NestedFail()
  {
    bool failed = false;
    int num = 0;
    int codeCapture = 0;
    unwind_protect({
        failed = true;
        codeCapture = MFMThrownFailCode;
      },{
        failIllegalArgumentHelper(&num);
        FAIL(UNREACHABLE_CODE);
      });

    assert(failed==true);
    assert(num==3);
    assert(codeCapture==MFM_FAIL_CODE_NUMBER(ILLEGAL_ARGUMENT));  // Not UNREACHABLE_CODE
  }

  static void modifyArg(int * pnum) { *pnum = 1; FAIL(ILLEGAL_STATE); }

  void Fail_Test::Test_UnsafeModification()
  {
    int num = 0;
    unwind_protect({ },{ modifyArg(&num); });
    // UNRELIABLE, CANNOT MAKE THIS ASSERTION:
    // assert(num==1);

    // (Note it seems to work if the 'static' is omitted from the
    // modifyArg definition, since then, even when optimizing, the
    // compiler seems to give up on the idea of inlining modifyArg..
    // But it's still unreliable and unsafe so we can't do it!)
  }


} /* namespace MFM */

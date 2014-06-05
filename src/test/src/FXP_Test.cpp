#include <math.h>
#include <assert.h>   /* for assert */
#include <stdlib.h>   /* for abort */
#include "FXP_Test.h"
#include "itype.h"

namespace MFM {

  void FXP_Test::Test_RunTests() {
    Test_FXPCtors();
    Test_FXPOps();
    Test_FXPCloser();
    Test_FXPFarther();
  }

  void FXP_Test::Test_FXPCtors()
  {
    {
      FXP16 s;
      assert(s.intValue==0);
    }
    {
      FXP16 s(1);
      assert(s.intValue==1<<16);
    }
    {
      FXP16 s(-1);
      assert(s.intValue==-1<<16);
    }
    {
      FXP16 s(0.5);
      assert(s.intValue==1<<15);
    }
    {
      FXP16 s(1.0);
      assert(s.intValue==1<<16);
    }
    {
      FXP16 s(-0.5);
      assert(s.intValue==-1<<15);
    }

    for (int i = -100; i < 100; i += 7) {
      FXP<0> foo = FXP<0>(i);
      assert(foo.intValue==i);
    }
    for (int i = -100; i < 100; i += 7) {
      FXP<3> foo = FXP<3>(i);
      assert(foo.intValue==i<<3);
    }
  }

  void FXP_Test::Test_FXPOps()
  {
    {
      FXP16 s;
      assert(s==0);
      ++s;
      assert(s==1);
      --s;
      assert(s==0);

      FXP16 s2;
      s2 = ++s;
      assert(s==1);
      assert(s2==1);

      s2 = --s;
      assert(s==0);
      assert(s2==0);

      s2 = s++;
      assert(s==1);
      assert(s2==0);

      s2 = s--;
      assert(s==0);
      assert(s2==1);

      ++s = 3;  // Doesn't make a lot of sense, but..
      assert(s==3);
      assert(s2==1);

      --s = 6;  // ..pre-inc/dec is an l-value.
      assert(s==6);
      assert(s2==1);
    }

    {
      FXP16 s;       assert(s==0);
      FXP16 t;       assert(t==0);
      t = s + 1;     assert(t==1);   assert(s==0);
      s += 1;        assert(s==1);
      t = s - 1;     assert(t==0);   assert(s==1);
      s -= 2;        assert(s==-1);

      s = t = 3;     assert(t==3);   assert(s==3);
      s = t * 3;     assert(t==3);   assert(s==9);
      s = 2 * t;     assert(t==3);   assert(s==6);
      s = t * t;     assert(t==3);   assert(s==9);

      s -= 1;        assert(s==8);
      s /= 2;        assert(s==4);
      s = s/2;       assert(s==2);
      s /= 2;        assert(s==1);
      s /= 2;        assert(s==0.5);
      s /= 2;        assert(s==0.25);
      s /= 2;        assert(s==0.125);
      s /= 2;        assert(s==0.125/2);

      s *= 2;        assert(s==0.125);
      s = s*2;       assert(s==0.25);
      s = 2*s;       assert(s==0.5);
      s *= 2;        assert(s==1);

      s *= s;        assert(s==1);
      s *= 2;        assert(s==2);
      s *= s;        assert(s==4);
      s *= s;        assert(s==16);
      s *= s;        assert(s==256);
      s *= s;        assert(s==65536);

      s *= 2;        assert(s==65536*2);

      s++;           assert(s==65537);  assert(s.intValue==65537<<16);
    }

    {
      FXP16 s, t;    assert(s==0);  assert(t==0);
      s = 10;        assert(s==10); assert(t==0);
      t = 2;         assert(s==10); assert(t==2);
      s /= t;        assert(s==5);  assert(t==2);
      s /= t;        assert(s==2.5);assert(t==2);

      s = t*t*t*t;   assert(s==16); assert(t==2);
      t = t/s;       assert(s==16); assert(t==0.125);
    }
  }

  template <int p>
  void FXP_Test::assertTolerance(const char * file, const int lineno, const int ulp, FXP<p> num, double toThis) {
    double is = num.toDouble();
    double absDiff = (is-toThis);
    const double tolerance = ((double) ulp)/(1<<p);
    if (absDiff<0) absDiff = -absDiff;
    if (absDiff > tolerance) {
      fprintf(stderr,"%s:%d: Assertion failed ",file,lineno);
      num.Print(stderr);
      fprintf(stderr,"!=%f (err=%f, tol=%f)\n",toThis,absDiff,tolerance);
      abort();
    }

    // num.Print(stderr); fprintf(stderr," is within %d ulp of %f\n",ulp, toThis);

  }

#define assertClose(a,b) assertTolerance(__FILE__,__LINE__,2,(a),(b))
#define assertFar(a,b) assertTolerance(__FILE__,__LINE__,50,(a),(b))

  void FXP_Test::Test_FXPCloser()
  {
    FXP16 s = 77, t = 9;
    double a, b;

    a = s.toDouble();
    b = t.toDouble();

    assertClose(s,a);
    assertClose(t,b);

    a /= b;  s /= t;

    assertClose(s,a);
    assertClose(t,b);

    b /= a;  t /= s;

    assertClose(s,a);
    assertClose(t,b);

    a += b;  s += t;

    assertClose(s,a);
    assertClose(t,b);

    a /= b;  s /= t;

    assertClose(s,a);
    assertClose(t,b);

    a = s.toDouble();
    b = t.toDouble();

    assertClose(s,a);
    assertClose(t,b);

    a = sqrt(b*a);  s = Sqrt(t*s);

    assertFar(s,a);

    b = 1/b; t = 1/t;

    assertFar(t,b);

    b -= a;  t -= s;

    assertFar(t,b);

    s = 12.34;
    t = 56.78;

    a = s.toDouble();
    b = t.toDouble();

    a = b*a;  s = t*s;

    assertClose(s,a);

    a /= a;  s /= s;

    assertClose(s,a);

    b -= a; t -= s;
    assertClose(t,b);
  }

  void FXP_Test::Test_FXPFarther()
  {
    FXP16 s = 77, t = 9;
    double a, b;

    a = s.toDouble();
    b = t.toDouble();

    a = sqrt(a);  s = Sqrt(s);

    assertFar(s,a);

    b = 1/b; t = 1/t;

    assertFar(t,b);

    b -= a;  t -= s;

    assertFar(t,b);

    s = 12.34;
    t = 56.78;

    a = s.toDouble();
    b = t.toDouble();

    a = sin(a); s = Sin(s);

    assertFar(s,a);

  }

} /* namespace MFM */

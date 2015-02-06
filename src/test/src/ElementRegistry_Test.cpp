#include "assert.h"
#include "Fail.h"
#include "Test_Common.h"
#include "ElementRegistry_Test.h"

namespace MFM {

  static void Test_Basic() {
    ElementRegistry<TestEventConfig> er;
    UUID u1("Sorter",1,2,3,4);
    assert(!er.IsRegistered(u1));
    assert(!er.IsLoaded(u1));
    Element_Empty<TestEventConfig>::THE_INSTANCE.AllocateType();
    er.RegisterElement(Element_Empty<TestEventConfig>::THE_INSTANCE);

    assert(!er.IsRegistered(u1));
    assert(!er.IsLoaded(u1));

    UUID eu = Element_Empty<TestEventConfig>::THE_INSTANCE.GetUUID();
    assert(er.IsRegistered(eu));
    assert(er.IsLoaded(eu));

    assert(!er.IsRegistered(u1));
    assert(!er.IsLoaded(u1));

    er.RegisterUUID(u1);
    assert(er.IsRegistered(u1));
    assert(!er.IsLoaded(u1));

    const Element<TestEventConfig> * ee = er.Lookup(eu);
    assert(ee != 0);
  }

  void ElementRegistry_Test::Test_RunTests() {
    Test_Basic();
  }

} /* namespace MFM */

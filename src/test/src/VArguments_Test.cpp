#include "assert.h"
#include "VArguments_Test.h"
#include "Fail.h"

#include <string.h> /* for strcmp */

namespace MFM {

  static const char* handlerArg;
  static void handler(const char* arg, void* not_used)
  {
    handlerArg = arg;
  }

  static void Test_Register()
  {
    VArguments args;

    args.RegisterArgument("test description", "-t|--test", 0, 0, false);

    assert(!args.Get("-t"));

    bool unwound = false;

    unwind_protect({

	unwound = true;
      },
      {
	args.Get("--foo");
      });

    assert(unwound);

    args.RegisterArgument("test handler", "-f|--foo", &handler, 0, true);

    const char* argv[] =
      {
	"-t",
	"--foo",
	"bar",
	0
      };
    u32 argc = sizeof(argv) / sizeof(argv[0]) - 1;

    args.ProcessArguments(argc, argv);

    assert(args.Appeared("-t"));
    assert(args.Appeared("--test"));
    assert(args.Appeared("-f"));
    assert(args.Appeared("--foo"));

    assert(!strcmp(args.Get("-f"), handlerArg));
    assert(!strcmp(args.Get("--foo"), handlerArg));
    assert(!strcmp(args.Get("--foo"), "bar"));
    assert(!strcmp("bar", handlerArg));
  }

  void VArguments_Test::Test_RunTests() {
    Test_Register();
  }

} /* namespace MFM */

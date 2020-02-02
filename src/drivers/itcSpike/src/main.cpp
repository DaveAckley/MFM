
#include "ITCSpikeDriver.h"
namespace MFM {

  int Run(int argc, const char** argv) {
    ITCSpikeDriver driver(argc,argv);
    driver.onceOnly();
    return driver.run();
  }

  int MainDispatch(int argc, const char** argv)
  {
    // Early early logging
    LOG.SetByteSink(STDERR);
    LOG.SetLevel(LOG.MESSAGE);

    return Run(argc, argv);
  }
}


int main(int argc, const char** argv)
{
  return MFM::MainDispatch(argc,argv);
}

#include "main.h"

#include <signal.h>

#include "Logger.h"
#include "T2Utils.h"
#include "Panel.h"

#include "T2Viz.h"
#include "SDLI.h"


namespace MFM {
  static SDLI* sdliPtr = 0;
  static s32 MFMPid = -1;
  static s32 signalMFM(int sig) {
    if (MFMPid > 0) return kill(MFMPid,sig);
    return -1;
  }
  static void continueMFM() {
    if (sdliPtr != 0) sdliPtr->stop();
    signalMFM(SIGUSR2);
  }
  static void cvtSignalToExit(int sig) {
    printf("CRASHING OUT\n");
    exit(9);
  }

  int MainDispatch(int argc, const char** argv) {
    // Early early logging
    LOG.SetByteSink(STDERR);
    LOG.SetLevel(LOG.MESSAGE);

    LOG.Message("Starting");

    const char * PROG_TO_SIGNAL = "mfmt2";
    u32 mfmpid = findPidOfProgram(PROG_TO_SIGNAL);
    if (mfmpid > 0) {
      MFMPid = mfmpid;
      s32 ret = signalMFM(SIGUSR1); // We're taking over the display
      LOG.Message("Signalled %s at pid %d -> %d",
                  PROG_TO_SIGNAL, MFMPid, ret);
    }

    atexit(continueMFM);

    signal(SIGINT, cvtSignalToExit);
    signal(SIGHUP, cvtSignalToExit);
    signal(SIGTERM, cvtSignalToExit);
    signal(SIGUSR1, cvtSignalToExit);
    signal(SIGUSR2, cvtSignalToExit);


    SDLI sdli;
    T2Viz model(sdli.getScreen());
    sdliPtr = &sdli;

    sdli.mainLoop(model);
    sdliPtr = 0;
    return 19;
  }
}

int main(int argc, const char** argv)
{
  return MFM::MainDispatch(argc,argv);
}

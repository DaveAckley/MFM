#include "main.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "TimeQueue.h"
#include "T2Utils.h"

namespace MFM {

  int MainDispatch(int argc, char** argv)
  {
    // Early early logging
    LOG.SetByteSink(STDERR);
    LOG.SetLevel(LOG.MESSAGE);

    // MAKE THE SINGLETON
    Stub::initInstance();

    Stub & cdm = Stub::get();
    cdm.initEverything(argc,argv);

    unwind_protect({
        MFMErrorEnvironmentPointer_t errenv = &unwindProtect_errorEnvironment;
        volatile const char * file = errenv->file;
        int line = errenv->lineno;
        int code = errenv->thrown;
        const char * msg = MFMFailCodeReason(code);

        if (!file) {
          file = "unknown";
          line = 0;
        }

        // Get the failure into the log file
        MFMPrintErrorEnvironment(stderr, &unwindProtect_errorEnvironment);
        fprintf(stderr,"%s:%d: %s [%d]\n",
                file, line, msg ? msg : "", code);
        fprintf(stderr,"Failed out of top-level\n");
        exit(99);
      },{
        cdm.main();
      });

    return 0;
  }
}


int main(int argc, char** argv)
{
  unwind_protect({
      MFMPrintErrorEnvironment(stderr, &unwindProtect_errorEnvironment);
      fprintf(stderr,"Failed out of main\n");
      exit(99);
  },{
    return MFM::MainDispatch(argc,argv);
  });
}

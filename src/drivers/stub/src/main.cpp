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
        const char * file = (const char *) unwindProtect_FailException.mFile;
        int line = unwindProtect_FailException.mLine;
        int code = unwindProtect_FailException.mCode;
        //const char * msg = MFMFailCodeReason(code);

        MFMPrintError(stderr,file,line,code);
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
      const char * file = (const char *) unwindProtect_FailException.mFile;
      int line = unwindProtect_FailException.mLine;
      int code = unwindProtect_FailException.mCode;
      //const char * msg = MFMFailCodeReason(code);

      MFMPrintError(stderr,file,line,code);
      fprintf(stderr,"Failed out of main\n");
      exit(99);
  },{
    return MFM::MainDispatch(argc,argv);
  });
}

#include "main.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <execinfo.h>  /* for backtrace_symbols */
#include <dlfcn.h>     /* for dladdr */
#include <cxxabi.h>    /* for __cxa_demangle */

#include "TimeQueue.h"
#include "SDLI.h"
#include "ADCCtl.h"
#include "T2Utils.h"
#include "TraceTypes.h"


namespace MFM {
  extern "C" void T2TileAttemptTraceLogging(const FailException & fe, const char * unwindFile, int unwindLine) {
    const char * msg = MFMFailCodeReason(fe.mCode);
    T2Tile & tile = T2Tile::get();
    tile.tlog(Trace(tile,TTC_Tile_TopLevelFailure,"%s:%d: %s [%d] {%s:%d}",
                    fe.mFile,
                    fe.mLine,
                    msg ? msg : "",
                    fe.mCode,
                    unwindFile,
                    unwindLine));
    char ** strings;
    strings = backtrace_symbols(fe.mBacktraceArray, fe.mBacktraceSize);
    if (strings) {
      for (u32 i = 0; i < fe.mBacktraceSize; ++i) {
        Dl_info info;
        if (dladdr(fe.mBacktraceArray[i],&info)) {
          char * demangled = NULL;
          int status;
          demangled = abi::__cxa_demangle(info.dli_sname, NULL, 0, &status);
          tile.tlog(Trace(tile,TTC_Tile_TopLevelFailure," -> %2d: %s + 0x%x",
                          i,
                          status == 0 ? demangled : info.dli_sname,
                          (char*) fe.mBacktraceArray[i] - (char*) info.dli_saddr));
          free(demangled);
        } else 
          tile.tlog(Trace(tile,TTC_Tile_TopLevelFailure," -> %2d: %s",
                          i, strings[i]));
      }
      free(strings);
    }
  }

  struct TOT : public TimeoutAble {
    virtual void onTimeout(TimeQueue& src) { }
    virtual const char * getName() const { return "TOT"; }
  };
  void tqtest(Random & random) {
    TimeQueue tq(random);
    TOT t1,t2,t3;
    t1.insert(tq,100);
    t2.insert(tq,10);
    t3.insert(tq,50);
    TimeoutAble * ptr;
    while (!tq.isEmpty()) {
      u32 now = tq.now();
      printf("Scan at %d\n",now);
      while ((ptr = tq.getEarliestExpired()) != 0) {
        printf("to=%d, %p \n", ptr->getTimeout(), ptr);
      }
      for (u32 i = 0; i < 100000; ++i) random.Create(); // wastongo
    }
    
  }

  struct SDLDisplayer : public TimeoutAble {
    SDLDisplayer(T2Tile & tile, SDLI & sdli, const char * name)
      : mTile(tile)
      , mSDLI(sdli)
      , mName(name)
    { }
    T2Tile & mTile;
    SDLI & mSDLI;
    const char * mName;
    
    virtual void onTimeout(TimeQueue& src) {
      //    LOG.Message("Redisplongo\n");
      mSDLI.redisplay();
      insert(src,200,6);
    }
    virtual const char * getName() const { return mName; }
  };

  void demo(T2Tile & tile, SDLI & display) {
    TimeQueue tq(tile.getRandom());
    SDLDisplayer sdld(tile, display, "demoDisplayer");
    sdld.insert(tq,200,6);
    while (!tq.isEmpty()) {
      u32 now = tq.now();
      printf("Scan at %d\n",now);
      TimeoutAble * ptr;
      while ((ptr = tq.getEarliestExpired()) != 0) {
        printf("to=%d, %p \n", ptr->getTimeout(), ptr);
        ptr->onTimeout(tq);
      }
    }
  }

  struct UnexpectedExit {
    T2Tile& mTile;
    bool mExpectedExit;
    UnexpectedExit(T2Tile & tile)
      : mTile(tile)
      , mExpectedExit(false)
    { }
    void main() {
      try {
        unwind_protect({throw std::exception();},{
            mTile.main();
            mExpectedExit = true;
          });
      }
      catch (std::exception const & e) {
        LOG.Error("EXCEPTION COT");
      }
    }
    ~UnexpectedExit() {
      if (!mExpectedExit) {
        LOG.Error("UNEXPECTED EXIT");
        bool tracing = TRACEPrintf(Logger::ERR,"Unexpected exit");
        if (tracing) {
          T2FlashTrafficManager & mgr = T2Tile::get().getFlashTrafficManager();
          mgr.shipAndExecuteFlashDump();
          LOG.Error("TRACE DUMPED");
        }
      }
      else
        LOG.Error("GOOD OUT");
    }
  };

  int MainDispatch(int argc, char** argv)
  {
    // Early early logging
    LOG.SetByteSink(STDERR);
    LOG.SetLevel(LOG.MESSAGE);

    // MAKE THE SINGLETON
    T2Tile::initInstance();

    T2Tile & tile = T2Tile::get();
    tile.initEverything(argc,argv);

    // START TRACE LOGGING FAILS
    MFMUnwindProtectLoggingHook = T2TileAttemptTraceLogging;
    
    UnexpectedExit ue(tile);
    ue.main();
#if 0
    unwind_protect({
        /*
        MFMErrorEnvironmentPointer_t errenv = &unwindProtect_errorEnvironment;
        volatile const char * file = errenv->file;
        int line = errenv->lineno;
        int code = errenv->thrown;
        */
        const char * file = (const char *) unwindProtect_FailException.mFile;
        int line = unwindProtect_FailException.mLine;
        int code = unwindProtect_FailException.mCode;
        const char * msg = MFMFailCodeReason(code);

        if (!file) {
          file = "unknown";
          line = 0;
        }

        // Get the failure into the log file
        MFMPrintError(stderr,file,line,code);
        //        MFMPrintErrorEnvironment(stderr, &unwindProtect_errorEnvironment);
        fprintf(stderr,"Failed out of top-level\n");

        // And get it into the trace file
        tile.tlog(Trace(tile,TTC_Tile_TopLevelFailure,"%s:%d: %s [%d]",
                        file, line, msg ? msg : "", code));

        // Aand spam it out on the screen
        tile.showFail((const char *) file, line, msg);

        // Aaand try to trigger an explicit packet status dump to syslog (ignore errors)
        {
          char buf[100];
          u32 size = sizeof(buf);
          snprintf(buf,size,"%s:%d: %s [%d]",
                   file, line, msg ? msg : "", code);
          writeWholeFile("/sys/class/itc_pkt/dump",buf);
        }
        
        // Aaaaand die
        exit(99);
      },{
        tile.main();
      });
#endif
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

#include "Weaver.h"

#include <getopt.h>

#include "FileByteSink.h"  // For STDERR
#include "Logger.h"

namespace MFM {

  Trace * WeaverLogFile::read() {
    return mTraceLogReader.read();
  }

  void WeaverLogFile::findITCSyncs(Alignment & a) {
    if (!mFileByteSource.Seek(0u, SEEK_SET)) {
      FAIL(ILLEGAL_STATE);
    }
    Trace * ptr;
    while ((ptr = read()) != 0) {
      ptr->printPretty(STDOUT);
      s32 tag = ptr->reportSyncIfAny();
      if (tag) a.addSyncPoint(tag, this, ptr);
      else delete ptr;
    }
  }

  WeaverLogFile::WeaverLogFile(const char * filePath, u32 num)
    : mFilePath(filePath)
    , mFileNumber(num)
    , mFileByteSource(mFilePath)
    , mTraceLogReader(mFileByteSource)
  { }
    
  void Alignment::addSyncPoint(s32 sync, WeaverLogFile * file, Trace * evt) {
    MFM_API_ASSERT_NONNULL(file);
    MFM_API_ASSERT_NONNULL(evt);
    FileTracePtrPair pair(file,evt);
    FileTracePtrPairVector & v = mSyncUseMap[sync];
    v.push_back(pair);
  }

  Alignment::~Alignment() {
    while (mWeaverLogFiles.size() > 0) {
      WeaverLogFile* ptr = mWeaverLogFiles.back();
      delete ptr;
      mWeaverLogFiles.pop_back();
    }
    for (SyncUseMap::iterator itr = mSyncUseMap.begin(); itr != mSyncUseMap.end(); ++itr) {
      FileTracePtrPairVector & v = itr->second;
      while (v.size()) {
        FileTracePtrPair p = v.back();
        delete p.second;
        v.pop_back();
      }
    }
  }

  u32 Alignment::logFileCount() const {
    return mWeaverLogFiles.size();
  }

  void Alignment::processLogs() {
    alignLogs();
    reportLogs();
  }

  void Alignment::printSyncMap(ByteSink& bs) const {
    for (SyncUseMap::const_iterator itr = mSyncUseMap.begin();
         itr != mSyncUseMap.end(); ++itr) {
      s32 val = itr->first;
      const FileTracePtrPairVector & v = itr->second;
      if (v.size() > 1) {
        bs.Printf("%08x\n",val);
        for (FileTracePtrPairVector::const_iterator it2 = v.begin();
             it2 != v.end(); ++it2) {
          const WeaverLogFile * wlf = it2->first;
          const Trace* trace = it2->second;
          bs.Printf("  #%d:",wlf->getFileNum());
          trace->getTraceAddress().printPretty(bs);
          bs.Printf("\n");
        }
      }
    }
  }
  void Alignment::countStats(u32 fn1, u32 fn2, double delta12) {
    FileNumberPair fnp(fn1,fn2);
    CountAndSumsPair & casp = mDisparityMap[fnp];
    ++casp.first;
    SumAndSumSquaredPair & sassp = casp.second;
    sassp.first += delta12;
    sassp.second += delta12*delta12;
  }

  void Alignment::alignLogs() {
    for (WeaverLogFilePtrVector::iterator itr = mWeaverLogFiles.begin();
         itr != mWeaverLogFiles.end(); ++itr) {
      WeaverLogFile* ptr = *itr;
      ptr->findITCSyncs(*this);
    }
    for (SyncUseMap::iterator itr = mSyncUseMap.begin();
         itr != mSyncUseMap.end(); ++itr) {
      FileTracePtrPairVector & v = itr->second;
      if (v.size() < 2) continue;
      for (FileTracePtrPairVector::iterator it1 = v.begin();
           it1 != v.end(); ++it1) {
        for (FileTracePtrPairVector::iterator it2 = v.begin();
             it2 != v.end(); ++it2) {
          WeaverLogFile & w1 = *it1->first;
          WeaverLogFile & w2 = *it2->first;
          if (&w1 == &w2) continue;
          u32 n1 = w1.getFileNum();
          u32 n2 = w1.getFileNum();
          
          struct timespec t1 = w1.getFirstTimespec();
          struct timespec t2 = w2.getFirstTimespec();
          double delta12 =
            (t1.tv_sec - t2.tv_sec) +
            (t1.tv_nsec - t2.tv_nsec)/1000000000.0;

          countStats(n1,n2,delta12);
          countStats(n2,n1,-delta12);
        }
      }
    }

    for (WeaverLogFilePtrVector::iterator it1 = mWeaverLogFiles.begin();
         it1 != mWeaverLogFiles.end(); ++it1) {
      for (WeaverLogFilePtrVector::iterator it2 = mWeaverLogFiles.begin();
           it2 != mWeaverLogFiles.end(); ++it2) {
        WeaverLogFile & w1 = **it1;
        WeaverLogFile & w2 = **it2;
        if (&w1 == &w2) continue;
        u32 n1 = w1.getFileNum();
        u32 n2 = w1.getFileNum();

        FileNumberPair fnp(n1,n2);
        const CountAndSumsPair & casp = mDisparityMap[fnp];
        const SumAndSumSquaredPair & sassp = casp.second;

        STDOUT.Printf("#%d -> #%d: n=%d, s=%f, ss=%f\n",
                      n1, n2,
                      casp.first,
                      sassp.first,
                      sassp.second);
      }
    }
  }

  void Alignment::reportLogs() {
    FAIL(INCOMPLETE_CODE);
  }

  bool Alignment::addLogFile(const char * path) {
    // Try opening it to frontload errors
    FILE * file = fopen(path, "r");
    if (!file) {
      LOG.Error("Can't read %s: %s", path, strerror(errno));
      return false;
    }
    fclose(file);
    mWeaverLogFiles.push_back(new WeaverLogFile(path,mWeaverLogFiles.size()));
    return true;
  }


#define ALL_CMD_ARGS()                                          \
  XX(help,h,N,,"Print this help")                               \

#if 0
  XX(log,l,O,LEVEL,"Set or increase logging")                   \
  XX(mfzid,z,R,MFZID,"Specify MFZ file to run")                 \
  XX(paused,p,N,,"Start up paused")                             \
  XX(trace,t,R,PATH,"Trace output to file")                     \
  XX(version,v,N,,"Print version and exit")                     \
  XX(wincfg,w,R,PATH,"Specify window configuration file")       \

#endif

//GENERATE SHORT OPTION STRING
#define XXR ":"
#define XXN ""
#define XXO "::"
#define XX(L,S,A,V,D) #S XX##A
  static const char * CMD_LINE_SHORT_OPTIONS =
    ALL_CMD_ARGS()
    ;
#undef XXR
#undef XXN
#undef XXO  
#undef XX

//GENERATE LONG OPTION STRUCT
#define XXR required_argument
#define XXN no_argument
#define XXO optional_argument
#define XX(L,S,A,V,D) { #L, XX##A, 0, #S[0] },

  static struct option CMD_LINE_LONG_OPTIONS[] =
    {
     ALL_CMD_ARGS()
     {0,0,0,0}
    };
#undef XXR
#undef XXN
#undef XXO  
#undef XX

//GENERATE ENUM
#define XX(L,S,A,V,D) CMDARG_##L,
  enum CmdArg {
      ALL_CMD_ARGS()
      COUNT_CMDARG
  };
#undef XX


//GENERATE HELP STRING
#define XXR(v) #v " "
#define XXN(v) ""
#define XXO(v) "[" #v "] "
#define XX(L,S,A,V,D) \
  "  --" #L " " XX##A(V) "or -" #S XX##A(V) "\n\t " D "\n\n"
static const char * CMD_HELP_STRING =
  "COMMAND LINE ARGUMENTS:\n\n"
  ALL_CMD_ARGS()
  "\n"
  ;
#undef XXR
#undef XXN
#undef XXO  
#undef XX

  struct Weaver {
    Alignment mAlignment;

    void processArgs(int argc, char ** argv) {
      int c;
      int option_index;
      int fails = 0;
      int loglevel = -1;
      int wantpaused = 0;
      while ((c = getopt_long(argc,argv,
                              CMD_LINE_SHORT_OPTIONS,
                              CMD_LINE_LONG_OPTIONS,
                              &option_index)) != -1) {
        switch (c) {
        case 'z':
          LOG.Message("Z");
          break;

        case 'p':
          wantpaused = 1;
          break;

        case 'w':
          LOG.Message("Z");
          break;

        case 't':
          LOG.Message("Z");
          break;

        case 'l':
          if (optarg) {
            s32 level = LOG.ParseLevel(optarg);
            if (level < 0) {
              error("Not a logging level '%s'",optarg);
              ++fails;
            } else {
              if (loglevel >= 0)
                message("Overriding previous logging level '%s'",
                        Logger::StrLevel((Logger::Level) loglevel));
              loglevel = (Logger::Level) level;
            }
          } else ++loglevel;
          break;

        case 'h':
          printf("%s",CMD_HELP_STRING);
          exit(0);

        case 'v':
          printf("For MFM%d.%d.%d (%s)\nBuilt on %08x at %06x by %s\n",
                 MFM_VERSION_MAJOR, MFM_VERSION_MINOR, MFM_VERSION_REV,
                 xstr(MFM_TREE_VERSION),
                 MFM_BUILD_DATE, MFM_BUILD_TIME,
                 xstr(MFM_BUILT_BY));
          exit(0);
        case '?':
          ++fails;
          break;

        default:
          abort();
        }
      }
      /*
      if (!fails && getMFZId()==0) {
        error("Missing -z MFZID");
        ++fails;
      }
      */
      while (optind < argc) {
        if (!mAlignment.addLogFile(argv[optind++])) {
          LOG.Error("Bad log file");
          ++fails;
        }
      }
      if (fails) {
        fatal("%d command line problem%s",fails,fails==1?"":"s");
      }
      if (mAlignment.logFileCount() == 0)
        fatal("No log files supplied on command line");
      
      if (loglevel >= 0) {
        LOG.SetLevel(loglevel);
      }

      if (wantpaused >= 0) {
        LOG.Message("WP %d", wantpaused);
      }
      mAlignment.processLogs();
    }

    int main(int argc, char ** argv) {
      processArgs(argc,argv);
      return 0;
    }
  };

  int MainDispatch(int argc, char** argv)
  {
    // Early early logging
    LOG.SetByteSink(STDERR);
    LOG.SetLevel(LOG.MESSAGE);
    Weaver weaver;
    return weaver.main(argc,argv);
  }
}

int main(int argc, char **argv) {
  unwind_protect({
      MFMPrintErrorEnvironment(stderr, &unwindProtect_errorEnvironment);
      fprintf(stderr,"Failed out of top level\n");
      exit(99);
  },{
    return MFM::MainDispatch(argc,argv);
  });
}

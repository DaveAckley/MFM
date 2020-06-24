#include "Weaver.h"

#include <getopt.h>
#include <time.h>

#include "FileByteSink.h"  // For STDERR
#include "Logger.h"

namespace MFM {

  struct timespec WeaverLogFile::getTraceEffectiveTime(Trace & trace) const {
    MFM_API_ASSERT_STATE(hasEffectiveOffset());
    UniqueTime eo(getEffectiveOffset(),0);
    UniqueTime tt(trace.mLocalTimestamp);
    UniqueTime diff = tt - eo;
    return diff.getTimespec();
  }

  Trace * WeaverLogFile::read(bool useOffset) {
    struct timespec zero;
    zero.tv_sec = 0;
    zero.tv_nsec = 0;
    if (useOffset) MFM_API_ASSERT_STATE(hasEffectiveOffset());
    return mTraceLogReader.read(useOffset ? getEffectiveOffset() : zero);
  }

  void WeaverLogFile::reread() {
    if (!mFileByteSource.Seek(0u, SEEK_SET)) {
      FAIL(ILLEGAL_STATE);
    }
  }

  void WeaverLogFile::findITCSyncs(Alignment & a) {
    reread();
    Trace * ptr;
    while ((ptr = read(false)) != 0) {
      //      ptr->printPretty(STDOUT);
      s32 tag;
      if (ptr->reportSyncIfAny(tag)) a.addSyncPoint(tag < 0 ? -tag : tag, this, ptr);
      else delete ptr;
    }
  }

  WeaverLogFile::WeaverLogFile(const char * filePath, u32 num)
    : mFilePath(filePath)
    , mFileNumber(num)
    , mFileByteSource(mFilePath)
    , mTraceLogReader(mFileByteSource)
    , mAverageOffset(0)
    , mOutlierDistance(10e100)
    , mHasEffectiveOffset(false)
    , mEffectiveOffset()
  { }
    
  bool Alignment::parseTweak(const char * arg) {
    CharBufferByteSource cbbs(arg,strlen(arg));
    u32 filenum;
    s32 usec;
    if (3 == cbbs.Scanf("%d/%d",&filenum, &usec)) {
      mWLFTweakMap[filenum] += usec;
      STDOUT.Printf("%d/ tweak = %dus\n",filenum, mWLFTweakMap[filenum]);
      return true;
    }
    return false;
  }

  void Alignment::addSyncPoint(s32 sync, WeaverLogFile * file, Trace * evt) {
    MFM_API_ASSERT_NONNULL(file);
    MFM_API_ASSERT_NONNULL(evt);
    FileTracePtrPair pair(file,evt);
    FileTracePtrPairVector & v = mSyncUseMap[sync];
    v.push_back(pair);
  }

  Alignment::Alignment()
    : mPrintSyncMap(false)
  { }

  Alignment::~Alignment() {
    while (mWeaverLogFiles.size() > 0) {
      WLFandTracePtrsPair & pp = mWeaverLogFiles.back();
      WeaverLogFile* wptr = pp.first;
      Trace* tptr = pp.second;
      delete wptr;
      delete tptr;
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

  void Alignment::assignEffectiveOffsetsLaterThan(FileNumber n1) {
    MFM_API_ASSERT_NONNULL(mWeaverLogFiles[n1].first);
    const WeaverLogFile & w1 = *(mWeaverLogFiles[n1].first);
    MFM_API_ASSERT_STATE(w1.hasEffectiveOffset());
    for (WeaverLogFilePtrVector::iterator it2 = mWeaverLogFiles.begin();
         it2 != mWeaverLogFiles.end();
         ++it2) {
      WeaverLogFile & w2 = *(it2->first);
      FileNumber n2 = w2.getFileNum();
      if (n1 == n2) continue;
      if (w2.hasEffectiveOffset()) continue;
      FileNumberPair fnp(n2,n1); // Want n2 relative to n1
      InterFileData & ifd = mDisparityMap[fnp];
      if (ifd.mDisparityCount == 0) continue;

      struct timespec eff1 = w1.getEffectiveOffset();
      double avgOffset = 1.0*ifd.mSumOfDisparities/ifd.mDisparityCount;
      STDOUT.Printf("TWEAKING n%d %f\n",n2,mWLFTweakMap[n2]/1000000.0);
      avgOffset += mWLFTweakMap[n2]/1000000.0; // ADJUST OFFSET BY TWEAK
      struct timespec avgOff = UniqueTime::timespecFromDouble(avgOffset);
      struct timespec sum = UniqueTime::sum(eff1,avgOff);
      w2.setEffectiveOffset(sum);
      assignEffectiveOffsetsLaterThan(n2);
    }
  }

  void Alignment::alignLogs() {
    typedef std::set<FileNumber> NoneEarlierSet;
    NoneEarlierSet noneEarlier;
    for (WeaverLogFilePtrVector::iterator it1 = mWeaverLogFiles.begin();
         it1 != mWeaverLogFiles.end();
         ++it1) {
      const WeaverLogFile & w1 = *(it1->first);
      FileNumber n1 = w1.getFileNum();
      bool iStretchBackFarther = true;
      for (WeaverLogFilePtrVector::iterator it2 = mWeaverLogFiles.begin();
           it2 != mWeaverLogFiles.end();
           ++it2) {
        const WeaverLogFile & w2 = *(it2->first);
        if (&w1 == &w2) continue;
        FileNumber n2 = w2.getFileNum();
        FileNumberPair fnp(n1,n2);
        InterFileData & ifd = mDisparityMap[fnp];
        if (ifd.mFile2StretchesBackFartherCount > 0) {
          iStretchBackFarther = false;
          break;
        }
      }
      if (iStretchBackFarther) {
        noneEarlier.insert(n1);
      }
    }
    if (noneEarlier.size() != 1) {
      STDERR.Printf("%d anchor files detected\n", noneEarlier.size());
      FAIL(INCOMPLETE_CODE);
    } else { // One
      struct timespec zero;
      zero.tv_sec = 0;
      zero.tv_nsec = 0;
      FileNumber fn = *noneEarlier.begin();
      STDOUT.Printf("Anchor file is %d/\n",fn);
      WeaverLogFile & anchor = *(mWeaverLogFiles[fn].first);
      anchor.setEffectiveOffset(zero); // Oldest anchor starts consolidated relative time
      assignEffectiveOffsetsLaterThan(fn);
    }
    
  }

  void Alignment::processLogs() {
    analyzeLogSync();
    if (mPrintSyncMap)
      printSyncMap(STDOUT);
    if (reportOutliers()) 
      FAIL(INCOMPLETE_CODE);
    
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
          bs.Printf("  %d/",wlf->getFileNum());
          trace->getTraceAddress().printPretty(bs);
          bs.Printf("\n");
        }
      }
    }
  }

  void Alignment::countStats(u32 fn1, u32 fn2, double delta12, double sb1, double sb2) {
    FileNumberPair fnp(fn1,fn2);
    InterFileData & ifd = mDisparityMap[fnp];
    ++ifd.mDisparityCount;
    ifd.mSumOfDisparities += delta12;
    ifd.mSumSquareOfDisparities += delta12*delta12;
    if (sb1 > sb2) ++ifd.mFile1StretchesBackFartherCount;
    if (sb2 > sb1) ++ifd.mFile2StretchesBackFartherCount;
  }

  bool Alignment::reportOutlier(u32 fn1, u32 fn2, double delta12, s32 syncVal) {
    FileNumberPair fnp(fn1,fn2);
    InterFileData & ifd = mDisparityMap[fnp];
    double avg = ifd.mSumOfDisparities / ifd.mDisparityCount;
    double var = ifd.mSumSquareOfDisparities / ifd.mDisparityCount;
    double err = delta12 - avg;
    if (err < 0) err = -err;
    if (err > 0.010) {
      STDOUT.Printf("WARNING: SYNC ALIAS? %08x (#%d->#%d) avg=%f, var=%f, outlier=%f, err=%f\n",
                    syncVal,
                    fn1,fn2,
                    avg,var,
                    delta12, err);
      return true;
    }
    return false;
  }

  bool Alignment::reportOutliers() {
    u32 count = 0;
    for (SyncUseMap::iterator itr = mSyncUseMap.begin();
         itr != mSyncUseMap.end(); ++itr) {
      s32 syncVal = itr->first;
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
          u32 n2 = w2.getFileNum();
          
          struct timespec t1 = w1.getFirstTimespec();
          struct timespec t2 = w2.getFirstTimespec();
          double delta12 =
            (t1.tv_sec - t2.tv_sec) +
            (t1.tv_nsec - t2.tv_nsec)/1000000000.0;

          if (reportOutlier(n1,n2,delta12,syncVal))
            ++count;
        }
      }
    }
    return count > 0;
  }

  void Alignment::analyzeLogSync() {
    for (WeaverLogFilePtrVector::iterator itr = mWeaverLogFiles.begin();
         itr != mWeaverLogFiles.end(); ++itr) {
      WeaverLogFile* ptr = itr->first;
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
          u32 n2 = w2.getFileNum();
          
          Trace& trace1 = *it1->second;
          Trace& trace2 = *it2->second;
          struct timespec tt1 = trace1.mLocalTimestamp.getTimespec();
          struct timespec tt2 = trace2.mLocalTimestamp.getTimespec();

          struct timespec ft1 = w1.getFirstTimespec();
          struct timespec ft2 = w2.getFirstTimespec();
          double delta12 = UniqueTime::getIntervalSeconds(ft1, ft2);

          double stretchBack1 = UniqueTime::getIntervalSeconds(tt1,ft1);
          double stretchBack2 = UniqueTime::getIntervalSeconds(tt2,ft2);

          countStats(n1, n2, delta12, stretchBack1, stretchBack2);
          countStats(n2, n1, -delta12, stretchBack2, stretchBack1);

        }
      }
    }

    for (WeaverLogFilePtrVector::iterator it1 = mWeaverLogFiles.begin();
         it1 != mWeaverLogFiles.end(); ++it1) {
      WeaverLogFile & w1 = *(it1->first);
        u32 n1 = w1.getFileNum();
        time_t sec = w1.getFirstTimespec().tv_sec;
        STDOUT.Printf("%d/ %s: %s", n1, w1.getFilePath(), ctime(&sec));
    }
    for (WeaverLogFilePtrVector::iterator it1 = mWeaverLogFiles.begin();
         it1 != mWeaverLogFiles.end(); ++it1) {
      for (WeaverLogFilePtrVector::iterator it2 = mWeaverLogFiles.begin();
           it2 != mWeaverLogFiles.end(); ++it2) {
        WeaverLogFile & w1 = *(it1->first);
        WeaverLogFile & w2 = *(it2->first);
        if (&w1 == &w2) continue;
        u32 n1 = w1.getFileNum();
        u32 n2 = w2.getFileNum();

        FileNumberPair fnp(n1,n2);
        const InterFileData & ifd = mDisparityMap[fnp];
        double avg = ifd.mSumOfDisparities / ifd.mDisparityCount;
        double var = ifd.mSumSquareOfDisparities / ifd.mDisparityCount;
        STDOUT.Printf("%d/ -> %d/ n=%d, s=%f, ss=%f, avg=%f, var=%f\n",
                      n1, n2,
                      ifd.mDisparityCount,
                      ifd.mSumOfDisparities,
                      ifd.mSumSquareOfDisparities,
                      avg,var);
      }
    }
  }

  void Alignment::reportLogs() {
    for (WeaverLogFilePtrVector::iterator it1 = mWeaverLogFiles.begin();
         it1 != mWeaverLogFiles.end(); ++it1) {
      WeaverLogFile & w1 = *(it1->first);
      u32 n1 = w1.getFileNum();
      STDOUT.Printf("%d/ ",n1);
      UniqueTime eo(w1.getEffectiveOffset(),0);
      eo.printPretty(STDOUT);
      STDOUT.Printf("\n");
      w1.reread(); // Set up to reread for dumping
      if (it1->second != 0) {
        delete it1->second; // Clean up leftover traces?  
        it1->second = 0;
      }

    }
    bool somebodyLive = true;
    struct timespec lastTime;
    lastTime.tv_sec = 0;
    lastTime.tv_nsec = 0;
    while (somebodyLive) {
      somebodyLive = false;
      FileTracePtrPair * pftpp = 0;
      for (WeaverLogFilePtrVector::iterator it1 = mWeaverLogFiles.begin();
           it1 != mWeaverLogFiles.end(); ++it1) {
        FileTracePtrPair & ftpp = *it1;

        if (!pftpp) {
          if (advanceToNextTraceIfNeededAndPossible(ftpp)) {
            pftpp = &ftpp;
            somebodyLive = true;
          }
        } else {
          if (advanceToNextTraceIfNeededAndPossible(*pftpp)) {
            Trace * existingt = pftpp->second;
            MFM_API_ASSERT_NONNULL(existingt);
            struct timespec existing = existingt->getTimespec();

            if (advanceToNextTraceIfNeededAndPossible(ftpp)) {
              Trace * newt = ftpp.second;
              MFM_API_ASSERT_NONNULL(newt);
              struct timespec challenget = newt->getTimespec();
              if (UniqueTime(challenget,0) < UniqueTime(existing,0)) {
                pftpp = &ftpp;
                somebodyLive = true;
              }
            }
          }
        }
      }
      if (somebodyLive) {
        WeaverLogFile & wlf = *(pftpp->first);
        Trace * reportt = extractNextEarliestTrace(*pftpp);
        struct timespec thisTime = reportt->getTimespec();
        //double seconds = UniqueTime::getIntervalSeconds(thisTime,lastTime);
        lastTime = thisTime;
        printf("%0.3f ", UniqueTime::doubleFromTimespec(thisTime));
        //        STDOUT.Printf("%d ",(u32) );
        //STDOUT.Printf("%4d ",(u32) (1000*seconds));
        for (u32 i = 0; i < wlf.getFileNum(); ++i) 
          STDOUT.Printf("        ");
        STDOUT.Printf("%d/",wlf.getFileNum());
        reportt->printPretty(STDOUT,false);
        delete reportt;
      }
    }
  }

  bool Alignment::advanceToNextTraceIfNeededAndPossible(FileTracePtrPair & ftpp) {
    if (ftpp.second == 0) {
      Trace * raw = ftpp.first->read(true);
      if (raw) {
        ftpp.second = raw;        
      }
    }
    return (ftpp.second != 0);    // False if trace needed but no more found
  }

  struct timespec Alignment::nextEarliestTime(FileTracePtrPair & ftpp) {
    struct timespec ret;
    if (!advanceToNextTraceIfNeededAndPossible(ftpp)) {
      ret.tv_sec = S32_MAX;
      ret.tv_nsec = 0;
    } else {
      ret = ftpp.second->getTimespec();
    }
    return ret;
  }

  Trace * Alignment::extractNextEarliestTrace(FileTracePtrPair & ftpp) {
    Trace * ret;
    if (!advanceToNextTraceIfNeededAndPossible(ftpp)) {
      FAIL(ILLEGAL_STATE);
    }
    ret = ftpp.second;
    ftpp.second = 0;
    return ret;
  }

  bool Alignment::addLogFile(const char * path) {
    // Try opening it to frontload errors
    FILE * file = fopen(path, "r");
    if (!file) {
      LOG.Error("Can't read %s: %s", path, strerror(errno));
      return false;
    }
    fclose(file);
    WLFandTracePtrsPair pp(new WeaverLogFile(path,mWeaverLogFiles.size()), 0);
    mWeaverLogFiles.push_back(pp); // Takes ownership
    return true;
  }


#define ALL_CMD_ARGS()                                          \
  XX(help,h,N,,"Print this help")                               \
  XX(map,m,N,,"Print the sync map")                             \
  XX(tweak,t,O,TWEAK,"Tweak file timing")                       \
  XX(version,v,N,,"Print version and exit")                     \

#if 0
  XX(log,l,O,LEVEL,"Set or increase logging")                   \
  XX(mfzid,z,R,MFZID,"Specify MFZ file to run")                 \
  XX(paused,p,N,,"Start up paused")                             \
  XX(trace,t,R,PATH,"Trace output to file")                     \
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
      int wantmap = 0;
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

        case 'm':
          wantmap = 1;
          break;

        case 'w':
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

        case 't':
          if (optarg) {
            if (!mAlignment.parseTweak(optarg)) exit(1);
          }
          break;

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
      mAlignment.setPrintSyncMap(wantmap > 0);
      LOG.Message("WM %d", wantmap);
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

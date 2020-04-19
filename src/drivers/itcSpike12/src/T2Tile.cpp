#include <getopt.h>

#include "T2Tile.h"

// mfms files
#include "Point.h"   // For UPoint

// Spike files
#include "T2Types.h"
#include "T2EventWindow.h"

#define ALL_DIR6_MACRO() XX(ET)YY XX(SE)YY XX(SW)YY XX(WT)YY XX(NW)YY XX(NE)ZZ

namespace MFM {
  void EWInitiator::onTimeout(TimeQueue& srctq) {
    T2Tile& tile = T2Tile::get();
    if (tile.isLiving()) {
      //GENERATE EVENT LOAD      for (u32 i = 0; i < 1000000; ++i) tile.getRandom().Create();
      schedule(srctq);
      tile.maybeInitiateEW();
    }
  }



  T2Tile::T2Tile()
    : mArgc(0)
    , mArgv(0)
    , mMFZId(0)
    , mWindowConfigPath(0)
    , mWidth(T2TILE_WIDTH)
    , mHeight(T2TILE_HEIGHT)
    , mExitRequest(false)
    , mITCs{
#define XX(dir6) T2ITC(*this,DIR6_##dir6,#dir6)
#define YY ,
#define ZZ
        ALL_DIR6_MACRO()
#undef XX
#undef YY
#undef ZZ
      }
    , mFree(*this)
    , mTimeQueue(this->getRandom())
    , mSDLI(*this,"SDLI")
    , mADCCtl(*this)
    , mSites() 
    , mSiteOwners() // Initted in earlyInit
    , mLiving(false)
    , mMDist()
  {
  }

  void T2Tile::earlyInit() {
    processArgs();

    for (u32 i = 0; i <= MAX_EWSLOT; ++i) {
      if (i==0) mEWs[i] = 0;
      else {
        mEWs[i] = new T2EventWindow(*this, i);
        mEWs[i]->insertInEWSet(&mFree);
      }
    }

    for (u32 x = 0; x < T2TILE_WIDTH; ++x)
      for (u32 y = 0; y < T2TILE_HEIGHT; ++y) 
        mSiteOwners[x][y] = 0;

    initTimeQueueDrivers();
  }

  void T2Tile::insertOnMasterTimeQueue(TimeoutAble & ta, u32 fromNow, s32 fuzzbits) {
    ta.insert(mTimeQueue,fromNow,fuzzbits);
  }

  void T2Tile::initTimeQueueDrivers() {
    debug("initTimeQueueDrivers");
  }

#define ALL_CMD_ARGS()                                          \
  XX(help,h,N,,"Print this help")                               \
  XX(log,l,O,LEVEL,"Set or increase logging")                   \
  XX(mfzid,z,R,MFZID,"Specify MFZ file to run")                 \
  XX(paused,p,N,,"Start up paused")                             \
  XX(version,v,N,,"Print version and exit")                     \
  XX(wincfg,w,R,PATH,"Specify window configuration file")       \

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

  void T2Tile::processArgs() {
    int c;
    int option_index;
    int fails = 0;
    int loglevel = -1;
    int wantpaused = 0;
    while ((c = getopt_long(mArgc,mArgv,
                            CMD_LINE_SHORT_OPTIONS,
                            CMD_LINE_LONG_OPTIONS,
                            &option_index)) != -1) {
      switch (c) {
      case 'z':
        setMFZId(optarg);
        break;

      case 'p':
        wantpaused = 1;
        break;

      case 'w':
        setWindowConfigPath(optarg);
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
    if (!fails && getMFZId()==0) {
      error("Missing -z MFZID");
      ++fails;
    }
    while (optind < mArgc) {
        error("Extra argument '%s'",mArgv[optind++]);
        ++fails;
    }
    if (fails) {
      fatal("%d command line problem%s",fails,fails==1?"":"s");
    }
    if (loglevel >= 0) {
      LOG.SetLevel(loglevel);
    }
    if (wantpaused >= 0) 
      this->setLiving(wantpaused == 0);
  }

#define MFZID_DEV "/sys/class/itc_pkt/mfzid"

  const char * T2Tile::getMFZId() const { return mMFZId; }

  void T2Tile::setMFZId(const char * mfzid) {
    assert(mfzid!=0);
    assert(mMFZId==0);
    mMFZId = mfzid;

    int fd = ::open(MFZID_DEV,O_WRONLY);
    if (fd < 0) fatal("open " MFZID_DEV ": %s", strerror(errno));

    if (write(fd,mMFZId,strlen(mMFZId)) < 0) fatal("write " MFZID_DEV ": %s", strerror(errno));

    ::close(fd);
  }

  void T2Tile::setWindowConfigPath(const char * path) {
    assert(path!=0);
    if (mWindowConfigPath) {
      fatal("Duplicate --wincfg");
    }
    mWindowConfigPath = path;
  }

  T2Tile::~T2Tile() {
    closeITCs();
    for (u32 i = 0; i <= MAX_EWSLOT; ++i) {
      if (i) delete mEWs[i];
      mEWs[i] = 0;
    }
  }

  bool T2Tile::openITCs() {
    int failed = 0;
    for (int i = 0; i < DIR6_COUNT; ++i) {
      T2ITC & itc = mITCs[i];
      int err = itc.open();
      if (err < 0) 
        warn("openITC error %d: Could not open %s: %s",
              ++failed,
              itc.path(),
              strerror(-err));
      else debug("opened %s",itc.path());
      insertOnMasterTimeQueue(itc,100);
    }
    return failed == 0;
  }

  void T2Tile::closeITCs() {
    for (int i = 0; i < DIR6_COUNT; ++i) {
      T2ITC & itc = mITCs[i];
      itc.close();
      debug("closed %s\n",itc.path());
    }
  }

  T2EventWindow * T2Tile::allocEW() {
    EWLinks * el = mFree.removeRandom();
    if (!el) return 0;
    return el->asEventWindow();
  }

  void T2Tile::freeEW(T2EventWindow * ew) {
    assert(ew!=0);
    if (ew->isInSet()) ew->removeFromEWSet();
    ew->insertInEWSet(&mFree);
  }

  T2EventWindow * T2Tile::tryAcquireEW(const UPoint center, u32 radius, bool forActive) {
    T2EventWindow * ew = allocEW();  // See if any EWs left to acquire
    if (!ew) return ew;  // Nope

    const SPoint origin(0,0);
    const SPoint maxSite(T2TILE_WIDTH-1,T2TILE_HEIGHT-1);
    const u32 first = mMDist.GetLastIndex(0);
    const u32 last = mMDist.GetLastIndex(radius);

    // First check if region is all available
    for (u32 sn = first; sn <= last; ++sn) {
      SPoint offset = mMDist.GetPoint(sn);
      SPoint site = MakeSigned(center) + offset;
      if (!site.BoundedBy(origin,maxSite)) continue;
      if (mSiteOwners[site.GetX()][site.GetY()] != 0) {
        freeEW(ew); // Toss unused
        return 0;  // It's not
      }
    }

    // OK we're going for it.  Hog the region
    for (u32 sn = first; sn <= last; ++sn) {
      SPoint offset = mMDist.GetPoint(sn);
      SPoint site = MakeSigned(center) + offset;
      if (!site.BoundedBy(origin,maxSite)) continue;
      mSiteOwners[site.GetX()][site.GetY()] = ew;
    }

    ew->assignCenter(center, radius, forActive);
    return ew;
  }

  void T2Tile::resourceAlert(ResourceType type, ResourceLevel level) {
    // XXX Umm until we have some clue
    LOG.Warning("%s %s",
                resourceTypeName(type),
                resourceLevelName(level));
  }

  void T2Tile::releaseEW(T2EventWindow * ew) {
    assert(ew != 0);
    assert(ew->isAssigned());
    UPoint center = ew->getCenter();
    u32 radius = ew->getRadius();
    const SPoint origin(0,0);
    const SPoint maxSite(T2TILE_WIDTH-1,T2TILE_HEIGHT-1);
    const u32 first = mMDist.GetLastIndex(0);
    const u32 last = mMDist.GetLastIndex(radius);

    // OK, unhog the region
    for (u32 sn = first; sn <= last; ++sn) {
      SPoint offset = mMDist.GetPoint(sn);
      SPoint site = MakeSigned(center) + offset;
      if (!site.BoundedBy(origin,maxSite)) continue;
      assert(mSiteOwners[site.GetX()][site.GetY()] == ew);
      mSiteOwners[site.GetX()][site.GetY()] = 0;
    }

    ew->releaseCenter();
    freeEW(ew);
  }

  void T2Tile::initEverything(int argc, char **argv) {
    mArgc = argc;
    mArgv = argv;
    earlyInit();
    mSDLI.init();
    insertOnMasterTimeQueue(mSDLI, 0); // Live for display and input
    if (!openITCs()) fatal("Open failed\n");
  }

  void T2Tile::maybeInitiateEW() {
    UPoint ctr = UPoint(getRandom(), T2TILE_OWNED_WIDTH, T2TILE_OWNED_HEIGHT) + UPoint(CACHE_LINES,CACHE_LINES);
    u32 phonyRadius = getRandom().Between(1,4);
    T2EventWindow * ew = tryAcquireEW(ctr,phonyRadius,true); // true -> it will be an active EW if we get it
    if (!ew) return; 
    debug("INITIATING ew %p at (%d,%d)+%d\n",ew,ctr.GetX(), ctr.GetY(), phonyRadius);
    insertOnMasterTimeQueue(*ew,0);
  }

  void T2Tile::advanceITCs() { DIE_UNIMPLEMENTED(); }

  void T2Tile::maybeDisplay() { DIE_UNIMPLEMENTED(); }

  void T2Tile::shutdownEverything() {
    closeITCs();
  }

  void T2Tile::main() {
    while (!isDone()) {
      TimeoutAble * ta = mTimeQueue.getEarliestExpired();
      if (!ta)
        usleep(1000); // Or ??
      else ta->onTimeout(mTimeQueue);
    }
    shutdownEverything();
  }

#if 0
  void T2Tile::onePass() {
    maybeInitiateEW();
    updateActiveEWs();
    advanceITCs();
    maybeDisplay();
  }
#endif
}

#include <getopt.h>

#include "T2Tile.h"

// mfms files
#include "Point.h"   // For UPoint

// Spike files
#include "T2Types.h"
#include "T2EventWindow.h"
#include "TraceTypes.h"

#define ALL_DIR6_MACRO() XX(ET)YY XX(SE)YY XX(SW)YY XX(WT)YY XX(NW)YY XX(NE)ZZ

namespace MFM {

  const char * getDir6Name(Dir6 dir6) {
    switch (dir6) {
    default: return "illegal";
#define XX(dir6) case DIR6_##dir6: return #dir6;
#define YY 
#define ZZ
      ALL_DIR6_MACRO()
#undef XX
#undef YY
#undef ZZ
     }
  }

  CoreTempChecker::CoreTempChecker() {
    LOG.Debug("%s",__PRETTY_FUNCTION__);
  }

  void CoreTempChecker::onTimeout(TimeQueue& srctq) {
    T2Tile& tile = T2Tile::get();
    CoreTemperatureWatcher & ctw = tile.getADCCtl().mCoreTemperature;
    ResourceLevel ctmp = ctw.mLevel;
    CPUFreq & cf = tile.getCPUFreq();
    CPUSpeed last = cf.getLastSetSpeed();
    
    if (ctmp <= RL_ACCEPTABLE) {
      if (last < CPUSpeed_Fastest) 
        cf.setSpeed(cf.getFaster(last));
    } else if (ctmp <= RL_HIGH) {
      if (last > CPUSpeed_Slowest) {
        cf.setSpeed(cf.getSlower(last));
      }
    }
    scheduleWait(WC_LONG);
  }


  EWInitiator::EWInitiator() {
    LOG.Debug("%s",__PRETTY_FUNCTION__);
  }

  void EWInitiator::onTimeout(TimeQueue& srctq) {
    T2Tile& tile = T2Tile::get();
    if (!tile.isLiving()) scheduleWait(WC_LONG);  // Not running EWs
    else if (tile.maybeInitiateEW()) scheduleWait(WC_NOW);  // EW started
    else scheduleWait(WC_RANDOM_SHORT); // No EWs?  Short wait
  }

  void KITCPoller::onTimeout(TimeQueue& srctq) {
    u8 buf[8];
    ::lseek(mKITCStatusFD, 0, SEEK_SET);
    if (read(mKITCStatusFD,buf,8) != 8) abort();
    for (ITCIterator itr = mITCIteration.begin(); itr.hasNext(); ) {
      Dir6 dir6 = itr.next();
      Dir8 dir8 = mapDir6ToDir8(dir6);
      u8 ch = buf[(DIR8_COUNT-1)-dir8];
      if (ch >= '0' && ch <= '9') ch = ch-'0';
      else if (ch >= 'a' && ch <= 'f') ch = ch-'a'+10;
      else if (ch >= 'A' && ch <= 'F') ch = ch-'A'+10;
      else abort();
      if (getKITCEnabledStatus(dir8) != ch) {
        setKITCEnabledStatus(dir8, ch);
        mTile.getITC(dir6).bump(); // Something's changed
      }        
    }
    scheduleWait(WC_FULL);
  }

  KITCPoller::KITCPoller(T2Tile& tile)
    : mTile(tile)
    , mITCIteration(mTile.getRandom(), 1000)
    , mKITCStatusFD(-1)
    , mKITCEnabledStatus(0)
  {
    const char * STATUS_PATH = "/sys/class/itc_pkt/status";
    int ret = ::open(STATUS_PATH, O_RDONLY);
    if (ret < 0) {
      LOG.Error("Can't open %s: %s",STATUS_PATH,strerror(errno));
      FAIL(ILLEGAL_STATE);
    }
    mKITCStatusFD = ret;
    schedule(mTile.getTQ(),0);  // Not scheduleWait here: T2Tile ctor is running
  }

  u32 KITCPoller::updateKITCEnabledStatusFromStatus(u32 status, Dir8 dir8, u32 val) {
    const u32 shift = (dir8<<2);
    const u32 mask = 0xf<<shift;
    const u32 posval = val<<shift;
    const u32 newstatus = (status&~mask)|(posval&mask);
    return newstatus;
  }

  void T2Tile::seedPhysics() {
    // Pick a random hidden site and init it
    SPoint at(getOwnedRect().PickRandom(getRandom()));
    Sites & sites = getSites();
    OurT2Site & site = sites.get(MakeUnsigned(at));
    OurT2Atom & ar =  site.GetAtom();
    ////XXXX
    OurT2Atom phonyDReg(T2_PHONY_DREG_TYPE);
    ar = phonyDReg;
  }

  void T2Tile::clearPrivateSites() {
    // Clear all sites that are definitely not cached offtile
    Sites & sites = getSites();
    for (u32 x = 0; x < T2TILE_WIDTH; ++x) {
      for (u32 y = 0; y < T2TILE_HEIGHT; ++y) {
        UPoint u(x,y);
        SPoint s(MakeSigned(u));
        bool clearable = true;
        for (Dir6 dir6 = 0; dir6 < DIR6_COUNT; ++dir6) {
          if (getVisibleAndCacheRect(dir6).Contains(s)) {
            T2ITC & itc = getITC(dir6);
            if (itc.isCacheUsable()) {
              clearable = false;
              break;
            }
          }
        }
        if (!clearable) continue;
        OurT2Site & site = sites.get(u);
        OurT2Atom & atom = site.GetAtom();
        atom.SetEmpty();
      }
    }
  }

  T2Tile::T2Tile()
    : mRandom()                      // Earliest service!
    , mTimeQueue(this->getRandom())  // Next earliest!
    , mTraceLoggerPtr(0)
    , mArgc(0)
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
    , mOwnedRect(SPoint(CACHE_LINES,CACHE_LINES),
                 UPoint(T2TILE_WIDTH-2*CACHE_LINES,
                        T2TILE_HEIGHT-2*CACHE_LINES))
    , mITCVisible{
#define XX(dir6) mITCs[DIR6_##dir6].getRectForTileInit(CACHE_LINES,CACHE_LINES)
#define YY ,
#define ZZ
        ALL_DIR6_MACRO()
#undef XX
#undef YY
#undef ZZ
      }
    , mITCCache{
#define XX(dir6) mITCs[DIR6_##dir6].getRectForTileInit(CACHE_LINES,0u)
#define YY ,
#define ZZ
        ALL_DIR6_MACRO()
#undef XX
#undef YY
#undef ZZ
      }
    , mITCVisibleAndCache{
#define XX(dir6) mITCs[DIR6_##dir6].getRectForTileInit(2*CACHE_LINES,0u)
#define YY ,
#define ZZ
        ALL_DIR6_MACRO()
#undef XX
#undef YY
#undef ZZ
      }
    , mEWs{ 0 }
    , mFree(*this)
    , mSDLI(*this,"SDLI")
    , mADCCtl(*this)
    , mSites()    // Initted (for now) in earlyInit
    , mSiteOwners() // Initted in earlyInit
    , mEWInitiator()
    , mKITCPoller(*this)
    , mLiving(false)
    , mPacketPoller(*this)
    , mListening(false)
    , mMDist()
    , mTotalEventsCompleted(0)
    , mCPUFreq(CPUSpeed_Slow)
    , mCoreTempChecker()
  {
    mCoreTempChecker.schedule(getTQ(),0);
  }

  void T2Tile::earlyInit() {
    processArgs();

    for (u32 i = 0; i < MAX_EWSLOT; ++i) {
      mEWs[i] = new T2ActiveEventWindow(*this, i, "AC");
      mEWs[i]->insertInEWSet(&mFree);
    }

    for (u32 x = 0; x < T2TILE_WIDTH; ++x)
      for (u32 y = 0; y < T2TILE_HEIGHT; ++y) 
        mSiteOwners[x][y] = 0;

#if 0  // LEAVE INITIAL WORLD EMPTY   
    /////XXXXX MAKE A PHONYDREG
    OurT2Atom phonyDReg(T2_PHONY_DREG_TYPE);
    //UPoint ctr(3*CACHE_LINES/2,3*CACHE_LINES/2); // upper left + half EWR
    UPoint ctr(CACHE_LINES,CACHE_LINES); // should be upper left
    //UPoint ctr(T2TILE_WIDTH/2,T2TILE_HEIGHT/2); // dead center (so no itcs)
    //UPoint ctr(T2TILE_WIDTH-CACHE_LINES-1,T2TILE_HEIGHT-CACHE_LINES-1); // lower right
    OurT2Site & sr =  mSites.get(ctr);
    OurT2Atom & ar =  sr.GetAtom();
    ar = phonyDReg;
#endif
    
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
  XX(map,m,O,CSV,"Print tile map [in CSV] and exit")            \
  XX(mfzid,z,R,MFZID,"Specify MFZ file to run")                 \
  XX(paused,p,N,,"Start up paused")                             \
  XX(trace,t,O,PATH,"Trace output to PATH or default")          \
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

      case 't':
        if (optarg) {
          startTracing(optarg);
        } else {
          u32 gen = 0;
          const char * zpath = 0;
          do {
            OString128 path;
            path.Printf("/tmp/t2trace%D.dat",gen++);
            const char * zmaybe = path.GetZString();
            FILE * file = fopen(zmaybe,"r");
            if (file != 0) {  // File already exists
              fclose(file);
              continue;
            }
            file = fopen(zmaybe,"w");
            if (file == 0) 
              fatal("Can't write '%s': %s", zmaybe, strerror(errno));
            else {
              fclose(file);
              zpath = zmaybe;
              break;
            }
          } while (gen < 1000);
          if (!zpath) 
            fatal("Filename autogen failed for -t");
          STDOUT.Printf("TRACING TO: %s\n",zpath);
          startTracing(zpath);
        }
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
        if (loglevel > 0) {
          LOG.SetLevel(loglevel);
        }
        break;

      case 'h':
        printf("%s",CMD_HELP_STRING);
        exit(0);

      case 'm':
        {
          const char * oa = optarg;
          if (!oa) oa = "csv";
          if (!strcmp(oa,"csv") || !strcmp(oa,"all"))
            dumpTileMap(STDOUT,true);
          if (!strcmp(oa,"map") || !strcmp(oa,"all"))
            dumpTileMap(STDOUT,false);
          if (!strcmp(oa,"itc") || !strcmp(oa,"all"))
            dumpITCRects(STDOUT);

        }
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
    if (wantpaused >= 0) {
      this->setLiving(wantpaused == 0);
      this->setListening(wantpaused == 0);
    }
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
    stopTracing();
    closeITCs();
    for (u32 i = 0; i < MAX_EWSLOT; ++i) {
      if (i) delete mEWs[i];
      mEWs[i] = 0;
    }
  }

  void T2Tile::resetITCs() {
    for (int i = 0; i < DIR6_COUNT; ++i) 
      mITCs[i].reset();
  }

  void T2Tile::closeITCs() {
    for (int i = 0; i < DIR6_COUNT; ++i) {
      T2ITC & itc = mITCs[i];
      itc.close();
      debug("closed %s",itc.path());
    }
  }

  T2ActiveEventWindow * T2Tile::allocEW() {
    EWLinks * el = mFree.removeRandom();
    if (!el) return 0;
    T2EventWindow * ew = el->asEventWindow();
    T2ActiveEventWindow * aew = ew->as<T2ActiveEventWindow>();
    MFM_API_ASSERT_STATE(aew != 0);
    return aew;
    
  }

  void T2Tile::freeEW(T2EventWindow & ew) {
    if (ew.isInSet()) ew.removeFromEWSet();
    ew.insertInEWSet(&mFree);
  }

  bool T2Tile::tryAcquireEW(const UPoint center, u32 radius, bool forActive) {
    T2ActiveEventWindow * ew = allocEW();  // See if any EWs left to acquire
    if (!ew) return false;  // Nope
    ew->initializeEW(); // clear gunk
    if (ew->tryInitiateActiveEvent(center,radius)) return true; // ew in use
    ew->finalizeEW();
    freeEW(*ew);
    return false;
  }

  void T2Tile::releaseActiveEW(T2EventWindow & ew) {
    Sites & sites = getSites(); // GET CENTER SITE FOR STATS!
    UPoint uctr(MakeUnsigned(ew.getCenter()));
    OurT2Site & ctrSite = sites.get(uctr); 
    recordCompletedEvent(ctrSite);  // That Was NOT Easy!

    ew.finalizeEW();
    freeEW(ew);
  }

  void T2Tile::resourceAlert(ResourceType type, ResourceLevel level) {
    // XXX Umm until we have some clue
    LOG.Warning("%s %s",
                resourceTypeName(type),
                resourceLevelName(level));
  }

#if 0 // ARE WE USING THIS?
  void T2Tile::releaseEW(T2EventWindow * ew) {
    assert(ew != 0);
    assert(ew->isAssigned());
    SPoint center = ew->getCenter();
    u32 radius = ew->getRadius();
    const SPoint origin(0,0);
    const SPoint maxSite(T2TILE_WIDTH-1,T2TILE_HEIGHT-1);
    const u32 first = mMDist.GetLastIndex(0);
    const u32 last = mMDist.GetLastIndex(radius);

    // OK, unhog the region
    for (u32 sn = first; sn <= last; ++sn) {
      SPoint offset = mMDist.GetPoint(sn);
      SPoint site = center + offset;
      if (!site.BoundedBy(origin,maxSite)) continue;
      assert(mSiteOwners[site.GetX()][site.GetY()] == ew);
      mSiteOwners[site.GetX()][site.GetY()] = 0;
    }

    ew->releaseCenter();
    freeEW(*ew);
  }
#endif

  void T2Tile::initEverything(int argc, char **argv) {
    mArgc = argc;
    mArgv = argv;
    earlyInit();
    mSDLI.init();
    insertOnMasterTimeQueue(mSDLI, 0); // Live for display and input
    resetITCs();
  }

  u32 T2Tile::getRadius(const OurT2Atom & atom) {
    if (atom.GetType() == OurT2Atom::ATOM_EMPTY_TYPE) return 0u;
    return 4u;
  }

  void T2Tile::recordCompletedEvent(OurT2Site & site) {
    site.RecordEventAtSite(++mTotalEventsCompleted);
  }

  u32 T2Tile::considerSiteForEW(UPoint idx) {
    const SPoint origin(0,0);
    const SPoint maxSite(T2TILE_WIDTH-1,T2TILE_HEIGHT-1);
    MFM_API_ASSERT_ARG(MakeSigned(idx).BoundedBy(origin,maxSite));
    if (mSiteOwners[idx.GetX()][idx.GetY()] != 0) return 0u;
    OurT2Site & site = mSites.get(idx);
    const OurT2Atom & atom = site.GetAtom();
    u32 radius = getRadius(atom);
    if (radius == 0u) 
      recordCompletedEvent(site);  // That was easy
    return radius;
  }

  bool T2Tile::maybeInitiateEW() {
    // Empty events drain here; they're cheap
    //    const u32 MAX_TRIES = T2TILE_OWNED_WIDTH*T2TILE_OWNED_HEIGHT; 
    const u32 MAX_TRIES = 250;
    u32 radius = 0;
    UPoint ctr;
    for (u32 i = 0; i < MAX_TRIES; ++i) {
      ctr =
        UPoint(getRandom(), T2TILE_OWNED_WIDTH, T2TILE_OWNED_HEIGHT)
        + UPoint(CACHE_LINES,CACHE_LINES);
      radius = considerSiteForEW(ctr);
      if (radius > 0) break;
    }
    if (radius == 0) return false;
    return tryAcquireEW(ctr, radius, true);
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
      else {
        //        LOG.Message("TO %s",ta->getName());
        ta->onTimeout(mTimeQueue);
      }
    }
    shutdownEverything();
  }

  void T2Tile::addRandomSyncTag(ByteSink & bs) {
    s32 pos;
    do { pos = mRandom.Create()&0x7fffffff; } while (pos == 0);
    bs.Printf("%l",pos);
  }

  bool T2Tile::tryReadRandomSyncTag(ByteSource & bs, s32 & tagFound) {
    s32 tag;
    if (1!=bs.Scanf("%l",&tag)) return false;
    tagFound = tag;
    return true;
  }

  void T2Tile::startTracing(const char * path) {
    if (mTraceLoggerPtr != 0) stopTracing();
    mTraceLoggerPtr = new TraceLogger(path);
    Trace evt(*this,TTC_Tile_Start);
    evt.payloadWrite().Printf("%D", TRACE_REC_FORMAT_VERSION);
    trace(evt);
    TLOG(DBG,"HEWO to %s",path);
  }

  void T2Tile::stopTracing() {
    if (mTraceLoggerPtr != 0) {
      trace(*this, TTC_Tile_Stop, "%q", mTotalEventsCompleted);
      delete mTraceLoggerPtr;
      mTraceLoggerPtr = 0;
    }
  }

  void T2Tile::showFail(const char * file, int line, const char * msg) {
    mSDLI.showFail(file, line, msg);
  }

  const char * T2Tile::coordMap(u32 x, u32 y) const {
    SPoint sp(x,y);
    const u32 BUF_SIZE = 10;
    static char buf[BUF_SIZE];
    u32 i;
    for (i = 0; i < BUF_SIZE; ++i) buf[i] = '\0';
    i = 0;
    if (getOwnedRect().Contains(sp)) buf[i++] = 'o';
    else buf[i++] = 'c';
    for (Dir6 dir6 = 0; dir6 < DIR6_COUNT; ++dir6) {
      if (getVisibleAndCacheRect(dir6).Contains(sp)) buf[i++] = '0'+dir6;
      //      if (getVisibleRect(dir6).Contains(sp)) buf[i++] = 'a'+dir6;
      //      if (getCacheRect(dir6).Contains(sp)) buf[i++] = 'A'+dir6;
    }
    return buf;
  }

  void T2Tile::dumpTileMap(ByteSink& bs, bool csv) const {
    bs.Printf("   ");
    for (u32 x = 0; x < T2TILE_WIDTH; ++x) {
      if (csv) bs.Printf(",");
      bs.Printf(" %2d", x);
    }
    bs.Printf("\n");

    for (u32 y = 0; y < T2TILE_HEIGHT; ++y) {
      bs.Printf("%2d ",y);
      for (u32 x = 0; x < T2TILE_WIDTH; ++x) {
        if (csv) bs.Printf(",");
        bs.Printf(csv ? "%s": "%3s",coordMap(x,y));
      }
      if (!csv) bs.Printf(" %2d",y);
      bs.Printf("\n");
    }
    if (!csv) {
      bs.Printf("   ");
      for (u32 x = 0; x < T2TILE_WIDTH; ++x) bs.Printf(" %2d", x);
      bs.Printf("\n");
    }
  }

  void T2Tile::dumpITCRects(ByteSink& bs) const {
    const char names[][10] = { "VIZ", "CCH", "ALL" };
    bs.Printf("      ");
    for (u32 type = 0; type < 3; ++type) {
      bs.Printf("%s                    ",names[type]);
    }
    bs.Printf("\n");
    for (Dir6 dir6 = 0; dir6 < DIR6_COUNT; ++dir6) {
      bs.Printf("%s %d: ",
                getDir6Name(dir6), dir6);
      for (u32 type = 0; type < 3; ++type) {
        Rect rect;
        switch (type) {
        case 0: rect = getVisibleRect(dir6); break;
        case 1: rect = getCacheRect(dir6); break;
        case 2: rect = getVisibleAndCacheRect(dir6); break;
        default: FAIL(UNREACHABLE_CODE);
        }
        SPoint from = rect.GetPosition();
        UPoint size = rect.GetSize();
        SPoint to   = from + MakeSigned(size) + SPoint(-1,-1);
        bs.Printf("(%2d,%2d)-(%2d,%2d) %2dx%2d  ",
                  from.GetX(),from.GetY(),
                  to.GetX(),to.GetY(),
                  size.GetX(),size.GetY());
      }
      bs.Printf("\n");
    }
  }

  void T2Tile::traceSite(const UPoint at, const char * msg, Logger::Level level) const {
    const Sites & sites = getSites();
    const OurT2Site & site = sites.get(at);
    OurT2Atom copy = site.GetAtom();
    OurT2AtomSerializer as(copy);
    TLOGLEV(level,"%s/Site(%d,%d)%s [%04x/%@]",
            msg,
            at.GetX(), at.GetY(),
            coordMap(at.GetX(), at.GetY()),
            copy.GetType(),
            &as);
  }

  void T2Tile::debugSetup() {
    clearPrivateSites();
    ////BUG 10 SETUP
    //    SPoint at(26,28);
    //    u8 ngb = 8;
    ////BUG 11 SETUP
    //    SPoint at(51,5);
    //    u8 ngb = 2;
    ////BUG 12 SETUP
    SPoint at(32,4);
    u8 ngb = 4;

    Sites & sites = getSites();
    OurT2Site & site = sites.get(MakeUnsigned(at));
    OurT2Atom & ar =  site.GetAtom();

    OurT2Atom atom(T2_PHONY_DREG_TYPE);
    atom.SetStateField(0,3,ngb-1);

    ar = atom;
    traceSite(MakeUnsigned(at));
  }

}

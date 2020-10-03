#include "T2FlashTrafficManager.h"
#include "T2UIComponents.h"
#include "T2Tile.h"
#include "SDLI.h"
#include "Logger.h"
#include "FlashTraffic.h"
#include "AbstractRadioButton.h"

#include <string.h> /*For strcmp, strerror */

namespace MFM {
  static u32 getNow() {
    T2Tile & tile = T2Tile::get();
    return tile.getTQ().now();
  }

  const u32 MS_DELAY_PER_TTL = 666u; // 3rd future tick @~250ms/tick
  TimedFlashTraffic::TimedFlashTraffic(const FlashTraffic packet)
    : mPacket(packet)
    , mWhen(getNow() + MS_DELAY_PER_TTL * mPacket.mTimeToLive)
  { }

#if 0
#define XX(N,OP) OP,
  static const bool t2FlashButtonIsOp[] = {
     ALL_T2FLASH_BUTTON_NAMES()
     0
  };
#undef XX
#endif


  void T2FlashTrafficManager::initMaps() {
#if 0
#define XX(categ,num,nam) m_suffixToCmd[ T2FLASH_CMD_STRING(categ,nam) ] = T2FLASH_CMD_VALUE(categ,num);
    ALL_FLASH_CMDS()
#undef XX
#endif
#define XX(categ,num,nam) m_cmdToCmdName[ T2FLASH_CMD(categ,nam) ] = T2FLASH_CMD_NAME(categ,nam);
    ALL_FLASH_CMDS()
#undef XX
#define XX(categ,nam,panel) m_cmdToButtonPanelName[ T2FLASH_CMD(categ,nam) ] = #panel;
      ALL_FLASH_COMMANDABLE_BUTTON_PANELS()
#undef XX
  }

  const char * T2FlashTrafficManager::getCmdName(T2FlashCmd cmd) const {
    U32ToString::const_iterator itr = m_cmdToCmdName.find(cmd);
    if (itr == m_cmdToCmdName.end()) return "unkcmd?";
    return itr->second.c_str();
  }
    
  T2FlashCmd T2FlashTrafficManager::findCmd(const char * panelName) const {
    MFM_API_ASSERT_NONNULL(panelName);
    std::string spname(panelName);
#define XX(categ,num,nam) if (endsWith(spname,T2FLASH_CMD_STRING(categ,nam))) return T2FLASH_CMD(categ,nam);
    ALL_FLASH_CMDS();
#undef XX    
    return T2FLASH_CMD__COUNT;
  }

  T2FlashControlCmd T2FlashTrafficManager::findControlCmd(const char * panelName) const {
    MFM_API_ASSERT_NONNULL(panelName);
    std::string spname(panelName);
#define XX(nam) if (endsWith(spname,T2FLASH_CONTROL_CMD_STRING(nam))) return T2FLASH_CONTROL_CMD(nam);
    ALL_FLASH_TRAFFIC_CONTROL_PANELS();
#undef XX    
    return T2FLASH_CONTROL_CMD__COUNT;
  }

#if 0
  bool T2FlashTrafficManager::isOp(T2FlashCmd cmd) const {
    if (cmd < 0 || cmd >= T2FLASH_CMD__COUNT) return false;
    return t2FlashButtonIsOp[cmd];
  }
#endif
  
  const char * T2FlashTrafficManager::getPanelName(T2FlashCmd cmd) const {
    FAIL(INCOMPLETE_CODE);
#if 0
    
    if (cmd < 0 || cmd >= T2FLASH_CMD__COUNT) return 0;
    return t2FlashButtonNames[cmd];
#endif
  }

  void T2FlashTrafficManager::report(const FlashTraffic pkt, ByteSink& to) {
    to.Printf("%s%c%s+%d/%d(%d,%d)",
              Dirs::GetCode(pkt.mPktHdr&0x7),
              pkt.checksumValid() ? '+' : '-',
              getCmdName((T2FlashCmd) pkt.mCommand),
              pkt.mTimeToLive,
              pkt.mRange,
              pkt.mOrigin.GetX(),
              pkt.mOrigin.GetY()
              );
  }

  T2FlashTrafficManager::T2FlashTrafficManager()
    : mTTL(8)
    , mLastIndex(-1)
    , mPreparedCmd()
    , mFD(-1)
  {
  }

  void T2FlashTrafficManager::init() {
    initMaps();
    T2Tile & tile = T2Tile::get();
    int ret = open();
    if (ret < 0) {
      LOG.Error("Can't init flash traffic: %s", strerror(-ret));
      FAIL(ILLEGAL_STATE);
    }
    flushPendingPackets();
    schedule(tile.getTQ(),0);
  }

  T2FlashTrafficManager::~T2FlashTrafficManager() {
    close();
  }
  
  const char * T2FlashTrafficManager::path() const {
    return "/dev/itc/flash";
  }

  int T2FlashTrafficManager::open() {
    int ret = ::open(path(),O_RDWR|O_NONBLOCK);
    if (ret < 0) return -errno;
    mFD = ret;
    return ret;
  }

  int T2FlashTrafficManager::close() {
    int ret = ::close(mFD);
    mFD = -1;
    if (ret < 0) return -errno;
    return ret;
  }

  void T2FlashTrafficManager::sendFlashPacket(const FlashTraffic packet) {
    static ITCIteration dir6Iteration(T2Tile::get().getRandom(), 10);

    if (mFD < 0) FAIL(ILLEGAL_STATE);
    for (ITCIterator itr = dir6Iteration.begin(); itr.hasNext(); ) {
      u32 dir6 = itr.next();
      Dir8 dir8 = mapDir6ToDir8(dir6);
      if (!packet.canPropagateTo(dir8)) continue;
      FlashTraffic tosend(packet, dir8);
      ssize_t amt = write(mFD, (const char *) &tosend, sizeof(tosend));
      if (amt < 0) {
        if (errno == EHOSTUNREACH) continue;
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
          LOG.Message("Flash skipped %d", dir6);
          continue;
        }
      }
    }
  }

  void T2FlashTrafficManager::handleInboundTraffic() {
    MFM_API_ASSERT_STATE(mFD >= 0);
    while (true) {
      T2PacketBuffer pb;
      const u32 MAX_PACKET_SIZE = 255;
      int len = pb.Read(mFD, MAX_PACKET_SIZE);

      if (len == 0) {
        LOG.Error("EOF on flash %d\n", mFD);
        FAIL(ILLEGAL_STATE);
      }

      if (len < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
          return;
        }
        FAIL(ILLEGAL_STATE);
      }
      handleInboundPacket(pb);
    }
  }

  void T2FlashTrafficManager::handleInboundPacket(T2PacketBuffer & pb) {
    const char * buf = pb.GetBuffer();
    u32 len = pb.GetLength();
    if (!isStandardFlash(pb)) {
      LOG.Error("NOT FLASH? 0x%02x+%d", buf[0], len);
      return;
    }
    if (len < sizeof(FlashTraffic)) {
      LOG.Error("SHORT FLASH? 0x%02x+%d", buf[0], len);
      return;
    }
    FlashTraffic & ft = *(FlashTraffic*) buf;
    receiveFlashPacket(ft);
  }

  bool T2FlashTrafficManager::matchesPacketInPQ(const FlashTraffic packet, bool & diffOrigin) const {
    for (MultisetTimedFlashTraffic::const_iterator itr = mMultisetTimedFlashTraffic.begin();
         itr != mMultisetTimedFlashTraffic.end(); ++itr) {
      if (packet.matchesCIRO(itr->mPacket)) {
        diffOrigin = false;
        return true;
      }
      if (packet.matchesCIR(itr->mPacket)) {  // Might be loopback or other nonsense
        diffOrigin = true;
        return true;
      }

    }
    return false;
  }

  bool T2FlashTrafficManager::matchesPreviousFromOrigin(const FlashTraffic packet) const {
    BPoint origin = packet.mOrigin;
    u16 key = (((u8) origin.GetX())<<8)|((u8) origin.GetY());
    OriginKeyToFlashTraffic::const_iterator itr = mOriginKeyToFlashTraffic.find(key);
    if (itr == mOriginKeyToFlashTraffic.end()) return false;
    return packet.matchesCIRO(itr->second);
  }

  void T2FlashTrafficManager::acceptPacket(const FlashTraffic packet) {
    mMultisetTimedFlashTraffic.insert(TimedFlashTraffic(packet));
    BPoint origin = packet.mOrigin;
    u16 key = (((u8) origin.GetX())<<8)|((u8) origin.GetY());
    mOriginKeyToFlashTraffic[key] = packet;
  }

  void T2FlashTrafficManager::receiveFlashPacket(const FlashTraffic packet) {
    OString32 buf;
    report(packet,buf);
    LOG.Message("rFP:%s",buf.GetZString());
    if (!packet.checksumValid()) return;

    bool diffOrigin = false; // True if holding same command from different origin
    if (matchesPacketInPQ(packet,diffOrigin) && !diffOrigin) return; // exact dupe
    if (matchesPreviousFromOrigin(packet)) return; // late dupe?

    // If we don't already have this command, accept and remember, it
    if (!diffOrigin) {
      acceptPacket(packet);                  // accept
      mPreparedCmd = packet;                 // remember
    }
    // And propagate anything that's not an exact dupe
    sendFlashPacket(packet);               // propagate
  }

  bool T2FlashTrafficManager::flushPendingPackets() {

    u32 packetsHandled = 0;
    MFM_API_ASSERT_STATE(mFD >= 0);

    while (true) {
      T2PacketBuffer pb;
      const u32 MAX_PACKET_SIZE = 255;
      int len = pb.Read(mFD, MAX_PACKET_SIZE);

      if (len == 0) {
        fprintf(stderr,"EOF on %d\n", mFD);
        exit(5);
      }

      if (len < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
          LOG.Message("Flushed %d flash packet(s)", packetsHandled);
          mLastIndex = -1;
          break;
        } else {
          abort();
        }
      }

      ++packetsHandled;
    }
    return packetsHandled>0; /* nothing to read or flush finished */
  }

  T2FlashCommandLabel & T2FlashTrafficManager::getGOGOGO() const {
    T2Tile & tile = T2Tile::get();
    SDLI & sdli = tile.getSDLI();
    Panel * panel = sdli.lookForPanel(T2FLASH_DIST_PANEL(gogogo));
    MFM_API_ASSERT_NONNULL(panel);
    T2FlashCommandLabel * label = dynamic_cast<T2FlashCommandLabel*>(panel);
    MFM_API_ASSERT_NONNULL(label);
    return *label;
  }

  T2FlashCommandLabel & T2FlashTrafficManager::getPreparedPanel() const {
    T2Tile & tile = T2Tile::get();
    SDLI & sdli = tile.getSDLI();
    Panel * panel = sdli.lookForPanel(T2FLASH_DIST_PANEL(prepared));
    MFM_API_ASSERT_NONNULL(panel);
    T2FlashCommandLabel * label = dynamic_cast<T2FlashCommandLabel*>(panel);
    MFM_API_ASSERT_NONNULL(label);
    return *label;
  }

  void T2FlashTrafficManager::setGoGoColors(u32 now) {
    T2FlashCommandLabel & gogo = getGOGOGO();
    if (getPrepared() >= T2FLASH_CMD__COUNT) {
      gogo.SetEnabledBg(0x993333);
      gogo.SetEnabledFg(0x993333);
    } else if ((now >> 10)&1) {
      gogo.SetEnabledBg(0x00000000);
      gogo.SetEnabledFg(0xffffffff);
    } else {
      gogo.SetEnabledBg(0xffffffff);
      gogo.SetEnabledFg(0x00000000);
    }
  }

  void T2FlashTrafficManager::managePending(TimeQueue& srcTQ) {
    while (1) {
      MultisetTimedFlashTraffic::iterator oldest = mMultisetTimedFlashTraffic.begin();
      if (oldest == mMultisetTimedFlashTraffic.end()) return; // Empty pq
      const TimedFlashTraffic & elt = *oldest;
      if (time_after_eq(elt.mWhen,srcTQ.now())) return;  // Execution time not yet reached
      const FlashTraffic ready = elt.mPacket;                    // Copy command
      mMultisetTimedFlashTraffic.erase(oldest);            // Delete oldest
      executeFlashTrafficCommand((T2FlashCmd) ready.mCommand); // Do it
    }
  }
  
  void T2FlashTrafficManager::executeFlashTrafficCommand(T2FlashCmd cmd) {
    U32ToString::iterator itr = m_cmdToButtonPanelName.find(cmd);
    if (itr == m_cmdToButtonPanelName.end()) {
      LOG.Message("XXX IMPLEMENT ME NOW NOW NOW: %s",getCmdName(cmd));
    } else {
      const char * buttonPanel = itr->second.c_str();
      T2Tile & tile = T2Tile::get();
      SDLI & sdli = tile.getSDLI();
      Panel * panel = sdli.lookForPanel(buttonPanel);
      if (!panel) LOG.Error("No '%s' panel found",buttonPanel);
      else {
        u32 mods = 0;
        SDL_Event evt;
        evt.type = SDL_MOUSEBUTTONUP;
        MouseButtonEvent event(mods,evt);
        panel->Handle(event);
      }
    }
  }

  void T2FlashTrafficManager::onTimeout(TimeQueue& srcTQ) {
    setGoGoColors(srcTQ.now());
    processTraffic();
    showPrepared();
    managePending(srcTQ);
    schedule(srcTQ,250);
  }

  void T2FlashTrafficManager::processTraffic() {
    handleInboundTraffic();
  }

  void T2FlashTrafficManager::onClick(T2FlashCommandLabel *button) {
    MFM_API_ASSERT_NONNULL(button);
    const char * name = button->GetName();
    MFM_API_ASSERT_NONNULL(name);
    T2FlashControlCmd got = findControlCmd(name);
    if (got >= T2FLASH_CONTROL_CMD__COUNT) {
      LOG.Message("T2FlashTrafficManager::onClick?? %s", name);
      return;
    }
    doControlCmd(got);
  }

  void T2FlashTrafficManager::clearPrepared() {
    setPrepared(T2FLASH_CMD__COUNT);
  }

  void T2FlashTrafficManager::setPrepared(T2FlashCmd cmd) {
    mPreparedCmd = cmd;
    showPrepared();
  }

  void T2FlashTrafficManager::launchPreparedCommand() {
    T2FlashCmd cmd = getPrepared();
    u32 range = getRange();
    if (cmd < T2FLASH_CMD__COUNT && range > 0) {
      FlashTraffic ft(0x80, cmd, ++mLastIndex, range);
      sendFlashPacket(ft);      // ship it
      acceptPacket(ft);         // also accept it
      mPreparedCmd = ft;        // and remember it

      LOG.Message("LAUNCHED FLASH TRAFFIC %d",cmd);
    }
    dismissPanel();
  }

  T2FlashCmd T2FlashTrafficManager::getPrepared() const {
    T2Tile & tile = T2Tile::get();
    SDLI & sdli = tile.getSDLI();
    Panel & root = sdli.getRootPanel();
    AbstractRadioButton * pushed = AbstractRadioButton::GetPushedIfAny("FC",&root);
    if (!pushed) return T2FLASH_CMD__COUNT;
    return findCmd(pushed->GetName());
  }

  void T2FlashTrafficManager::showRange() const {
    T2Tile & tile = T2Tile::get();
    SDLI & sdli = tile.getSDLI();
    Panel * panel = sdli.lookForPanel(T2FLASH_DIST_PANEL(dist));
    MFM_API_ASSERT_NONNULL(panel);
    T2FlashCommandLabel * label = dynamic_cast<T2FlashCommandLabel*>(panel);
    if (label) {
      char buff[10];
      snprintf(buff,10,"%3d",getRange());
      label->SetText(buff);
    }
    showPrepared();
  }


  void T2FlashTrafficManager::showPrepared() const {
    T2FlashCommandLabel & prepared = getPreparedPanel();
    T2FlashCommandLabel & gogogo = getGOGOGO();
    T2FlashCmd cmd = getPrepared();
    if (cmd >= T2FLASH_CMD__COUNT) {
      prepared.SetText("--");
      gogogo.SetEnabled(false);
    } else {
      const u32 LEN = 100;
      char buff[LEN];
      snprintf(buff,LEN,"%s+%d", getCmdName(cmd), getRange());
      prepared.SetText(buff);
      gogogo.SetEnabled(true);
    }
  }

  s32 T2FlashTrafficManager::getRange() const {
    return (s32) mTTL;
  }

  s32 T2FlashTrafficManager::setRange(s32 range) {
    if (range < 0) range = 0;
    else if (range > 255) range = 255;
    mTTL = (u32) range;
    showRange();
    if (range == 0) clearPrepared();
    return getRange();
  }

  void T2FlashTrafficManager::dismissPanel() {
    T2Tile & tile = T2Tile::get();
    SDLI & sdli = tile.getSDLI();
    MenuManager & mm = sdli.getMenuManager();
    clearPrepared();
    MenuAction ma;
    ma.mKeyToPress = SDLK_LEFT; /*RETURN*/
    mm.execute(ma, 0);
  }

  void T2FlashTrafficManager::doControlCmd(T2FlashControlCmd cmd) {
    switch (cmd) {
    case T2FLASH_CONTROL_CMD_less: setRange(getRange() / 2); break;
    case T2FLASH_CONTROL_CMD_down: setRange(getRange() - 1); break;
    case T2FLASH_CONTROL_CMD_dist: LOG.Message("DO IT"); break;
    case T2FLASH_CONTROL_CMD_up:   setRange(getRange() + 1); break;
    case T2FLASH_CONTROL_CMD_more: setRange(getRange() == 0 ? 1 : getRange() * 2); break;
    case T2FLASH_CONTROL_CMD_gogogo: launchPreparedCommand(); break;
    default: LOG.Error("WTF %d?",cmd); return;
#if 0
    case T2FLASH_CONTROL_CMD_dismiss: dismissPanel(); break;
    case T2FLASH_CMD_clear: clearPrepared(); break;
    case T2FLASH_CMD_boot:
    case T2FLASH_CMD_off:
    case T2FLASH_CMD_tile:
    case T2FLASH_CMD_sites: setPrepared(cmd); break;
#endif
    }
  }
}

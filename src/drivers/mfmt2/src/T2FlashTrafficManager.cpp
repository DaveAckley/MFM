#include "T2FlashTrafficManager.h"
#include "T2UIComponents.h"
#include "T2Tile.h"
#include "SDLI.h"
#include "Logger.h"
#include "FlashTraffic.h"

#include <string.h> /*For strcmp, strerror */

namespace MFM {

#define XX(N,OP) "Flash_Button_CMD_" #N,
  static const char * t2FlashButtonNames[] = {
     ALL_T2FLASH_BUTTON_NAMES()
     0
  };
#undef XX

#define XX(N,OP) OP,
  static const bool t2FlashButtonIsOp[] = {
     ALL_T2FLASH_BUTTON_NAMES()
     0
  };
#undef XX

  T2FlashCmd T2FlashTrafficManager::findCmd(const char * name) const {
    MFM_API_ASSERT_NONNULL(name);
    for (u32 i = 0; i < T2FLASH_CMD__COUNT; ++i) {
      if (!strcmp(name,t2FlashButtonNames[i]))
        return (T2FlashCmd) i;
    }
    return T2FLASH_CMD__COUNT;
  }

  bool T2FlashTrafficManager::isOp(T2FlashCmd cmd) const {
    if (cmd < 0 || cmd >= T2FLASH_CMD__COUNT) return false;
    return t2FlashButtonIsOp[cmd];
  }

  const char * T2FlashTrafficManager::getPanelName(T2FlashCmd cmd) const {
    if (cmd < 0 || cmd >= T2FLASH_CMD__COUNT) return 0;
    return t2FlashButtonNames[cmd];
  }

  T2FlashTrafficManager::T2FlashTrafficManager()
    : mTTL(8)
    , mLastIndex(-1)
    , mPendingCmd(T2FLASH_CMD__COUNT)
    , mFD(-1)
  {
  }

  void T2FlashTrafficManager::init() {
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

  void T2FlashTrafficManager::sendFlashPacket(T2FlashCmd cmd, u32 ttl) {
    static ITCIteration dir6Iteration(T2Tile::get().getRandom(), 10);

    if (mFD < 0) FAIL(ILLEGAL_STATE);
    for (ITCIterator itr = dir6Iteration.begin(); itr.hasNext(); ) {
      u32 dir6 = itr.next();
      FlashTraffic ft(0x80|dir6, cmd, mLastIndex, ttl);
      ssize_t amt = write(mFD, (const char *) &ft, sizeof(ft));
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
      handleFlashPacket(pb);
    }
  }

  void T2FlashTrafficManager::handleFlashPacket(T2PacketBuffer & pb) {
    const char * buf = pb.GetBuffer();
    u32 len = pb.GetLength();
    if (len < 5)
      LOG.Error("SHORT FLASH %d", len);
    LOG.Message("FLASH: 0x%02x 0x%02x 0x%02x 0x%02x ",
                buf[1], buf[2], buf[3], buf[4]);
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
    const char * name = getPanelName(T2FLASH_CMD_gogogo);
    MFM_API_ASSERT_NONNULL(name);
    Panel * panel = sdli.lookForPanel(name);
    MFM_API_ASSERT_NONNULL(panel);
    T2FlashCommandLabel * label = dynamic_cast<T2FlashCommandLabel*>(panel);
    MFM_API_ASSERT_NONNULL(label);
    return *label;
  }

  void T2FlashTrafficManager::onTimeout(TimeQueue& srcTQ) {
    T2FlashCommandLabel & gogo = getGOGOGO();
    if (getPending() >= T2FLASH_CMD__COUNT) {
      gogo.SetEnabledBg(0x993333);
      gogo.SetEnabledFg(0x993333);
    } else if ((srcTQ.now() >> 10)&1) {
      gogo.SetEnabledBg(0x00000000);
      gogo.SetEnabledFg(0xffffffff);
    } else {
      gogo.SetEnabledBg(0xffffffff);
      gogo.SetEnabledFg(0x00000000);
    }
    processTraffic();
    schedule(srcTQ,250);
  }

  void T2FlashTrafficManager::processTraffic() {
    handleInboundTraffic();
  }

  void T2FlashTrafficManager::onClick(T2FlashCommandLabel *button) {
    MFM_API_ASSERT_NONNULL(button);
    const char * name = button->GetName();
    MFM_API_ASSERT_NONNULL(name);
    T2FlashCmd got = findCmd(name);
    if (got >= T2FLASH_CMD__COUNT) {
      LOG.Message("T2FlashTrafficManager::onClick?? %s", name);
      return;
    }
    doCmd(got);
  }

  void T2FlashTrafficManager::clearPending() {
    setPending(T2FLASH_CMD__COUNT);
  }

  void T2FlashTrafficManager::setPending(T2FlashCmd cmd) {
    mPendingCmd = cmd;
    showPending();
  }

  void T2FlashTrafficManager::executePending() {
    T2FlashCmd cmd = getPending();
    if (cmd < T2FLASH_CMD__COUNT) {
      sendFlashPacket(cmd,getRange());
      LOG.Message("EXECUTED FLASH TRAFFIC %d",cmd);
    }
    dismissPanel();
  }

  T2FlashCmd T2FlashTrafficManager::getPending() const {
    return mPendingCmd;
  }

  void T2FlashTrafficManager::showRange() const {
    T2Tile & tile = T2Tile::get();
    SDLI & sdli = tile.getSDLI();
    const char * name = getPanelName(T2FLASH_CMD_dist);
    MFM_API_ASSERT_NONNULL(name);
    Panel * panel = sdli.lookForPanel(name);
    MFM_API_ASSERT_NONNULL(panel);
    T2FlashCommandLabel * label = dynamic_cast<T2FlashCommandLabel*>(panel);
    if (label) {
      char buff[10];
      snprintf(buff,10,"%3d",getRange());
      label->SetText(buff);
    }
    showPending();
  }


  void T2FlashTrafficManager::showPending() const {
    T2Tile & tile = T2Tile::get();
    SDLI & sdli = tile.getSDLI();
    T2FlashCommandLabel & label = getGOGOGO();
    T2FlashCmd cmd = getPending();
    if (cmd >= T2FLASH_CMD__COUNT) {
      label.SetText("");
      label.SetEnabled(false);
    } else {
      const char * cmdname = getPanelName(cmd);
      MFM_API_ASSERT_NONNULL(cmdname);
      Panel * cmdpanel = sdli.lookForPanel(cmdname);
      MFM_API_ASSERT_NONNULL(cmdpanel);
      T2FlashCommandLabel * cmdlabel = dynamic_cast<T2FlashCommandLabel*>(cmdpanel);
      
      const u32 LEN = 100;
      char buff[LEN];
      snprintf(buff,LEN,"%s%d",cmdlabel->GetText(),getRange());
      label.SetText(buff);
      label.SetEnabled(true);
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
    if (range == 0) clearPending();
    return getRange();
  }

  void T2FlashTrafficManager::dismissPanel() {
    T2Tile & tile = T2Tile::get();
    SDLI & sdli = tile.getSDLI();
    MenuManager & mm = sdli.getMenuManager();
    clearPending();
    MenuAction ma;
    ma.mKeyToPress = SDLK_LEFT; /*RETURN*/
    mm.execute(ma, 0);
  }

  void T2FlashTrafficManager::doCmd(T2FlashCmd cmd) {
    switch (cmd) {
    case T2FLASH_CMD_less: setRange(getRange() / 2); break;
    case T2FLASH_CMD_down: setRange(getRange() - 1); break;
    case T2FLASH_CMD_dist: LOG.Message("DO IT"); break;
    case T2FLASH_CMD_up:   setRange(getRange() + 1); break;
    case T2FLASH_CMD_more: setRange(getRange() == 0 ? 1 : getRange() * 2); break;
    case T2FLASH_CMD_dismiss: dismissPanel(); break;
    case T2FLASH_CMD_clear: clearPending(); break;
    case T2FLASH_CMD_boot:
    case T2FLASH_CMD_off:
    case T2FLASH_CMD_tile:
    case T2FLASH_CMD_sites: setPending(cmd); break;
    case T2FLASH_CMD_gogogo: executePending(); break;
    default: LOG.Error("WTF %d?",cmd); return;
    }
  }
}

#include "T2TitleCard.h"
#include <algorithm>

#include "TimeQueue.h"
#include "T2Utils.h"
#include "T2Tile.h"

namespace MFM {
  T2TitleCard::T2TitleCard()
    : mTitlePanel(0)
    , mAuthorPanel(0)
    , mDetailPanel(0)
    , mFirstPosted(0)
  {
  }

  void T2TitleCard::onTimeout(TimeQueue& srcTQ) {
    const u32 INITIAL_POST_TIME_MS = 10000;
    u32 now = srcTQ.now();
    if (!mFirstPosted) {
      mFirstPosted = now;
      this->setPosting(true);
    } else if (isPosted()) {
      if (time_after(now, mFirstPosted+INITIAL_POST_TIME_MS)) {
        switchToSites();
      }
    } 
    scheduleWait(WC_FULL);
  }

  void T2TitleCard::configureTitle(const char * title, u32 len, s32 year) {
    MFM_API_ASSERT_NONNULL(title);
    MFM_API_ASSERT_NONNULL(mTitlePanel);
    std::string s(title,len);
    if (year > 0) s += " ("+std::to_string(year)+")";
    mTitlePanel->SetText(s);
  }

  void T2TitleCard::configureAuthor(const char * author, u32 len) {
    MFM_API_ASSERT_NONNULL(author);
    MFM_API_ASSERT_NONNULL(mAuthorPanel);
    std::string s(author,len);
    mAuthorPanel->SetText(s);
  }

  void T2TitleCard::configureDetail(const char * detail, u32 len) {
    MFM_API_ASSERT_NONNULL(detail);
    MFM_API_ASSERT_NONNULL(mDetailPanel);
    std::string s(detail,len);
    mDetailPanel->SetText(s);
  }

  void T2TitleCard::configure(SDLI & sdli) {
    {
      const char * pname = "TitleCardTitle";
      WrappedText * p = dynamic_cast<WrappedText*>(sdli.lookForPanel(pname));
      if (!p) fatal("Couldn't find WrappedText '%s'",pname);
      mTitlePanel = p;
    }
    {
      const char * pname = "TitleCardAuthor";
      WrappedText * p = dynamic_cast<WrappedText*>(sdli.lookForPanel(pname));
      if (!p) fatal("Couldn't find WrappedText '%s'",pname);
      mAuthorPanel = p;
    }
    {
      const char * pname = "TitleCardDetail";
      WrappedText * p = dynamic_cast<WrappedText*>(sdli.lookForPanel(pname));
      if (!p) fatal("Couldn't find WrappedText '%s'",pname);
      mDetailPanel = p;
    }
    MFM_API_ASSERT_NONNULL(this->GetParent());
    setPosting(true);

  }

  void T2TitleCard::PaintComponent(Drawing & config) {
#if 0
    static u32 bg = 0;
    bg += 0x010307;
    this->SetBackground(bg);
#endif
    Super::PaintComponent(config);
  }

  void T2TitleCard::switchToSites() {
    this->setPosting(false);

    // POST THE SITES PANEL
    const char * action = "GO Sites";
    CharBufferByteSource cbbs(action,strlen(action));
    MenuAction ma;
    bool ret = ma.parse(cbbs);
    MFM_API_ASSERT_STATE(ret);
    ma.performAction(0);
  }

  // Handle mouse button action inside our walls
  bool T2TitleCard::Handle(MouseButtonEvent & event)
  {
    switchToSites();
    return true;
  }

}


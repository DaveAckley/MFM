#include "IWeave.h"
#include "Weaver.h"
#include "TeeByteSink.h"
#include "LineTailByteSink.h"

#include <panel.h>

#include <stdlib.h>

//// STRUCTS INVOLVING NAMESPACE-POLLUTING NCURSES TYPES AND MACROS
namespace MFM {

  struct IWPanelData {
    IWPanelData(SPoint pos, UPoint size, const char * label) {
      mPos = pos;
      mSize = size;
      mLabel.Printf("%s",label);
      mNext = 0;
    }
    virtual ~IWPanelData() { }
    virtual bool handleKey(PANEL* panel, s32 key) {
      return false;
    }
    virtual void drawPanel(PANEL* panel) {
      WINDOW * win = panel_window(panel);
      wclear(win);
      IWPanelData * ipw = (IWPanelData*) panel_userptr(panel);
      box(win,0,0);
      mvwprintw(win, 0, 1, "%s", ipw->mLabel.GetZString());
    }

    SPoint mPos;
    UPoint mSize;
    OString32 mLabel;
    PANEL * mNext;
  };

  struct IWPanelDataEWMap : public IWPanelData {
    typedef IWPanelData Super;
    IWPanelDataEWMap(Weaver & w, SPoint pos, UPoint size, const char * label)
    : IWPanelData(pos,size,label)
    , mAlignment(w.mAlignment)
    , mEWSlotMap(mAlignment.getEWSlotMap())
    { }

    virtual bool handleKey(PANEL* panel, s32 key) {
      bool handled = true;
#if 0 // DEIMPLEMENTED
      u32 max = mAlignment.getTraceLocCount();
      WINDOW * win = panel_window(panel);
      s32 w;
      s32 h;
      getmaxyx(win, h, w);

      switch (key) {
      case KEY_UP:
        --mTopRecord;
        break;
      case KEY_DOWN:
        ++mTopRecord;
        break;
      case KEY_HOME:
        mTopRecord = 0;
        break;
      case KEY_END:
        mTopRecord = max-1;
        break;
      case KEY_NPAGE:
        mTopRecord += h-2;
        break;
      case KEY_PPAGE:
        mTopRecord -= h-2;
        break;
      default:
        handled = false;
      }

      if (mTopRecord < 0) mTopRecord = 0;
      else if (mTopRecord >= max - 5) mTopRecord = max - 5;
      mEWSlotMap.slewTo(mTopRecord);
#endif
      return handled;
    }
    
    virtual void drawPanel(PANEL* panel) {
      Super::drawPanel(panel);
      WINDOW * win = panel_window(panel);
      s32 w;
      s32 h;
      getmaxyx(win, h, w);

      const u32 cEWM_WIDTH = 16;
      char spaces[cEWM_WIDTH+1];
      for (u32 i = 0; i < cEWM_WIDTH;++i)
        spaces[i] = ' ';
      spaces[cEWM_WIDTH] = '\0';

      if (w-2 < (s32) cEWM_WIDTH) return; //??
      u32 effw = (u32) (w-2);

      EWSlotMap & slotmap = mAlignment.getEWSlotMap();
      EWSlotMap::OrderEWModelMap & orderMap = slotmap.getOrderEWModelMap();
      OString256 row;
      u32 fillingRow = 1;
      u32 currentOrder = 0;
      for (EWSlotMap::OrderEWModelMap::iterator orders = orderMap.begin();
           orders != orderMap.end();
           ++orders) {
        for (;currentOrder <= orders->first;++currentOrder) {
          EWModel * ewmp = 0;
          if (currentOrder == orders->first)
            ewmp = orders->second;
          
          u32 pos = row.GetLength();
          if (pos + cEWM_WIDTH >= effw) {  // Need a new row
            mvwprintw(win, fillingRow, 1, "%s", row.GetZString());
            ++fillingRow;
            if (fillingRow > (u32) h + 1) return;
            row.Reset();
          }
          if (ewmp)
            ewmp->printPretty(row,cEWM_WIDTH);
          else
            row.Printf("%s",spaces);
        }
      }
      if (row.GetLength() > 0) { // Flush last partial row
        mvwprintw(win, fillingRow, 1, "%s", row.GetZString());
      }
    }
    Alignment & mAlignment;
    EWSlotMap & mEWSlotMap;
    
  };

  struct IWPanelDataTrace : public IWPanelData {
    typedef IWPanelData Super;
    IWPanelDataTrace(Weaver & w, SPoint pos, UPoint size, const char * label)
    : IWPanelData(pos,size,label)
    , mAlignment(w.mAlignment)
    , mEWSlotMap(mAlignment.getEWSlotMap())
    , mTopRecord(0)
    , mActiveRow(0)
    { }
    virtual bool handleKey(PANEL* panel, s32 key) {
      bool handled = true;
      u32 max = mAlignment.getTraceLocCount();
      WINDOW * win = panel_window(panel);
      s32 w;
      s32 h;
      getmaxyx(win, h, w);

      switch (key) {
      case KEY_UP:
        --mTopRecord;
        break;
      case KEY_DOWN:
        ++mTopRecord;
        break;
      case KEY_HOME:
        mTopRecord = 0;
        break;
      case KEY_END:
        mTopRecord = max-1;
        break;
      case KEY_NPAGE:
        mTopRecord += h-2;
        break;
      case KEY_PPAGE:
        mTopRecord -= h-2;
        break;
      default:
        handled = false;
      }

      if (mTopRecord < 0) mTopRecord = 0;
      else if (mTopRecord >= max - 5) mTopRecord = max - 5;
      mEWSlotMap.slewTo(mTopRecord);
      return handled;
    }
    virtual void drawPanel(PANEL* panel) {
      Super::drawPanel(panel);
      WINDOW * win = panel_window(panel);
      s32 w;
      s32 h;
      getmaxyx(win, h, w);

      OString128 buf;
      buf.Printf("%d", mTopRecord);
      u32 size = buf.GetLength();
      mvwprintw(win, 0, w-size-1, "%s", buf.GetZString());

      u32 max = mAlignment.getTraceLocCount();

      for (u32 row = 0; row < (u32) h-2; ++row) {
        buf.Reset();
        u32 rec = row+mTopRecord;
        if (rec >= max) break;
        FileTrace * ft = mAlignment.getTraceAtLoc(rec);
        if (!ft) buf.Printf("Missing %d?",rec);
        else {
          ft->printPretty(buf);
          buf.Chomp();

          delete ft;
        }
        if (row == mActiveRow) {
          mvwprintw(win, row+1, 0, ">");
        }
        mvwprintw(win, row+1, 1, "%.*s", w-2, buf.GetZString());
      }
    }
    Alignment & mAlignment;
    EWSlotMap & mEWSlotMap;
    u32 mTopRecord;
    u32 mActiveRow;
  };

  struct IWPanelDataLog : public IWPanelData {
    typedef IWPanelData Super;
    IWPanelDataLog(Weaver & w, SPoint pos, UPoint size, const char * label)
    : IWPanelData(pos,size,label)
    {
      LOG.SetByteSink(mLogBuffer);
    }
#if 0
    virtual bool handleKey(PANEL* panel, s32 key) {
      switch (key) {
        
      }
      return false;
    }
#endif    
    virtual void drawPanel(PANEL* panel) {
      Super::drawPanel(panel);
      WINDOW * win = panel_window(panel);
      s32 w;
      s32 h;
      getmaxyx(win, h, w);

      s32 max = mLogBuffer.GetLines();
      s32 linesToDisplay = h-1;
      for (s32 row = linesToDisplay-1; row >= 0; --row) {
        s32 idx = ((s32) max) - 1 - (linesToDisplay-1-row);
        if (idx < 0) break;
        const char * line = mLogBuffer.GetZString((u32) idx);
        if (!line)
          FAIL(ILLEGAL_STATE);
        mvwprintw(win, row+1, 1, "%s", line);
      }
    }
    typedef LineTailByteSink<100,80> OurLogBuffer;
    OurLogBuffer mLogBuffer;
  };

  struct IWeavePrivate {
    typedef std::pair<WINDOW*,PANEL*> WindowAndPanelPtrs;
    typedef std::vector<WindowAndPanelPtrs> WAPPtrVector;
    WAPPtrVector mWAPPtrVector;

    void makePanel(IWPanelData * toOwn) {
      IWPanelData * data = toOwn;
      u32 ct = mWAPPtrVector.size();
      data->mNext =
        (ct == 0) ? 0 :
        mWAPPtrVector[ct-1].second;

      WINDOW * win = newwin(data->mSize.GetY(), data->mSize.GetX(),
                            data->mPos.GetY(), data->mPos.GetX());
      PANEL* panel = new_panel(win);
      set_panel_userptr(panel, data);
      mWAPPtrVector.push_back(WindowAndPanelPtrs(win,panel));
    }

    IWeavePrivate(Weaver& weaver)
      : mWAPPtrVector()
    {
      initCurse();
      makePanel(new IWPanelDataEWMap(weaver,SPoint(0,0),UPoint(40,24),"EWMap"));
      makePanel(new IWPanelDataLog(weaver,SPoint(0,16),UPoint(80,8),"Log"));
      makePanel(new IWPanelDataTrace(weaver,SPoint(40,0),UPoint(40,24),"Trace"));
    }

    ~IWeavePrivate() {
      while (mWAPPtrVector.size() > 0) {
        WindowAndPanelPtrs wapp = mWAPPtrVector.back();
        del_panel(wapp.second);
        delwin(wapp.first);
        mWAPPtrVector.pop_back();
      }
      endCurse();
    }

    
    void initCurse() {
      initscr();
      cbreak();
      keypad(stdscr,TRUE);
      noecho();
      clear();
    }

    void endCurse() {
      endwin();
      exit(0);
    }

    void redraw_panels() {
      for (u32 i = 0; i < mWAPPtrVector.size(); ++i) {
        WindowAndPanelPtrs wapp = mWAPPtrVector[i];
        PANEL* panel = wapp.second;
        IWPanelData * ipd = (IWPanelData*) panel_userptr(panel);
        ipd->drawPanel(panel);
      }
    }

    bool doCommand() {
      redraw_panels();
      update_panels();
      refresh();
      ///GDBISH?
      cbreak();
      keypad(stdscr,TRUE);
      noecho();
      ///
      s32 ch = getch();
      PANEL * top = panel_below(0);
      IWPanelData * ipd = (IWPanelData*) panel_userptr(top);
      switch (ch) {
      case KEY_F(9):
        return false;
      case '\f':
        clear();
        break;
      case '\t':
        {
          PANEL * bot = panel_above(0);
          top_panel(bot);
          break;
        }
      default:
        ipd->handleKey(top, ch);
        break;
      }
      return true;
    }
  };

}

namespace MFM {

  IWeave::IWeave(Weaver&w)
    : mWeaver(w)
    , mIWeavePrivatePtr(new IWeavePrivate(mWeaver))
  {
  }

  IWeave::~IWeave() {
    delete mIWeavePrivatePtr;
    mIWeavePrivatePtr = 0;
  }

  void IWeave::runInteractive() {
    while (mIWeavePrivatePtr->doCommand()) { }
  }
}

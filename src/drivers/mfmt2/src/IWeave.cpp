#include "IWeave.h"
#include "Weaver.h"
#include "TeeByteSink.h"
#include "LineTailByteSink.h"
#include "T2Tile.h" /* for getDir6Name grrr */

#include <stdlib.h>

#include <algorithm>
#include <panel.h>


//// STRUCTS INVOLVING NAMESPACE-POLLUTING NCURSES TYPES AND MACROS
namespace MFM {
  const u32 cRED_BLACK=1;
  const u32 cGREEN_BLACK=2;
  const u32 cYELLOW_BLACK=3;
  const u32 cCYAN_BLACK=4;

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
    
    static bool ewmOlder(EWModel* e1, EWModel * e2) {
      MFM_API_ASSERT_NONNULL(e1);
      MFM_API_ASSERT_NONNULL(e2);
      return e1->mTraceLoc < e2->mTraceLoc;
    }

    typedef std::vector<EWModel*> EWModelPtrVector;
    void selectAndSort(u32 fn, bool active, EWModelPtrVector & ewmpv) {
      u32 indexMin = active ? 6 : 0;
      u32 indexMax = active ? 6 : 5;
      EWSlotMap & ewslotmap = mAlignment.getEWSlotMap();
      ewmpv.clear();
      for (u32 slotnum = 0; slotnum < 32; ++slotnum) {
        for (u32 index = indexMin; index <= indexMax; ++index) {
          EWModel * ewmp = ewslotmap.findEWModel(slotnum, fn, index);
          if (!ewmp) continue; // XXXX ??
          EWModel & ewm = *ewmp;
          if (ewm.isDisplayable()) ewmpv.push_back(&ewm);
        }
      }
      std::sort(ewmpv.begin(), ewmpv.end(), ewmOlder);
    }

    virtual void drawPanel(PANEL* panel) {
      Super::drawPanel(panel);
      WINDOW * win = panel_window(panel);
      s32 w;
      s32 h;
      getmaxyx(win, h, w);
      u32 fnc = mAlignment.logFileCount();
      MFM_API_ASSERT_STATE(fnc > 0);
      MFM_API_ASSERT_STATE((u32) w > 10*fnc);
      const u32 cEWM_WIDTH = (u32) ((w-2)/(2*fnc));
      const u32 cCOL_PAD = 4;
#if 0
      char fullSpaces[cEWM_WIDTH+1];
      for (u32 i = 0; i < cEWM_WIDTH;++i) fullSpaces[i] = ' ';
      fullSpaces[cEWM_WIDTH] = '\0';
      char padSpaces[cCOL_PAD+1];
      for (u32 i = 0; i < cCOL_PAD;++i) padSpaces[i] = ' ';
      padSpaces[cCOL_PAD] = '\0';
      u32 effw = (u32) (w-2);
#endif
      if (w-2 < (s32) cEWM_WIDTH) return; //??

      EWModelPtrVector vec;
      OString64 buf;
      for (u32 fn = 0; fn < fnc; ++fn) {
        for (u32 passive = 0; passive < 2; ++passive) {
          selectAndSort(fn, !passive, vec);
          u32 col = (2*fn+passive)*cEWM_WIDTH;
          u32 row = 1;
          for (u32 item = 0; item < vec.size(); ++item) {
            EWModel * ewmp = vec[item];
            MFM_API_ASSERT_NONNULL(ewmp);
            EWModel & ewm = *ewmp;
            buf.Reset();
            ewm.printPretty(buf,cEWM_WIDTH-cCOL_PAD);
            if (row < (u32) (h-1))
              mvwprintw(win, row++, col+1, "%s", buf.GetZString());
            for (u32 idx = 0; idx < 2; ++idx) {
              Dir6 dir6;
              CircuitState cs;
              if (ewm.getCircuitInfo(idx,dir6,cs)) {
                if (row < (u32) (h-1))
                  mvwprintw(win, row++, col+2, "%s %s",
                            getDir6Name(dir6),
                            getCircuitStateName(cs));
              }
            }
          }
        }
      }
#if 0
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
          if (ewmp) {
            ewmp->printPretty(row,cEWM_WIDTH-cCOL_PAD);
            row.Printf("%s",padSpaces);
          }
          else
            row.Printf("%s",fullSpaces);
        }
      }
      if (row.GetLength() > 0) { // Flush last partial row
        mvwprintw(win, fillingRow, 1, "%s", row.GetZString());
      }
#endif
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
    , mActiveRow((size.GetY()-2)/2)
    , mSearchString()
    , mSearchDir(0)
    { }
    void doisearch() {
      if (mSearchDir == 0) return;
      u32 max = mAlignment.getTraceLocCount();
      OString128 buf;

      for (s32 rec = mSearchDir > 0 ? mTopRecord : mTopRecord - 1;
           mSearchDir > 0 ? (u32) rec < max : rec >= 0;
           mSearchDir > 0 ? ++rec : --rec) {
        FileTrace * ft = mAlignment.getTraceAtLoc(rec);
        if (!ft) {
          LOG.Error("Missing rec %d?", rec);
          return;
        }
        buf.Reset();
        ft->printPretty(buf);
        delete ft;
        if (strstr(buf.GetZString(), mSearchString.GetZString())) {
          if (mSearchDir > 0 && (u32) rec == mTopRecord) continue;
          mTopRecord = rec;
          mEWSlotMap.slewTo(mTopRecord);
          return;
        }
      }
      LOG.Message("'%s' not found", mSearchString.GetZString());
    }

    bool handleKeySearchMode(PANEL* panel, s32 key) {
      switch (key) {
      case KEY_UP: mSearchDir = -1; doisearch(); return true;
      case KEY_DOWN: mSearchDir = 1; doisearch(); return true;
        
      case '\007':
        mSearchString.Reset();
        /* FALL THROUGH */
      case '\033':
      case '\n': mSearchDir = 0; return true;
      case KEY_BACKSPACE:
        if (mSearchString.GetLength() > 0)
          mSearchString.Chomp(-1);
        return true;
      default:
        if (isprint(key)) {
          mSearchString.Printf("%c",(u8) key);
          doisearch();
          return true;
        } else {
          LOG.Message("Got %d 0x%02x '\\%03o' '%c'", key, key, key, key);
        }
        return false;
      }
    }
    virtual bool handleKey(PANEL* panel, s32 key) {
      bool handled = true;
      u32 max = mAlignment.getTraceLocCount();
      WINDOW * win = panel_window(panel);
      s32 w;
      s32 h;
      getmaxyx(win, h, w);
      
      if (mSearchDir != 0)
        return handleKeySearchMode(panel, key);
      switch (key) {
      case '/': mSearchDir = 1; break;
      case '\\': mSearchDir = -1; break;
      case KEY_UP:    --mTopRecord; break;
      case KEY_DOWN:  ++mTopRecord; break;
      case KEY_HOME:  mTopRecord = 0; break;
      case KEY_END:   mTopRecord = max-1; break;
      case KEY_NPAGE: mTopRecord += h-2; break;
      case KEY_PPAGE: mTopRecord -= h-2; break;
      default: handled = false;
      }

      if (mTopRecord < 0) mTopRecord = 0;
      else if (mTopRecord >= max - 1) mTopRecord = max - 1;
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

      if (mSearchDir != 0) {
        mvwprintw(win, h-1, 1, "%c[%.*s]",
                  mSearchDir > 0 ? 'v' : '^',
                  w-4,
                  mSearchString.GetZString());
      }

      u32 max = mAlignment.getTraceLocCount();

      s32 baseRec = mTopRecord-mActiveRow;
      for (u32 row = 0; row < (u32) h-2; ++row) {
        buf.Reset();
        s32 srec = row+baseRec;
        if (srec < 0) continue;
        u32 rec = (u32) srec;
        if (rec >= max) break;
        FileTrace * ft = mAlignment.getTraceAtLoc(rec);
        if (!ft) buf.Printf("Missing %d?",rec);
        else {
          ft->printPretty(buf);
          buf.Chomp();

          delete ft;
        }
        if (row == mActiveRow) {
          //          wattron(win, A_BOLD);
          wattron(win, COLOR_PAIR(cYELLOW_BLACK));
          mvwprintw(win, row+1, 0, ">");
        }
        mvwprintw(win, row+1, 1, "%.*s", w-2, buf.GetZString());
        if (row == mActiveRow) {
          wattroff(win, COLOR_PAIR(cYELLOW_BLACK));
          //          wattroff(win, A_BOLD);
        }
      }
    }
    Alignment & mAlignment;
    EWSlotMap & mEWSlotMap;
    u32 mTopRecord;
    u32 mActiveRow;
    OString32 mSearchString;
    s32 mSearchDir;
  };

  struct IWPanelDataLog : public IWPanelData {
    typedef IWPanelData Super;
    IWPanelDataLog(Weaver & w, SPoint pos, UPoint size, const char * label)
    : IWPanelData(pos,size,label)
    , mLogBufferP(&w.mLogBuffer)
    {
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
      if (!mLogBufferP) return;
      WINDOW * win = panel_window(panel);
      s32 w;
      s32 h;
      getmaxyx(win, h, w);

      s32 max = mLogBufferP->GetLines();
      s32 linesToDisplay = h-1;
      for (s32 row = linesToDisplay-1; row >= 0; --row) {
        s32 idx = ((s32) max) - 1 - (linesToDisplay-1-row);
        if (idx < 0) break;
        const char * line = mLogBufferP->GetZString((u32) idx);
        if (!line)
          FAIL(ILLEGAL_STATE);
        mvwprintw(win, row+1, 1, "%.*s", w-2, line);
      }
    }
    OurLogBuffer * mLogBufferP;
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
      u32 w = COLS;
      u32 h = LINES;
      u32 hs = h/2;
      u32 ws = 2*w/5;
      LOG.Message("COLS=%d, LINES=%d", w, h);
      LOG.Message("env COLUMNS=%s, LINES=%s",
                  getenv("COLUMNS"),
                  getenv("LINES"));
      makePanel(new IWPanelDataEWMap(weaver,
                                     SPoint(0,0),UPoint(w,hs),
                                     "EWMap"));
      makePanel(new IWPanelDataLog(weaver,
                                   SPoint(0,hs),UPoint(ws,h-hs),
                                   "Log"));
      makePanel(new IWPanelDataTrace(weaver,
                                     SPoint(ws,hs),UPoint(w-ws,h-hs),
                                     "Trace"));
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
      start_color();
      LOG.Message("can_change_color=%d",can_change_color());
      cbreak();
      keypad(stdscr,TRUE);
      noecho();
      clear();

      /* Initialize all the colors */
      init_color(COLOR_BLACK, 10, 10, 10);
      init_color(COLOR_YELLOW, 900, 900, 0);
      init_color(COLOR_RED, 900, 0, 0);
      init_pair(cRED_BLACK, COLOR_RED, COLOR_BLACK);
      init_pair(cGREEN_BLACK, COLOR_GREEN, COLOR_BLACK);
      init_pair(cYELLOW_BLACK, COLOR_YELLOW, COLOR_BLACK);
      init_pair(cCYAN_BLACK, COLOR_CYAN, COLOR_BLACK);
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

#include "T2TracePanel.h"

#include "TimeQueue.h"
#include "T2Utils.h"
#include "T2Tile.h"
#include "T2UIComponents.h"

namespace MFM {
  u32 T2TracePanel::changeSelection(s32 incr) {
    s32 adj = ((s32) mSelectedRow)+incr;
    if (adj < 0) adj = 0;
    else if (adj > (s32) (mRows-1))
      adj = mRows-1;
    mSelectedRow = (u32) adj;
    this->bump();
    getActionButton().SetVisible(0);
    return mSelectedRow;
  }

  void T2TracePanel::clearLogToRequest() {
    mTLIToRequest.mSeq = 0;
  }

  void T2TracePanel::setLogToRequest(const TraceLogInfo & tli) {
    mTLIToRequest = tli;
    T2TraceCtlButton & actbut = getActionButton();
    OString32 bs;
    mTLIToRequest.formatInfo(bs);
    actbut.SetText(bs.GetZString());
    actbut.SetVisible(1);
  }
  
  void T2TracePanel::requestSelection() {
    T2Tile & tile = T2Tile::get();
    TraceLogDirManager & dm = tile.getTraceLogDirManager();
    u32 seq = mDisplayedSeqs[mSelectedRow];
    if (seq == 0) {
      LOG.Error("No seq for row %d", mSelectedRow);
      return;
    }
    const TraceLogInfo * tlip = dm.getTraceLogInfoFromSeq(seq);
    if (!tlip) {
      LOG.Error("No tli for seq %d in row", seq, mSelectedRow);
      return;
    }
    setLogToRequest(*tlip);
    this->bump();
  }

  void T2TracePanel::deleteSelection() {
    LOG.Message("%s WRITE ME",__func__);
    getActionButton().SetVisible(1);
    this->bump();
  }

  void T2TracePanel::confirmAction() {
    if (mTLIToRequest.mSeq != 0) {
      T2Tile & tile = T2Tile::get();
      T2FlashTrafficManager & ftm = tile.getFlashTrafficManager();
      ftm.shipGetlogRequest(mTLIToRequest.mTag);
    }
    cancelAction();
  }

  void T2TracePanel::cancelAction() {
    T2TraceCtlButton & action = getActionButton();
    clearLogToRequest(); // Flush request regardless
    if (action.IsVisible()) // Just hide action if posted
      action.SetVisible(0);
    else { // Else dismiss parent
      MenuAction ma;
      const char * cmd = "GO GlobalMenu";
      CharBufferByteSource cbbs(cmd,strlen(cmd));
      if (!ma.parse(cbbs)) FAIL(ILLEGAL_STATE);
      ma.performAction(0);
    }
    this->bump();
  }

  T2TraceCtlButton & T2TracePanel::getActionButton() {
    Panel * sib = this;
    do {
      sib = sib->GetForward();
      MFM_API_ASSERT_NONNULL(sib); // Circular list (unless no parent)
      if (dynamic_cast<T2TraceCtlButton*>(sib)) {
        T2TraceCtlButton & ret = *dynamic_cast<T2TraceCtlButton*>(sib);
        if (endsWith(ret.GetName(),"_Action")) return ret;
      }
    } while (sib != this);
    FAIL(ILLEGAL_STATE);
  }

  void T2TracePanel::configure(SDLI & sdli) {
    /* Nothing to do so far */
  }

  void T2TracePanel::onTimeout(TimeQueue& srcTQ) {
    insert(srcTQ,5*1024);         // Otherwise update at about ~1/5HZ

    T2Tile & tile = T2Tile::get();
    TraceLogDirManager & dm = tile.getTraceLogDirManager();
    dm.initAndManageTraceDir();

    ResettableByteSink & rbs = this->GetByteSink();
    rbs.Reset();
    rbs.Printf("%d\n",mSelectedRow);
    for (u32 i = 0; i < TraceLogDirManager::MAX_TRACE_SEQ_TO_KEEP; ++i) {
      mDisplayedSeqs[i] = 0;
    }
    mRows = 0;
    for (u32 i = 0; i < TraceLogDirManager::MAX_TRACE_SEQ_TO_KEEP; ++i) {
      const TraceLogInfo * tli = dm.getTraceLogInfoByIndex(i);
      if (!tli) break;
      if (tli->mSeq == 0) continue;
      rbs.Printf(mRows == mSelectedRow ? ">" : " ");
      tli->formatInfo(rbs);
      rbs.Print("\n");
      mDisplayedSeqs[mRows] = tli->mSeq;
      ++mRows;
    }
    // Fill to top
    for (u32 i = 0; i < TraceLogDirManager::MAX_TRACE_SEQ_TO_KEEP; ++i) 
      rbs.Print("\n");
  }

}

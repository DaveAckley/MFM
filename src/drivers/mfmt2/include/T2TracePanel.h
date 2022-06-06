/* -*- C++ -*- */
#ifndef T2TRACEPANEL_H
#define T2TRACEPANEL_H

#include "TextPanel.h"
#include "TimeoutAble.h"
#include "SDLI.h"
#include "Menu.h"
#include "TraceLogInfo.h"

namespace MFM {

  typedef TextPanel<40,10> TraceText;

  struct T2TraceCtlButton; // FORWARD
  
  struct T2TracePanel : public TraceText, public TimeoutAble, public SDLIConfigurable {
    typedef TraceText Super;

    T2TracePanel()
    {
      this->setKeepNewest(false);
    }
    void configure(SDLI & sdli) ;

    virtual void onTimeout(TimeQueue& srcTQ) ;
    virtual const char * getName() const { return "T2TracePanel"; }

    virtual ~T2TracePanel() { }

    virtual void addItems(Menu& menu) {
      DIE_UNIMPLEMENTED();
    }

    u32 changeSelection(s32 incr) ;
    void requestSelection() ;
    void deleteSelection() ;
    void confirmAction() ;
    void cancelAction() ;
    const TraceLogInfo & getSelected() ;

    T2TraceCtlButton & getActionButton() ;
    
    void setLogToRequest(const TraceLogInfo & tli) ;
    void clearLogToRequest() ;

  private:
    TraceLogInfo mTLIToRequest;
    u32 mRows;
    u32 mSelectedRow;
    u32 mDisplayedSeqs[TraceLogDirManager::MAX_TRACE_SEQ_TO_KEEP];
  };
}

#endif /* T2TRACEPANEL_H */

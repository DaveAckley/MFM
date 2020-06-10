/* -*- C++ -*- */
#ifndef T2TIMEQUEUEPANEL_H
#define T2TIMEQUEUEPANEL_H

#include "TextPanel.h"
#include "TimeoutAble.h"
#include "SDLI.h"
#include "ITCStatus.h"
#include "T2ADCs.h"
#include "Menu.h"

namespace MFM {

  typedef TextPanel<48,23> TimeQueueText;

  struct T2TimeQueuePanel : public TimeQueueText, public TimeoutAble, public SDLIConfigurable {
    typedef TimeQueueText Super;

    T2TimeQueuePanel()
    {
      this->setKeepNewest(false);
    }
    void configure(SDLI & sdli) ;

    virtual void onTimeout(TimeQueue& srcTQ) ;
    virtual const char * getName() const { return "T2TimeQueuePanel"; }

    virtual ~T2TimeQueuePanel() { }

    virtual void addItems(Menu& menu) {
      DIE_UNIMPLEMENTED();
    }

    void refreshTimeQueueText() ;
  private:
  };
}

#endif /* T2TIMEQUEUEPANEL_H */

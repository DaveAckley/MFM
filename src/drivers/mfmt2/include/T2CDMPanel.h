/* -*- C++ -*- */
#ifndef T2CDMPANEL_H
#define T2CDMPANEL_H

#include "TextPanel.h"
#include "TimeoutAble.h"
#include "SDLI.h"
#include "ITCStatus.h"
#include "T2ADCs.h"
#include "Menu.h"

namespace MFM {

  typedef TextPanel<48,14> CDMText;

  struct T2CDMPanel : public CDMText, public TimeoutAble, public SDLIConfigurable {
    typedef CDMText Super;

    T2CDMPanel()
    {
      this->setKeepNewest(false);
    }
    void configure(SDLI & sdli) ;

    virtual void onTimeout(TimeQueue& srcTQ) ;
    virtual const char * getName() const { return "T2CDMPanel"; }

    virtual ~T2CDMPanel() { }

    virtual void addItems(Menu& menu) {
      DIE_UNIMPLEMENTED();
    }

  private:
  };
}

#endif /* T2CDMPANEL_H */

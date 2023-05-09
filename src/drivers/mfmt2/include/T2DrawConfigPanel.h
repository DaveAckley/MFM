/* -*- C++ -*- */
#ifndef T2DRAWCONFIGPANEL_H
#define T2DRAWCONFIGPANEL_H

#include "TextPanel.h"
#include "TimeoutAble.h"
#include "SDLI.h"
#include "Menu.h"

namespace MFM {

  typedef TextPanel<48,23> DrawConfigText;

  struct T2DrawConfigPanel : public DrawConfigText, public TimeoutAble, public SDLIConfigurable {
    typedef DrawConfigText Super;

    T2DrawConfigPanel()
    {
      this->setKeepNewest(false);
    }
    void configure(SDLI & sdli) ;

    virtual void onTimeout(TimeQueue& srcTQ) ;
    virtual const char * getName() const { return "T2DrawConfigPanel"; }

    virtual ~T2DrawConfigPanel() { }

    virtual void addItems(Menu& menu) {
      DIE_UNIMPLEMENTED();
    }

    void refreshDrawConfigText() ;
  private:
  };
}

#endif /* T2DRAWCONFIGPANEL_H */

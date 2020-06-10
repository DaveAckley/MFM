/* -*- C++ -*- */
#ifndef CHOOSERPANEL_H
#define CHOOSERPANEL_H

#include "itype.h"
#include "Panel.h"
#include "Menu.h"

namespace MFM {
  struct ChooserPanel : public MenuMakerPanel {
    
    ChooserPanel() ;
    virtual ~ChooserPanel() { }

    bool choose(int kidnum) ;

    virtual void configureItem(MenuItem& menu) ;

    virtual void addItems(Menu& menu) ;

  };
}
#endif /* CHOOSERPANEL_H */

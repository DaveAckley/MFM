/* -*- C++ -*- */
#ifndef T2TITLECARD_H
#define T2TITLECARD_H

#include "TimeoutAble.h"
#include "SDLI.h"
#include "Menu.h"
#include "WrappedText.h"

namespace MFM {

  struct T2TitleCard : public MenuMakerPanel, public TimeoutAble, public SDLIConfigurable {
    typedef MenuMakerPanel Super;

    //>> API TimeoutAble
    virtual void onTimeout(TimeQueue& srcTq) ;
    //<< API TimeoutAble

    //>>API MenuMakerPanel(Panel)
    virtual void PaintComponent(Drawing & config) ;

    virtual bool Handle(MouseButtonEvent & event) ;
    //<<API MenuMakerPanel(Panel)

    void switchToSites() ;

    void configureTitle(const char * title, u32 len, s32 year) ;

    void configureAuthor(const char * author, u32 len) ;

    void configureDetail(const char * detail, u32 len) ;

    T2TitleCard() ;

    void configure(SDLI & sdli) ;

    virtual const char * getName() const { return "T2TitleCard"; }

    virtual ~T2TitleCard() { }

    virtual void addItems(Menu& menu) {
      DIE_UNIMPLEMENTED();
    }

  private:
    WrappedText * mTitlePanel;
    WrappedText * mAuthorPanel;
    WrappedText * mDetailPanel;
    u32 mFirstPosted;
  };
}

#endif /* T2TITLECARD_H */

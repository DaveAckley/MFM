#ifndef STDPANEL_H    /* -*- C++ -*- */
#define STDPANEL_H

#include "itype.h"
#include "Panel.h"
#include "SDL/SDL.h"

namespace MFM {

  /**
     StdPanel adds several common behaviors to a Panel.
   */
  class StdPanel : public Panel
  {
  private:

    SPoint m_dragStart;
    bool m_dragging;

  public:

    StdPanel() : m_dragging(false) { }

    virtual bool Handle(SDL_MouseButtonEvent & event) ;

    virtual bool Handle(SDL_MouseMotionEvent & event) ;

    virtual bool Click(const SPoint & position, u32 button, u32 modifiers) {
      return false;
    }

    virtual bool ScrollUp() {
      return false;
    }

    virtual bool ScrollDown() {
      return false;
    }

    virtual void DragStart(const SPoint & startPosition) {
      /* empty */
    }

    virtual void Drag(const SPoint & relativeDelta) {
      /* empty */
    }

    virtual void DragEnd(const SPoint & endPosition) {
      /* empty */
    }

  };
} /* namespace MFM */
#endif /*STDPANEL_H*/


/* -*- C++ -*- */
#ifndef ULAMEVENTSYSTEM_H
#define ULAMEVENTSYSTEM_H

#include <map>

#include "T2Types.h"
#include "ElementRegistry.h"
#include "UlamClassRegistry.h"
#include "T2EventWindow.h"

namespace MFM {
  struct T2Tile; // FORWARD
  
  struct UlamEventSystem {
    enum { ELEMENT_EMPTY_TYPE = OurT2EventConfig::ATOM_CONFIG::ATOM_EMPTY_TYPE };

    UlamEventSystem(T2Tile &) ;
    
    /** \return null if good else static error message */
    const char * setUlamLibraryPath(const char * path) ;

    OurUlamClassRegistry & getUlamClassRegistry() ;

    OurElementRegistry & getElementRegistry() {
      return mElementRegistry;
    }

    const OurElement * getSeedElementIfExists() ;

    void initUlamClasses() ;

    void getUlamLibDigest(ByteSink & out) const ;

    bool doUlamEvent(T2ActiveEventWindow & aew) ;

    const OurElement * getElementIfAny(u32 type) ;

    T2Tile & mTile;
    
    OurElementRegistry mElementRegistry;

    OurEventWindow & mOurEventWindow;

    void loadOurEventWindow(T2ActiveEventWindow & aew) ;

    void saveOurEventWindow(T2ActiveEventWindow & aew) ;

    OString64 mUlamLibDigest;
  };

}

#endif /* ULAMEVENTSYSTEM_H */

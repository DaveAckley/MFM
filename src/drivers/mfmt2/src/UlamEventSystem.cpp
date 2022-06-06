#include "UlamEventSystem.h"
#include "T2Tile.h"
#include "UlamElement.h"

namespace MFM {
  UlamEventSystem::UlamEventSystem(T2Tile & tile)
    : mTile(tile)
    , mElementRegistry()
    , mOurEventWindow(mTile.GetEventWindow())
    , mUlamLibDigest()
  { }

  OurUlamClassRegistry & UlamEventSystem::getUlamClassRegistry() {
    return mTile.GetUlamClassRegistry();
  }

  const char * UlamEventSystem::setUlamLibraryPath(const char * path) {
    MFM_API_ASSERT_ARG(path!=0);
    if (mElementRegistry.GetLibraryPathsCount() > 0) 
      return "Multiple libraries illegal";
    const char * ret = mElementRegistry.AddLibraryPath(path);
    if (ret) return ret;
    mUlamLibDigest.Reset();
    if (!digestWholeFile(path, mUlamLibDigest, true))
      return "Digest failed";
    return 0;
  }

  void UlamEventSystem::getUlamLibDigest(ByteSink & out) const {
    mUlamLibDigest.AppendTo(out);
  }

  const OurElement * UlamEventSystem::getSeedElementIfExists() {
    OurElementRegistry & er = mElementRegistry;
    u32 count = er.GetEntryCount();
    for (u32 idx = 0; idx < count; ++idx) {
      const OurElement * elt = er.GetEntryElement(idx);
      // We're looking for the element whose name AND symbol are both 'S'
      if (!!strcmp("S",elt->GetAtomicSymbol())) continue;
      if (!!strcmp("S",elt->GetName())) continue;
      return elt;
    }
    return 0;
  }

  void UlamEventSystem::initUlamClasses() {
    OurElementTable & et = mTile.GetElementTable();
    OurUlamClassRegistry & ucr = this->getUlamClassRegistry();

    mElementRegistry.Init(ucr);
    
    u32 dlcount = mElementRegistry.GetRegisteredElementCount();
    for (u32 i = 0; i < dlcount; ++i) {
      OurElement * elt = mElementRegistry.GetRegisteredElement(i);
      MFM_API_ASSERT_NONNULL(elt);
      elt->AllocateType(); // Push type number info from ulam level to Element level
      if (elt->GetType() != ELEMENT_EMPTY_TYPE) // Empty already handled. ?
        et.Insert(*elt); // Put in map
    }

    const OurUlamClass * uempty = ucr.GetUlamElementEmpty();
    if (uempty) {
      const OurUlamElement * uelt = uempty->AsUlamElement();
      if (uelt) {
        et.ReplaceEmptyElement(*uelt);
      }
    }

  }

  const OurElement * UlamEventSystem::getElementIfAny(u32 type) {
    OurElementTable & et = mTile.GetElementTable();
    const OurElement * elt = et.Lookup(type);
    return elt;
  }

  void UlamEventSystem::loadOurEventWindow(T2ActiveEventWindow & aew) {
    
    /*
    OurEventWindow & ew = mOurEventWindow;
    const u32 SITES = EVENT_WINDOW_SITES(MAX_EVENT_WINDOW_RADIUS);
    for (u32 site = 0; site < SITES; ++site) {
      OurP3Atom & from = aew.getSiteAtom(site);
      OurAtomBitStorage & to = ew.GetAtomBitStorage(site);
      to.WriteAtom(from);
    }
    */
      FAIL(INCOMPLETE_CODE);
  }


  void UlamEventSystem::saveOurEventWindow(T2ActiveEventWindow & aew) {
    // Copy atoms and center site from mOurEventWindow to aew
    aew.getCenterSite().GetBase() = mOurEventWindow.GetBase();
    u32 sitecount = mOurEventWindow.GetBoundedSiteCount();
    for (u32 sn = 0; sn < sitecount; ++sn) {
      if (mOurEventWindow.IsLiveSiteDirect(sn)) 
        aew.getSiteAtom(sn) = mOurEventWindow.GetAtomDirect(sn);
    }
  }

  bool UlamEventSystem::doUlamEvent(T2ActiveEventWindow & aew) {
    SPoint evtCenter = aew.getCenter();
    bool initted = mOurEventWindow.InitForEvent(evtCenter,false); // Don't try for locks, we handled that
    if (!initted) {
      LOG.Error("Event initialization failed");
      return false;
    }

    const OurElement * elt = mOurEventWindow.GetCenterElementAfterInit();
    MFM_API_ASSERT_NONNULL(elt); // Init said true

#if 0
    const OurUlamElement * uelt = elt->AsUlamElement();
    if (uelt) {
      const UlamElementInfo<OurT2EventConfig> & uei = uelt->GetUlamElementInfo();
      LOG.Message("UE (%s) is placeable: %d",uei.GetName(),uelt->GetPlaceable()); // something to pull in UlamElement at compile-time
    }
#endif
    
    bool okay = true;
    unwind_protect(
    {
      // Backtrace already written to trace file
      okay = false;
    },
    {
      elt->Behavior(mOurEventWindow);
    });
    if (okay) 
      saveOurEventWindow(aew);  // copy traditional EW -> T2AEW
    else
      aew.emptyCenterAtom();    // dump trad EW, T2AEW empty [0]
    mOurEventWindow.SetFree();
    return okay;
  }

}

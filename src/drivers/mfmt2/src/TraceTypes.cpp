#include "TraceTypes.h"
#include "T2Tile.h"
#include "T2ITC.h"

namespace MFM {
  static const char *(traceTypeTypes[]) = {
#define XX(TYPE,BRIEF,NAME) #TYPE,
    ALL_TRACE_TYPES_MACRO()
#undef XX
    0
  };
  const char *(traceTypeBrief[])= {
#define XX(TYPE,BRIEF,NAME) #BRIEF,
    ALL_TRACE_TYPES_MACRO()
#undef XX
    0
  };
  const char *(traceTypeNames[])= {
#define XX(TYPE,BRIEF,NAME) #TYPE "_" #BRIEF ":" #NAME,
    ALL_TRACE_TYPES_MACRO()
#undef XX
    0
  };

  const char * getTraceTypeType(TraceTypeCode ttc) {
    MFM_API_ASSERT_ARG((u32) ttc < TTC_COUNT);
    return traceTypeTypes[ttc];
  }

  const char * getTraceTypeBrief(TraceTypeCode ttc) {
    MFM_API_ASSERT_ARG((u32) ttc < TTC_COUNT);
    return traceTypeBrief[ttc];
  }

  const char * getTraceTypeName(TraceTypeCode ttc) {
    MFM_API_ASSERT_ARG((u32) ttc < TTC_COUNT);
    return traceTypeNames[ttc];
  }

#if 0
  TraceTile::TraceTile(u8 traceTypeCode) 
    : Trace(traceTypeCode)
  {
    setTraceAddress(TraceAddress(T2Tile::get()));
  }

  TraceITC::TraceITC(const T2ITC& itc, u8 traceTypeCode) 
    : Trace(traceTypeCode)
  {
    setTraceAddress(TraceAddress(itc));
  }
#endif

#if 0 /*ditching the 'Arg' stuff*/
  void T2TileArg::printTo(ByteSink & bs) { /*nothing needed*/}
  bool T2TileArg::readFrom(ByteSource & bs) {
    mT2Tile = T2Tile::get();
    return true;
  }

  void ITCArg::printTo(ByteSink & bs) { bs.Printf("%c", mDir6); }
  bool T2TileArg::readFrom(ByteSource & bs) {
    u8 ch;
    if (1 != bs.Scanf("%c",&ch)) return false;
    if (ch > 5) return false;
    mDir6 = ch;
    mT2ITC = 0;
    return true;
  }

  ITCArg::ITCArg(T2ITC& itc)
    : mT2ITC(&itc)
    , mDir6(itc.mDir6)
  { }
#endif

#if 0  /*Ditching automation left and right*/
///// DEFINE AUTO-SUBCLASS REGISTRARS
#define XX(NM,CR,CW,CT,ST)                              \
  struct Trace##NM##Registrar {                         \
    Trace##NM##Registrar() {                            \
      TraceBase::registerReader(TT_##NM, new Trace##NM());      \
    }                                                   \
  } _auto_Trace##NM##_registrar;                        \

  ALL_TRACE_TYPES_MACRO()
#undef XX
    
///// DEFINE SUBCLASS METHODS

#define XX(NM,CR,CW,CT,ST)                              \
  const char * Trace##NM::getName() const {             \
    return #NM;                                         \
  }                                                     \
  
  ALL_TRACE_TYPES_MACRO()
#undef XX

////// DEFINE CUSTOM METHODS
  void TraceITCReset::writeBody(ByteSink& bs) const {
    FAIL(INCOMPLETE_CODE); //    bs.Printf("%c",mITCArg.mDir6); // Identify the itc
  }

  TraceBase * TraceITCReset::readBody(ByteSource& ){
    FAIL(INCOMPLETE_CODE); // 
  }

#endif

  bool TRACEPrintf(Logger::Level level, const char * format, ...) {
    T2Tile & tile = T2Tile::get();
    if (tile.isTracingActive()) {
      Trace evt(TTC_Log_LogTrace, level);
      va_list ap;
      va_start(ap, format);
      evt.payloadWrite().Vprintf(format, ap);
      va_end(ap);
      tile.tlog(evt);
      return true;
    }
    return false;
  }

  
}

/* -*- C++ -*- */
#ifndef TRACETYPES_H
#define TRACETYPES_H

#include "dirdatamacro.h"

#include "itype.h"
#include "T2Types.h"
#include "Trace.h"

namespace MFM {

#define ALL_TRACE_TYPES_MACRO()                 \
  XX(,ILL,Illegal)                              \
  XX(Tile,STR,Start)                            \
  XX(Tile,STP,Stop)                             \
  XX(ITC,PIN,PacketIn)                          \
  XX(ITC,POU,PacketOut)                         \

  enum TraceTypeCode {
#define XX(TYPE,BRIEF,NAME) TTC_##TYPE##_##NAME,
   ALL_TRACE_TYPES_MACRO()
#undef XX
   TTC_COUNT

  };
  const char * getTraceTypeType(TraceTypeCode ttc) ;
  const char * getTraceTypeBrief(TraceTypeCode ttc) ;
  const char * getTraceTypeName(TraceTypeCode ttc) ;

  struct T2Tile; // FORWARD
  struct T2ITC; // FORWARD
  
#if 0
  struct TraceTile : public Trace {
    TraceTile(u8 traceTypeCode) ;
    TraceTile(u8 traceTypeCode, UniqueTime when)
      : Trace(traceTypeCode, when)
    { }

    virtual void printTo(ByteSink & bs) { }
    virtual bool readFrom(ByteSource & bs) { return true; }
  };

#define MAKE_TRACE_SUBTYPE(TYPE,SUBNAME,DATAMEMBERS)                    \
  struct Trace##TYPE##SUBNAME : public Trace##TYPE {                    \
    Trace##TYPE##SUBNAME() : Trace##TYPE(TTC_##TYPE##_##SUBNAME) { }    \
    const char* getName() const { return "Trace" #TYPE #SUBNAME; }      \
    DATAMEMBERS                                                         \
  }                                                                     \

  MAKE_TRACE_SUBTYPE(Tile,Start,);
  MAKE_TRACE_SUBTYPE(Tile,Stop,);

  struct TraceITC : public Trace {
    TraceITC(const T2ITC& itc, u8 traceType) ;
    
    TraceITC(Dir6 dir6, u8 traceType, UniqueTime when)
      : Trace(traceType, when)
      , mDir6(dir6)
    { }
    Dir6 mDir6;

    virtual void printTo(ByteSink & bs) ;
    virtual bool readFrom(ByteSource & bs) ;
  };

  MAKE_TRACE_SUBTYPE(ITC,In,);
  MAKE_TRACE_SUBTYPE(ITC,Out,);

  struct TraceEW : public Trace {
    TraceEW(EWSlotNum ewsn, u8 traceType)
      : Trace(traceType)
      , mEWSlotNum(ewsn)
    { }
    TraceEW(EWSlotNum ewsn, u8 traceType, UniqueTime when)
      : Trace(traceType, when)
      , mEWSlotNum(ewsn)
    { }
    EWSlotNum mEWSlotNum;

    virtual void printTo(ByteSink & bs) ;
    virtual bool readFrom(ByteSource & bs) ;
  };
#endif

#if 0

///// DECLARE TYPES

  enum TraceTypes {
    TT_ILLEGAL_TYPE = 0,
#define XX(NM,CR,CW,CT,TP)                      \
    TT_##NM,                                    \
    
    ALL_TRACE_TYPES_MACRO()
#undef XX
    TTCOUNT
  };

///// DECLARE SUBCLASSES
#define XXWB0(NM)
#define XXWB1(NM) virtual void writeBody(ByteSink& ) const ; 
#define XXRB0(NM) virtual Trace* readBody(ByteSource& ) { return new Trace##NM(); }
#define XXRB1(NM) virtual Trace* readBody(ByteSource& ) ;
#define XXTG0(NM)
#define XXTG1(NM) virtual s32 tag() const ;                         
#define XX(NM,CR,CW,CT,TP)                              \
  struct Trace##NM : public Trace {                     \
    Trace##NM()                                         \
    : Trace(TT_##NM)                                    \
    { }                                                 \
    virtual const char * getName() const ;              \
    XXWB##CW(NM)                                        \
    XXRB##CR(NM)                                        \
    XXTG##CT(NM)                                        \
  };                                                    \
  
  ALL_TRACE_TYPES_MACRO()
#undef XXWB0
#undef XXWB1
#undef XXRB0
#undef XXRB1
#undef XXTG0
#undef XXTG1
#undef XX

#endif

}
#endif /* TRACETYPES_H */

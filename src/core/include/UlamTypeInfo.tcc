/* -*- C++ -*- */
#include "Fail.h"
//#include "Tile.h"
#include "Random.h"
//#include "EventWindow.h"
#include "Base.h"

namespace MFM {
  template <class EC>
  UlamTypeInfoModelParameterS32<EC>::UlamTypeInfoModelParameterS32(
                  UlamElement<EC> & theElement,
                  const char * mangledType,
                  const char * ulamName,
                  const char * briefDescription,
                  const char * details,
                  s32 * minOrNull,
                  s32 * defaultOrNull,
                  s32 * maxOrNull,
                  const char * units)
  : ElementParameterS32<EC>(&theElement, ulamName, briefDescription, details,
                            GetDefaulted(minOrNull, GetMinOfAs<s32>(mangledType)),
                            GetDefaulted(defaultOrNull, 0),
                            GetDefaulted(maxOrNull, GetMaxOfAs<s32>(mangledType)))
    , m_parameterUnits(units)
  { }

  template <class EC>
  UlamTypeInfoModelParameterU32<EC>::UlamTypeInfoModelParameterU32(
                  UlamElement<EC> & theElement,
                  const char * mangledType,
                  const char * ulamName,
                  const char * briefDescription,
                  const char * details,
                  u32 * minOrNull,
                  u32 * defaultOrNull,
                  u32 * maxOrNull,
                  const char * units)
  : ElementParameterU32<EC>(&theElement, ulamName, briefDescription, details,
                            GetDefaulted(minOrNull, GetMinOfAs<u32>(mangledType)),
                            GetDefaulted(defaultOrNull, 0u),
                            // Even unsigned model parameters are
                            // handled as signed internally, so limit
                            // Unsigned(32) max to Int(32) max.
                            (strcmp(mangledType,"Ut_102321u") 
                             ? GetDefaulted(maxOrNull, GetMaxOfAs<u32>(mangledType))
                             : GetDefaulted(maxOrNull, (u32) GetMaxOfAs<s32>("Ut_102321i"))))
    , m_parameterUnits(units)
  { }


  template <class EC>
  UlamTypeInfoModelParameterUnary<EC>::UlamTypeInfoModelParameterUnary(
                  UlamElement<EC> & theElement,
                  const char * mangledType,
                  const char * ulamName,
                  const char * briefDescription,
                  const char * details,
                  u32 * minOrNull,
                  u32 * defaultOrNull,
                  u32 * maxOrNull,
                  const char * units)
  : ElementParameterUnary<EC>(&theElement, ulamName, briefDescription, details,
                              GetDefaulted(minOrNull, GetMinOfAs<u32>(mangledType)),
                              GetDefaulted(defaultOrNull, 0u),
                              GetDefaulted(maxOrNull, GetMaxOfAs<u32>(mangledType)))
    , m_parameterUnits(units)
  { }

  template <class EC>
  UlamTypeInfoModelParameterBool<EC>::UlamTypeInfoModelParameterBool(
                  UlamElement<EC> & theElement,
                  const char * mangledType,
                  const char * ulamName,
                  const char * briefDescription,
                  const char * details,
                  bool defvalue)
    : ElementParameterBool<EC>(&theElement, ulamName, briefDescription, details, defvalue)
  { }

} //MFM

#include "Version.h"

#define xstr(s) str(s)
#define str(s) #s

namespace MFM {
  const char * MFM_VERSION_STRING_SHORT =
    "MFM-"
    xstr(MFM_VERSION_MAJOR) "."
    xstr(MFM_VERSION_MINOR) "."
    xstr(MFM_VERSION_REV);
  const char * MFM_VERSION_STRING_LONG =
    "MFM-"
    xstr(MFM_VERSION_MAJOR) "."
    xstr(MFM_VERSION_MINOR) "."
    xstr(MFM_VERSION_REV) "-"
    xstr(MFM_BUILD_DATE) "-"
    xstr(MFM_BUILD_TIME) "-"
    xstr(MFM_BUILT_BY) "@"
    xstr(MFM_BUILT_ON);
}

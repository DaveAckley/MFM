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
    "MFM-" xstr(MFM_VERSION_MAJOR) "." xstr(MFM_VERSION_MINOR) "." xstr(MFM_VERSION_REV)
    " on " xstr(MFM_BUILD_DATE) "+" xstr(MFM_BUILD_TIME)
    " by " xstr(MFM_BUILT_BY) "@" xstr(MFM_BUILT_ON)
    " of " xstr(MFM_TREE_VERSION)
    ;
  const char * MFM_TREE_VERSION_STRING =
    "MFM-"
    xstr(MFM_TREE_VERSION);
}

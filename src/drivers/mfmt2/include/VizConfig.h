#ifndef VIZCONFIG_H
#define VIZCONFIG_H

#include "Panel.h"
#include "ByteSource.h"

namespace MFM {

  struct VizConfig {
    static Panel * parseAndBuildFromPath(const char * path) ;
    static Panel * parseAndBuild(ByteSource & bs) ;
  };

}

#endif /*VIZCONFIG_H*/

#include "VizConfig.h"
#include "T2Constants.h"

#include "FileByteSource.h"

namespace MFM {
  Panel * VizConfig::parseAndBuildFromPath(const char * path) {
    FileByteSource fbs(path);
    return parseAndBuild(fbs);
  }

  Panel * VizConfig::parseAndBuild(ByteSource & bs) {
    DIE_UNIMPLEMENTED();
  }
}

#ifndef DEFAULTCONFIG_H
#define DEFAULTCONFIG_H

#include "CoreConfig.h"
#include "ParamConfig.h"
#include "P3Atom.h"

/**
  Typedefs for use by element libraries.  These definitions must track
  the desired values used by UUID::ComputeConfigurationCode --
  specifically, the ATOM_CATEGORY (via P3Atom), the
  EVENT_WINDOW_RADIUS (defaulted here to 4), and the BITS_PER_ATOM
  (defaulted here to 96).
 */
namespace MFM {
  typedef ParamConfig<> DefaultParamConfig;
  typedef P3Atom<DefaultParamConfig> DefaultAtom;
  typedef CoreConfig<DefaultAtom,DefaultParamConfig> DefaultCoreConfig;
}

#endif /* DEFAULTCONFIG_H */

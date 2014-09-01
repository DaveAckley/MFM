#ifndef LOCALCONFIG_H
#define LOCALCONFIG_H

#include "CoreConfig.h"
#include "ParamConfig.h"
#include "main.h"
#include "P3Atom.h"

namespace MFM {
  //  typedef ParamConfig<64,4,8,40> OurParamConfig;
  typedef ParamConfig<96,4,8,50> OurParamConfig;
  typedef P3Atom<OurParamConfig> OurAtom;
  typedef CoreConfig<OurAtom,OurParamConfig> OurCoreConfig;
}

#endif /* LOCALCONFIG_H */

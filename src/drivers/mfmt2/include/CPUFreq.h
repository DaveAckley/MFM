/* -*- C++ -*- */
#ifndef CPUFREQ_H
#define CPUFREQ_H

#include "T2Constants.h"

namespace MFM {

  enum CPUSpeed {
    CPUSpeed_300MHz = 300,
    CPUSpeed_600MHz = 600,
    CPUSpeed_720MHz = 720,
    CPUSpeed_800MHz = 800,
    CPUSpeed_1000MHz = 1000,

    CPUSpeed_Slowest = CPUSpeed_300MHz,
    CPUSpeed_Slower = CPUSpeed_300MHz,
    CPUSpeed_Slow = CPUSpeed_600MHz,
    CPUSpeed_Fast = CPUSpeed_720MHz,
    CPUSpeed_Faster = CPUSpeed_800MHz,
    CPUSpeed_Fastest = CPUSpeed_1000MHz,

    CPUSpeed_UNKNOWN=0
  };

  struct CPUFreq {
    CPUFreq()
      : mLastSet(CPUSpeed_UNKNOWN)
    { }

    CPUFreq(CPUSpeed speed) : CPUFreq()
    {
      setSpeed(speed);
    }

    CPUSpeed getLastSetSpeed() const { return mLastSet; }
    CPUSpeed getFaster(CPUSpeed than) const ;
    CPUSpeed getSlower(CPUSpeed than) const ;
    void setSpeed(CPUSpeed to) ;

    CPUSpeed mLastSet;
  };
}

#endif /* CPUFREQ_H */

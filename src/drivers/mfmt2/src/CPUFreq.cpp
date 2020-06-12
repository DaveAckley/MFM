#include <stdio.h>  /* For snprintf */
#include <stdlib.h> /* For system */

#include "CPUFreq.h"

namespace MFM {
  CPUSpeed CPUFreq::getFaster(CPUSpeed than) const {
    if (than < CPUSpeed_300MHz) return CPUSpeed_300MHz;
    if (than < CPUSpeed_600MHz) return CPUSpeed_600MHz;
    if (than < CPUSpeed_720MHz) return CPUSpeed_720MHz;
    if (than < CPUSpeed_800MHz) return CPUSpeed_800MHz;
    if (than < CPUSpeed_1000MHz) return CPUSpeed_1000MHz;
    return CPUSpeed_UNKNOWN;
  }

  CPUSpeed CPUFreq::getSlower(CPUSpeed than) const {
    if (than > CPUSpeed_1000MHz) return CPUSpeed_1000MHz;
    if (than > CPUSpeed_800MHz) return CPUSpeed_800MHz;
    if (than > CPUSpeed_720MHz) return CPUSpeed_720MHz;
    if (than > CPUSpeed_600MHz) return CPUSpeed_600MHz;
    if (than > CPUSpeed_300MHz) return CPUSpeed_300MHz;
    return CPUSpeed_UNKNOWN;
  }

  void CPUFreq::setSpeed(CPUSpeed newspeed) {
    if (newspeed == mLastSet) return;
    switch (newspeed) {
    case CPUSpeed_300MHz: 
    case CPUSpeed_600MHz: 
    case CPUSpeed_720MHz: 
    case CPUSpeed_800MHz: 
    case CPUSpeed_1000MHz:
      break;
    default:
      LOG.Error("Illegal CPUSpeed value %d ignored", newspeed);
      return;
    }
    char buff[100];
    snprintf(buff,100,"cpufreq-set -f %dMHz",(u32) newspeed);
    system(buff);

    LOG.Message("CPU Frequency -> %dMHz", newspeed);
    mLastSet = newspeed;

  }
}

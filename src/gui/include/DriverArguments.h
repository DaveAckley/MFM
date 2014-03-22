#ifndef DRIVERARGUMENTS_H   /* -*- C++ -*- */
#define DRIVERARGUMENTS_H

#include "itype.h"

namespace MFM { 

  class DriverArguments
  {
  private:
    const char * m_programName;
    const char * m_dataDirPath;
    u32 m_seed;
    s32 m_recordEventCountsPerAEPS;
    s32 m_recordScreenshotPerAEPS;

  public:

    DriverArguments(int argc, char **argv) :
      m_programName(0),
      m_dataDirPath(0),
      m_seed(0),
      m_recordEventCountsPerAEPS(-1),
      m_recordScreenshotPerAEPS(-1)
    {
      ProcessArguments(argc,argv);
    }

    void Die(const char * format, ...) __attribute__ ((noreturn));

    int ProcessArguments(int argc, char **argv) ;

    const char * GetProgramName() const { return m_programName; }
    const char * GetDataDirPath() const { return m_dataDirPath; }
    u32 GetSeed() const { return m_seed; }
    s32 GetRecordEventCountsPerAEPS() const { return m_recordEventCountsPerAEPS; }
    s32 GetRecordScreenshotPerAEPS() const { return m_recordScreenshotPerAEPS; }

  };
} /* namespace MFM */

#endif /* DRIVERARGUMENTS_H */

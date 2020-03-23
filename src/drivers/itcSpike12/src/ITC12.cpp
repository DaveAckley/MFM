#include "ITC12.h"

#include <sys/types.h> 
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>    // For close
#include <stdio.h>     // For snprintf
#include <errno.h>     // For errno

namespace MFM {
  
  const char * ITC::path() const {
    static char buf[100];
    snprintf(buf,100,"/dev/itc/mfm/%s",mName);
    return buf;
  }

  int ITC::open() {
    int ret = ::open(path(),O_RDWR|O_NONBLOCK);
    if (ret < 0) return -errno;
    mFD = ret;
    return ret;
  }

  int ITC::close() {
    int ret = ::close(mFD);
    mFD = -1;
    if (ret < 0) return -errno;
    return ret;
  }
}

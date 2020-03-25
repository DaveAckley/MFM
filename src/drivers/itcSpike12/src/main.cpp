#include "main.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#define MFZID_DEV "/sys/class/itc_pkt/mfzid"
namespace MFM {

  void setMFZId(const char * mfzid) {
    int fd = ::open(MFZID_DEV,O_WRONLY);
    if (fd < 0) {
      perror("open " MFZID_DEV);
      exit(3);
    }
    if (write(fd,mfzid,strlen(mfzid)) < 0) {
      perror("write " MFZID_DEV);
      exit(4);
    }
    ::close(fd);
  }

  int MainDispatch(int argc, const char** argv)
  {
    T2Tile tile(60,40);
    if (argc != 2) {
      tile.error("Usage:\n%s MFZID\n", argv[0]);
      exit(2);
    } else {
      setMFZId(argv[1]);
    }
    if (!tile.openITCs()) {
      tile.error("Open failed");
      exit(4);
    }
    T2EventWindow * ew = tile.allocEW();
    assert(ew!=0);
    printf("hewo12 ew#%d\n", ew->slotNum());
    tile.setPassive(ew);
    ew->unlink();       // Free myself
    tile.setActive(ew); // Should succeed
    return 0;
  }
}


int main(int argc, const char** argv)
{
  return MFM::MainDispatch(argc,argv);
}

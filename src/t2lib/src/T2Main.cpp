#include <getopt.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#include "T2Main.h"

namespace MFM {
  T2Main* T2Main::mTheInstance = 0;
  T2Main::~T2Main() { }
}

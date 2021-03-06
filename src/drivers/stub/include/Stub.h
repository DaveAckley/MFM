/* -*- C++ -*- */
#ifndef STUB_H
#define STUB_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include <vector>

#include "itype.h"
#include "Dirs.h"
#include "DateTimeStamp.h"
#include "Random.h"

#include "T2Constants.h"
#include "T2Main.h"

namespace MFM {

  struct Stub : public T2Main {

    // Get the T2Main singleton set up;
    static void initInstance() { new Stub(); }

    static Stub & get() {
      T2Main * o = &T2Main::get();
      Stub * c = dynamic_cast<Stub*>(o);
      MFM_API_ASSERT_NONNULL(c);
      return (Stub&) *c;
    }

    void initEverything(int argc, char ** argv) ;
    void main() ;

  protected:
    Stub() ;
    virtual ~Stub() ;

  };
}
#endif /* STUB_H */

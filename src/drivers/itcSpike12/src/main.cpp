#include "main.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "TimeQueue.h"
#include "SDLI.h"
#include "ADCCtl.h"

namespace MFM {

  struct TOT : public TimeoutAble {
    virtual void onTimeout(TimeQueue& src) { }
    virtual const char * getName() const { return "TOT"; }
  };
  void tqtest(Random & random) {
    TimeQueue tq(random);
    TOT t1,t2,t3;
    t1.insert(tq,100);
    t2.insert(tq,10);
    t3.insert(tq,50);
    TimeoutAble * ptr;
    while (!tq.isEmpty()) {
      u32 now = tq.now();
      printf("Scan at %d\n",now);
      while ((ptr = tq.getEarliestExpired()) != 0) {
        printf("to=%d, %p \n", ptr->getTimeout(), ptr);
      }
      for (u32 i = 0; i < 100000; ++i) random.Create(); // wastongo
    }
    
  }

  struct SDLDisplayer : public TimeoutAble {
    SDLDisplayer(T2Tile & tile, SDLI & sdli, const char * name)
      : mTile(tile)
      , mSDLI(sdli)
      , mName(name)
    { }
    T2Tile & mTile;
    SDLI & mSDLI;
    const char * mName;
    
    virtual void onTimeout(TimeQueue& src) {
      LOG.Message("Redisplongo\n");
      mSDLI.redisplay();
      insert(src,200,6);
    }
    virtual const char * getName() const { return mName; }
  };

  void demo(T2Tile & tile, SDLI & display) {
    TimeQueue tq(tile.getRandom());
    SDLDisplayer sdld(tile, display, "demoDisplayer");
    sdld.insert(tq,200,6);
    while (!tq.isEmpty()) {
      u32 now = tq.now();
      printf("Scan at %d\n",now);
      TimeoutAble * ptr;
      while ((ptr = tq.getEarliestExpired()) != 0) {
        printf("to=%d, %p \n", ptr->getTimeout(), ptr);
        ptr->onTimeout(tq);
      }
    }
  }

  int MainDispatch(int argc, char** argv)
  {
    // Early early logging
    LOG.SetByteSink(STDERR);
    LOG.SetLevel(LOG.MESSAGE);

    T2Tile & tile = T2Tile::get();
    tile.initEverything(argc,argv);

    /*
    SDLI sdli(tile,"SDLI"); // SDL1.2 initializes here
    ADCCtl adcs(tile);
    */

    // tqtest(tile.getRandom());

    //demo(tile, sdli);
    tile.main();
    /*
    if (!tile.openITCs()) {
      tile.fatal("Open failed");
    }
    T2EventWindow * ew = tile.tryAcquireEW(UPoint(20,10),4);
    assert(ew!=0);
    printf("hewo12 ew#%d\n", ew->slotNum());
    tile.setPassive(ew);
    ew->unlink();       // Free myself
    tile.setActive(ew); // Should succeed
    tile.releaseEW(ew);
    */
    return 0;
  }
}


int main(int argc, char** argv)
{
  unwind_protect({
      MFMPrintErrorEnvironment(stderr, &unwindProtect_errorEnvironment);
      fprintf(stderr,"Failed out of top level\n");
      exit(99);
  },{
    return MFM::MainDispatch(argc,argv);
  });
}
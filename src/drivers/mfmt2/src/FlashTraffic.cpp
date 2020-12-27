#include "FlashTraffic.h"
#include "T2FlashTrafficManager.h"
#include "T2Tile.h"

namespace MFM
{
  const BPoint FlashTraffic::dir8Offsets[Dirs::DIR_COUNT] =
    {
     BPoint( 0,+1), // (NT)
     BPoint(+1,+1), // NE
     BPoint(+2, 0), // ET
     BPoint(+1,-1), // SE
     BPoint( 0,-1), // (ST)
     BPoint(-1,-1), // SW
     BPoint(-2, 0), // WT
     BPoint(-1,+1)  // NW
    };

  FlashTraffic FlashTraffic::make(T2FlashCmd cmd) {
    return make(cmd, 0, 0, T2Tile::makeTag());
  }

  bool FlashTraffic::execute(const FlashTraffic & ft) {
    T2Tile & tile = T2Tile::get();
    u32 seedtype;
    switch (ft.mCommand) {
    case T2FLASH_CMD(dsp,sites):
    case T2FLASH_CMD(dsp,tile):
    case T2FLASH_CMD(dsp,cdm):
    case T2FLASH_CMD(dsp,tq):
    case T2FLASH_CMD(dsp,log): 
    case T2FLASH_CMD(dsp,trace): {
      T2FlashTrafficManager & ftm = tile.getFlashTrafficManager();
      ftm.executeFlashTrafficCommand((T2FlashCmd) ft.mCommand);
      return true;
    }

    case T2FLASH_CMD(mfm,dump): {
      tile.dumpTrace(ft.mArg.get(), ft.mRange, ft.mOrigin);
      return true;
    }

    case T2FLASH_CMD(mfm,getlog): {
      TraceLogDirManager & dm = tile.getTraceLogDirManager();
      dm.tryMoveLog(ft.mArg.get(), ft.mRange, ft.mOrigin);
      return true;
    }

    case T2FLASH_CMD(mfm,crash): {
      FAIL(UNSPECIFIED_EXPLICIT_FAIL);
#if 0
      throw std::exception(); // Throw something that isn't a FailException
      exit(1); /* 'crash' */
#endif
    }
    case T2FLASH_CMD(mfm,quit): {
      T2Tile::get().getSDLI().stop();
      return true;
    }
    case T2FLASH_CMD(t2t,off): {
      T2Tile::get().stopTracing();
      system("poweroff");
      return true; // Laugh while you can monkeyboy
    }
    case T2FLASH_CMD(t2t,boot): {
      T2Tile::get().stopTracing();
      system("reboot");
      return true; // Laugh while you can monkeyboy
    }
    case T2FLASH_CMD(t2t,xcdm): {
      system("pkill cdm.pl");
      return true;
    }

    case T2FLASH_CMD(phy,seed1):
      seedtype = 1;
      goto doseed;

    case T2FLASH_CMD(phy,seed2):
      seedtype = 2;
      goto doseed;

    case T2FLASH_CMD(phy,debugsetup):
      T2Tile::get().debugSetup();
      goto showsites;

    case T2FLASH_CMD(phy,clear):
      T2Tile::get().clearPrivateSites();
      goto showsites;

    doseed: 
      T2Tile::get().seedPhysics(seedtype);
      goto showsites;

    showsites: {
        SDLI & sdli = tile.getSDLI();
        const char * panelName = "GlobalMenu_Button_Sites";
        AbstractButton * sites = dynamic_cast<AbstractButton*>(sdli.lookForPanel(panelName));
        if (!sites) LOG.Error("Couldn't find '%s'",panelName);
        else sites->OnClick(1);
        return true;
      }

    default:
      FAIL(INCOMPLETE_CODE);
    }
    return false;
  }
}


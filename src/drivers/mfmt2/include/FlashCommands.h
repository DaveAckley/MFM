/* -*- C++ -*- */
#ifndef FLASHCOMMANDS_H
#define FLASHCOMMANDS_H

namespace MFM {

#define T2FLASH_CATEG(nam) T2FLASH_CATEG_##nam
#define T2FLASH_CMD(categ,nam) T2FLASH_CMD_##categ##_##nam
#define T2FLASH_CMD_VALUE(categ,num) ((T2FLASH_CATEG(categ)<<4)|num)
#define T2FLASH_CMD_STRING(categ,nam) ("_" #categ "_" #nam)

#define T2FLASH_CMD_NAME(categ,nam) (#categ ":" #nam)

#define T2FLASH_CONTROL_CMD(nam) T2FLASH_CONTROL_CMD_##nam
#define T2FLASH_CONTROL_CMD_STRING(nam) ("_fcdist_" #nam)
  

#define ALL_FLASH_CMD_CATEGORIES()     \
  XX(t2t,0)                            \
  XX(mfm,1)                            \
  XX(phy,2)                            \
  XX(dsp,3)                            \
  /*CATEGORIES 4..7 RESERVED*/         \
  /* NOTE VALUES 8..15 ILLEGAL! */     \
  

#define ALL_FLASH_CMDS()               \
  XX(t2t,0,boot)                       \
  XX(t2t,1,off)                        \
  XX(t2t,2,xcdm)                       \
  /* t2t,3..15 reserved*/              \
  XX(mfm,0,run)                        \
  XX(mfm,1,pause)                      \
  XX(mfm,2,crash)                      \
  XX(mfm,3,quit)                       \
  XX(mfm,4,dump)                       \
  /* mfm,5..15 reserved*/              \
  XX(phy,0,clear)                      \
  XX(phy,1,seed1)                      \
  XX(phy,2,seed2)                      \
  XX(phy,3,debugsetup)                 \
  /* phy,4..15 reserved*/              \
  XX(dsp,0,sites)                      \
  XX(dsp,1,tile)                       \
  XX(dsp,2,cdm)                        \
  XX(dsp,3,tq)                         \
  XX(dsp,4,log)                        \
  /* dsp,5..15 reserved*/              \

#define ALL_FLASH_COMMANDABLE_BUTTON_PANELS() \
  XX(t2t,boot,T2Info_Boot_Button)             \
  XX(t2t,off,T2Info_Off_Button)               \
  XX(t2t,xcdm,T2Info_KillCDM_Button)          \
  XX(mfm,run,PhysicsCtl_MFMRun)               \
  XX(mfm,pause,PhysicsCtl_MFMPause)           \
  XX(mfm,crash,T2Info_Crash_Button)           \
  XX(mfm,quit,T2Info_Quit_Button)             \
  XX(mfm,dump,T2Info_Dump_Button)             \
  XX(phy,clear,PhysicsCtl_Button_Clear)       \
  XX(phy,seed1,PhysicsCtl_Button_Seed1)       \
  XX(phy,seed2,PhysicsCtl_Button_Seed2)       \
  XX(dsp,sites,GlobalMenu_Button_Sites)       \
  XX(dsp,tile,GlobalMenu_Button_T2Viz)        \
  XX(dsp,cdm,GlobalMenu_Button_CDM)           \
  XX(dsp,tq,GlobalMenu_Button_TQ)             \
  XX(dsp,log,GlobalMenu_Button_Log)           \

#define T2FLASH_DIST_PANEL(nam) ("Grid_k_fcdist_" #nam)

  enum T2FlashCategory {
#define XX(nam,num) T2FLASH_CATEG(nam) = num,
     ALL_FLASH_CMD_CATEGORIES()
#undef XX
     T2FLASH_CATEG__COUNT
  };

  enum T2FlashCmd {
#define XX(categ,num,nam) T2FLASH_CMD(categ,nam) = T2FLASH_CMD_VALUE(categ,num),
     ALL_FLASH_CMDS()
#undef XX
     T2FLASH_CMD__COUNT
  };
#undef XX      

#define ALL_FLASH_TRAFFIC_CONTROL_PANELS() \
  XX(less)                                 \
  XX(down)                                 \
  XX(dist)                                 \
  XX(up)                                   \
  XX(more)                                 \
  XX(prepared)                             \
  XX(gogogo)                               \


  enum T2FlashControlCmd {
#define XX(nam) T2FLASH_CONTROL_CMD(nam),
    ALL_FLASH_TRAFFIC_CONTROL_PANELS()
#undef XX
    T2FLASH_CONTROL_CMD__COUNT
  };

}

#endif /* FLASHCOMMANDS_H */

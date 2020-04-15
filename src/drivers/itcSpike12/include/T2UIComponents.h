/* -*- C++ -*- */
#ifndef T2UICOMPONENTS_H
#define T2UICOMPONENTS_H

#include "AbstractCheckbox.h"

namespace MFM {

  struct T2TileLiveCheckbox : public AbstractCheckbox {
    virtual void OnCheck(bool value) ;
    virtual bool IsChecked() const ;
    virtual void SetChecked(bool checked) ;
  };

}


#endif /* T2UICOMPONENTS_H */

/* -*- C++ -*- */
#ifndef SIMLOGPANEL_H
#define SIMLOGPANEL_H

#include "TextPanel.h"
#include "TeeByteSink.h"
#include "Logger.h"

namespace MFM {

  typedef TextPanel<80,40> SimLogTextPanel;
  struct SimLogPanel : public SimLogTextPanel {

    SimLogPanel() {
      ByteSink * old = LOG.SetByteSink(m_logSplitter);
      m_logSplitter.SetSink1(old);
      m_logSplitter.SetSink2(&GetByteSink());
    }

    virtual ~SimLogPanel() { }

    TeeByteSink m_logSplitter;
  };
}

#endif /* SIMLOGPANEL_H */

/*                                              -*- mode:C++ -*-
  ThreadPauser.h Structure allowing a thread to pause another
  Copyright (C) 2014 The Regents of the University of New Mexico.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
  USA
*/

/**
  \file ThreadPauser.h Structure allowing a thread to pause another
  \author Trent R. Small.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef THREAD_PAUSER_H
#define THREAD_PAUSER_H

#include <pthread.h>

namespace MFM
{
  /**
   * A threading construct which pauses a looping thread by blocking it.
   */
  class ThreadPauser
  {
  private:

    /**
     * The mutex required to lock the looping thread.
     */
    pthread_mutex_t m_lock;

    /**
     * The thread condition required to lock the looping thread.
     */
    pthread_cond_t m_pauseCond;

    /**
     * The thread condition used to announce to the controlling thread
     * that the looping thread is paused.
     */
    pthread_cond_t m_joinCond;

    /**
     * A boolean, describing whether or not the looping thread is
     * paused.
     */
    bool m_paused;

  public:

    /**
     * Constructs a new ThreadPauser.
     */
    ThreadPauser();

    /**
     * Destroys this ThreadPauser.
     */
    ~ThreadPauser();

    /**
     * Finds out if this ThreadPauser is actively trying to pause the
     * looping thread.
     */
    bool IsPaused();

    /**
     * To be called by the looping thread. This pauses the thread if it
     * is supposed to be paused.
     */
    void WaitIfPaused();

    /**
     * To be called by the controlling thread. This returns as soon as
     * the looping thread is waiting.
     */
    void PauseBlocking();

    /**
     * To be called by the controlling thread. This returns immediately
     * and pauses the looping thread.
     */
    void Pause();

    /**
     * To be called by the controlling thread. This wakes up the looping
     * thread if it is paused.
     */
    void Unpause();
  };
}

#endif /* THREAD_PAUSER_H */

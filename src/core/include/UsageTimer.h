/*                                              -*- mode:C++ -*-
  UsageTimer.h Structure for system timing
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
  \file UsageTimer.h Structure for system timing
  \author Trent R. Small.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */

#include <sys/time.h>
#include <sys/resource.h>

namespace MFM
{
  class UsageTimer
  {
   private:
    typedef struct rusage RUsage;

    const RUsage m_rusage;

    UsageTimer(const RUsage r) : m_rusage(r)
    { }

   public:
    static UsageTimer Now()
    {
      return Now(RUSAGE_SELF);
    }

    static UsageTimer NowThread()
    {
      return Now(RUSAGE_THREAD);
    }

    static UsageTimer Now(int who)
    {
      RUsage r;
      getrusage(who, &r);
      return UsageTimer(r);
    }

    long UserMilliseconds()
    {
      return m_rusage.ru_utime.tv_sec * 1000 + m_rusage.ru_utime.tv_usec / 1000;
    }

    long SystemMilliseconds()
    {
      return m_rusage.ru_stime.tv_sec * 1000 + m_rusage.ru_stime.tv_usec / 1000;
    }

    long TotalMilliseconds()
    {
      return UserMilliseconds() + SystemMilliseconds();
    }


    long UserMicroseconds()
    {
      return m_rusage.ru_utime.tv_sec * 1000000 + m_rusage.ru_utime.tv_usec;
    }

    long SystemMicroseconds()
    {
      return m_rusage.ru_stime.tv_sec * 1000000 + m_rusage.ru_stime.tv_usec;
    }

    long TotalMicroseconds()
    {
      return UserMicroseconds() + SystemMicroseconds();
    }

    friend UsageTimer operator-(const UsageTimer& lhs, const UsageTimer& rhs)
    {
      RUsage t;
      long uus = (lhs.m_rusage.ru_utime.tv_sec * 1000000 + lhs.m_rusage.ru_utime.tv_usec) -
                 (rhs.m_rusage.ru_utime.tv_sec * 1000000 + rhs.m_rusage.ru_utime.tv_usec);

      long sus = (lhs.m_rusage.ru_stime.tv_sec * 1000000 + lhs.m_rusage.ru_stime.tv_usec) -
                 (rhs.m_rusage.ru_stime.tv_sec * 1000000 + rhs.m_rusage.ru_stime.tv_usec);

      t.ru_utime.tv_sec  = uus / 1000000;
      t.ru_utime.tv_usec = uus % 1000000;

      t.ru_stime.tv_sec  = sus / 1000000;
      t.ru_stime.tv_usec = sus % 1000000;

      t.ru_nvcsw    = lhs.m_rusage.ru_nvcsw    - rhs.m_rusage.ru_nvcsw;
      t.ru_nivcsw   = lhs.m_rusage.ru_nivcsw   - rhs.m_rusage.ru_nivcsw;
      t.ru_nsignals = lhs.m_rusage.ru_nsignals - rhs.m_rusage.ru_nsignals;
      t.ru_msgrcv   = lhs.m_rusage.ru_msgrcv   - rhs.m_rusage.ru_msgrcv;
      t.ru_msgsnd   = lhs.m_rusage.ru_msgsnd   - rhs.m_rusage.ru_msgsnd;
      t.ru_oublock  = lhs.m_rusage.ru_oublock  - rhs.m_rusage.ru_oublock;
      t.ru_inblock  = lhs.m_rusage.ru_inblock  - rhs.m_rusage.ru_inblock;
      t.ru_nswap    = lhs.m_rusage.ru_nswap    - rhs.m_rusage.ru_nswap;
      t.ru_minflt   = lhs.m_rusage.ru_minflt   - rhs.m_rusage.ru_minflt;
      t.ru_majflt   = lhs.m_rusage.ru_majflt   - rhs.m_rusage.ru_majflt;

      return UsageTimer(t);
    }
  };
}

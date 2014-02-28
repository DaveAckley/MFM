#include "ThreadPauser.h"

ThreadPauser::ThreadPauser()
{
  pthread_mutex_init(&m_lock, NULL);
  pthread_cond_init(&m_pauseCond, NULL);
  pthread_cond_init(&m_joinCond, NULL);

  m_paused = false;
}

ThreadPauser::~ThreadPauser()
{
  pthread_mutex_destroy(&m_lock);
  pthread_cond_destroy(&m_pauseCond);
  pthread_cond_destroy(&m_joinCond);
}

bool ThreadPauser::IsPaused()
{
  return m_paused;
}

void ThreadPauser::WaitIfPaused()
{
  if(m_paused)
  { /* Don't bother grabbing the lock unless we're sure about being paused. */
    pthread_mutex_lock(&m_lock);
    {
      while(m_paused)
      {
	pthread_cond_signal(&m_joinCond);
	pthread_cond_wait(&m_pauseCond, &m_lock);
      }
    }
    pthread_mutex_unlock(&m_lock);
  }
}

void ThreadPauser::PauseBlocking()
{
  pthread_mutex_lock(&m_lock);
  {
    m_paused = true;
    pthread_cond_wait(&m_joinCond, &m_lock);
  }
  pthread_mutex_unlock(&m_lock);
}

void ThreadPauser::Pause()
{
  pthread_mutex_lock(&m_lock);
  {
    m_paused = true;
  }
  pthread_mutex_unlock(&m_lock);
}

void ThreadPauser::Unpause()
{
  pthread_mutex_lock(&m_lock);
  {
    m_paused = false;
    pthread_cond_signal(&m_pauseCond);
  }
  pthread_mutex_unlock(&m_lock);
}

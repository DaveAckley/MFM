#include "Connection.h"
#include <stdlib.h>
#include <time.h>

#define PACKETSIZE 64

class ConnectionThread
{
private:

  Connection* m_connection;

  bool m_child;

  pthread_t m_pthread;

  u8 readBuffer[4096];

public:

  ConnectionThread(Connection* con, bool child)
  {
    m_connection = con;
    m_child = child;
  }

  bool IsChild()
  {
    return m_child;
  }

  bool Lock()
  {
    return m_connection->Lock();
  }

  void Unlock()
  {
    m_connection->Unlock();
  }

  u32 Read()
  {
    return m_connection->Read(m_child, readBuffer, PACKETSIZE);
  }

  void Write(u8* str)
  {
    m_connection->Write(m_child, str, PACKETSIZE);
  }

  u8* GetBuffer()
  {
    return readBuffer;
  }

  static void NSleep(u32 secs, u32 nanos)
  {
    struct timespec tspec;
    tspec.tv_sec = secs;
    tspec.tv_nsec = nanos;

    nanosleep(&tspec, NULL);
  }

  static const char* IDStr(bool child)
  {
    return child ? "CHILD" : "PARENT";
  }

  static void SmallProcedure(ConnectionThread* connection)
  {
    printf("[%s]: Begin Small Procedure...\n", IDStr(connection->IsChild()));
    NSleep(1, 0);
    printf("[%s]: End Small Procedure.\n", IDStr(connection->IsChild()));
  }

  static void LargeProcedure(ConnectionThread* connection)
  {    
    if(connection->Lock())
    {
      printf("[%s]: Begin Large Procedure...\n", IDStr(connection->IsChild()));
      NSleep(9, 0);
      connection->Write((u8*)"Here is a lovely 64 character packet to digest. Need more char?");
      printf("[%s]: End Large Procedure.\n", IDStr(connection->IsChild()));
      connection->Unlock();
    }
  }

  static void FlushOutput(ConnectionThread* connection)
  {
    if(connection->Read())
    {
      printf("[%s:READ]%s", IDStr(connection->IsChild()), connection->GetBuffer());
    }
  }

  static void* InternalThread(void* connection)
  {
    ConnectionThread* self = (ConnectionThread*)connection;

    for(u32 i = 0; i < 1000; i++)
    {
      if(rand() & 1)
      {
	SmallProcedure(self);
      }
      else
      {
	LargeProcedure(self);
      }

      FlushOutput(self);
    }
    
    pthread_exit(NULL);
  }

  void Run()
  {
    pthread_create(&m_pthread, NULL, InternalThread, (void*)this);
  }

  void Join()
  {
    pthread_join(m_pthread, NULL);
  }
};

int main(int argc, char** argv)
{
  Connection mainConnection;

  ConnectionThread 
    owner(&mainConnection, false), 
    child(&mainConnection, true);

  owner.Run();
  child.Run();

  owner.Join();
  child.Join();
  
  return 0;
}

#include "SharedMemory.hpp"
#include <sys/mman.h>
#include <errno.h>
#include <cstdio>
#include <cstdlib>
 
CSharedMemory::CSharedMemory( const string& sName ):m_sName(sName),m_Ptr(NULL),m_iD(-1),
   m_SemID(NULL), m_nSize(0)
{
   /**
   * Semaphore open
   */
   m_SemID = sem_open(sName.c_str(), O_CREAT, S_IRUSR | S_IWUSR, 1);
}
 
bool CSharedMemory::Create( size_t nSize, int mode /*= READ_WRITE*/ )
{
   m_nSize = nSize;
   m_iD = shm_open(m_sName.c_str(), O_CREAT | mode, S_IRWXU | S_IRWXG);
   if(m_iD < 0)
   {
      switch(errno)
      {
      case EACCES:
         throw CSharedMemoryException("Permission Exception ");
         break;
      case EEXIST:
         throw CSharedMemoryException("Shared memory object specified by name already exists.");
         break;
      case EINVAL:
         throw CSharedMemoryException("Invalid shared memory name passed.");
         break;
      case EMFILE:
         throw CSharedMemoryException("The process already has the maximum number of files open.");
         break;
      case ENAMETOOLONG:
         throw CSharedMemoryException("The length of name exceeds PATH_MAX.");
         break;
      case ENFILE:
         throw CSharedMemoryException("The limit on the total number of files open on the system has been reached");
         break;
      default:
         throw CSharedMemoryException("Invalid exception occurred in shared memory creation");
         break;
      }
   }
   /* adjusting mapped file size (make room for the whole segment to map)      --  ftruncate() */
   ftruncate(m_iD, m_nSize);
 
   return true;
}
 
bool CSharedMemory::Attach( int mode /*= A_READ | A_WRITE*/ )
{
   /* requesting the shared segment    --  mmap() */
   m_Ptr = mmap(NULL, m_nSize, mode, MAP_SHARED, m_iD, 0);
   if (m_Ptr == NULL)
   {
      throw CSharedMemoryException("Exception in attaching the shared memory region");
   }
   return true;
}
 
bool CSharedMemory::Detach()
{
   munmap(m_Ptr, m_nSize);
   return true;
}
 
bool CSharedMemory::Lock()
{
   int sval;
   sem_getvalue(m_SemID, &sval);
   printf("sem val: %d\n", sval);
   sem_wait(m_SemID);
   return true;
}
 
bool CSharedMemory::UnLock()
{
   int sval;
   sem_getvalue(m_SemID, &sval);
   if (sval == 0) {
      sem_post(m_SemID);
   }
   return true;
}
 
CSharedMemory::~CSharedMemory()
{
   // Clear();
}
 
void CSharedMemory::Clear()
{
   if(m_iD != -1)
   {
      if ( shm_unlink(m_sName.c_str()) < 0 )
      {
         perror("shm_unlink");
      }
   }
   /**
   * Semaphore unlink: Remove a named semaphore  from the system.
   */
   if(m_SemID != NULL)
   {
      /**
      * Semaphore Close: Close a named semaphore
      */
      if ( sem_close(m_SemID) < 0 )
      {
         perror("sem_close");
      }
      /**
      * Semaphore unlink: Remove a named semaphore  from the system.
      */
      if ( sem_unlink(m_sName.c_str()) < 0 )
      {
         perror("sem_unlink");
      }
   }
}
 
CSharedMemoryException::CSharedMemoryException( const string &message, bool bSysMsg /*= false*/ ) throw()
{
 
}
 
CSharedMemoryException::~CSharedMemoryException() throw ()
{
 
}
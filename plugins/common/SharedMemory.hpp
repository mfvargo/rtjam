#include<string>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <sys/mman.h>
#include <fcntl.h>           /* For O_* constants */
#include <semaphore.h>
#include <unistd.h>
using namespace std;
 
/**
 *   Signals a problem with the execution of a SharedMemory call.
 */
 
class CSharedMemoryException: public std::exception
{
public:
    /**
   *   Construct a SharedMemoryException with a explanatory message.
   *   @param message explanatory message
   *   @param bSysMsg true if system message (from strerror(errno))
   *   should be postfixed to the user provided message
   */
    CSharedMemoryException(const string &message, bool bSysMsg = false) throw();
 
 
    /** Destructor.
     * Virtual to allow for subclassing.
     */
    virtual ~CSharedMemoryException() throw ();
 
    /** Returns a pointer to the (constant) error description.
     *  @return A pointer to a \c const \c char*. The underlying memory
     *          is in posession of the \c Exception object. Callers \a must
     *          not attempt to free the memory.
     */
    virtual const char* what() const throw (){  return m_sMsg.c_str(); }
 
protected:
    /** Error message.
     */
    std::string m_sMsg;
};
 
class CSharedMemory
{
public:
   enum
   {
      C_READ_ONLY  = O_RDONLY,
      C_READ_WRITE = O_RDWR,
   } CREATE_MODE;
    
   enum
   {
      A_READ  = PROT_READ,
      A_WRITE = PROT_WRITE,
   } ATTACH_MODE;
 
public:
   CSharedMemory(const string& sName );
   ~CSharedMemory();
 
   
   bool Create(size_t nSize, int mode = C_READ_WRITE);
   bool Attach(int mode = A_READ | A_WRITE);
   bool Detach();
   bool Lock();
   bool UnLock();
   int GetID() { return m_iD; }
   void* GetData() { return m_Ptr; };
   const void* GetData() const { return m_Ptr; }
private:
   void Clear();
private:
   string m_sName;
   int m_iD;
   sem_t* m_SemID;
   size_t m_nSize;
   void* m_Ptr;
 
};
#ifndef MESSAGE_QUEUE_RTJAM
#define MESSAGE_QUEUE_RTJAM

#include<string>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <mqueue.h>
#include <unistd.h>
using namespace std;
 
/**
 *   Signals a problem with the execution of a SharedMemory call.
 */
 
class CMessageQueueException: public std::exception
{
public:
    /**
   *   Construct a MessageQueueException with a explanatory message.
   *   @param message explanatory message
   *   @param bSysMsg true if system message (from strerror(errno))
   *   should be postfixed to the user provided message
   */
    CMessageQueueException(const string &message, bool bSysMsg = false) throw();
 
 
    /** Destructor.
     * Virtual to allow for subclassing.
     */
    virtual ~CMessageQueueException() throw ();
 
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
 
class CMessageQueue
{
public:
   CMessageQueue(const string& sName, int msgSize );
   ~CMessageQueue();
   void flush();
   void send(void* msg, int size);
   void recv(void* msg, int size);
 
private:
   string m_sName;
   mqd_t m_queueID;
   mq_attr m_attrs;
};

#endif
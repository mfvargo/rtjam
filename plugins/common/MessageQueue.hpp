#ifndef MESSAGE_QUEUE_RTJAM
#define MESSAGE_QUEUE_RTJAM

#include <string>
#include <sys/mman.h>
#include <sys/stat.h> /* For mode constants */
#include <fcntl.h>    /* For O_* constants */
#include <mqueue.h>
#include <unistd.h>
#include <queue>

using namespace std;

class CMessageQueue
{
public:
    CMessageQueue(const string &sName, int msgSize);
    ~CMessageQueue();
    void flush();
    void send(void *msg, int size);
    void recv(void *msg, int size);

private:
    string m_sName;
    mqd_t m_queueID;
    mq_attr m_attrs;
};

#endif
#include "MessageQueue.hpp"
#include <sys/mman.h>
#include <errno.h>
#include <cstdio>
#include <cstdlib>
#include <sys/time.h>

CMessageQueue::CMessageQueue(const string &sName, int msgSize) : m_sName(sName)
{
  /**
   * Semaphore open
   */
  m_attrs.mq_flags = 0;
  m_attrs.mq_maxmsg = 10;
  m_attrs.mq_msgsize = msgSize;
  m_attrs.mq_curmsgs = 0;
  if ((m_queueID = mq_open(sName.c_str(), O_CREAT | O_RDWR, 0666, &m_attrs)) == -1)
  {
    perror("mq_open failed");
  }
  fchmod(m_queueID, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
}

CMessageQueue::~CMessageQueue()
{
  mq_close(m_queueID);
}

void CMessageQueue::flush()
{
  char flushBuf[m_attrs.mq_msgsize];
  printf("flushing\n");
  timespec currentTime;
  clock_gettime(CLOCK_REALTIME, &currentTime);
  while (mq_timedreceive(m_queueID, flushBuf, m_attrs.mq_msgsize, NULL, &currentTime) > 0)
  {
    printf("flushing a stale message\n");
  }
}

void CMessageQueue::recv(void *msg, int size)
{
  if (mq_receive(m_queueID, (char *)msg, size, NULL) == -1)
  {
    perror("CMessageQueue: mq_receive");
  }
}

void CMessageQueue::send(void *msg, int size)
{
  // printf("sending\n");
  // mq_attr attrs;
  // mq_getattr(m_queueID, &attrs);
  // printf("sending, depth: %ld\n", attrs.mq_curmsgs);
  if (mq_send(m_queueID, (char *)msg, size, 0) == -1)
  {
    perror("CMessageQueue: Not able to send message to client");
  }
}

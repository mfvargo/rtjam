#pragma once

#include <queue>
#include <mutex>

using namespace std;

template <class ParamType>
class ParamQueue
{
public:
  void enqueue(ParamType x)
  {
    m_mutex.lock();
    m_queue.push(x);
    m_mutex.unlock();
  }
  bool empty()
  {
    m_mutex.lock();
    bool rval = m_queue.empty();
    m_mutex.unlock();
    return rval;
  }
  ParamType dequeue()
  {
    m_mutex.lock();
    ParamType rval = m_queue.front();
    m_queue.pop();
    m_mutex.unlock();
    return rval;
  }

private:
  queue<ParamType> m_queue;
  mutex m_mutex;
};

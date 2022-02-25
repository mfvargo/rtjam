#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <thread>
#include <queue>
#include <vector>
#include <condition_variable>
#include <mutex>
using namespace std;

class PacketStreamWriter
{
  ofstream m_outfile;
  int m_sizeCount;
  int m_writeCounter;
  bool m_writing;
  bool m_done;
  queue<vector<char>> m_queue;
  thread m_thread;
  mutex m_mutex;
  condition_variable m_cv;

  void worker()
  {
    vector<char> buff;
    while (!m_done)
    {
      unique_lock<std::mutex> lk(m_mutex);
      // m_cv.wait(lk, [this]
      //           { return !m_done; });
      m_cv.wait(lk);
      while (!m_queue.empty())
      {
        buff = m_queue.front();
        // we would write to the file here...
        m_outfile.write(&buff[0], buff.size());
        m_queue.pop();
      }
      if (m_outfile.tellp() > 1500000000)
      {
        // Shut the file when it gets full...
        m_writing = false;
      }
      // Check for file close
      if (!m_writing)
      {
        // we have flushed out the rest of the stuff.
        cout << "closing" << endl;
        m_outfile.close();
      }
    }
  }

public:
  PacketStreamWriter()
      : m_writing(false), m_done(false), m_thread(&PacketStreamWriter::worker, this)
  {
    // Do some constructin
  }
  ~PacketStreamWriter()
  {
    m_done = true;
    m_cv.notify_all();
    m_thread.join();
    // do some destructin
  }
  bool open(const char *filename)
  {
    // Open the file
    if (m_writing)
    {
      return false;
    }

    // This file open should probably go in the worker...
    m_outfile.open(filename, ios::out | ios::binary);
    if (!m_outfile.good())
    {
      return false;
    }
    m_writing = true;
    m_sizeCount = 0;
    return m_writing;
  }
  bool close()
  {
    m_writing = false;
    m_cv.notify_one();
    // This close needs to be done by the worker thread when it's done flushing.
    // m_outfile.close();
    return true;
  }
  bool is_open()
  {
    return m_writing;
  }
  bool write(const char *buffer, int size)
  {
    if (!m_writing)
    {
      return false;
    }
    // Queue the data and signal the worker
    vector<char> buff(buffer, buffer + size);
    unique_lock<mutex> lk(m_mutex);
    m_queue.push(buff);
    m_sizeCount += size;
    if (m_sizeCount > 1500000000)
    {
      return close();
    }
    // Don't signal the queue so the queue stays in memory
    // m_cv.notify_one();
    return true;
  }
};
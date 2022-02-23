#pragma once

#include <string>
#include <iostream>
#include <fstream>
using namespace std;

class PacketStreamWriter
{
  ofstream m_outfile;
  int m_framecount;

public:
  PacketStreamWriter()
  {
    // Do some constructin
  }
  ~PacketStreamWriter()
  {
    // do some destructin
  }
  bool open(const char *filename)
  {
    // Open the file
    if (m_outfile.is_open())
    {
      return false;
    }
    m_outfile.open(filename, ios::out | ios::binary);
    return m_outfile.good();
  }
  bool close()
  {
    return true;
  }
  bool is_open()
  {
    return m_outfile.is_open();
  }
  bool write(const char *buffer, int size)
  {
    if (!m_outfile.is_open())
    {
      return false;
    }
    if (m_outfile.tellp() > 1500000000)
    {
      // Shut the file when it gets full...
      close();
      return false;
    }
    // write the stuff.
    m_outfile.write(buffer, size);
    return m_outfile.good();
  }
  bool good()
  {
    return m_outfile.good();
  }
};
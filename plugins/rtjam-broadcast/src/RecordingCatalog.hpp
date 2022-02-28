#pragma once

#include <string>
#include <vector>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

#include "json.hpp"

using json = nlohmann::json;
using namespace std;

class SessionRecording
{
  string m_directory;
  string m_name;

public:
  struct stat m_stat;

  bool operator<(const SessionRecording &b)
  {
    return (m_stat.st_ctime < b.m_stat.st_ctime);
  }
  bool operator>(const SessionRecording &b)
  {
    return (m_stat.st_ctime > b.m_stat.st_ctime);
  }
  SessionRecording(string directory, string filename)
  {
    m_directory = directory;
    m_name = filename;
    if (stat((directory + "/" + filename).c_str(), &m_stat) != 0)
    {
      perror("failed to stat file");
    }
  }

  string name()
  {
    return m_name;
  }

  json toJson()
  {
    struct tm *timeinfo = gmtime(&m_stat.st_ctime); // or gmtime() depending on what you want
    string date = asctime(gmtime(&m_stat.st_ctime));
    return {{"name", m_directory + "/" + m_name}, {"date", date}, {"size", m_stat.st_size}};
  }
};

class RecordingCatalog
{
  // List of files in the catalogs directory
  vector<SessionRecording> m_list;
  string m_directory;

public:
  RecordingCatalog()
  {
    m_directory = "";
  }

  void init(string directory)
  {
    m_directory = directory;
    cout << "init Catalog" << endl;
    buildList();
  }

  json list()
  {
    buildList();
    json rval = json::array();
    for (auto &value : m_list)
    {
      rval.push_back(value.toJson());
    }
    return rval;
  }

  string getNewFilename()
  {
    m_list.clear();
    buildList();
    char buffer[256];
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, 256, "/rec_%m-%d_%H:%M:%S.raw", timeinfo);
    return m_directory + string(buffer);
  }

  string getLastFilename()
  {
    buildList();
    if (m_list.empty())
    {
      return "";
    }
    return m_directory + "/" + m_list.back().name();
  }

  void del(string filename)
  {
    // delete the file
    unlink((m_directory + filename).c_str());
  }

protected:
  void buildList()
  {
    // force cache clear for now
    m_list.clear();
    if (m_list.empty())
    {
      cout << "building list" << endl;
      DIR *dirp = opendir(m_directory.c_str());
      if (dirp != NULL)
      {
        struct dirent *dp;
        while ((dp = readdir(dirp)) != NULL)
        {
          string entry = dp->d_name;
          if (entry.find(".raw") != string::npos)
          {
            m_list.push_back(SessionRecording(m_directory, entry));
          }
        }
        // Sort list
        sort(m_list.begin(), m_list.end());
      }
      closedir(dirp);
    }
  }
};
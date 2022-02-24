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
  string m_name;
  string m_date;

public:
  SessionRecording(string filename)
  {
    m_name = filename;
    struct stat t_stat;
    stat(filename.c_str(), &t_stat);
    struct tm *timeinfo = localtime(&t_stat.st_ctime); // or gmtime() depending on what you want
    m_date = asctime(timeinfo);
  }

  json toJson()
  {
    return {{"name", m_name}, {"date", m_date}};
  }
};

class RecordingCatalog
{

  string m_directory;

public:
  RecordingCatalog(string dir)
  {
    m_directory = dir;
  }

  json list()
  {
    // load directory
    json rval = json::array();
    DIR *dirp = opendir(m_directory.c_str());
    if (dirp != NULL)
    {
      struct dirent *dp;
      while ((dp = readdir(dirp)) != NULL)
      {
        string entry = dp->d_name;
        if (entry.find(".raw") != string::npos)
        {
          SessionRecording rec(entry);
          rval.push_back(rec.toJson());
        }
      }
    }
    closedir(dirp);
    return rval;
  }

  void del(string filename)
  {
    // delete the file
  }
};
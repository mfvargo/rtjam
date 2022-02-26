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
  SessionRecording(string directory, string filename)
  {
    m_name = filename;
    struct stat t_stat;
    if (stat((directory + "/" + filename).c_str(), &t_stat) != 0)
    {
      perror("failed to stat file");
    }
    struct tm *timeinfo = gmtime(&t_stat.st_ctime); // or gmtime() depending on what you want
    m_date = asctime(timeinfo);
  }

  json toJson()
  {
    return {{"name", m_name}, {"date", m_date}};
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
    return m_directory + "/take_1.raw";
  }

  void del(string filename)
  {
    // delete the file
    unlink((m_directory + filename).c_str());
  }

protected:
  void buildList()
  {
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
      }
      closedir(dirp);
    }
  }
};
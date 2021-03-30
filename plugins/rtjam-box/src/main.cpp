#include "LevelData.hpp"
#include "ParamData.hpp"
#include "RTJamNationApi.hpp"
#include "Settings.hpp"
#include <iostream>
#include <string>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include<cstdio>

#include <inttypes.h>

#include <fastcgi++/request.hpp>
#include <fastcgi++/manager.hpp>

class Echo: public Fastcgipp::Request<char>
{
public:
    Echo():
        Fastcgipp::Request<char>(50*1024)
    {}


private:
    LevelData m_levelData;
    RTJamLevels m_jamLevels;

    void doGet() {
      // get request goes here
      memcpy(&m_jamLevels, m_levelData.m_pJamLevels, sizeof(RTJamLevels));
      for (int i=0; i<MIX_CHANNELS; i++) {
        out << "chan: " << i << "  Depth:  " << m_jamLevels.bufferDepths[i] * 40;
        out << " level:  " << m_jamLevels.channelLevels[i] << "<br/>";
      }
    }
    void doPut() {
        RTJamParam param;
        for(const auto& post: environment().posts) {
            if (post.first == "command") {
                param.param = (RTJamParameters) atoi(post.second.c_str());
            }
            if (post.first == "sValue") {
                snprintf(param.sValue, 126, "%s", post.second.c_str());
            }
            if (post.first == "iValue1") {
                param.iValue = atoi(post.second.c_str());
            }
            if (post.first == "iValue2") {
                param.iValue2 = atoi(post.second.c_str());
            }
        }
        ParamData paramData;
        paramData.flush();
        paramData.send(&param);
    }
    void writeBody() {
        using Fastcgipp::Encoding;
        out <<  "<h1>RTJAM_BOX</h1>";
        switch(environment().requestMethod) {
            case Fastcgipp::Http::RequestMethod::GET:
                doGet();
            break;
            case Fastcgipp::Http::RequestMethod::PUT:
                doPut();
                echostuff();
            break;
            default:
            break;
        }

    }

    bool response()
    {
        using Fastcgipp::Encoding;

        out << "Set-Cookie: echoCookie=" << Encoding::URL << "<\"rtjam_cook\">;"
            << Encoding::NONE << "; path=/\n";

        out << "Content-Type: text/html; charset=utf-8\r\n\r\n";
        out <<
        "<!DOCTYPE html>\n"
        "<html>"
            "<head>"
                "<meta charset='utf-8' />"
                "<title>fastcgi++: Echo</title>"
            "</head>";
        out << 
        "<body>";
        writeBody();

 
        out << "</body>"
                "</html>";
 
        return true;
    }

    void echostuff() {
        using Fastcgipp::Encoding;

        out <<
        "<h2>Environment Parameters</h2>"
        "<p>"
            "<b>FastCGI Version:</b> "
                << Fastcgipp::Protocol::version << "<br />"
            "<b>fastcgi++ Version:</b> " << Fastcgipp::version << "<br />"
            "<b>Hostname:</b> " << Encoding::HTML << environment().host
                << Encoding::NONE << "<br />"
            "<b>Origin Server:</b> " << Encoding::HTML << environment().origin
                << Encoding::NONE << "<br />"
            "<b>User Agent:</b> " << Encoding::HTML << environment().userAgent
                << Encoding::NONE << "<br />"
            "<b>Accepted Content Types:</b> " << Encoding::HTML
                << environment().acceptContentTypes << Encoding::NONE
                << "<br />"
            "<b>Accepted Languages:</b> " << Encoding::HTML;
        if(!environment().acceptLanguages.empty())
        {
            auto language = environment().acceptLanguages.cbegin();
            while(true)
            {
                out << language->c_str();
                ++language;
                if(language == environment().acceptLanguages.cend())
                    break;
                out << ',';
            }
        }
        out << Encoding::NONE << "<br />"
            "<b>Accepted Characters Sets:</b> " << Encoding::HTML
                << environment().acceptCharsets << Encoding::NONE << "<br />"
            "<b>Referer:</b> " << Encoding::HTML << environment().referer
                << Encoding::NONE << "<br />"
            "<b>Content Type:</b> " << Encoding::HTML
                << environment().contentType << Encoding::NONE << "<br />"
            "<b>Root:</b> " << Encoding::HTML << environment().root
                << Encoding::NONE << "<br />"
            "<b>Script Name:</b> " << Encoding::HTML
                << environment().scriptName << Encoding::NONE << "<br />"
            "<b>Request URI:</b> " << Encoding::HTML
                << environment().requestUri << Encoding::NONE << "<br />"
            "<b>Request Method:</b> " << environment().requestMethod
                << "<br />"
            "<b>Content Length:</b> " << environment().contentLength
                << " bytes<br />"
            "<b>Keep Alive Time:</b> " << environment().keepAlive
                << " seconds<br />"
            "<b>Server Address:</b> " << environment().serverAddress
                << "<br />"
            "<b>Server Port:</b> " << environment().serverPort << "<br />"
            "<b>Client Address:</b> " << environment().remoteAddress << "<br />"
            "<b>Client Port:</b> " << environment().remotePort << "<br />"
            "<b>Etag:</b> " << environment().etag << "<br />"
            "<b>If Modified Since:</b> " << Encoding::HTML <<
        "</p>";
 
        out <<
        "<h2>Path Info</h2>";
        if(environment().pathInfo.size())
        {
            out <<
        "<p>";
            std::string preTab;
            for(const auto& element: environment().pathInfo)
            {
                out << preTab << Encoding::HTML << element << Encoding::NONE
                    << "<br />";
                preTab += "&nbsp;&nbsp;&nbsp;&nbsp;";
            }
            out <<
        "</p>";
        }
        else
            out <<
        "<p>No Path Info</p>";
 
        out <<
        "<h2>Other Environment Parameters</h2>";
        if(!environment().others.empty())
            for(const auto& other: environment().others)
                out << "<b>" << Encoding::HTML << other.first << Encoding::NONE
                    << ":</b> " << Encoding::HTML << other.second
                    << Encoding::NONE << "<br />";
        else
            out <<
        "<p>No Other Environment Parameters</p>";
 
        out <<
        "<h2>GET Data</h2>";
        if(!environment().gets.empty())
            for(const auto& get: environment().gets)
                out << "<b>" << Encoding::HTML << get.first << Encoding::NONE
                    << ":</b> " << Encoding::HTML << get.second
                    << Encoding::NONE << "<br />";
        else
            out <<
        "<p>No GET data</p>";
 
        out <<
        "<h2>POST Data</h2>";
        if(!environment().posts.empty())
            for(const auto& post: environment().posts)
                out << "<b>" << Encoding::HTML << post.first << Encoding::NONE
                    << ":</b> " << Encoding::HTML << post.second
                    << Encoding::NONE << "<br />";
        else
            out <<
        "<p>No POST data</p>";
 
        out <<
        "<h2>Cookies</h2>";
        if(!environment().cookies.empty())
            for(const auto& cookie: environment().cookies)
                out << "<b>" << Encoding::HTML << cookie.first
                    << Encoding::NONE << ":</b> " << Encoding::HTML
                    << cookie.second << Encoding::NONE << "<br />";
        else
            out <<
        "<p>No Cookies</p>";
 
        out <<
        "<h2>Files</h2>";
        if(!environment().files.empty())
        {
            for(const auto& file: environment().files)
            {
                out <<
        "<h3>" << Encoding::HTML << file.first << Encoding::NONE << "</h3>"
        "<p>"
            "<b>Filename:</b> " << Encoding::HTML << file.second.filename
                << Encoding::NONE << "<br />"
            "<b>Content Type:</b> " << Encoding::HTML
                << file.second.contentType << Encoding::NONE << "<br />"
            "<b>Size:</b> " << file.second.size << "<br />"
            "<b>Data:</b>"
        "</p>"
        "<pre>";
                dump(file.second.data.get(), file.second.size);
                out <<
        "</pre>";
            }
        }
        else
            out <<
        "<p>No files</p>";
 
        out <<
        "<h1>Form</h1>"
        "<h3>multipart/form-data</h3>"
        "<form action='?getVar=testing&amp;secondGetVar=tested&amp;"
            "utf8GetVarTest=проверка&amp;enctype=multipart' method='post' "
            "enctype='multipart/form-data' accept-charset='utf-8'>"
            "Name: <input type='text' name='+= aquí está el campo' value='Él "
                "está con un niño' /><br />"
            "File: <input type='file' name='aFile' /> <br />"
            "<input type='submit' name='submit' value='submit' />"
        "</form>"
        "<h3>application/x-www-form-urlencoded</h3>"
        "<form action='?getVar=testing&amp;secondGetVar=tested&amp;"
            "utf8GetVarTest=проверка&amp;enctype=url-encoded' method='post' "
            "enctype='application/x-www-form-urlencoded' "
            "accept-charset='utf-8'>"
            "Name: <input type='text' name='+= aquí está el campo' value='Él "
                "está con un niño' /><br />"
            "File: <input type='file' name='aFile' /><br />"
            "<input type='submit' name='submit' value='submit' />"
        "</form>";
    }
};

using namespace std;
int main(int argc, char* argv[])
{
    Fastcgipp::Manager<Echo> manager;
    //! [Manager]
    //! [Signals]
    manager.setupSignals();
    //! [Signals]
    //! [Listen]
    manager.listen("/tmp/rtjambox.sock", 0666);
    //! [Listen]
    //! [Start]
    manager.start();
    //! [Start]
    //! [Join]
    manager.join();

    return 0;

  // int cnt = 0;
  // LevelData levelData;
  // ParamData paramData;
  // RTJamParam param;
  // Settings settings;
  // settings.loadFromFile();
  // string urlBase = settings.getOrSetValue("rtjam-nation", "rtjam-nation.basscleftech.com/api/");
  // string token = settings.getOrSetValue("rtjam-unit-token", "");
  // settings.saveToFile();
  // RTJamNationApi api(urlBase);

  // api.status();

  // api.jamUnitPing(token);
  // clog << api.m_resultBody.dump(2) << endl;


  // paramData.flush();

  // param.param = paramRoomChange;
  // param.iValue = 7891;
  // param.iValue2 = 4567;
  // sprintf(param.sValue, "192.168.1.245");
  // paramData.send(&param);

  // while(cnt++ < 10) {
  //   // send a param
  //   levelData.lock();
  //   printf("%d:reading level Data\n", cnt);
  //   for (int i=0; i<MAX_JAMMERS; i++) {
  //     printf("Depth: %0.0f  ", levelData.m_pJamLevels->bufferDepths[i] * 40);
  //   }
  //   printf("\ninput %f\n", levelData.m_pJamLevels->inputLeft);
  //   // sleep(1);
  // }

  // param.param = paramDisconnect;
  // paramData.send(&param);

}

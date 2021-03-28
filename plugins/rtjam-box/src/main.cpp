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

#include <fastcgi++/request.hpp>
#include <fastcgi++/manager.hpp>

class Echo: public Fastcgipp::Request<wchar_t>
{
public:
    Echo():
        Fastcgipp::Request<wchar_t>(50*1024)
    {}


private:
    LevelData m_levelData;
    RTJamLevels m_jamLevels;

    void doGet() {
      // get request goes here
      memcpy(&m_jamLevels, m_levelData.m_pJamLevels, sizeof(RTJamLevels));
      for (int i=0; i<MIX_CHANNELS; i++) {
        out << L"chan: " << i << L"  Depth:  " << m_jamLevels.bufferDepths[i] * 40;
        out << L" level:  " << m_jamLevels.channelLevels[i] << L"<br/>";
      }
    }
    void doPut() {
        // put request goes here
    }
    void writeBody() {
        using Fastcgipp::Encoding;
        out <<  L"<h1>RTJAM_BOX</h1>";
        switch(environment().requestMethod) {
            case Fastcgipp::Http::RequestMethod::GET:
                doGet();
            break;
            case Fastcgipp::Http::RequestMethod::PUT:
                doPut();
            break;
            default:
            break;
        }

    }

    bool response()
    {
        using Fastcgipp::Encoding;

        out << L"Set-Cookie: echoCookie=" << Encoding::URL << L"<\"rtjam_cook\">;"
            << Encoding::NONE << L"; path=/\n";

        out << L"Content-Type: text/html; charset=utf-8\r\n\r\n";
        out <<
        L"<!DOCTYPE html>\n"
        L"<html>"
            L"<head>"
                L"<meta charset='utf-8' />"
                L"<title>fastcgi++: Echo</title>"
            L"</head>";
        out << 
        L"<body>";
        writeBody();

 
        out << L"</body>"
                L"</html>";
 
        return true;
    }

    void echostuff() {
        using Fastcgipp::Encoding;

        out <<
        L"<h2>Environment Parameters</h2>"
        L"<p>"
            L"<b>FastCGI Version:</b> "
                << Fastcgipp::Protocol::version << L"<br />"
            L"<b>fastcgi++ Version:</b> " << Fastcgipp::version << L"<br />"
            L"<b>Hostname:</b> " << Encoding::HTML << environment().host
                << Encoding::NONE << L"<br />"
            L"<b>Origin Server:</b> " << Encoding::HTML << environment().origin
                << Encoding::NONE << L"<br />"
            L"<b>User Agent:</b> " << Encoding::HTML << environment().userAgent
                << Encoding::NONE << L"<br />"
            L"<b>Accepted Content Types:</b> " << Encoding::HTML
                << environment().acceptContentTypes << Encoding::NONE
                << L"<br />"
            L"<b>Accepted Languages:</b> " << Encoding::HTML;
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
        out << Encoding::NONE << L"<br />"
            L"<b>Accepted Characters Sets:</b> " << Encoding::HTML
                << environment().acceptCharsets << Encoding::NONE << L"<br />"
            L"<b>Referer:</b> " << Encoding::HTML << environment().referer
                << Encoding::NONE << L"<br />"
            L"<b>Content Type:</b> " << Encoding::HTML
                << environment().contentType << Encoding::NONE << L"<br />"
            L"<b>Root:</b> " << Encoding::HTML << environment().root
                << Encoding::NONE << L"<br />"
            L"<b>Script Name:</b> " << Encoding::HTML
                << environment().scriptName << Encoding::NONE << L"<br />"
            L"<b>Request URI:</b> " << Encoding::HTML
                << environment().requestUri << Encoding::NONE << L"<br />"
            L"<b>Request Method:</b> " << environment().requestMethod
                << L"<br />"
            L"<b>Content Length:</b> " << environment().contentLength
                << L" bytes<br />"
            L"<b>Keep Alive Time:</b> " << environment().keepAlive
                << L" seconds<br />"
            L"<b>Server Address:</b> " << environment().serverAddress
                << L"<br />"
            L"<b>Server Port:</b> " << environment().serverPort << L"<br />"
            L"<b>Client Address:</b> " << environment().remoteAddress << L"<br />"
            L"<b>Client Port:</b> " << environment().remotePort << L"<br />"
            L"<b>Etag:</b> " << environment().etag << L"<br />"
            L"<b>If Modified Since:</b> " << Encoding::HTML <<
        L"</p>";
 
        out <<
        L"<h2>Path Info</h2>";
        if(environment().pathInfo.size())
        {
            out <<
        L"<p>";
            std::wstring preTab;
            for(const auto& element: environment().pathInfo)
            {
                out << preTab << Encoding::HTML << element << Encoding::NONE
                    << L"<br />";
                preTab += L"&nbsp;&nbsp;&nbsp;&nbsp;";
            }
            out <<
        L"</p>";
        }
        else
            out <<
        L"<p>No Path Info</p>";
 
        out <<
        L"<h2>Other Environment Parameters</h2>";
        if(!environment().others.empty())
            for(const auto& other: environment().others)
                out << L"<b>" << Encoding::HTML << other.first << Encoding::NONE
                    << L":</b> " << Encoding::HTML << other.second
                    << Encoding::NONE << L"<br />";
        else
            out <<
        L"<p>No Other Environment Parameters</p>";
 
        out <<
        L"<h2>GET Data</h2>";
        if(!environment().gets.empty())
            for(const auto& get: environment().gets)
                out << L"<b>" << Encoding::HTML << get.first << Encoding::NONE
                    << L":</b> " << Encoding::HTML << get.second
                    << Encoding::NONE << L"<br />";
        else
            out <<
        L"<p>No GET data</p>";
 
        out <<
        L"<h2>POST Data</h2>";
        if(!environment().posts.empty())
            for(const auto& post: environment().posts)
                out << L"<b>" << Encoding::HTML << post.first << Encoding::NONE
                    << L":</b> " << Encoding::HTML << post.second
                    << Encoding::NONE << L"<br />";
        else
            out <<
        L"<p>No POST data</p>";
 
        out <<
        L"<h2>Cookies</h2>";
        if(!environment().cookies.empty())
            for(const auto& cookie: environment().cookies)
                out << L"<b>" << Encoding::HTML << cookie.first
                    << Encoding::NONE << L":</b> " << Encoding::HTML
                    << cookie.second << Encoding::NONE << L"<br />";
        else
            out <<
        L"<p>No Cookies</p>";
 
        out <<
        L"<h2>Files</h2>";
        if(!environment().files.empty())
        {
            for(const auto& file: environment().files)
            {
                out <<
        L"<h3>" << Encoding::HTML << file.first << Encoding::NONE << L"</h3>"
        L"<p>"
            L"<b>Filename:</b> " << Encoding::HTML << file.second.filename
                << Encoding::NONE << L"<br />"
            L"<b>Content Type:</b> " << Encoding::HTML
                << file.second.contentType << Encoding::NONE << L"<br />"
            L"<b>Size:</b> " << file.second.size << L"<br />"
            L"<b>Data:</b>"
        L"</p>"
        L"<pre>";
                dump(file.second.data.get(), file.second.size);
                out <<
        L"</pre>";
            }
        }
        else
            out <<
        L"<p>No files</p>";
 
        out <<
        L"<h1>Form</h1>"
        L"<h3>multipart/form-data</h3>"
        L"<form action='?getVar=testing&amp;secondGetVar=tested&amp;"
            L"utf8GetVarTest=проверка&amp;enctype=multipart' method='post' "
            L"enctype='multipart/form-data' accept-charset='utf-8'>"
            L"Name: <input type='text' name='+= aquí está el campo' value='Él "
                L"está con un niño' /><br />"
            L"File: <input type='file' name='aFile' /> <br />"
            L"<input type='submit' name='submit' value='submit' />"
        L"</form>"
        L"<h3>application/x-www-form-urlencoded</h3>"
        L"<form action='?getVar=testing&amp;secondGetVar=tested&amp;"
            L"utf8GetVarTest=проверка&amp;enctype=url-encoded' method='post' "
            L"enctype='application/x-www-form-urlencoded' "
            L"accept-charset='utf-8'>"
            L"Name: <input type='text' name='+= aquí está el campo' value='Él "
                L"está con un niño' /><br />"
            L"File: <input type='file' name='aFile' /><br />"
            L"<input type='submit' name='submit' value='submit' />"
        L"</form>";
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

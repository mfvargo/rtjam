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

class HelloWorld: public Fastcgipp::Request<wchar_t> {
  bool response() {
        out << L"Content-Type: text/html; charset=utf-8\r\n\r\n";
        //! [HTTP header]

        //! [Output]
        out <<
L"<!DOCTYPE html>\n"
L"<html>"
    L"<head>"
        L"<meta charset='utf-8' />"
        L"<title>fastcgi++: Hello World</title>"
    L"</head>"
    L"<body>"
        L"<p>"
            L"English: Herro World<br>"
            L"Russian: Привет мир<br>"
            L"Greek: Γεια σας κόσμο<br>"
            L"Chinese: 世界您好<br>"
            L"Japanese: 今日は世界<br>"
            L"Runic English?: ᚺᛖᛚᛟ ᚹᛟᛉᛚᛞ<br>"
        L"</p>"
    L"</body>"
L"</html>";
        //! [Output]

        //! [Return]
        return true;
  }
};

using namespace std;
int main(int argc, char* argv[])
{
    Fastcgipp::Manager<HelloWorld> manager;
    //! [Manager]
    //! [Signals]
    manager.setupSignals();
    //! [Signals]
    //! [Listen]
    manager.listen("/tmp/rtjambox");
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
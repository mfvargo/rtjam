#include "LevelData.hpp"
#include "ParamData.hpp"
#include "RTJamNationApi.hpp"
#include <iostream>
#include <string>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include<cstdio>
using namespace std;
int main(int argc, char* argv[])
{
  int cnt = 0;
  LevelData levelData;
  ParamData paramData;
  RTJamParam param;
  RTJamNationApi api;

  // api.testMe("http://music.basscleftech.com/users/index.json");

  // sleep(1);
  
  api.testMe("https://dev.neucourt.com/api/status");

  // paramData.flush();

  // param.param = paramRoomChange;
  // param.iValue = 7892;
  // param.iValue2 = 4567;
  // sprintf(param.sValue, "music.basscleftech.com");
  // paramData.send(&param);

  // while(cnt++ < 3000) {
  //   // send a param
  //   param.param = paramMasterVol;
  //   param.fValue = cnt%2 ? -60.0 : 0.0;
  //   sprintf(param.sValue, "dude! %d", rand());
  //   paramData.send(&param);
  //   levelData.lock();
  //     printf("%d:reading level Data\n", cnt);
  //     for (int i=0; i<MAX_JAMMERS; i++) {
  //       printf("Depth: %0.0f  ", levelData.m_pJamLevels->bufferDepths[i] * 40);
  //     }
  //     printf("\ninput %f\n", levelData.m_pJamLevels->inputLeft);
  //     // sleep(1);
  // }
 
}
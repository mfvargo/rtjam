#include "LevelData.hpp"
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

  while(cnt < 10000) {
    levelData.lock();
      printf("%d:reading level Data\n", cnt++);
      for (int i=0; i<MAX_JAMMERS; i++) {
        printf("Depth: %0.0f  ", levelData.m_pJamLevels->bufferDepths[i] * 40);
      }
      printf("\ninput %f\n", levelData.m_pJamLevels->inputLeft);
      // sleep(1);
  }
 
}
#include "SharedMemory.hpp"
#include <iostream>
#include <string>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include<cstdio>
using namespace std;
int main(int argc, char* argv[])
{
   try
   {
      CSharedMemory shmMemory("/testSharedmemory1");
      shmMemory.Create(100);
      shmMemory.Attach();
      char* str = (char*)shmMemory.GetData();
 
      if(std::string(argv[1]) =="1")
      {
         for(int i=0;i<100;i++)
         {
            char sTemp[10];
            
            shmMemory.Lock();
            sprintf(sTemp,"Data:%d", rand()%100);
            strcpy(str,sTemp);
            printf("Writing:%s\n",str);
            shmMemory.UnLock();
            sleep(4);
         }
 
      }else
      {
         for(int i=0;i<100;i++)
         {
            char sTemp[10];
            printf("Reading:%d",i+1);
            shmMemory.Lock();
            printf("--->%s\n",str);
            shmMemory.UnLock();
            sleep(1);
         }
      }
 
   }
   catch (std::exception& ex)
   {
      cout<<"Exception:"<<ex.what();
   }
 
}
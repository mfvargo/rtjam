#include "RTJamNationApi.hpp"
#include "Settings.hpp"
#include "LightData.hpp"
#include "UnitChatRobot.hpp"
#include <iostream>

using namespace std;

string s_token = "";

using easywsclient::WebSocket;
LevelData s_levelData;


bool isRunning = true;

vector<thread> myThreads;

// Thread to keep the unit room websocket alive
void websocket_thread()
{
  UnitChatRobot robot;
  while (true)
  {
    if (s_token != "") {
        robot.init("ws://rtjam-nation.basscleftech.com/primus", s_token, &s_levelData);
        robot.readMessages();
        cout << "Room lost" << endl;
    }
    sleep(5);
  }
}

int jamNationStuff()
{
    LightData lightData;
    lightData.m_pLightSettings->status = red;
    Settings settings;
    settings.saveVersionFile();
    settings.loadFromFile();
    string urlBase = settings.getOrSetValue("rtjam-nation", "rtjam-nation.basscleftech.com/api/1/");
    settings.setValue("gitCommit", GIT_HASH);
    int version = stoi(settings.getOrSetValue("rtjam-unit-version", "0"));
    settings.saveToFile();
    string token = "";
    RTJamNationApi api(urlBase);
    int loopCount = 0;
    while (isRunning)
    {
        // printf("Light color: %d\n", lightData.m_pLightSettings->status);
        if (api.checkLinkStatus())
        {
            if (loopCount % 10 == 0)
            {
                if (token == "")
                {
                    // We don't have a token.  Register the device.
                    lightData.m_pLightSettings->status = orange;
                    if (api.jamUnitDeviceRegister() && api.m_httpResponseCode == 200)
                    {
                        // get the token
                        token = api.m_resultBody["jamUnit"]["token"];
                        s_token = token;
                    }
                }
                if (token != "")
                {
                    lightData.m_pLightSettings->status = green;
                    if (!api.jamUnitPing(token) || api.m_httpResponseCode != 200)
                    {
                        lightData.m_pLightSettings->status = orange;
                        // Something is wrong with this token
                        token = "";
                    };
                }
            }
        }
        else
        {
            // Set the loopCount to 10 so if it passes on the next iteration it will immediately try to check in with the nation
            loopCount = 9;
            // This code will make the light flash red with 1 second period
            if (lightData.m_pLightSettings->status != red)
                lightData.m_pLightSettings->status = red;
            else
                lightData.m_pLightSettings->status = black;
        }
        loopCount++;
        sleep(1);
    }
    return 0;
}

int main(int argc, char *argv[])
{
    //myThreads.push_back(thread(fastCGIStuff));
    myThreads.push_back(thread(websocket_thread));
    myThreads.push_back(thread(jamNationStuff));
    for (auto &element : myThreads)
    {
        element.join();
    }
    return 0;
}
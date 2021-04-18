#include "BoxAPI.hpp"
#include "RTJamNationApi.hpp"
#include "Settings.hpp"
#include <iostream>

using namespace std;

LevelData BoxAPI::s_levelData;
string BoxAPI::s_token = "";
bool isRunning = true;

vector<thread> myThreads;

int fastCGIStuff() {
    Fastcgipp::Manager<BoxAPI> manager;
    manager.setupSignals();
    manager.listen("/tmp/rtjambox.sock", 0666);
    manager.start();
    manager.join();
    isRunning = false;
    return 0;
}

int jamNationStuff() {
    Settings settings;
    settings.loadFromFile();
    string urlBase = settings.getOrSetValue("rtjam-nation", "rtjam-nation.basscleftech.com/api/1/");
    int version = stoi(settings.getOrSetValue("rtjam-unit-version", "0"));
    settings.saveToFile();
    string token = "";
    RTJamNationApi api(urlBase);
    while(isRunning) {
        if (token == "") {
            // We don't have a token.  Register the device.
            if (api.jamUnitDeviceRegister() && api.m_httpResponseCode == 200) {
                // get the token
                token = api.m_resultBody["jamUnit"]["token"];
                BoxAPI::s_token = token;
            }
        } else {
            if (!api.jamUnitPing(token) || api.m_httpResponseCode != 200) {
                // Something is wrong with this token
                token = "";
            };
            if (version < api.m_resultBody["version"]) {
                clog << "new version exists" << endl;
            }
            // clog << "version: " << api.m_resultBody["version"] << endl;
        }
        sleep(10);
    }
    return 0;
}

int main(int argc, char* argv[])
{
    myThreads.push_back(thread(fastCGIStuff));
    myThreads.push_back(thread(jamNationStuff));
    for (auto & element : myThreads) {
        element.join();
    }
    return 0;

 
}

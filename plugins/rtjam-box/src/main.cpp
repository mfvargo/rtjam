#include "BoxAPI.hpp"
#include "RTJamNationApi.hpp"
#include "Settings.hpp"
#include <iostream>

using namespace std;

LevelData BoxAPI::m_levelData;

vector<thread> myThreads;

int fastCGIStuff() {
    Fastcgipp::Manager<BoxAPI> manager;
    manager.setupSignals();
    manager.listen("/tmp/rtjambox.sock", 0666);
    manager.start();
    manager.join();
    return 0;
}

int jamNationStuff() {
    Settings settings;
    settings.loadFromFile();
    string urlBase = settings.getOrSetValue("rtjam-nation", "rtjam-nation.basscleftech.com/api/");
    settings.saveToFile();
    string token = "";
    RTJamNationApi api(urlBase);
    while(1) {
        if (token == "") {
            // We don't have a token.  Register the device.
            if (api.jamUnitDeviceRegister() && api.m_httpResponseCode == 200) {
                // get the token
                token = api.m_resultBody["jamUnit"]["token"];
            }
        } else {
            if (!api.jamUnitPing(token)) {
                // Something is wrong with this token
                token = "";
            };
        }
        clog << api.m_resultBody.dump(2) << endl;
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

#pragma once

#include "LevelData.hpp"
#include "ParamData.hpp"
#include "json.hpp"
#include <fastcgi++/request.hpp>
#include <fastcgi++/manager.hpp>
#include <iostream>

using namespace std;
using json = nlohmann::json;

class BoxAPI: public Fastcgipp::Request<char>
{
public:
    BoxAPI():
        Fastcgipp::Request<char>(50*1024)
    {}


private:
    LevelData m_levelData;
    RTJamLevels m_jamLevels;

    void doGet() {
        // This get called on GETs
        if (environment().requestUri.find("params") != string::npos) {
            getParamForm();
        } else {
            getLevels();
        }

    }
    void getLevels() {
        json result = json::array();
        // get request goes here
        memcpy(&m_jamLevels, m_levelData.m_pJamLevels, sizeof(RTJamLevels));
        for (int i=0; i<MIX_CHANNELS; i++) {
            result.push_back({ 
                {"channel", i}, 
                {"depth", m_jamLevels.bufferDepths[i] * 40},
                {"level", m_jamLevels.channelLevels[i]}
            });
        }
        out << result.dump(2);
    }
    void setParam() {
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
        out << "OK";
        getParamForm();
    }
    void getParamForm() {
        out <<
        "<h1>Set Param Form</h1>"
        "<form method='post' "
            "enctype='application/x-www-form-urlencoded' "
            "accept-charset='utf-8'>"
            "Command: <input type='text' name='command' value='21'/><br />"
            "sValue: <input type='text' name='sValue' value='music.basscleftech.com'/><br />"
            "iValue1: <input type='text' name='iValue1' value='7891'/><br />"
            "iValue2: <input type='text' name='iValue2' value='5025'/><br />"
            "<input type='submit' name='submit' value='submit' />"
        "</form>";

        // "<form action='/rtjambox/param' method='put' "
        //     "enctype='application/x-www-form-urlencoded' "
        //     "accept-charset='utf-8'>"

    }
    void writeBody() {
        using Fastcgipp::Encoding;
        switch(environment().requestMethod) {
            case Fastcgipp::Http::RequestMethod::GET:
                doGet();
            break;
            case Fastcgipp::Http::RequestMethod::POST:
                setParam();
            break;
            default:
            break;
        }

    }

    bool response()
    {
        try {
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
            out << "<body>";
    
            writeBody();
    
            out << "</body></html>";
            return true;
        } catch (exception& e) {
            clog << e.what() << endl;
        }
        return true;
    }
};

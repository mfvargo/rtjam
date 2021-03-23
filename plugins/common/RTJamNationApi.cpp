#include "RTJamNationApi.hpp"
#include <stdio.h>
#include <ifaddrs.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>


using namespace std;
using namespace restincurl;

json resultJson;

RTJamNationApi::RTJamNationApi(string urlbase) {
  m_urlBase = urlbase;
  getLanIp();
}

bool RTJamNationApi::status() {
  json args;
  return get(m_urlBase + "status", args);
}

bool RTJamNationApi::jamUnitPing(string token) {
  json args;
  args["token"] = token;
  args["lanIp"] = m_lanIp;
  return put(m_urlBase + "jamUnit/ping", args);
}

bool RTJamNationApi::broadcastUnitPing(string token) {
  json args;
  args["token"] = token;
  args["lanIp"] = m_lanIp;
  return put(m_urlBase + "broadcastUnit/ping", args);
}

bool RTJamNationApi::activateRoom(string token, string name, int port) {
  json args;
  args["token"] = token;
  args["name"] = name;
  args["port"] = port;
  args["lanIp"] = m_lanIp;
  return post(m_urlBase + "room", args);
}


bool RTJamNationApi::get(string url, json body) {
  string data;
  restincurl::Client client;
  CURLcode curlCode;

  client.Build()->Get(url)
        .Option(CURLOPT_VERBOSE, 0L)
        .AcceptJson()
        .StoreData(data)
        .WithJson()
        .SendData<string>(body.dump())
        .Header("X-Client", "restincurl")
        .WithCompletion([&](const Result& result) {
          curlCode = result.curl_code;
          clog << result.msg << endl;
          // clog << "PUT response: " << data << endl;
        })
        .ExecuteSynchronous();
  if (curlCode == CURLE_OK) {
    try {
      m_resultBody = json::parse(data);
    } catch (...) {
      clog << "Failed to parse server json" << endl;
    }
  }
  return (curlCode == CURLE_OK);
}
bool RTJamNationApi::put(string url, json body) {
  string data;
  restincurl::Client client;
  CURLcode curlCode;

  client.Build()->Put(url)
        .Option(CURLOPT_VERBOSE, 0L)
        .AcceptJson()
        .StoreData(data)
        .WithJson()
        .SendData<string>(body.dump())
        .Header("X-Client", "restincurl")
        .WithCompletion([&](const Result& result) {
          curlCode = result.curl_code;
          clog << result.msg << endl;
          // clog << "PUT response: " << data << endl;
        })
        .ExecuteSynchronous();
  if (curlCode == CURLE_OK) {
    try {
      m_resultBody = json::parse(data);
    } catch (...) {
      clog << "Failed to parse server json" << endl;
    }
  }
  return (curlCode == CURLE_OK);
}
bool RTJamNationApi::post(string url, json body) {
  string data;
  restincurl::Client client;
  CURLcode curlCode;

  client.Build()->Post(url)
        .Option(CURLOPT_VERBOSE, 0L)
        .AcceptJson()
        .StoreData(data)
        .WithJson()
        .SendData<string>(body.dump())
        .Header("X-Client", "restincurl")
        .WithCompletion([&](const Result& result) {
          curlCode = result.curl_code;
          clog << result.msg << endl;
          // clog << "PUT response: " << data << endl;
        })
        .ExecuteSynchronous();
  if (curlCode == CURLE_OK) {
    try {
      m_resultBody = json::parse(data);
    } catch (...) {
      clog << "Failed to parse server json" << endl;
    }
  }
  return (curlCode == CURLE_OK);
}

void RTJamNationApi::getLanIp() {
  ifaddrs *addrs;
  getifaddrs(&addrs);
  ifaddrs *tmp = addrs;

  while (tmp) 
  {
      if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET)
      {
          struct sockaddr_in *pAddr = (struct sockaddr_in *)tmp->ifa_addr;
          m_lanIp = inet_ntoa(pAddr->sin_addr);
      }

      tmp = tmp->ifa_next;
  }

  freeifaddrs(addrs);
}

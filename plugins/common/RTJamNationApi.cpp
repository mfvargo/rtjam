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

RTJamNationApi::RTJamNationApi(string urlbase)
{
  m_urlBase = urlbase;
  getLanIp();
  getMacAddress();
}

bool RTJamNationApi::status()
{
  json args;
  args["lanIp"] = m_lanIp;
  args["macAddress"] = m_macAddress;
  return get(m_urlBase + "status", args);
}

bool RTJamNationApi::jamUnitDeviceRegister()
{
  json args;
  args["lanIp"] = m_lanIp;
  args["macAddress"] = m_macAddress;
  return post(m_urlBase + "jamUnit", args);
}

bool RTJamNationApi::jamUnitPing(string token)
{
  json args;
  args["token"] = token;
  args["lanIp"] = m_lanIp;
  args["macAddress"] = m_macAddress;
  args["gitHash"] = GIT_HASH;
  return put(m_urlBase + "jamUnit/ping", args);
}

bool RTJamNationApi::broadcastUnitDeviceRegister()
{
  json args;
  args["lanIp"] = m_lanIp;
  args["macAddress"] = m_macAddress;
  return post(m_urlBase + "broadcastUnit", args);
}

bool RTJamNationApi::broadcastUnitPing(string token)
{
  json args;
  args["token"] = token;
  args["lanIp"] = m_lanIp;
  args["macAddress"] = m_macAddress;
  args["gitHash"] = GIT_HASH;
  return put(m_urlBase + "broadcastUnit/ping", args);
}

bool RTJamNationApi::activateRoom(string token, string name, int port)
{
  json args;
  args["token"] = token;
  args["name"] = name;
  args["port"] = port;
  args["lanIp"] = m_lanIp;
  args["macAddress"] = m_macAddress;
  return post(m_urlBase + "room", args);
}

bool RTJamNationApi::playerList(string roomToken)
{
  json args;
  args["token"] = roomToken;
  return put(m_urlBase + "room/playerList", args);
}

bool RTJamNationApi::get(string url, json body)
{
  string data;
  restincurl::Client client;
  CURLcode curlCode;
  long httpResponseCode;

  client.Build()->Get(url).Option(CURLOPT_VERBOSE, 0L).AcceptJson().StoreData(data).WithJson().SendData<string>(body.dump()).Header("X-Client", "restincurl").WithCompletion([&](const Result &result)
                                                                                                                                                                             {
                                                                                                                                                                               curlCode = result.curl_code;
                                                                                                                                                                               httpResponseCode = result.http_response_code;
                                                                                                                                                                             })
      .ExecuteSynchronous();
  if (curlCode == CURLE_OK)
  {
    try
    {
      m_httpResponseCode = httpResponseCode;
      m_resultBody = json::parse(data);
    }
    catch (...)
    {
      clog << "Failed to parse server json" << endl;
    }
  }
  return (curlCode == CURLE_OK);
}
bool RTJamNationApi::put(string url, json body)
{
  string data;
  restincurl::Client client;
  CURLcode curlCode;
  long httpResponseCode;

  client.Build()->Put(url).Option(CURLOPT_VERBOSE, 0L).AcceptJson().StoreData(data).WithJson().SendData<string>(body.dump()).Header("X-Client", "restincurl").WithCompletion([&](const Result &result)
                                                                                                                                                                             {
                                                                                                                                                                               curlCode = result.curl_code;
                                                                                                                                                                               httpResponseCode = result.http_response_code;
                                                                                                                                                                             })
      .ExecuteSynchronous();
  if (curlCode == CURLE_OK)
  {
    try
    {
      m_httpResponseCode = httpResponseCode;
      m_resultBody = json::parse(data);
    }
    catch (...)
    {
      clog << "Failed to parse server json" << endl;
    }
  }
  return (curlCode == CURLE_OK);
}
bool RTJamNationApi::post(string url, json body)
{
  string data;
  restincurl::Client client;
  CURLcode curlCode;
  long httpResponseCode;

  client.Build()->Post(url).Option(CURLOPT_VERBOSE, 0L).AcceptJson().StoreData(data).WithJson().SendData<string>(body.dump()).Header("X-Client", "restincurl").WithCompletion([&](const Result &result)
                                                                                                                                                                              {
                                                                                                                                                                                curlCode = result.curl_code;
                                                                                                                                                                                httpResponseCode = result.http_response_code;
                                                                                                                                                                              })
      .ExecuteSynchronous();
  if (curlCode == CURLE_OK)
  {
    try
    {
      m_httpResponseCode = httpResponseCode;
      m_resultBody = json::parse(data);
    }
    catch (...)
    {
      clog << "Failed to parse server json" << endl;
    }
  }
  return (curlCode == CURLE_OK);
}

void RTJamNationApi::getLanIp()
{
  ifaddrs *addrs;
  getifaddrs(&addrs);
  ifaddrs *tmp = addrs;

  while (tmp)
  {
    if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET) // && strcmp(tmp->ifa_name, "eth0") == 0)
    {
      printf("Found interface %s\n", tmp->ifa_name);
      struct sockaddr_in *pAddr = (struct sockaddr_in *)tmp->ifa_addr;
      m_lanIp = inet_ntoa(pAddr->sin_addr);
    }

    tmp = tmp->ifa_next;
  }

  freeifaddrs(addrs);
}

void RTJamNationApi::getMacAddress()
{
  int sock = socket(PF_INET, SOCK_DGRAM, 0);
  char mac[32];
  struct ifreq ifr;
  ifr.ifr_addr.sa_family = AF_INET;
  strncpy(ifr.ifr_name, "eth0", IFNAMSIZ - 1);
  ioctl(sock, SIOCGIFHWADDR, &ifr);
  sprintf(mac, "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
          (unsigned char)ifr.ifr_hwaddr.sa_data[0],
          (unsigned char)ifr.ifr_hwaddr.sa_data[1],
          (unsigned char)ifr.ifr_hwaddr.sa_data[2],
          (unsigned char)ifr.ifr_hwaddr.sa_data[3],
          (unsigned char)ifr.ifr_hwaddr.sa_data[4],
          (unsigned char)ifr.ifr_hwaddr.sa_data[5]);
  m_macAddress = mac;
  close(sock);
}

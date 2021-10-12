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
  checkLinkStatus();
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
  args["canTalkOnWebsocket"] = true;
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

#define ERROR(fmt, ...)       \
  do                          \
  {                           \
    printf(fmt, __VA_ARGS__); \
    return -1;                \
  } while (0)

bool RTJamNationApi::checkLinkStatus()
{
  bool rval = false;
  int rv;
  char mac[32];

  // Create a socket so we can do ioctls on it
  int socId = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
  if (socId < 0)
    ERROR("Socket failed. Errno = %d\n", errno);

  struct ifreq if_req;
  (void)strncpy(if_req.ifr_name, "eth0", sizeof(if_req.ifr_name));

  // get the interface up flags
  if (ioctl(socId, SIOCGIFFLAGS, &if_req) == -1)
  {
    ERROR("Ioctl failed. Errno = %d\n", errno);
    close(socId);
    return false;
  }

  // rval is up if the ethernet is up and running (plugged in)
  rval = (if_req.ifr_flags & IFF_UP) && (if_req.ifr_flags & IFF_RUNNING);

  if (rval)
  {
    // get the Hardware address
    if (ioctl(socId, SIOCGIFHWADDR, &if_req) == -1)
    {
      ERROR("Ioctl failed. Errno = %d\n", errno);
      close(socId);
      return false;
    }
    sprintf(mac, "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
            (unsigned char)if_req.ifr_hwaddr.sa_data[0],
            (unsigned char)if_req.ifr_hwaddr.sa_data[1],
            (unsigned char)if_req.ifr_hwaddr.sa_data[2],
            (unsigned char)if_req.ifr_hwaddr.sa_data[3],
            (unsigned char)if_req.ifr_hwaddr.sa_data[4],
            (unsigned char)if_req.ifr_hwaddr.sa_data[5]);
    m_macAddress = mac;

    // Get the ip address
    if (ioctl(socId, SIOCGIFADDR, &if_req) == -1)
    {
      ERROR("Ioctl failed. Errno = %d\n", errno);
      close(socId);
      return false;
    }
    struct sockaddr_in *addr = (struct sockaddr_in *)&(if_req.ifr_addr);
    m_lanIp = inet_ntoa(addr->sin_addr);

    // The link might be up and have a hardware address, but we might not have a DHCP address yet
    if (m_lanIp.size() == 0)
    {
      ERROR("No DHCP address yet: %ld\n", m_lanIp.size());
      close(socId);
      return false;
    }
  }

  close(socId);
  return rval;
}
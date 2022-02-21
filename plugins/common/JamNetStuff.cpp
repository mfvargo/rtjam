/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 */

#include "JamNetStuff.hpp"
#include <cmath>
#include <string.h>
#include <stdlib.h>
#include <netinet/ip.h>
#include <time.h>
#include <errno.h>

namespace JamNetStuff
{
    JamPacket::JamPacket()
    {
        // Some constructor stuff goes here...
        srand(getMicroTime());
        char *client_env = getenv("RTJAM_CLIENT");
        if (client_env)
        {
            clientId = atoi(client_env);
        }
        else
        {
            clientId = rand() % 32768;
        }
        fprintf(stderr, "Client %d\n", clientId);
        channelMap.setMyId(clientId);
        bufferSize = 0;
        sequenceNo = 0;
        sampleRate = FORTY_EIGHT_K; // Default.  Need to get value from plugin...
        isClient = true;
        numSamples = 0;
    }

    void JamPacket::dumpPacket(const char *intro)
    {
        printf("%s cid: %d pcid: %d c: %d, r: %d, Sub: %d, Seq: %d, Beat: %d, Size: %d\n",
               intro,
               clientId,
               jamMessage.ClientId,
               jamMessage.Channel,
               jamMessage.SampleRate,
               jamMessage.NumSubChannels,
               jamMessage.SequenceNumber,
               jamMessage.Beat,
               bufferSize);
    }

    void JamPacket::clearChannelMap()
    {
        // clientId = rand() % 32768;
        channelMap.clear();
        channelMap.setMyId(clientId);
    }

    void JamPacket::encodeAudio(const float **inputs, int frames)
    {
        numSamples = frames;
        // copy two channels into the jamMessage packet
        unsigned char *bufPtr = jamMessage.buffer;
        encodeJamBuffer(bufPtr, inputs[0], frames);
        bufPtr += frames * sizeof(uint16_t);
        encodeJamBuffer(bufPtr, inputs[1], frames);
        bufferSize = frames * jamMessage.NumSubChannels * sizeof(uint16_t);
    }

    void JamPacket::encodeHeader()
    {
        jamMessage.SampleRate = sampleRate;
        jamMessage.NumSubChannels = 2;
        jamMessage.TimeStamp = htobe64(getMicroTime());
        jamMessage.ServerTime = htobe64(jamMessage.ServerTime);
        // Note that these next two fields are just pass through if you are not a client
        jamMessage.SequenceNumber = htonl(isClient ? sequenceNo++ : jamMessage.SequenceNumber);
        jamMessage.ClientId = htonl(isClient ? clientId : jamMessage.ClientId);
    }

    void JamPacket::encodeJamBuffer(unsigned char *jamBuffer, const float *buffer, int frames)
    {
        uint16_t *dst = (uint16_t *)jamBuffer;
        for (int i = 0; i < frames; i++)
        {
            float sample = buffer[i];
            // prevent clip
            if (sample > 1.0)
            {
                sample = 1.0;
            }
            if (sample < -1.0)
            {
                sample = -1.0;
            }
            // offset and mash into unsigned int
            dst[i] = htons((sample + 1.0) * 32766);
        }
    }

    int JamPacket::decodeHeader(int nBytes)
    {
        // decode header
        jamMessage.TimeStamp = be64toh(jamMessage.TimeStamp);
        jamMessage.ServerTime = be64toh(jamMessage.ServerTime);
        jamMessage.SequenceNumber = ntohl(jamMessage.SequenceNumber);
        jamMessage.ClientId = ntohl(jamMessage.ClientId);
        if (!validPacket(nBytes))
        {
            // Don't try to decode invalid packets.
            printf("bad packet\n");
            return 0;
        }
        bufferSize = nBytes - (sizeof(struct JamMessage) - JAM_BUF_SIZE);
        numSamples = bufferSize / (sizeof(uint16_t) * jamMessage.NumSubChannels);
        return numSamples;
    }

    int JamPacket::decodeJamBuffer(float **outputs)
    {
        // Now decode the samples back into floats
        uint16_t *src = (uint16_t *)jamMessage.buffer;
        for (int i = 0; i < numSamples; i++)
        {
            outputs[0][i] = (1.0 / 32768.0 * ntohs(*src++)) - 1.0;
        }
        for (int i = 0; i < numSamples; i++)
        {
            outputs[1][i] = (1.0 / 32768.0 * ntohs(*src++)) - 1.0;
        }
        return numSamples;
    }

    void JamPacket::setServerChannel(int channel)
    {
        jamMessage.Channel = channel;
        clientId = jamMessage.ClientId;
    }

    bool JamPacket::validPacket(int nBytes)
    {
        if (sampleRate != jamMessage.SampleRate)
        {
            // Sample Rate mismatch!
            printf("bad sample rate\n");
            return false;
        }
        if (jamMessage.NumSubChannels != 2)
        {
            printf("bad sub channel\n");
            // hard code to 2 sub channels always.
            return false;
        }
        if ((int)(sizeof(struct JamMessage) - JAM_BUF_SIZE) > nBytes)
        {
            // Packet is not big enough to even have the header
            printf("bad size\n");
            return false;
        }
        return true;
    }

    bool JamPacket::isMe()
    {
        return (jamMessage.ClientId == clientId);
    }

    int JamPacket::getChannel()
    {
        return channelMap.getChannel(jamMessage.ClientId);
    }

    int JamPacket::getSize(bool headerOnly)
    {
        return (sizeof(struct JamMessage) - JAM_BUF_SIZE + (headerOnly ? 0 : bufferSize));
    }

    uint32_t JamPacket::getSequenceNo()
    {
        return jamMessage.SequenceNumber;
    }

    void *JamPacket::getPacket()
    {
        return &jamMessage;
    }

    PlayerList::PlayerList()
    {
        m_roomSize = MAX_JAMMERS;
        m_allowedClientIds.clear();
    }

    void PlayerList::setAllowedClientIds(std::vector<unsigned> &ids)
    {
        m_allowedClientIds = ids;
    }

    bool PlayerList::isAllowed(unsigned /* id*/)
    {
        // TODO implement allowed list search and max room size
        return true;
    }

    std::map<unsigned, float> PlayerList::getLatency()
    {
        std::map<unsigned, float> rval;
        for (Player p : m_players)
        {
            rval.insert(std::pair<unsigned, float>(p.clientId, p.networkTime.mean / 1000));
        }
        return rval;
    }

    void PlayerList::dump(std::string msg)
    {
        std::cout << msg << " clients: [";
        for (Player p : m_players)
        {
            std::cout << p.clientId << "-" << p.networkTime.mean << ", ";
        }
        std::cout << "]" << std::endl;
    }

    void PlayerList::Prune()
    {
        // See if any of the clients have disappeared
        uint64_t now = getMicroTime();
        for (auto it = m_players.begin(); it != m_players.end();)
        {
            if ((now - (*it).KeepAlive) > SERVER_EXPIRATION_IN_MICROSECONDS)
            {
                m_players.erase(it);
                dump("prune");
            }
            else
            {
                ++it;
            }
        }
    }

    int PlayerList::updateChannel(unsigned clientId, sockaddr_in *addr, uint64_t pingTime)
    {
        uint64_t now = getMicroTime();
        int i = 0;
        // Do we know about this guy?
        for (auto it = m_players.begin(); it != m_players.end(); ++it)
        {
            if ((*it).clientId == clientId)
            {
                if ((*it).bPinging && pingTime != 0)
                {
                    // filter out garbage data
                    if (now - pingTime < 200000)
                    {
                        (*it).networkTime.addSample(now - pingTime);
                    }
                    (*it).bPinging = false;
                }
                (*it).KeepAlive = now;
                return i;
            }
            i++;
        }
        // If we got here, we don't know him.  Do we have space to add him?
        if (i <= m_roomSize)
        {
            Player p;
            p.clientId = clientId;
            p.KeepAlive = now;
            p.Address = *addr;
            p.bPinging = false;
            p.networkTime.windowSize = 20;
            m_players.push_back(p);
            dump("add");
        }
        // TODO implement channel map find and update
        return -1;
    }

    int PlayerList::numPlayers()
    {
        return m_players.size();
    }

    Player PlayerList::get(int i)
    {
        return m_players.at(i);
    }

    void PlayerList::startPing()
    {
        for (unsigned int i = 0; i < m_players.size(); i++)
        {
            m_players.at(i).bPinging = true;
        }
    }

    JamSocket::JamSocket()
    {
        isActivated = false;
        beatCount = 0;
        lastPingTime = 0;
        jamSocket = socket(PF_INET, SOCK_DGRAM, 0);
        printf("socket is %d\n", jamSocket);
    }

    void JamSocket::initClient(const char *servername, int port, uint32_t clientId)
    {
        // Try to set the Type of Service to Voice (for whatever that is worth)
        int tos_local = IPTOS_LOWDELAY;
        if (setsockopt(jamSocket, IPPROTO_IP, IP_TOS, &tos_local, sizeof(tos_local)))
        {
            printf("set TOS failed. %d\n", h_errno);
        }
        // Set socket to non-blocking
        fcntl(jamSocket, F_SETFL, fcntl(jamSocket, F_GETFL) | O_NONBLOCK);
        // Clear out the channelMap on the socket
        m_packet.setClientId(clientId);
        // Set that bad boy up.
        char ip[100];
        struct hostent *he;
        struct in_addr **addr_list;
        if ((he = gethostbyname(servername)) == NULL)
        {
            // get the host info
            herror("gethostbyname");
            return;
        }
        addr_list = (struct in_addr **)he->h_addr_list;
        strcpy(ip, inet_ntoa(*addr_list[0]));
        printf("server IP is %s\n", ip);

        /*Configure settings in address struct*/
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        serverAddr.sin_addr.s_addr = inet_addr(ip);
        memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
        addr_size = sizeof(serverAddr);

        // Set the packet to client mode
        m_packet.setIsClient(true);
        m_packet.clearChannelMap();
    }

    void JamSocket::initServer(short port)
    {
        m_port = port;
        // Try to set the Type of Service to Voice (for whatever that is worth)
        int tos_local = IPTOS_LOWDELAY;
        if (setsockopt(jamSocket, IPPROTO_IP, IP_TOS, &tos_local, sizeof(tos_local)))
        {
            printf("set TOS failed. %d\n", h_errno);
        }
        // Set the socket to timeout every 1 seconds if nobody is around
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        if (setsockopt(jamSocket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv))
        {
            printf("set SO_RCVTIMEO failed\n");
        }
        // Set the packet to server mode
        m_packet.setIsClient(false);
        // Set the default tempo
        m_tempo = 120;
        switch (port)
        {
        case 7891:
            m_tempo = 85;
            break;
        case 7892:
            m_tempo = 95;
            break;
        case 7893:
            m_tempo = 105;
            break;
        }
        /*Configure settings in address struct*/
        memset(&serverAddr, 0, sizeof(struct sockaddr_in));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        // memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
        /*Bind socket with address struct*/
        if (bind(jamSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)))
        {
            printf("port address bind failed. %d\n", h_errno);
        }
        m_tempoStart = lastPingTime = getMicroTime();
        m_pinging = true;
        // Open the save file
    }

    string JamSocket::recordRoom()
    {
        string filename = std::to_string(m_port) + ".raw";
        return m_capture.writeOpen(filename.c_str());
    }

    string JamSocket::stopAudio()
    {
        return m_capture.close();
    }

    string JamSocket::playAudio()
    {
        string filename = std::to_string(m_port) + ".raw";
        return m_capture.readOpen(filename.c_str());
    }

    int JamSocket::sendPacket(const float **buffer, int frames)
    {
        if (!isActivated)
        {
            return 0;
        }
        // Send a packet to the server!
        m_packet.encodeAudio(buffer, frames);
        m_packet.encodeHeader();
        return sendData(&serverAddr, false, &m_packet);
    }

    // Client side function to read incoming packets and shove them into the mixer.
    // Socket api is non-blocking so it will read until there is no data and then return
    void JamSocket::readPackets(JamMixer *jamMixer)
    {
        // Stale out any channels
        m_packet.checkChannelTimeouts();
        if (!isActivated)
        {
            return;
        }

        int nBytes;

        do
        {
            nBytes = readData();
            // check if we got any data and if this is from the current server (ignore residual packets from previous room)
            if (nBytes > 0 && senderAddr.sin_port == serverAddr.sin_port && senderAddr.sin_addr.s_addr == serverAddr.sin_addr.s_addr)
            {
                if (jamMixer)
                {
                    jamMixer->addData(&m_packet);
                }
            }
        } while (isActivated && nBytes > 0);
    }

    // Broadcast server function to read the socket (blocking) and
    // send that data to people in the room (as per the PlayerList object)
    void JamSocket::sendDataToRoomMembers(JamMixer *jamMixer)
    {
        int nBytes = readData(); // server socket is blocking so it will wait here
        // If there was an error, just bail out here.
        m_playerList.Prune();
        if (nBytes < 0)
            return;
        uint32_t clientId = m_packet.getClientIdFromPacket();
        // Check if that client is an allowed person in the room
        if (!m_playerList.isAllowed(clientId))
            return;
        // Get server assigned channel
        int serverChannel = m_playerList.updateChannel(clientId, &senderAddr, m_packet.getServerTime());
        // Check for full room,  a negative serverChannel means there is no room for them
        if (serverChannel < 0)
            return;

        // If we get here, we have a valid player and need to broadcast them

        // Are we sending a live mix to the webstream?
        if (jamMixer != NULL)
        {
            jamMixer->addData(&m_packet);
        }

        m_packet.setServerChannel(serverChannel);
        // Time calculations for beat tempo and ping timing
        uint64_t nowTime = getMicroTime();
        char beatCount = ((nowTime - m_tempoStart) / (60 * 1000000 / m_tempo)) % 4;
        m_packet.setBeatCount(beatCount);
        uint64_t deltaT = nowTime - lastPingTime;
        // We send pings in 10 msec windows every 100msec
        if (deltaT < 10000)
        {
            if (!m_pinging)
            {
                // rising edge of ping window
                m_playerList.startPing();
                // m_playerList.dump("click");
            }
            m_pinging = true;
        }
        if (deltaT > 10000)
        {
            m_pinging = false;
        }
        if (deltaT > 100000)
        {
            lastPingTime = nowTime;
        }
        m_packet.setServerTime(m_pinging ? nowTime : 0);
        m_packet.encodeHeader();

        // Are we saving the session to a file?
        // This code is here so that the file always contains network encoded packets
        // do the file save stuff.
        m_capture.writePacket(&m_packet);

        // Are we playing back some recorded audio?
        JamPacket *pPlaybackPacket = m_capture.getPlayBackMix();

        for (int i = 0; i < m_playerList.numPlayers(); i++)
        {
            Player player = m_playerList.get(i);
            // Get the addresses and send (note: only send the header (for ping) to packet sender)
            sendData(&player.Address, player.clientId == clientId, &m_packet);
            if (pPlaybackPacket != NULL)
            {
                sendData(&player.Address, false, pPlaybackPacket);
            }
        }
    }

    int JamSocket::readData()
    {
        addr_size = sizeof(serverAddr);
        int nBytes = recvfrom(
            jamSocket,
            m_packet.getPacket(),
            sizeof(struct JamMessage),
            0,
            (struct sockaddr *)&senderAddr,
            &addr_size);
        if (nBytes < 0 && errno != 11)
        {
            fprintf(stderr, "recvfrom: %s (%d)\n", strerror(errno), errno);
        }
        if (nBytes > 0)
        {
            m_packet.decodeHeader(nBytes);
        }
        return nBytes;
    }
    int JamSocket::sendData(struct sockaddr_in *to_addr, bool headerOnly, JamPacket *pPacket)
    {
        int rval = sendto(
            jamSocket,
            pPacket->getPacket(),
            pPacket->getSize(headerOnly),
            0,
            (struct sockaddr *)to_addr,
            sizeof(struct sockaddr));
        return rval;
    }
}

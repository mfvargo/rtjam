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
    JamPacket::JamPacket() {
        // Some constructor stuff goes here...
        srand(getMicroTime());
        char* client_env = getenv("RTJAM_CLIENT");
        if (client_env) {
            clientId = atoi(client_env);
        } else {
            clientId = rand() % 32768;
        }
        fprintf(stderr, "Client %d\n", clientId);
        channelMap.setMyId(clientId);
        bufferSize = 0;
        sequenceNo = 0;
        sampleRate = FORTY_EIGHT_K;  // Default.  Need to get value from plugin...
        isClient = true;
        numSamples = 0;
    }

    void JamPacket::dumpPacket(const char* intro) {
        printf("%s cid: %d pcid: %d c: %d, r: %d, Sub: %d, Seq: %d, Beat: %d, Size: %d\n",
            intro,
            clientId,
            jamMessage.ClientId,
            jamMessage.Channel,
            jamMessage.SampleRate,
            jamMessage.NumSubChannels,
            jamMessage.SequenceNumber,
            jamMessage.Beat,
            bufferSize
        );
    }

    void JamPacket::clearChannelMap() {
        // clientId = rand() % 32768;
        channelMap.clear();
        channelMap.setMyId(clientId);
    }

    void JamPacket::encodeAudio(const float** inputs, int frames) {
        numSamples = frames;
        // copy two channels into the jamMessage packet
        unsigned char* bufPtr = jamMessage.buffer;
        encodeJamBuffer(bufPtr, inputs[0], frames);
        bufPtr += frames * sizeof(uint16_t);
        encodeJamBuffer(bufPtr, inputs[1], frames);
        bufferSize = frames * jamMessage.NumSubChannels * sizeof(uint16_t);
    }

    void JamPacket::encodeHeader() {
        jamMessage.SampleRate = sampleRate;
        jamMessage.NumSubChannels = 2;
        jamMessage.TimeStamp = htobe64(getMicroTime());
        jamMessage.ServerTime = htobe64(jamMessage.ServerTime);
        // Note that these next two fields are just pass through if you are not a client
        jamMessage.SequenceNumber = htonl(isClient ? sequenceNo++ : jamMessage.SequenceNumber);
        jamMessage.ClientId = htonl(isClient ? clientId : jamMessage.ClientId);
    }

    void JamPacket::encodeJamBuffer(unsigned char* jamBuffer, const float* buffer, int frames) {
        uint16_t* dst = (uint16_t*) jamBuffer;
        for (int i=0; i<frames; i++) {
            float sample = buffer[i];
            // prevent clip
            if (sample > 1.0) {
            sample = 1.0;
            }
            if (sample < -1.0) {
            sample = -1.0;
            }
            // offset and mash into unsigned int
            dst[i] = htons((sample + 1.0) * 32766);
        }
    }

    int JamPacket::decodeHeader(int nBytes) {
        // decode header
        jamMessage.TimeStamp = be64toh(jamMessage.TimeStamp);
        jamMessage.ServerTime = be64toh(jamMessage.ServerTime);
        jamMessage.SequenceNumber = ntohl(jamMessage.SequenceNumber);
        jamMessage.ClientId = ntohl(jamMessage.ClientId);
        if (!validPacket(nBytes)) {
            // Don't try to decode invalid packets.
            printf("bad packet\n");
            return 0;
        }
        bufferSize = nBytes - (sizeof(struct JamMessage) - JAM_BUF_SIZE);
        numSamples = bufferSize / (sizeof(uint16_t) * jamMessage.NumSubChannels);
        return numSamples;
    }

    int JamPacket::decodeJamBuffer(float** outputs) {
        // Now decode the samples back into floats
        uint16_t* src = (uint16_t*)jamMessage.buffer;
        for (int i=0; i<numSamples; i++) {
            outputs[0][i] = (1.0/32768.0 * ntohs(*src++))- 1.0;
        }
        for (int i=0; i<numSamples; i++) {
            outputs[1][i] = (1.0/32768.0 * ntohs(*src++))- 1.0;
        }
        return numSamples;
    }

    void JamPacket::setServerChannel(int channel) {
        jamMessage.Channel = channel;
        jamMessage.ServerTime = getMicroTime();
        clientId = jamMessage.ClientId;
    }

    bool JamPacket::validPacket(int nBytes) {
        if (sampleRate != jamMessage.SampleRate) {
            // Sample Rate mismatch!
            printf("bad sample rate\n");
            return false;
        }
        if (jamMessage.NumSubChannels != 2) {
            printf("bad sub channel\n");
            // hard code to 2 sub channels always.
            return false;
        }
        if ( (int) (sizeof(struct JamMessage) - JAM_BUF_SIZE) > nBytes) {
            // Packet is not big enough to even have the header
            printf("bad size\n");
            return false;
        }
        return true;
    }

    bool JamPacket::isMe() {
        return (jamMessage.ClientId == clientId);
    }

    int JamPacket::getChannel() {
        // channelMap.dumpOut();
        return channelMap.getChannel(jamMessage.ClientId);
    }

    int JamPacket::getSize() {
        return (sizeof(struct JamMessage) - JAM_BUF_SIZE + bufferSize);
    }

    uint32_t JamPacket::getSequenceNo() {
        return jamMessage.SequenceNumber;
    }

    void* JamPacket::getPacket() {
        return &jamMessage;
    }

    JamSocket::JamSocket() {
        isActivated = false;
        beatCount = 0;
        lastClickTime = 0;
        jamSocket = socket(PF_INET, SOCK_DGRAM, 0);
        printf("socket is %d\n", jamSocket);
    }

    void JamSocket::initClient(const char* servername, int port, uint32_t clientId) {
        // Clear the channel map
        channelMap.clear();
        packet.clearChannelMap();
        // Try to set the Type of Service to Voice (for whatever that is worth)
        int tos_local = IPTOS_LOWDELAY;
        if (setsockopt(jamSocket, IPPROTO_IP, IP_TOS,  &tos_local, sizeof(tos_local))) {
            printf("set TOS failed. %d\n", h_errno);
        }
        // Set socket to non-blocking
        fcntl(jamSocket, F_SETFL, fcntl(jamSocket, F_GETFL) | O_NONBLOCK);
        // Clear out the channelMap on the socket
        packet.setClientId(clientId);
        // Set that bad boy up.
        char ip[100];
        struct hostent *he;
        struct in_addr **addr_list;
        if ( (he = gethostbyname( servername ) ) == NULL) 
        {
            // get the host info
            herror("gethostbyname");
            return;
        }
        addr_list = (struct in_addr **) he->h_addr_list;
        strcpy(ip , inet_ntoa(*addr_list[0]) );
        printf("server IP is %s\n", ip);

        /*Configure settings in address struct*/
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        serverAddr.sin_addr.s_addr = inet_addr(ip);
        memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
        addr_size = sizeof(serverAddr);

        // Set the packet to client mode
        packet.setIsClient(true);
    }

    void JamSocket::initServer(short port) {
        // Try to set the Type of Service to Voice (for whatever that is worth)
        int tos_local = IPTOS_LOWDELAY;
        if (setsockopt(jamSocket, IPPROTO_IP, IP_TOS,  &tos_local, sizeof(tos_local))) {
            printf("set TOS failed. %d\n", h_errno);
        }
        // Set the socket to timeout every 1 seconds if nobody is around
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        if (setsockopt(jamSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv)) {
            printf("set SO_RCVTIMEO failed\n");
        }
        // Set the packet to server mode
        packet.setIsClient(false);
        // Set the default tempo
        tempo = 120;
       /*Configure settings in address struct*/
        memset(&serverAddr, 0, sizeof(struct sockaddr_in));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        // memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
        /*Bind socket with address struct*/
        if (bind(jamSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr))) {
            printf("port address bind failed. %d\n", h_errno);
        }
    }

    int JamSocket::sendPacket(const float** buffer, int frames) {
        if (!isActivated) {
            return 0;
        }
        // Send a packet to the server!
        packet.encodeAudio(buffer, frames);
        packet.encodeHeader();
        return sendData(&serverAddr);
    }

    int JamSocket::readPackets(JamMixer* jamMixer) {
        if (!isActivated) {
            return 0;
        }

        int rval = 0;
        int nBytes;

        do {
            nBytes = readData();
            if (nBytes > 0 && senderAddr.sin_port == serverAddr.sin_port) {
                packet.dumpPacket("mikey: ");
                jamMixer->addData(&packet);
            }
        } while( isActivated && nBytes > 0);
        return rval;
    }

    int JamSocket::readAndBroadcast(JamMixer* jamMixer) {
        // This is the read and broadcast for the server
        int nBytes = readData();
        time_t now = time(NULL);
        channelMap.pruneStaleChannels(now, 0);
        if (nBytes > 0) {
            if (packet.getServerChannel() == SET_TEMPO_CHAN) {
                // We have a tempo change
                tempo = packet.getBeatCount();
            }
            // Put the packet into the mixer
            if (jamMixer != NULL) {
                jamMixer->addData(&packet);
            }
            uint32_t clientId = packet.getClientIdFromPacket();
            packet.setServerChannel(channelMap.getChannelBySenderIp(clientId, &senderAddr));
            uint64_t deltaT = packet.getServerTime() - lastClickTime;
            if (deltaT > (60 * 1e6 / tempo )) {  // 120BPM
                // We have passed a click boundary
                lastClickTime = packet.getServerTime();
                beatCount++;
            }
            packet.setBeatCount(beatCount%4);
            // We encode the header just once.
            packet.encodeHeader();
            // printf("nBytes: %d from %lu\n", nBytes, from_addr);
            // channelMap.dumpOut();
            for (int i=0; i<MAX_JAMMERS; i++) {
                // Don't send back an echo to the sender
                sockaddr_in addr;
                uint32_t to_client = channelMap.getClientId(i);
                if (to_client != EMPTY_SLOT && to_client != clientId) {
                // if (to_addr != EMPTY_SLOT) {
                    channelMap.getClientAddr(i, &addr);
                    sendData(&addr);
                }
            }
        }
        // Get IP address (as unsigned long)
        return nBytes;
    }
    int JamSocket::readData() {
        addr_size = sizeof(serverAddr);
        int nBytes = recvfrom(
            jamSocket,
            packet.getPacket(),
            sizeof(struct JamMessage),
            0,
            (struct sockaddr *) &senderAddr, 
            &addr_size
        );
        if (nBytes < 0 && errno != 11) {
            fprintf(stderr, "recvfrom: %s (%d)\n", strerror(errno), errno);
        }
        if (nBytes > 0) {
            packet.decodeHeader(nBytes);
        }
        return nBytes;
    }
    int JamSocket::sendData(struct sockaddr_in* to_addr) {
        int rval = sendto(
                jamSocket,
                packet.getPacket(),
                packet.getSize(),
                0,
                (struct sockaddr *)to_addr,
                sizeof(struct sockaddr)
            );
        return rval;
    }
}

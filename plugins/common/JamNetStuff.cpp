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

namespace JamNetStuff
{
    JamPacket::JamPacket() {
        // Some constructor stuff goes here...
        srand(getMicroTime());
        clientId = rand() % 32768;
        channelMap.setMyId(clientId);
        bufferSize = 0;
        sequenceNo = 0;
        sampleRate = FORTY_EIGHT_K;  // Default.  Need to get value from plugin...

    }

    void JamPacket::dumpPacket(const char* intro) {
        printf("%s cid: %d pcid: %d c: %d, r: %d, Sub: %d, Seq: %d, Size: %d\n",
            intro,
            clientId,
            jamMessage.ClientId,
            jamMessage.Channel,
            jamMessage.SampleRate,
            jamMessage.NumSubChannels,
            jamMessage.SequenceNumber,
            bufferSize
        );
    }

    void JamPacket::encode(const float** inputs, int frames) {
        // copy two channels into the jamMessage packet
        jamMessage.SampleRate = sampleRate;
        jamMessage.NumSubChannels = 2;
        jamMessage.TimeStamp = htobe64(getMicroTime());
        jamMessage.SequenceNumber = htonl(sequenceNo++);
        jamMessage.ClientId = htonl(clientId);
        unsigned char* bufPtr = jamMessage.buffer;
        encodeJamBuffer(bufPtr, inputs[0], frames);
        bufPtr += frames * sizeof(uint16_t);
        encodeJamBuffer(bufPtr, inputs[1], frames);
        bufferSize = frames * jamMessage.NumSubChannels * sizeof(uint16_t);
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
        jamMessage.ServerTime = be64toh(jamMessage.ServerTime);
        jamMessage.TimeStamp = be64toh(jamMessage.TimeStamp);
        jamMessage.ClientId = ntohl(jamMessage.ClientId);
        jamMessage.SequenceNumber = ntohl(jamMessage.SequenceNumber);
        if (!validPacket(nBytes)) {
            // Don't try to decode invalid packets.
            printf("bad packet\n");
            return 0;
        }
        return (nBytes - (sizeof(struct JamMessage) - JAM_BUF_SIZE)) /
                            (sizeof(uint16_t) * jamMessage.NumSubChannels);
    }
    int JamPacket::decode(float** outputs, int nBytes) {
        int samples = decodeHeader(nBytes);
        if (samples == 0) {
            return 0;
        }
        // Now decode the samples back into floats
        uint16_t* src = (uint16_t*)jamMessage.buffer;
        for (int i=0; i<samples; i++) {
            outputs[0][i] = (1.0/32768.0 * ntohs(*src++))- 1.0;
        }
        for (int i=0; i<samples; i++) {
            outputs[1][i] = (1.0/32768.0 * ntohs(*src++))- 1.0;
        }
        bufferSize = samples * jamMessage.NumSubChannels * sizeof(uint16_t);
        return samples;
    }

    bool JamPacket::validPacket(int nBytes) {
        if (sampleRate != jamMessage.SampleRate) {
            // Sample Rate mismatch!
            printf("bad sample rate\n");
            return false;
        }
        if (jamMessage.Channel >= MAX_JAMMERS) {
            printf("bad channel\n");
            // Illegal channel
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
        jamSocket = socket(PF_INET, SOCK_DGRAM, 0);
        // Set socket to non-blocking
        fcntl(jamSocket, F_SETFL, fcntl(jamSocket, F_GETFL) | O_NONBLOCK);
        printf("socket is %d\n", jamSocket);
        // Try to set the Type of Service to Voice (for whatever that is worth)
        int tos_local = IPTOS_LOWDELAY;
        if (setsockopt(jamSocket, IPPROTO_IP, IP_TOS,  &tos_local, sizeof(tos_local))) {
            printf("set TOS failed. %d\n", h_errno);
        }
    }

    void JamSocket::initClient(const char* servername, int port) {
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
    }

    void JamSocket::initServer() {
        jamSocket = socket(PF_INET, SOCK_DGRAM, 0);
       /*Configure settings in address struct*/
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(7891);
        memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  
        /*Bind socket with address struct*/
        bind(jamSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
        // Try to set the Type of Service to Voice (for whatever that is worth)
        int tos_local = IPTOS_LOWDELAY;
        if (setsockopt(jamSocket, IPPROTO_IP, IP_TOS,  &tos_local, sizeof(tos_local))) {
            printf("set TOS failed. %d\n", h_errno);
        }
        // Set the socket to timeout every 5 seconds if nobody is around
        struct timeval tv;
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        if (setsockopt(jamSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv)) {
            printf("set SO_RCVTIMEO failed\n");
        }
    }

    int JamSocket::sendPacket(const float** buffer, int frames) {
        if (!isActivated) {
            return 0;
        }
        // Send a packet to the server!
        packet.encode(buffer, frames);
        // xmit_packet.dumpPacket("Xmit: ");
        int rval = sendto(
                jamSocket,
                packet.getPacket(),
                packet.getSize(),
                0,
                (struct sockaddr *)&serverAddr,addr_size
            );
        return rval;
    }

    int JamSocket::readPackets(JamMixer* jamMixer) {
        if (!isActivated) {
            return 0;
        }

        int rval = 0;
        int nBytes;

        do {
            nBytes = readData();
            if (nBytes > 0) {
                // recv_packet.dumpPacket("Recv: ");
                jamMixer->addData(&packet, nBytes);
            }
        } while( nBytes > 0);
        return rval;
    }

    #define EMPTY_SLOT 40000

    int JamSocket::readPackets() {
        // This is the read and broadcast for the server
        int nBytes = readData();
        printf("nBytes: %d from %d\n", nBytes, packet.getChannel());
        if (nBytes <= 0) {
            // This was a timeout reading
            // clear out dead sessions?
        } else {
            // int samples = packet.decodeHeader(nBytes);
            // if (samples <= 0) {
            //     return 0;
            // }
            unsigned long from_addr = ((struct sockaddr_in*) &senderAddr)->sin_addr.s_addr;
            int chan = channelMap.getChannel(from_addr);
            channelMap.dumpOut();
            for (int i=0; i<MAX_JAMMERS; i++) {
                // Don't send back an echo to the sender
                if (i != chan) {
                    unsigned long to_addr = channelMap.getClientId(i);
                    if (to_addr != EMPTY_SLOT) {
                        // send the packet to the address
                    }
                }
            }
        }
        // Get IP address (as unsigned long)
        return nBytes;
    }
    int JamSocket::readData() {
        return recvfrom(
            jamSocket,packet.getPacket(),
            sizeof(struct JamMessage),
            0,
            (struct sockaddr *) &senderAddr, 
            &addr_size
        );
    }
}

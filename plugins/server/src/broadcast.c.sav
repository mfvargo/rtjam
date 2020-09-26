/************* UDP SERVER CODE *******************/

#include "../server.h"

void encodeJamMessage(struct JamMessage* packet) {
  packet->ServerTime = htobe64(packet->ServerTime);
  packet->TimeStamp = htobe64(packet->TimeStamp);
  packet->ClientId = htonl(packet->ClientId);
  packet->SequenceNumber = htonl(packet->SequenceNumber);
};

void decodeJamMessage(struct JamMessage* packet) {
  packet->ServerTime = be64toh(packet->ServerTime);
  packet->TimeStamp = be64toh(packet->TimeStamp);
  packet->ClientId = ntohl(packet->ClientId);
  packet->SequenceNumber = ntohl(packet->SequenceNumber);
};

uint64_t getMicroTime(){
    uint64_t rval;
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);
    rval = (currentTime.tv_sec - THE_BEGINNING_OF_TIME) * (int)1e6;
    rval += currentTime.tv_usec;
    return rval;
}


struct ClientEntry {
  unsigned long s_addr;
  unsigned int SequenceNo;
  suseconds_t LastReceivedTime;
  time_t KeepAlive;
  struct sockaddr_storage Address;
};

void dumpChannels(struct ClientEntry* channels);
void dumpPacket(struct JamMessage* packet);
void clearChannel(struct ClientEntry* channel);
void pruneStaleSlots(struct ClientEntry* channels);
int updateChannels(struct ClientEntry* channels, struct JamMessage* packet, struct sockaddr_storage* addr);

int main(int argc,  char **argv){
  int dumpCount = 0;
  int logstats = 0;
  int udpSocket, nBytes;
  int port = 7891;
  char buffer[sizeof(struct JamMessage)]; // 48k stereo, 24 bit, 200msec
  struct sockaddr_in serverAddr, clientAddr;
  struct sockaddr_storage serverStorage;
  socklen_t addr_size, client_addr_size;
  struct ClientEntry channels[MAX_JAMMERS];

  if (argc > 1 && strcmp(argv[1],"--stats") == 0) {
    logstats = 1;
  }
  

  if (argc > 2 && strcmp(argv[1],"--port") == 0) {
    port = atoi(argv[2]);
    printf("port = %s\n", argv[2]);
  }
  
  struct JamMessage* packet = (struct JamMessage*) malloc(sizeof(struct JamMessage));

  /*Create UDP socket*/
  udpSocket = socket(PF_INET, SOCK_DGRAM, 0);
  /*Configure settings in address struct*/
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(port);
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  
  /*Bind socket with address struct*/
  bind(udpSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
  // Try to set the Type of Service to Voice (for whatever that is worth)
  int tos_local = IPTOS_LOWDELAY;
  if (setsockopt(udpSocket, IPPROTO_IP, IP_TOS,  &tos_local, sizeof(tos_local))) {
      printf("set TOS failed\n");
  }
  // Set the socket to timeout every 5 seconds if nobody is around
  struct timeval tv;
  tv.tv_sec = 5;
  tv.tv_usec = 0;
  if (setsockopt(udpSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv)) {
    printf("set SO_RCVTIMEO failed\n");
  }

  /*Initialize size variable to be used later on*/
  addr_size = sizeof serverStorage;

  // Clear out the channel table
  for (int i=0; i<MAX_JAMMERS; i++) {
    clearChannel(&channels[i]);
  }
  unsigned long startOfTime = getMicroTime();

  // Print CSV header
  if (logstats == 1)
    dumpPacket(NULL);

  while(1){
    dumpCount++;
    if (dumpCount%1000 == 0) {
      // dumpChannels(channels);
    }
    /* Try to receive any incoming UDP datagram. Address and port of 
      requesting client will be stored on serverStorage variable */
    nBytes = recvfrom(udpSocket,packet,sizeof(struct JamMessage),0,(struct sockaddr *)&serverStorage, &addr_size);
    unsigned long from_addr = ((struct sockaddr_in*) &serverStorage)->sin_addr.s_addr;
    pruneStaleSlots(channels);
    if (nBytes <= 0) {
      // no data, a timeout
      // dumpChannels(channels);
    } else {
      decodeJamMessage(packet);
      int channel = updateChannels(channels, packet, &serverStorage);
      if (channel == -1) {
        // system full
        // printf("jam room full \n");
      } else {
        // Multicast to users
        packet->Channel = channel;
        packet->ServerTime = getMicroTime();
        if (logstats == 1)
          dumpPacket(packet);
        encodeJamMessage(packet);
        for (int i=0; i<MAX_JAMMERS; i++) {
          unsigned long to_addr = ((struct sockaddr_in*) &channels[i].Address)->sin_addr.s_addr;
          if (channels[i].s_addr != 0 && from_addr != to_addr) {
            sendto(udpSocket,packet,nBytes,0,(struct sockaddr *)& channels[i].Address,addr_size);
          }
        }
      }
    }
  }

  return 0;
}

void clearChannel(struct ClientEntry* channel) {
  // printf("clearing channel\n");
  channel->s_addr = 0;
  channel->SequenceNo = 0;
  channel->LastReceivedTime = 0;
  channel->KeepAlive = 0;
}

void pruneStaleSlots(struct ClientEntry* channels) {
  struct timeval tval;
  gettimeofday(&tval, NULL);

  // prune stale slots
  for(int i=0; i<MAX_JAMMERS; i++) {
    // check for already cleared
    if (channels[i].s_addr != 0) {
      if ((tval.tv_sec - channels[i].KeepAlive) > EXPIRATION_IN_SECONDS) {
        clearChannel(&channels[i]);
      }
    }
  }
}

int updateChannels(struct ClientEntry* channels, struct JamMessage* packet, struct sockaddr_storage* addr) {
  // used for timestamp
  struct timeval tval;
  gettimeofday(&tval, NULL);
  unsigned long from_addr = ((struct sockaddr_in*) addr)->sin_addr.s_addr;

  int slot = -1;
  int emptySlot = -1;
  // see if we know this cat and at the same time, is there an available channel
  for(int i=MAX_JAMMERS-1; i>=0; i--) {
    if (from_addr == channels[i].s_addr) {
      // This packet came from a known user
      slot = i;
    }
    if (channels[i].s_addr == 0) {
      emptySlot = i;
    }
  }
  if (slot == -1) {
    dumpChannels(channels);
    // new user
    if (emptySlot == -1) {
      // This is a new dude, but there is no room
      return -1;
    }
    slot = emptySlot;
  }
  channels[slot].s_addr = from_addr;
  channels[slot].LastReceivedTime = tval.tv_usec;
  channels[slot].KeepAlive = tval.tv_sec;
  channels[slot].SequenceNo = packet->SequenceNumber;
  memcpy(&channels[slot].Address, addr, sizeof(struct sockaddr_storage));

  return slot;
}

void makeIpString(unsigned long, char* ipString);

void dumpChannels(struct ClientEntry* channels) {
  char ipString[32];
  struct timeval tval;
  gettimeofday(&tval, NULL);
  for (int i=0; i<MAX_JAMMERS; i++) {
    makeIpString(channels[i].s_addr, ipString);
    fprintf(stderr,
      "chan: %d\t age: %16ld\t ip: %s\t seq: %d <br/>\n",
      i,
      tval.tv_sec - channels[i].KeepAlive,
      ipString,
      channels[i].SequenceNo
    );
  }
  fprintf(stderr, "<br/>\n");
}

void makeIpString(unsigned long s_addr, char* ipString) {
  unsigned char octet[4]  = {0,0,0,0};
  for (int i=0; i<4; i++)
  {
    octet[i] = ( s_addr >> (i*8) ) & 0xFF;
  }
  sprintf(ipString, "%d.%d.%d.%d", octet[0],octet[1],octet[2],octet[3]);
}

void dumpPacket(struct JamMessage* packet){
  if (packet == NULL) {
    printf("Channel, ServerTime, TimeStamp, ClientId, SequenceNumber\n");
  } else {
    printf("%d,%ld,%ld,%d,%d\n",
        packet->Channel,
        packet->ServerTime,
        packet->TimeStamp,
        packet->ClientId,
        packet->SequenceNumber
    );
  }
}

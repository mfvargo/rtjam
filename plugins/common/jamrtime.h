#ifndef _JAMRTIME_H
#define _JAMRTIME_H
#include <sys/time.h>
#define SERVER_NAME "music.basscleftech.com"
#define SERVER_PORT 7891

#define MAX_JAMMERS 5
#define JAM_BUF_SIZE 14400
#define MAX_FRAME_SIZE 1024
#define EXPIRATION_IN_SECONDS 2
#define THE_BEGINNING_OF_TIME 1587337873
#define SET_TEMPO_CHAN 0xF0

typedef enum {
	FORTY_EIGHT_K,
  FORTY_FOUR_ONE_K,
  TWENTY_FOUR_K,
  TWENTY_TWO_FIFTY_K
} JAM_SAMPLE_RATES;

/* Message stuff goes here */

#pragma pack(push, 1)
struct JamMessage {
  uint8_t Channel;  // Assigned by server
  uint8_t SampleRate;  // Assigned by client
  uint8_t NumSubChannels; // Assigned by client
  uint8_t Beat;  // Assigned by server for shared synchonized metronome
  uint64_t ServerTime;  // Assigned by server
  uint64_t TimeStamp;   // Assigned by the client
  uint32_t ClientId;    // Assigned by the client to know which channel is their channel
  uint32_t SequenceNumber; // Assigned by client
  unsigned char buffer[JAM_BUF_SIZE];
};
#pragma pack(pop)

#ifdef __APPLE__

#include <libkern/OSByteOrder.h>

#define htobe16(x) OSSwapHostToBigInt16(x)
#define htole16(x) OSSwapHostToLittleInt16(x)
#define be16toh(x) OSSwapBigToHostInt16(x)
#define le16toh(x) OSSwapLittleToHostInt16(x)

#define htobe32(x) OSSwapHostToBigInt32(x)
#define htole32(x) OSSwapHostToLittleInt32(x)
#define be32toh(x) OSSwapBigToHostInt32(x)
#define le32toh(x) OSSwapLittleToHostInt32(x)

#define htobe64(x) OSSwapHostToBigInt64(x)
#define htole64(x) OSSwapHostToLittleInt64(x)
#define be64toh(x) OSSwapBigToHostInt64(x)
#define le64toh(x) OSSwapLittleToHostInt64(x)

#endif // __APPLE__

#endif /* _JAMRTIME_H */

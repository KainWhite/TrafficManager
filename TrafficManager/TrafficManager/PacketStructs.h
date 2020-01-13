#ifndef PACKETSTRUCTS_H
#define PACKETSTRUCTS_H

#include <chrono>

namespace PacketStructs {

struct IPv4Address {
  unsigned char byte1;
  unsigned char byte2;
  unsigned char byte3;
  unsigned char byte4;
  IPv4Address(unsigned int ip)
      : byte1(((unsigned char *)&ip)[0]),
        byte2(((unsigned char *)&ip)[1]),
        byte3(((unsigned char *)&ip)[2]),
        byte4(((unsigned char *)&ip)[3]) {}
  inline bool operator==(const int &r) const {
    return *((int *)this) == r;
  }
  inline bool operator!=(const int &r) const {
    return !(*this == r);
  }
  inline bool operator<(const int &r) const {
    return *((int *)this) < r;
  }
  inline bool operator>(const int &r) const {
    return *((int *)this) > r;
  }
  inline bool operator<=(const int &r) const {
    return !(*this > r);
  }
  inline bool operator>=(const int &r) const {
    return !(*this < r);
  }

  inline bool operator==(const IPv4Address &r) const {
    return *this == *((int *)&r);
  }
  inline bool operator!=(const IPv4Address &r) const {
    return !(*this == r);
  }
  inline bool operator<(const IPv4Address &r) const {
    return *this < *((int *)&r);
  }
  inline bool operator>(const IPv4Address &r) const {
    return r < *this;
  }
  inline bool operator<=(const IPv4Address &r) const {
    return !(*this > r);
  }
  inline bool operator>=(const IPv4Address &r) const {
    return !(*this < r);
  }
};

struct IPHeader {
  unsigned char versionAndInternetHeaderlen;  // Version (4 bits) + Internet header
                                       // length (4 bits)
  unsigned char typeOfService;
  unsigned short totalLen;
  unsigned short identification;
  unsigned short flagsAndFragmentOffset;  // Flags (3 bits) + Fragment offset (13 bits)
  unsigned char timeToLive;
  unsigned char protocol;
  unsigned short crc;  // Header checksum
  IPv4Address srcAddr;
  IPv4Address destAddr;
  unsigned int optionAndPadding;  // Option + Padding
};

struct UDPHeader {
  unsigned short srcPort;
  unsigned short destPort;
  unsigned short totalLen;
  unsigned short crc;  // Checksum
};

struct FullAddress {
  IPv4Address ip;
  unsigned short port;
  FullAddress(IPv4Address ip, unsigned short port) : ip(ip), port(port) {}
  inline bool operator==(const FullAddress &r) const {
    return (this->ip == r.ip) && (this->port == r.port);
  }
  inline bool operator!=(const FullAddress &r) const {
    return !(*this == r);
  }
  inline bool operator<(const FullAddress &r) const {
    return (this->ip < r.ip
                ? true
                : (this->ip == r.ip ? (this->port < r.port) : false));
  }
};

struct PacketInformation {
  unsigned short size;
  FullAddress src;
  FullAddress dest;
  bool incoming;
  std::chrono::milliseconds msSinceEpoch;
  PacketInformation(unsigned short size,
                    FullAddress src,
                    FullAddress dest,
                    bool incoming,
                    std::chrono::milliseconds msSinceEpoch)
      : size(size),
        src(src),
        dest(dest),
        incoming(incoming),
        msSinceEpoch(msSinceEpoch) {}
};

}  // namespace PacketStructs

#endif  // !PACKETSTRUCTS_H

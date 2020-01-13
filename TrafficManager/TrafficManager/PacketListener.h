#ifndef PACKETLISTENER_H
#define PACKETLISTENER_H
#include <WS2tcpip.h>
#include <pcap.h>

#include "ThreadPool.h"
#include "PacketStructs.h"
#include "TrafficManagerList.h"

namespace PacketListenerClass {

struct AdapterInfo {
  size_t frameHeaderLen;
};

class PacketListener {
 public:
  std::set<TrafficManagerListClass::ListItem> processes;
  void *udp4Table = nullptr;
  void *tcp4Table = nullptr;
  PacketListener();
  void StartListening();
  void StopListening();
  ~PacketListener();

 private:
  char errbuf[PCAP_ERRBUF_SIZE];
  bool stopListening;
  ThreadPoolClass::ThreadPool *threadPool;
  void StartListeningToDevice(pcap_if_t *device);
  void SetPacketFilter(pcap_t *adapterHandle,
                       unsigned int netmask,
                       std::string expression);
  void HandlePacket(PacketStructs::PacketInformation packet);
  void InitNpcapDllPath();
  char *IPv4ToStr(unsigned int ip, char *str, size_t strLen);
  char *IPv6ToStr(struct sockaddr *sockaddr, char *address, int addrlen);
};

}  // namespace PacketListenerClass

#endif  // !PACKETLISTENER_H

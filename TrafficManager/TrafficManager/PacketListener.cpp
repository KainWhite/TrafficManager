#include "PacketListener.h"

#include <iphlpapi.h>
#include <psapi.h>
#include <time.h>

#include <chrono>

#include "Debugger.h"

namespace PacketListenerClass {

PacketListener::PacketListener() : stopListening(false), threadPool(nullptr) {
  InitNpcapDllPath();
}

void PacketListener::StartListeningToDevice(pcap_if_t *device) {
  pcap_t *adapterHandle;
  if ((adapterHandle = pcap_open(device->name,
                                 65536,
                                 0,
                                 1000,  // read timeout
                                 NULL,  // authentication on the remote machine
                                 errbuf  // error buffer
                                 )) == nullptr) {
    DebuggerClass::Debugger::PrintlnA("Unable to open the adapter. ",
                                      device->name,
                                      " is not supported by Npcap");
    return;
  }
  AdapterInfo adapterInfo;
  switch (pcap_datalink(adapterHandle)) {
    case DLT_NULL:
      adapterInfo.frameHeaderLen = 4;
      return;  // we don't need loopback traffic
    case DLT_EN10MB:
      adapterInfo.frameHeaderLen = 14;
      break;
    case DLT_IEEE802_11:
      adapterInfo.frameHeaderLen = 30;
      break;
    case DLT_LOOP:
      adapterInfo.frameHeaderLen = 4;
      return;  // we don't need loopback traffic
    default:
      adapterInfo.frameHeaderLen = 0;
  }
  SetPacketFilter(
      adapterHandle,
      (device->addresses ? ((struct sockaddr_in *)device->addresses->netmask)
                               ->sin_addr.S_un.S_addr
                         : 0xffffff),  // If the interface is without addresses,
                                       // we suppose to be in a C class network
      "");
  int res;
  struct pcap_pkthdr *header;
  const u_char *packetData;
  while ((res = pcap_next_ex(adapterHandle, &header, &packetData)) >= 0 &&
          !stopListening) {
    if (res == 0) {
      continue;
    }

    PacketStructs::IPHeader *ipHeader =
        (PacketStructs::IPHeader *)(packetData + adapterInfo.frameHeaderLen);
    PacketStructs::UDPHeader *udpHeader =
        (PacketStructs::UDPHeader *)((u_char *)ipHeader +
                                      (ipHeader->versionAndInternetHeaderlen &
                                      0xf) *
                                          4);
    pcap_addr *deviceAddress = device->addresses;
    bool incoming = true;
    if (deviceAddress->addr->sa_family == AF_INET) {
      incoming = ((PacketStructs::IPv4Address)(
                      (struct sockaddr_in *)deviceAddress->addr)
                      ->sin_addr.s_addr == ipHeader->destAddr);
    }
    // TODO make for ipv6
    HandlePacket(PacketStructs::PacketInformation(
        header->len,
        PacketStructs::FullAddress(ipHeader->srcAddr, udpHeader->srcPort),
        PacketStructs::FullAddress(ipHeader->destAddr, udpHeader->destPort),
        incoming,
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch())));
  }
  if (res == -1) {
    DebuggerClass::Debugger::PrintlnA("Error reading the packets: ",
                                      pcap_geterr(adapterHandle));
  }
}

void PacketListener::SetPacketFilter(pcap_t *adapterHandle,
                                     unsigned int netmask,
                                     std::string expression) {
  bpf_program fcode;
  if (pcap_compile(adapterHandle, &fcode, expression.c_str(), 1, netmask) < 0) {
    DebuggerClass::Debugger::PrintlnA(
        "Unable to compile the packet filter. Check the syntax.");
    return;
  }
  if (pcap_setfilter(adapterHandle, &fcode) < 0) {
    DebuggerClass::Debugger::PrintlnA("Error setting the filter.");
    return;
  }
}

void PacketListener::StartListening() {
  pcap_if_t *devices = nullptr;

  if (pcap_findalldevs_ex(PCAP_SRC_IF_STRING, NULL, &devices, errbuf) == -1) {
    DebuggerClass::Debugger::PrintlnA(L"Error in pcap_findalldevs_ex: ",
                                      errbuf);
    return;
  }
  if (devices == nullptr) {
    DebuggerClass::Debugger::PrintlnA(
        L"No interfaces found! Make sure Npcap is installed.");
    return;
  }
  pcap_if_t *curDev = devices;
  size_t deviceCount = 0;
  for (; curDev != NULL; curDev = curDev->next, deviceCount++)
    ;
  threadPool = new ThreadPoolClass::ThreadPool(deviceCount);
  std::vector<std::future<void>> futures;
  for (curDev = devices; curDev != NULL; curDev = curDev->next) {
    futures.push_back(threadPool->EnqueueTask(
        [this](pcap_if_t *curDev) { this->StartListeningToDevice(curDev); },
        curDev));
  }
  for (int i = 0; i < futures.size(); i++) {
    futures[i].get();
  }
  pcap_freealldevs(devices);
}

void PacketListener::StopListening() {
  stopListening = true;
  if (threadPool) {
    delete threadPool;
    threadPool = nullptr;
  }
}

void PacketListener::HandlePacket(PacketStructs::PacketInformation packet) {
  MIB_UDPROW_OWNER_PID *udp4TableLocal =
      ((MIB_UDPTABLE_OWNER_PID *)udp4Table)->table;
  size_t udpTableSize = ((MIB_UDPTABLE_OWNER_PID *)udp4Table)->dwNumEntries;
  PacketStructs::FullAddress valueToSearch =
      packet.incoming ? packet.dest : packet.src;
  // tables are not sorted!!!!!!!!!!!!!!!!!!
  /*size_t left = 0;
  size_t right = udpTableSize - 1;
  while (left < right) {
    size_t mid = (left + right) / 2;
    if (valueToSearch.ip < udp4TableLocal[mid].dwLocalAddr) {
      right = mid;
    } else if (valueToSearch.ip == udp4TableLocal->dwLocalAddr) {
      if (valueToSearch.port < udp4TableLocal[mid].dwLocalPort) {
        right = mid;
      } else {
        left = mid + 1;
      }
    } else {
      left = mid + 1;
    }
  }*/
  size_t left = -1;
  for (int i = 0; i < udpTableSize; i++) {
    if (valueToSearch.ip == udp4TableLocal[i].dwLocalAddr &&
        valueToSearch.port == udp4TableLocal[i].dwLocalPort) {
      left = i;
    }
  }
  const size_t ipv4StrLen = 4 * 4 + 3 + 1;
  char ipv4Str[ipv4StrLen];
  /*DebuggerClass::Debugger::PrintlnA(
      "Packet\n",
      " incoming: ",
      packet.incoming,
      "\n",
      " ip: ",
      IPv4ToStr(*(int *)(&valueToSearch.ip), ipv4Str, ipv4StrLen),
      "\n",
      " port: ",
      valueToSearch.port,
      "\n",
      " Found:\n",
      "  ip: ",
      IPv4ToStr(udp4TableLocal[left].dwLocalAddr, ipv4Str, ipv4StrLen),
      "\n",
      "  port: ",
      udp4TableLocal[left].dwLocalPort,
      "\n");*/
  if (left != -1) {
    auto targetProcess = processes.find(TrafficManagerListClass::ListItem(
        L"",
        udp4TableLocal[left].dwOwningPid,
        0,
        std::set<PacketStructs::FullAddress>(),
        std::queue<PacketStructs::PacketInformation>()));
    if (targetProcess != processes.end()) {
      targetProcess->addresses.insert(
          PacketStructs::FullAddress(valueToSearch.ip, valueToSearch.port));
      targetProcess->packetQueue.push(packet);
      targetProcess->bytesPerMinute += packet.size;
    } else {
      HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                                    FALSE,
                                    udp4TableLocal[left].dwOwningPid);
      if (hProcess) {
        wchar_t fileName[MAX_PATH];
        if (GetModuleFileNameEx(hProcess, NULL, fileName, MAX_PATH)) {
          processes.insert(TrafficManagerListClass::ListItem(
              fileName,
              udp4TableLocal[left].dwOwningPid,
              packet.size,
              std::set<PacketStructs::FullAddress>({valueToSearch}),
              std::queue<PacketStructs::PacketInformation>({packet})));
        } else {
          DebuggerClass::Debugger::PrintlnW(
              L"PacketListener::HandlePacket GetModuleFileNameEx failed. Error code: ",
              GetLastError());
        }
        CloseHandle(hProcess);
      }
    }
  }

  MIB_TCPROW_OWNER_PID *tcp4TableLocal =
      ((MIB_TCPTABLE_OWNER_PID *)tcp4Table)->table;
  size_t tcpTableSize = ((MIB_TCPTABLE_OWNER_PID *)tcp4Table)->dwNumEntries;
  /*left = 0;
  right = tcpTableSize - 1;
  while (left < right) {
    size_t mid = (left + right) / 2;
    if (valueToSearch.ip < tcp4TableLocal[mid].dwLocalAddr) {
      right = mid;
    } else if (valueToSearch.ip == tcp4TableLocal->dwLocalAddr) {
      if (valueToSearch.port < tcp4TableLocal[mid].dwLocalPort) {
        right = mid;
      } else {
        left = mid + 1;
      }
    } else {
      left = mid + 1;
    }
  }*/
  left = -1;
  for (int i = 0; i < tcpTableSize; i++) {
    if (valueToSearch.ip == tcp4TableLocal[i].dwLocalAddr &&
        valueToSearch.port == tcp4TableLocal[i].dwLocalPort) {
      left = i;
    }
  }
  /*DebuggerClass::Debugger::PrintlnA(
    "Packet\n",
    " incoming: ",
    packet.incoming,
    "\n",
    " ip: ",
    IPv4ToStr(*(int *)(&valueToSearch.ip), ipv4Str, ipv4StrLen),
    "\n",
    " port: ",
    valueToSearch.port,
    "\n",
    " Found:\n",
    "  ip: ",
    IPv4ToStr(tcp4TableLocal[left].dwLocalAddr, ipv4Str, ipv4StrLen),
    "\n",
    "  port: ",
    tcp4TableLocal[left].dwLocalPort,
    "\n");*/
  if (left != -1) {
    auto targetProcess = processes.find(TrafficManagerListClass::ListItem(
        L"",
        tcp4TableLocal[left].dwOwningPid,
        0,
        std::set<PacketStructs::FullAddress>(),
        std::queue<PacketStructs::PacketInformation>()));
    if (targetProcess != processes.end()) {
      targetProcess->addresses.insert(
          PacketStructs::FullAddress(valueToSearch.ip, valueToSearch.port));
      targetProcess->packetQueue.push(packet);
      targetProcess->bytesPerMinute += packet.size;
    } else {
      HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                                    FALSE,
                                    tcp4TableLocal[left].dwOwningPid);
      if (hProcess) {
        wchar_t fileName[MAX_PATH];
        if (GetModuleFileNameEx(hProcess, NULL, fileName, MAX_PATH)) {
          processes.insert(TrafficManagerListClass::ListItem(
              fileName,
              tcp4TableLocal[left].dwOwningPid,
              packet.size,
              std::set<PacketStructs::FullAddress>({valueToSearch}),
              std::queue<PacketStructs::PacketInformation>({packet})));
        } else {
          DebuggerClass::Debugger::PrintlnW(
              L"PacketListener::HandlePacket GetModuleFileNameEx failed. Error code: ",
              GetLastError());
        }
        CloseHandle(hProcess);
      }
    }
  }
}

void PacketListener::InitNpcapDllPath() {
  BOOL(WINAPI * SetDllDirectory)(LPCTSTR);
  wchar_t sysdir_name[512];
  int len;
  SetDllDirectory = (BOOL(WINAPI *)(LPCTSTR))GetProcAddress(
      GetModuleHandle(L"kernel32.dll"), "SetDllDirectoryA");
  if (SetDllDirectory == NULL) {
    DebuggerClass::Debugger::PrintlnA(L"Error in SetDllDirectory");
  } else {
    len = GetSystemDirectory(sysdir_name, 480);  //	be safe
    if (!len) {
      DebuggerClass::Debugger::PrintlnA(L"Error in GetSystemDirectory: ",
                                        GetLastError());
    }
    wcscat_s(sysdir_name, L"\\Npcap");
    if (SetDllDirectory(sysdir_name) == 0) {
      DebuggerClass::Debugger::PrintlnA(
          L"Error in SetDllDirectory(\"System32\\Npcap\")");
    }
  }
}

char *PacketListener::IPv4ToStr(unsigned int ip, char *str, size_t strLen) {
  u_char *p = (u_char *)&ip;
  sprintf_s(str, strLen, "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
  return str;
}

char *PacketListener::IPv6ToStr(struct sockaddr *sockaddr,
                                char *address,
                                int addrlen) {
  socklen_t sockaddrlen;
#ifdef WIN32
  sockaddrlen = sizeof(struct sockaddr_in6);
#else
  sockaddrlen = sizeof(struct sockaddr_storage);
#endif
  /*int res;
  if (res = getnameinfo(sockaddr,
  sockaddrlen,
  address,
  addrlen,
  NULL,
  0,
  NI_NUMERICHOST) != 0) {
  address = nullptr;
  cout << gai_strerror(res) << endl;
  cout << WSAGetLastError() << endl;
  }*/

  return address;
}

PacketListener::~PacketListener() {
  StopListening();
}

}  // namespace PacketListenerClass
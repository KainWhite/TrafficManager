#ifndef TRAFFICMANAGERLIST_H
#define TRAFFICMANAGERLIST_H

#include <queue>
#include <set>
#include <string>
#include <vector>

#include "PacketStructs.h"
#include "BaseWindow.h"

namespace TrafficManagerListClass {

struct Margin {
  int left;
  int top;
  int right;
  int bottom;
  Margin(int width, int height)
      : left(width), top(height), right(width), bottom(height) {}
  Margin(int left, int top, int right, int bottom)
      : left(left), top(top), right(right), bottom(bottom) {}
};

struct ListColumn {
  std::wstring name;
  int width;
  ListColumn(std::wstring name, int width) : name(name), width(width) {}
};

struct ListItem {
  mutable std::wstring name;
  unsigned int pid;
  mutable unsigned int bytesPerMinute;
  mutable std::set<PacketStructs::FullAddress> addresses;
  mutable std::queue<PacketStructs::PacketInformation> packetQueue;
  ListItem(std::wstring name,
           unsigned int pid,
           unsigned int bytesPerMinute,
           std::set<PacketStructs::FullAddress> addresses,
           std::queue<PacketStructs::PacketInformation> packetQueue)
      : name(name),
        pid(pid),
        bytesPerMinute(bytesPerMinute),
        addresses(addresses),
        packetQueue(packetQueue) {}
  inline bool operator<(const ListItem &r) const {
    return pid < r.pid;
  }
};

class TrafficManagerList
    : public BaseWindowClass::BaseWindow<TrafficManagerList> {
 public:
  TrafficManagerList();
  HWND CreateControl(PCWSTR lpClassName,
                     DWORD dwStyle,
                     int x,
                     int y,
                     int nWidth,
                     int nHeight,
                     HWND hWndParent,
                     HMENU hMenu = nullptr,
                     HINSTANCE hInstance = nullptr,
                     LPVOID lpParam = nullptr) override;
  HWND CreateControl(HWND hWndParent);
  void InsertItems(std::set<TrafficManagerListClass::ListItem> &processes);
  void DeleteAllItems();
  LRESULT CALLBACK HandleMessage(UINT, WPARAM, LPARAM) override;
  PCWSTR ClassName() const override {
    return className.c_str();
  }

 private:
  const std::wstring className = L"TrafficManagerListClass";
  const Margin parentMargin = Margin(0, 0, 0, 0);
  std::vector<ListColumn> columns = {ListColumn(L"Path", 910),
                                     ListColumn(L"PID", 70),
                                     ListColumn(L"Bytes/second", 100)};
};

}  // namespace TrafficManagerListClass

#endif  // !TRAFFICMANAGERLIST_H

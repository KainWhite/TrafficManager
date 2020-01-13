#ifndef TRAFFICMANAGER_H
#define TRAFFICMANAGER_H

#include <thread>

#include "PacketListener.h"
#include "TrafficManagerList.h"
#include "BaseWindow.h"

namespace TrafficManagerClass {

class TrafficManager : public BaseWindowClass::BaseWindow<TrafficManager> {
 public:
  TrafficManager();
  int Create(DWORD dwExStyle = 0,
             COLORREF backgroundColor = RGB(0, 0, 0),
             PCWSTR lpWindowName = L"Traffic Manager",
             DWORD dwStyle = WS_OVERLAPPEDWINDOW,
             int x = CW_USEDEFAULT,
             int y = CW_USEDEFAULT,
             int nWidth = CW_USEDEFAULT,
             int nHeight = CW_USEDEFAULT,
             HWND hWndParent = nullptr,
             HMENU hMenu = nullptr) override;

  LRESULT CALLBACK HandleMessage(UINT, WPARAM, LPARAM) override;
  PCWSTR ClassName() const override {
    return className.c_str();
  }
  ~TrafficManager();

 private:
  static const int viewUpdateTimerId = 1;
  const double viewUpdateRate = 2;  // fps
  static const int udpTableUpdateTimerId = 2;
  const int udpTableUpdateInterval = 5000;  // msec
  const std::wstring className = L"TrafficManagerClass";
  TrafficManagerListClass::TrafficManagerList trafficManagerList;
  PacketListenerClass::PacketListener packetListener;
  std::thread packetListenerThread;

  void HandleTimer(WPARAM wParam, LPARAM lParam);
  void HandleNotify(WPARAM wParam, LPARAM lParam);
  void UpdateIpTables();
  void UpdateView();
};

}  // namespace TrafficManagerClass

#endif  // !TRAFFICMANAGER_H

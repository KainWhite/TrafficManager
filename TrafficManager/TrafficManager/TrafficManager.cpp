#include "TrafficManager.h"

#include <CommCtrl.h>
#include <iphlpapi.h>

#include <chrono>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "Debugger.h"

namespace TrafficManagerClass {

template <class T>
void SafeDelete(T *pT) {
  if (pT) {
    delete[] pT;
    pT = nullptr;
  }
}

TrafficManager::TrafficManager() {}

int TrafficManager::Create(DWORD dwExStyle,
                           COLORREF backgroundColor,
                           PCWSTR lpWindowName,
                           DWORD dwStyle,
                           int x,
                           int y,
                           int nWidth,
                           int nHeight,
                           HWND hWndParent,
                           HMENU hMenu) {
  if (!BaseWindowClass::BaseWindow<TrafficManager>::Create(dwExStyle,
                                                           backgroundColor,
                                                           lpWindowName,
                                                           dwStyle,
                                                           x,
                                                           y,
                                                           nWidth,
                                                           nHeight,
                                                           hWndParent,
                                                           hMenu)) {
    return 1;
  }
  // for controls initialization.
  INITCOMMONCONTROLSEX icex;
  icex.dwICC = ICC_LISTVIEW_CLASSES;
  InitCommonControlsEx(&icex);
  SetTimer(hWnd, viewUpdateTimerId, 1000 / viewUpdateRate, (TIMERPROC) nullptr);
  SetTimer(
      hWnd, udpTableUpdateTimerId, udpTableUpdateInterval, (TIMERPROC) nullptr);
  trafficManagerList.CreateControl(hWnd);
  if (!trafficManagerList.GetHWnd()) {
    DebuggerClass::Debugger::PrintlnW(L"List create failed");
    return 1;
  }
  UpdateIpTables();
  packetListenerThread = std::thread(
      &PacketListenerClass::PacketListener::StartListening, &packetListener);
  return 0;
}

void TrafficManager::HandleTimer(WPARAM wParam, LPARAM lParam) {
  switch (wParam) {
    case TrafficManager::udpTableUpdateTimerId:
      UpdateIpTables();
      break;
    case TrafficManager::viewUpdateTimerId:
      UpdateView();
      break;
  }
}

void TrafficManager::HandleNotify(WPARAM wParam, LPARAM lParam) {
  NMLVDISPINFO *plvdi;

  switch (((LPNMHDR)lParam)->code) {
    case LVN_GETDISPINFO:
      plvdi = (NMLVDISPINFO *)lParam;
      auto process = packetListener.processes.begin();
      for (int i = 0; i < plvdi->item.iItem; i++, process++)
        ;
      switch (plvdi->item.iSubItem) {
        case 0:
          plvdi->item.pszText = &process->name[0];
          break;
        case 1:
          plvdi->item.pszText = &std::to_wstring(process->pid)[0];
          break;
        case 2:
          plvdi->item.pszText = &std::to_wstring(process->bytesPerMinute)[0];
          break;
        default:
          break;
      }
      break;
  }
}

void TrafficManager::UpdateIpTables() {
  DWORD udp4TableSize;
  if (GetExtendedUdpTable(packetListener.udp4Table,
                          &udp4TableSize,
                          true,
                          AF_INET,
                          UDP_TABLE_OWNER_PID,
                          0) == ERROR_INSUFFICIENT_BUFFER) {
    SafeDelete(packetListener.udp4Table);
    packetListener.udp4Table = new void *[udp4TableSize];
    GetExtendedUdpTable(packetListener.udp4Table,
                        &udp4TableSize,
                        true,
                        AF_INET,
                        UDP_TABLE_OWNER_PID,
                        0);
  }
  DWORD tcp4TableSize;
  if (GetExtendedTcpTable(packetListener.tcp4Table,
                          &tcp4TableSize,
                          true,
                          AF_INET,
                          TCP_TABLE_OWNER_PID_ALL,
                          0) == ERROR_INSUFFICIENT_BUFFER) {
    packetListener.tcp4Table = new void *[tcp4TableSize];
    GetExtendedTcpTable(packetListener.tcp4Table,
                        &tcp4TableSize,
                        true,
                        AF_INET,
                        TCP_TABLE_OWNER_PID_ALL,
                        0);
  }
}

void TrafficManager::UpdateView() {
  trafficManagerList.DeleteAllItems();
  std::chrono::milliseconds now =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now().time_since_epoch());
  for (auto process = packetListener.processes.begin(); process != packetListener.processes.end(); process++) {
    while (!process->packetQueue.empty() &&
           now - process->packetQueue.front().msSinceEpoch >=
               std::chrono::milliseconds(60000)) {
      process->bytesPerMinute -= process->packetQueue.front().size;
      process->packetQueue.pop();
    }
  }
  trafficManagerList.InsertItems(packetListener.processes);
}

LRESULT CALLBACK TrafficManager::HandleMessage(UINT uMsg,
                                               WPARAM wParam,
                                               LPARAM lParam) {
  switch (uMsg) {
    case WM_DESTROY:
      PostQuitMessage(0);
      break;
    case WM_TIMER:
      HandleTimer(wParam, lParam);
      break;
    // case WM_NOTIFY:
    //  HandleNotify(wParam, lParam);
    //  break;
    default:
      return DefWindowProc(hWnd, uMsg, wParam, lParam);
  }
  return 0;
}

TrafficManager::~TrafficManager() {
  packetListener.StopListening();
  packetListenerThread.join();
}

}  // namespace TrafficManagerClass

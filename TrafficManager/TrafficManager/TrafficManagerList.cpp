#include "TrafficManagerList.h"

#include <commctrl.h>

#include <chrono>

#include "Debugger.h"

namespace TrafficManagerListClass {

TrafficManagerList::TrafficManagerList() {}

HWND TrafficManagerList::CreateControl(PCWSTR lpClassName,
                                       DWORD dwStyle,
                                       int x,
                                       int y,
                                       int nWidth,
                                       int nHeight,
                                       HWND hWndParent,
                                       HMENU hMenu,
                                       HINSTANCE hInstance,
                                       LPVOID lpParam) {
  BaseWindowClass::BaseWindow<TrafficManagerList>::CreateControl(lpClassName,
                                                                 dwStyle,
                                                                 x,
                                                                 y,
                                                                 nWidth,
                                                                 nHeight,
                                                                 hWndParent,
                                                                 hMenu,
                                                                 hInstance,
                                                                 lpParam);
  LVCOLUMN lvc;
  lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT |
             LVCF_SUBITEM;  // LVCF_IMAGE | LVCF_ORDER | LVCF_MINWIDTH |
                            // LVCF_DEFAULTWIDTH | LVCF_IDEALWIDTH
  lvc.fmt = LVCFMT_CENTER;
  for (int i = 0; i < columns.size(); i++) {
    lvc.cx = columns[i].width;
    lvc.pszText = &(columns[i].name[0]);
    lvc.iSubItem = i;
    ListView_InsertColumn(hWnd, lvc.iSubItem, &lvc);
  }
  ListView_SetExtendedListViewStyleEx(hWnd, LVS_EX_DOUBLEBUFFER, LVS_EX_DOUBLEBUFFER);
  return hWnd;
}

HWND TrafficManagerList::CreateControl(HWND hWndParent) {
  RECT rcClient;
  GetClientRect(hWndParent, &rcClient);
  return CreateControl(
      WC_LISTVIEW,
      WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_EDITLABELS,
      parentMargin.left,
      parentMargin.top,
      rcClient.right - parentMargin.right - parentMargin.left,
      rcClient.bottom - parentMargin.bottom - parentMargin.top,
      hWndParent);
}

void TrafficManagerList::InsertItems(std::set<TrafficManagerListClass::ListItem> &processes) {
  LVITEM lvItem;
  lvItem.mask      = LVIF_TEXT | LVIF_STATE;
  lvItem.stateMask = 0;
  lvItem.state     = 0;
  int iItem = 0;
  std::wstring wstr;
  for (auto process : processes)
  {
    // Path
    lvItem.iSubItem = 0;
    lvItem.iItem  = iItem;
    lvItem.pszText = &process.name[0];
    if (ListView_InsertItem(hWnd, &lvItem) == -1) {
      DebuggerClass::Debugger::PrintlnW(L"Insertion of item ", iItem, " failed");
      continue;
    }

    // PID
    lvItem.iSubItem = 1;
    wstr = std::to_wstring(process.pid);
    lvItem.pszText = &wstr[0];
    if (ListView_SetItem(hWnd, &lvItem) == false) {
      DebuggerClass::Debugger::PrintlnW(L"Setting item ", iItem, " subitem 1 failed");
      continue;
    }

    // Bytes/second
    lvItem.iSubItem = 2;
    wstr = std::to_wstring(process.bytesPerMinute/60);
    lvItem.pszText = &wstr[0];
    if (ListView_SetItem(hWnd, &lvItem) == false) {
      DebuggerClass::Debugger::PrintlnW(L"Setting item ", iItem, " subitem 2 failed");
      continue;
    }
  }
}

void TrafficManagerList::DeleteAllItems() {
  ListView_DeleteAllItems(hWnd);
}

LRESULT CALLBACK TrafficManagerList::HandleMessage(UINT uMsg,
                                                   WPARAM wParam,
                                                   LPARAM lParam) {
  switch (uMsg) {
    default:
      return DefWindowProc(hWnd, uMsg, wParam, lParam);
  }
  return 0;
}

}  // namespace TrafficManagerListClass
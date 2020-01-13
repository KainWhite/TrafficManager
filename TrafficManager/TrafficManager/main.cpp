#define WIN32_LEAN_AND_MEAN 

#include <iostream>

#include "TrafficManager.h"
#include <windows.h>

//#pragma comment(lib, "Comctl32")

int WINAPI wWinMain(HINSTANCE hThisInstance,
                    HINSTANCE hPrevInstance,
                    PWSTR lpszArgument,
                    int nCmdShow) {
  TrafficManagerClass::TrafficManager trafficManager;
  if (trafficManager.Create(WS_EX_ACCEPTFILES, RGB(255, 255, 255))) {
    return 1;
  }

  ShowWindow(trafficManager.GetHWnd(), nCmdShow);

  MSG msg = {};
  BOOL bRet;
  while (bRet = GetMessage(&msg, NULL, 0, 0)) {
    if (bRet == -1) {
      break;
    }
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  return 0;
}

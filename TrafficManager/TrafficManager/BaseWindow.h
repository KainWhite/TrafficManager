#ifndef BASEWINDOW_H
#define BASEWINDOW_H
#define LEAN_AND_MEAN
#include <windows.h>

namespace BaseWindowClass {

template <class T>
class BaseWindow {
 public:
  BaseWindow() : hWnd(nullptr) {}
  static LRESULT CALLBACK WindowProc(HWND hWnd,
                                     UINT uMsg,
                                     WPARAM wParam,
                                     LPARAM lParam) {
    T *pThis = nullptr;
    if (uMsg == WM_CREATE) {
      CREATESTRUCT *pCreate = reinterpret_cast<CREATESTRUCT *>(lParam);
      pThis = static_cast<T *>(pCreate->lpCreateParams);
      SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
      pThis->hWnd = hWnd;
    } else {
      pThis = reinterpret_cast<T *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }
    if (pThis) {
      return pThis->HandleMessage(uMsg, wParam, lParam);
    } else {
      return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
  }

  HWND GetHWnd() const {
    return hWnd;
  }

 protected:
  HWND hWnd;
  virtual int Create(DWORD dwExStyle = 0,
                     COLORREF backgroundColor = RGB(0, 0, 0),
                     PCWSTR lpWindowName = L"Sample Window Name",
                     DWORD dwStyle = WS_OVERLAPPEDWINDOW,
                     int x = CW_USEDEFAULT,
                     int y = CW_USEDEFAULT,
                     int nWidth = CW_USEDEFAULT,
                     int nHeight = CW_USEDEFAULT,
                     HWND hWndParent = nullptr,
                     HMENU hMenu = nullptr) {
    WNDCLASS wndClass = {0};
    wndClass.lpfnWndProc = T::WindowProc;
    wndClass.hInstance = GetModuleHandle(NULL);
    wndClass.lpszClassName = ClassName();
    wndClass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wndClass.hbrBackground = CreateSolidBrush(backgroundColor);
    RegisterClass(&wndClass);

    hWnd = CreateWindowEx(dwExStyle,
                          ClassName(),
                          lpWindowName,
                          dwStyle,
                          x,
                          y,
                          nWidth,
                          nHeight,
                          hWndParent,
                          hMenu,
                          GetModuleHandle(NULL),
                          this);
    return (hWnd ? true : false);
  }

  virtual HWND CreateControl(PCWSTR lpClassName,
                             DWORD dwStyle,
                             int x,
                             int y,
                             int nWidth,
                             int nHeight,
                             HWND hWndParent,
                             HMENU hMenu = nullptr,
                             HINSTANCE hInstance = nullptr,
                             LPVOID lpParam = nullptr) {
    hWnd = CreateWindow(lpClassName,
                        L"",
                        dwStyle,
                        x,
                        y,
                        nWidth,
                        nHeight,
                        hWndParent,
                        hMenu,
                        hInstance,
                        lpParam);
    return hWnd;
  }

  virtual LRESULT CALLBACK HandleMessage(UINT, WPARAM, LPARAM) = 0;
  virtual PCWSTR ClassName() const = 0;
};

}  // namespace BaseWindowClass

#endif  // !BASEWINDOW_H

#ifndef DEBUGGER_H
#define DEBUGGER_H

//#include <windows.h>

#include <sstream>

namespace DebuggerClass {

class Debugger {
 public:
  // If UNIDODE defined expands to PrintlW, otherwise to PrintlnA
  template <typename... Args>
  static void Println(Args... args);
  template <typename... Args>
  static void PrintlnA(Args... args);
  template <typename... Args>
  static void PrintlnW(Args... args);

 private:
  template <typename T>
  static void PlaceToSS(std::stringstream &ss, T t);
  template <typename T, typename... Args>
  static void PlaceToSS(std::stringstream &ss, T t, Args... args);

  template <typename T>
  static void PlaceToWSS(std::wstringstream &wss, T t);
  template <typename T, typename... Args>
  static void PlaceToWSS(std::wstringstream &wss, T t, Args... args);
};

template <typename... Args>
static void Debugger::Println(Args... args) {
#ifdef UNICODE
  PrintlnW(args...);
#else
  PrintlnA(args...);
#endif  // UNICODE
}

template <typename T>
static void Debugger::PlaceToSS(std::stringstream &ss, T t) {
  ss << t;
}
template <typename T, typename... Args>
static void Debugger::PlaceToSS(std::stringstream &ss, T t, Args... args) {
  PlaceToSS(ss, t);
  PlaceToSS(ss, args...);
}
template <typename... Args>
static void Debugger::PrintlnA(Args... args) {
  std::stringstream ss;
  PlaceToSS(ss, args...);
  ss << "\n";
  OutputDebugStringA(ss.str().c_str());
}

template <typename T>
static void Debugger::PlaceToWSS(std::wstringstream &wss, T t) {
  wss << t;
}
template <typename T, typename... Args>
static void Debugger::PlaceToWSS(std::wstringstream &wss, T t, Args... args) {
  PlaceToWSS(wss, t);
  PlaceToWSS(wss, args...);
}
template <typename... Args>
static void Debugger::PrintlnW(Args... args) {
  std::wstringstream wss;
  PlaceToWSS(wss, args...);
  wss << L"\n";
  OutputDebugStringW(wss.str().c_str());
}

}  // namespace DebuggerClass

#endif  // !DEBUGGER_H

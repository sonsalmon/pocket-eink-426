#pragma once

inline unsigned long host_now = 0;
inline int host_ladder_mv = 3300;
constexpr int INPUT = 0;
constexpr int INPUT_PULLUP = 1;
constexpr int LOW = 0;
constexpr int ADC_11db = 0;
struct HostSerial {
  void begin(int) {}
  void println(const char*) {}
};
inline HostSerial Serial;
inline void pinMode(int, int) {}
inline void analogSetAttenuation(int) {}
inline int analogReadMilliVolts(int pin) {
  return pin == 1 ? host_ladder_mv : 1900;
}
inline int digitalRead(int) { return 1; }
inline unsigned long millis() { return host_now; }
inline void delay(unsigned long ms) { host_now += ms; }

#include "wifi_transport.h"

#include <Arduino.h>
#include <WiFi.h>
#include <esp_system.h>

#include <cstdio>
#include <string>

namespace pocket {
namespace {

constexpr unsigned long kIdleTimeoutMs = 120000;

std::string random_password() {
  char password[17];
  std::snprintf(password, sizeof(password), "%08lx%08lx",
                static_cast<unsigned long>(esp_random()),
                static_cast<unsigned long>(esp_random()));
  return password;
}

}  // namespace

WifiTransport::WifiTransport() : server_(80) {}

void WifiTransport::attach(ProtocolService& service) {
  service_ = &service;
  server_.on("/op", HTTP_POST, [this]() { handle_op(); });
  server_.on("/page", HTTP_POST, [this]() { handle_page(); });
}

WifiInfo WifiTransport::enable() {
  if (active_) {
    last_request_at_ = millis();
    return info_;
  }
  const std::uint64_t mac = ESP.getEfuseMac();
  char ssid[16];
  std::snprintf(ssid, sizeof(ssid), "pocket-%04X",
                static_cast<unsigned>(mac & 0xFFFFU));
  info_ = WifiInfo{ssid, random_password(), "192.168.4.1", 80};

  WiFi.mode(WIFI_AP);
  if (!WiFi.softAP(info_.ssid.c_str(), info_.password.c_str())) {
    info_ = WifiInfo{};
    return info_;
  }
  server_.begin();
  active_ = true;
  disable_pending_ = false;
  last_request_at_ = millis();
  return info_;
}

void WifiTransport::disable() { disable_pending_ = true; }

void WifiTransport::loop() {
  if (!active_) {
    return;
  }
  server_.handleClient();
  if (disable_pending_ || millis() - last_request_at_ >= kIdleTimeoutMs) {
    stop();
  }
}

void WifiTransport::handle_op() {
  last_request_at_ = millis();
  if (service_ == nullptr) {
    server_.send(503, "application/json", R"({"ok":false,"err":"state"})");
    return;
  }
  const String body = server_.arg("plain");
  const ServiceResult result =
      service_->handle_control(std::string_view(body.c_str(), body.length()));
  server_.send(result.notify ? 200 : 204, "application/json",
               result.json.c_str());
}

void WifiTransport::handle_page() {
  last_request_at_ = millis();
  if (service_ == nullptr || !server_.hasArg("book") ||
      !server_.hasArg("n") || !server_.hasArg("crc32")) {
    server_.send(400, "application/json", R"({"ok":false,"err":"args"})");
    return;
  }
  const String body = server_.arg("plain");
  const std::string command =
      std::string(R"({"op":"begin_page","book":")") +
      "\"" + server_.arg("book").c_str() + "\",\"n\":" +
      server_.arg("n").c_str() + ",\"len\":" +
      std::to_string(body.length()) + ",\"crc32\":" +
      server_.arg("crc32").c_str() + "}";
  const ServiceResult begin = service_->handle_control(command);
  if (begin.notify) {
    server_.send(400, "application/json", begin.json.c_str());
    return;
  }
  const ServiceResult result = service_->handle_data(
      reinterpret_cast<const std::uint8_t*>(body.c_str()), body.length());
  server_.send(result.json.find(R"("ok":true)") != std::string::npos ? 200
                                                                    : 400,
               "application/json", result.json.c_str());
}

void WifiTransport::stop() {
  server_.stop();
  WiFi.softAPdisconnect(true);
  active_ = false;
  disable_pending_ = false;
}

}  // namespace pocket

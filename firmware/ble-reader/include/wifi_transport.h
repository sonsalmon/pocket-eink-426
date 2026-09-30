#pragma once

#include <WebServer.h>

#include <string>

#include "core/protocol_service.h"

namespace pocket {

class WifiTransport final : public WifiControl {
 public:
  WifiTransport();

  void attach(ProtocolService& service);
  WifiInfo enable() override;
  void disable() override;
  void loop();

 private:
  void handle_op();
  void handle_page();
  void stop();

  WebServer server_;
  ProtocolService* service_ = nullptr;
  WifiInfo info_;
  unsigned long last_request_at_ = 0;
  bool active_ = false;
  bool disable_pending_ = false;
};

}  // namespace pocket

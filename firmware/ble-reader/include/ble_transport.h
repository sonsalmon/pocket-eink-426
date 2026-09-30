#pragma once

#include <NimBLEDevice.h>

#include <string>

#include "core/protocol_service.h"

namespace pocket {

class BleTransport {
 public:
  explicit BleTransport(ProtocolService& service);

  void begin();
  bool connected() const;
  void notify_json(const std::string& json);

 private:
  class ControlCallbacks;
  class DataCallbacks;
  class ServerCallbacks;

  void notify(const ServiceResult& result);

  ProtocolService& service_;
  NimBLECharacteristic* control_ = nullptr;
  bool connected_ = false;
  ControlCallbacks* control_callbacks_ = nullptr;
  DataCallbacks* data_callbacks_ = nullptr;
  ServerCallbacks* server_callbacks_ = nullptr;
};

}  // namespace pocket

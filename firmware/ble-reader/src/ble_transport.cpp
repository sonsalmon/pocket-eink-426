#include "ble_transport.h"

#include <string>

namespace pocket {
namespace {

constexpr char kServiceUuid[] = "7b1e0001-8f4c-4d6a-9c3e-2f5a6b7c8d90";
constexpr char kControlUuid[] = "7b1e0002-8f4c-4d6a-9c3e-2f5a6b7c8d90";
constexpr char kDataUuid[] = "7b1e0003-8f4c-4d6a-9c3e-2f5a6b7c8d90";

}  // namespace

class BleTransport::ControlCallbacks final
    : public NimBLECharacteristicCallbacks {
 public:
  explicit ControlCallbacks(BleTransport& transport) : transport_(transport) {}

  void onWrite(NimBLECharacteristic* characteristic,
               NimBLEConnInfo& connection) override {
    (void)connection;
    const std::string value = characteristic->getValue();
    transport_.notify(transport_.service_.handle_control(value));
  }

 private:
  BleTransport& transport_;
};

class BleTransport::DataCallbacks final : public NimBLECharacteristicCallbacks {
 public:
  explicit DataCallbacks(BleTransport& transport) : transport_(transport) {}

  void onWrite(NimBLECharacteristic* characteristic,
               NimBLEConnInfo& connection) override {
    (void)connection;
    const std::string value = characteristic->getValue();
    transport_.notify(transport_.service_.handle_data(
        reinterpret_cast<const std::uint8_t*>(value.data()), value.size()));
  }

 private:
  BleTransport& transport_;
};

class BleTransport::ServerCallbacks final : public NimBLEServerCallbacks {
 public:
  explicit ServerCallbacks(BleTransport& transport) : transport_(transport) {}

  void onConnect(NimBLEServer* server, NimBLEConnInfo& connection) override {
    (void)server;
    (void)connection;
    transport_.connected_ = true;
  }

  void onDisconnect(NimBLEServer* server, NimBLEConnInfo& connection,
                    const int reason) override {
    (void)server;
    (void)connection;
    (void)reason;
    transport_.connected_ = false;
    transport_.service_.abort_transfer();
    NimBLEDevice::startAdvertising();
  }

 private:
  BleTransport& transport_;
};

BleTransport::BleTransport(ProtocolService& service) : service_(service) {}

void BleTransport::begin() {
  NimBLEDevice::init("Pocket426");
  NimBLEDevice::setMTU(517);

  NimBLEServer* server = NimBLEDevice::createServer();
  server_callbacks_ = new ServerCallbacks(*this);
  server->setCallbacks(server_callbacks_);

  NimBLEService* service = server->createService(kServiceUuid);
  control_ = service->createCharacteristic(
      kControlUuid, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);
  NimBLECharacteristic* data =
      service->createCharacteristic(kDataUuid, NIMBLE_PROPERTY::WRITE_NR);

  control_callbacks_ = new ControlCallbacks(*this);
  data_callbacks_ = new DataCallbacks(*this);
  control_->setCallbacks(control_callbacks_);
  data->setCallbacks(data_callbacks_);
  service->start();

  NimBLEAdvertising* advertising = NimBLEDevice::getAdvertising();
  advertising->addServiceUUID(kServiceUuid);
  advertising->setName("Pocket426");
  advertising->start();
}

bool BleTransport::connected() const { return connected_; }

void BleTransport::notify_json(const std::string& json) {
  if (control_ == nullptr) {
    return;
  }
  control_->setValue(json);
  control_->notify();
}

void BleTransport::notify(const ServiceResult& result) {
  if (!result.notify || control_ == nullptr) {
    return;
  }
  notify_json(result.json);
}

}  // namespace pocket

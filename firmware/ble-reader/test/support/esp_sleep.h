#pragma once

#include <cstdint>

constexpr int ESP_GPIO_WAKEUP_GPIO_LOW = 0;
constexpr int ESP_SLEEP_WAKEUP_GPIO = 0;
constexpr int ESP_SLEEP_WAKEUP_TIMER = 1;
inline void esp_deep_sleep_enable_gpio_wakeup(std::uint64_t, int) {}
inline void esp_deep_sleep_start() {}
inline void esp_sleep_enable_gpio_wakeup() {}
inline void esp_sleep_enable_timer_wakeup(int) {}
inline void esp_light_sleep_start() {}
inline void esp_sleep_disable_wakeup_source(int) {}

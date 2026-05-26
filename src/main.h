#pragma once

#include <FreeRTOS.h>
#include <timers.h>
#include <nrfx_gpiote.h>
#include <atomic>
#include <cstdint>

void nrfx_gpiote_evt_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action);
void DebounceTimerCallback(TimerHandle_t xTimer);

extern std::atomic<uint32_t> mallocFailedCount;
extern std::atomic<uint32_t> stackOverflowCount;
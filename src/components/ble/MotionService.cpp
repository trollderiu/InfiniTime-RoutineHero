#include "components/ble/MotionService.h"
#include "components/motion/MotionController.h"
#include "components/ble/NimbleController.h"
#include <nrf_log.h>

using namespace Pinetime::Controllers;

namespace {
  // 0003yyxx-78fc-48fe-8e23-433b3a1942d0
  constexpr ble_uuid128_t CharUuid(uint8_t x, uint8_t y) {
    return ble_uuid128_t {.u = {.type = BLE_UUID_TYPE_128},
                          .value = {0xd0, 0x42, 0x19, 0x3a, 0x3b, 0x43, 0x23, 0x8e, 0xfe, 0x48, 0xfc, 0x78, x, y, 0x03, 0x00}};
  }

  // 00030000-78fc-48fe-8e23-433b3a1942d0
  constexpr ble_uuid128_t BaseUuid() {
    return CharUuid(0x00, 0x00);
  }

  constexpr ble_uuid128_t motionServiceUuid {BaseUuid()};
  constexpr ble_uuid128_t stepCountCharUuid {CharUuid(0x01, 0x00)};
  constexpr ble_uuid128_t motionValuesCharUuid {CharUuid(0x02, 0x00)};
  // constexpr ble_uuid128_t totalSleepTimeCharUuid {CharUuid(0x03, 0x00)};
  constexpr ble_uuid128_t simplifiedMotionCharUuid {CharUuid(0x04, 0x00)};

  int MotionServiceCallback(uint16_t /*conn_handle*/, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt, void* arg) {
    auto* motionService = static_cast<MotionService*>(arg);
    return motionService->OnStepCountRequested(attr_handle, ctxt);
  }
}

// TODO Refactoring - remove dependency to SystemTask
MotionService::MotionService(NimbleController& nimble, Controllers::MotionController& motionController)
  : nimble {nimble},
    motionController {motionController},
    characteristicDefinition {{.uuid = &stepCountCharUuid.u,
                               .access_cb = MotionServiceCallback,
                               .arg = this,
                               .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
                               .val_handle = &stepCountHandle},
                              {.uuid = &motionValuesCharUuid.u,
                               .access_cb = MotionServiceCallback,
                               .arg = this,
                               .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
                               .val_handle = &motionValuesHandle},
                              {.uuid = &simplifiedMotionCharUuid.u,
                               .access_cb = MotionServiceCallback,
                               .arg = this,
                               .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
                               .val_handle = &simplifiedMotionHandle},
                              {0}},
    serviceDefinition {
      {.type = BLE_GATT_SVC_TYPE_PRIMARY, .uuid = &motionServiceUuid.u, .characteristics = characteristicDefinition},
      {0},
    } {

  // motionData[0] = 0;
  for (int i = 0; i < 300; i++)
    motionData[i] = 0;

  // TODO refactor to prevent this loop dependency (service depends on controller and controller depends on service)
  motionController.SetService(this);
}

void MotionService::Init() {
  int res = 0;
  res = ble_gatts_count_cfg(serviceDefinition);
  ASSERT(res == 0);

  res = ble_gatts_add_svcs(serviceDefinition);
  ASSERT(res == 0);
}

int MotionService::OnStepCountRequested(uint16_t attributeHandle, ble_gatt_access_ctxt* context) {
  if (attributeHandle == stepCountHandle) {
    NRF_LOG_INFO("Motion-stepcount : handle = %d", stepCountHandle);
    uint32_t buffer = motionController.NbSteps();

    int res = os_mbuf_append(context->om, &buffer, 4);
    return (res == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
  } else if (attributeHandle == motionValuesHandle) {
    int16_t buffer[3] = {motionController.X(), motionController.Y(), motionController.Z()};

    int res = os_mbuf_append(context->om, buffer, 3 * sizeof(int16_t));
    return (res == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
  } else if (attributeHandle == simplifiedMotionHandle) {
    int res = os_mbuf_append(context->om, motionData, 300 * sizeof(uint16_t));
    return (res == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
  }
  return 0;
}

void MotionService::OnNewStepCountValue(uint32_t stepCount) {
  if (!stepCountNoficationEnabled)
    return;

  uint32_t buffer = stepCount;
  auto* om = ble_hs_mbuf_from_flat(&buffer, 4);

  uint16_t connectionHandle = nimble.connHandle();

  if (connectionHandle == 0 || connectionHandle == BLE_HS_CONN_HANDLE_NONE) {
    return;
  }

  ble_gattc_notify_custom(connectionHandle, stepCountHandle, om);
}

void MotionService::OnDeltaMotionValues(int16_t x, int16_t y, int16_t z, uint8_t hours, uint8_t minutes) {
  // if (x || y || z || hours || minutes)
  //   return;

  // if (!simplifiedMotionNotificationEnabled)
  //   return;

  // EVERY 5 MIN INDEX
  uint16_t now = ((hours + 5) % 24) * 12 + minutes / 5;

  // if (now >= 288) now = 287;

  // RESET:
  // if (current > now) {
  //   current = now;
  //   for (uint16_t i = 0; i < 288; i++)
  //     motionData[i] = 0;
  // }
  // for (uint16_t i = now + 1; i < 288; i++) {
  //   if (motionData[i] > 0) {
  //     for (uint16_t j = 0; j < 288; ++j)
  //       motionData[j] = 0;
  //     // std::fill(std::begin(motionData), std::end(motionData), 0);
  //     break;
  //   }
  // }

  // UPDATE INDEX
  // if (current != now) {
  //   current = now;
  // }

  if (0 != now)
    now = 0;

  motionData[now] += std::abs(x) + std::abs(y) + std::abs(z);
}

void MotionService::OnNewMotionValues(int16_t x, int16_t y, int16_t z) {
  if (!motionValuesNoficationEnabled)
    return;

  int16_t buffer[3] = {x, y, z};
  auto* om = ble_hs_mbuf_from_flat(buffer, 3 * sizeof(int16_t));

  uint16_t connectionHandle = nimble.connHandle();

  if (connectionHandle == 0 || connectionHandle == BLE_HS_CONN_HANDLE_NONE) {
    return;
  }

  ble_gattc_notify_custom(connectionHandle, motionValuesHandle, om);
}

void MotionService::SubscribeNotification(uint16_t attributeHandle) {
  if (attributeHandle == stepCountHandle)
    stepCountNoficationEnabled = true;
  else if (attributeHandle == motionValuesHandle)
    motionValuesNoficationEnabled = true;
  // else if (attributeHandle == simplifiedMotionHandle)
  //   simplifiedMotionNotificationEnabled = true;
}

void MotionService::UnsubscribeNotification(uint16_t attributeHandle) {
  if (attributeHandle == stepCountHandle)
    stepCountNoficationEnabled = false;
  else if (attributeHandle == motionValuesHandle)
    motionValuesNoficationEnabled = false;
  // else if (attributeHandle == simplifiedMotionHandle)
  //   simplifiedMotionNotificationEnabled = false;
}

bool MotionService::IsMotionNotificationSubscribed() const {
  return motionValuesNoficationEnabled;
}

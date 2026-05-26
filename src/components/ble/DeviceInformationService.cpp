#include "components/ble/DeviceInformationService.h"
// #include "components/firmwarevalidator/FirmwareValidator.h"

using namespace Pinetime::Controllers;

constexpr ble_uuid16_t DeviceInformationService::manufacturerNameUuid;
constexpr ble_uuid16_t DeviceInformationService::modelNumberUuid;
constexpr ble_uuid16_t DeviceInformationService::serialNumberUuid;
constexpr ble_uuid16_t DeviceInformationService::fwRevisionUuid;
constexpr ble_uuid16_t DeviceInformationService::deviceInfoUuid;
constexpr ble_uuid16_t DeviceInformationService::hwRevisionUuid;
constexpr ble_uuid16_t DeviceInformationService::swRevisionUuid;
constexpr ble_uuid16_t DeviceInformationService::firmwareVerifiedUuid;

int DeviceInformationCallback(uint16_t /*conn_handle*/, uint16_t /*attr_handle*/, struct ble_gatt_access_ctxt* ctxt, void* arg) {
  auto deviceInformationService = static_cast<DeviceInformationService*>(arg);
  return deviceInformationService->OnDeviceInfoRequested(ctxt);
}
// int DeviceInformationCallback(uint16_t /*conn_handle*/, uint16_t /*attr_handle*/, struct ble_gatt_access_ctxt* ctxt, void* arg) {
//   auto deviceInformationService = static_cast<DeviceInformationService*>(arg);
//   auto uuid16 = ble_uuid_u16(ctxt->chr->uuid);
//   if (uuid16 == deviceInformationService->firmwareVerifiedId) {
//     return deviceInformationService->OnFirmwareVerifiedRequested(ctxt);
//   } else {
//     return deviceInformationService->OnDeviceInfoRequested(ctxt);
//   }
// }

void DeviceInformationService::Init() {
  int res = 0;
  res = ble_gatts_count_cfg(serviceDefinition);
  ASSERT(res == 0);

  res = ble_gatts_add_svcs(serviceDefinition);
  ASSERT(res == 0);
}

// int DeviceInformationService::OnDeviceInfoRequested(struct ble_gatt_access_ctxt* ctxt) {
//   const char* str;

//   switch (ble_uuid_u16(ctxt->chr->uuid)) {
//     case manufacturerNameId:
//       str = manufacturerName;
//       break;
//     case modelNumberId:
//       str = modelNumber;
//       break;
//     case serialNumberId:
//       str = serialNumber;
//       break;
//     case fwRevisionId:
//       str = fwRevision;
//       break;
//     case hwRevisionId:
//       str = hwRevision;
//       break;
//     case swRevisionId:
//       str = swRevision;
//       break;
//     default:
//       return BLE_ATT_ERR_UNLIKELY;
//   }

//   int res = os_mbuf_append(ctxt->om, str, strlen(str));
//   return (res == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
// }
int DeviceInformationService::OnDeviceInfoRequested(struct ble_gatt_access_ctxt* ctxt) {
  const char* str;

  switch (ble_uuid_u16(ctxt->chr->uuid)) {
    case manufacturerNameId:
      str = manufacturerName;
      break;
    case modelNumberId:
      str = modelNumber;
      break;
    case serialNumberId:
      str = serialNumber;
      break;
    case fwRevisionId:
      str = fwRevision;
      break;
    case hwRevisionId:
      str = hwRevision;
      break;
    case swRevisionId:
      str = swRevision;
      break;

    case firmwareVerifiedId: {
      // // Assuming FirmwareValidator instance accessible
      // Pinetime::Controllers::FirmwareValidator firmwareValidator;
      // bool verified = firmwareValidator.IsValidated();
      // // uint8_t verifiedByte = verified ? 1 : 0;
      // // if (ctxt->om == nullptr) {
      // //   return BLE_ATT_ERR_INSUFFICIENT_RES;
      // // }
      // str = verified ? "1" : "0";
      // int res = os_mbuf_append(ctxt->om, str, strlen(str));
      // return (res == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
      return 0;
    }
    // case firmwareVerifiedId:
    //   str = "true";  // or some valid string value representing firmware verification
    //   break;

    default:
      return BLE_ATT_ERR_UNLIKELY;
  }

  int res = os_mbuf_append(ctxt->om, str, strlen(str));
  return (res == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
}

// // int DeviceInformationService::OnFirmwareVerifiedRequested(struct ble_gatt_access_ctxt* ctxt) {
// //   Pinetime::Controllers::FirmwareValidator validator;
// //   bool isValid = validator.IsValidated();

// //   uint8_t verified = isValid ? 1 : 0;

// //   int rc = os_mbuf_append(ctxt->om, &verified, sizeof(verified));
// //   return (rc == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
// // }
// int DeviceInformationService::OnFirmwareVerifiedRequested(struct ble_gatt_access_ctxt* ctxt) {
//   uint8_t verified = 1; // always verified

//   int rc = os_mbuf_append(ctxt->om, &verified, sizeof(verified));
//   return (rc == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
// }

DeviceInformationService::DeviceInformationService()
  : characteristicDefinition {{
                                .uuid = &manufacturerNameUuid.u,
                                .access_cb = DeviceInformationCallback,
                                .arg = this,
                                .flags = BLE_GATT_CHR_F_READ,
                              },
                              {
                                .uuid = &modelNumberUuid.u,
                                .access_cb = DeviceInformationCallback,
                                .arg = this,
                                .flags = BLE_GATT_CHR_F_READ,
                              },
                              {
                                .uuid = &serialNumberUuid.u,
                                .access_cb = DeviceInformationCallback,
                                .arg = this,
                                .flags = BLE_GATT_CHR_F_READ,
                              },
                              {
                                .uuid = &fwRevisionUuid.u,
                                .access_cb = DeviceInformationCallback,
                                .arg = this,
                                .flags = BLE_GATT_CHR_F_READ,
                              },
                              {
                                .uuid = &hwRevisionUuid.u,
                                .access_cb = DeviceInformationCallback,
                                .arg = this,
                                .flags = BLE_GATT_CHR_F_READ,
                              },
                              {
                                .uuid = &swRevisionUuid.u,
                                .access_cb = DeviceInformationCallback,
                                .arg = this,
                                .flags = BLE_GATT_CHR_F_READ,
                              },
                              {
                                .uuid = &firmwareVerifiedUuid.u,
                                .access_cb = DeviceInformationCallback,
                                .arg = this,
                                .flags = BLE_GATT_CHR_F_READ,
                              },
                              {0}},
    serviceDefinition {
      {/* Device Information Service */
       .type = BLE_GATT_SVC_TYPE_PRIMARY,
       .uuid = &deviceInfoUuid.u,
       .characteristics = characteristicDefinition},
      {0},
    } {
}

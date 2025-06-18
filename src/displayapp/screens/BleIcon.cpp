#include "displayapp/screens/BleIcon.h"
// #include "displayapp/screens/Symbols.h"
#include <lvgl/lvgl.h>

using namespace Pinetime::Applications::Screens;

const char* BleIcon::GetIcon(bool isConnected) {
  if (isConnected) {
    // return Symbols::bluetooth;
    return LV_SYMBOL_BLUETOOTH;
  }

  // return Symbols::none;
  return "";
}

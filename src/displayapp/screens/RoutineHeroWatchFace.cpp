#include "displayapp/screens/RoutineHeroWatchFace.h"
#include <cmath>
#include <lvgl/lvgl.h>
#include "displayapp/screens/BatteryIcon.h"
#include "displayapp/screens/BleIcon.h"
#include "displayapp/screens/Symbols.h"
#include "components/brightness/BrightnessController.h"
#include "components/settings/Settings.h"
#include "displayapp/InfiniTimeTheme.h"
#include "displayapp/icons/arrow/arrowicon.c"
#include "displayapp/icons/heart/hearticon.c"
#include "displayapp/DisplayApp.h"
#include <hal/nrf_gpio.h>   //nrf_gpio_pin_clear(PinMap::Motor);
#include "drivers/PinMap.h" //nrf_gpio_pin_clear(PinMap::Motor);
// #include <iostream>

using namespace Pinetime::Applications::Screens;

// namespace {
//   constexpr uint16_t CanvasBufferSize = 520;
//   static uint8_t canvasBuffer[CanvasBufferSize] = {0}; // (64+1) * 64 / 8
// }

const Numbers RoutineHeroWatchFace::numbers[12] = {
  {173, 30},
  {211, 64},
  {227, 120},
  {211, 176},
  {173, 211},
  {120, 226},
  {68, 211},
  {30, 176},
  {14, 120},
  {30, 64},
  {68, 30},
  {121, 14},
};

uint8_t* RoutineHeroWatchFace::cbuf;
// uint8_t* RoutineHeroWatchFace::cbuf = nullptr;

RoutineHeroWatchFace::RoutineHeroWatchFace(Controllers::DateTime& dateTimeController,
                                           const Controllers::Battery& batteryController,
                                           const Controllers::Ble& bleController,
                                           Controllers::Settings& settingsController,
                                           Controllers::BrightnessController& brightnessController,
                                           //  Pinetime::Controllers::FS& filesystem
                                           DisplayApp::States& state)
  : state {state},
    batteryIcon(true),
    dateTimeController {dateTimeController},
    batteryController {batteryController},
    bleController {bleController},
    settingsController {settingsController}, // filesystem {filesystem}
    brightnessController {brightnessController},
    currentDateTime {{}} {

  vibrationTimer = xTimerCreate("vibrationTimer", 1, pdFALSE, this, Vibrate);

  InitLvgl();

  DrawTime(dateTimeController.Hours(), dateTimeController.Minutes());

  taskRefresh = lv_task_create(RefreshTaskCallback, 500, LV_TASK_PRIO_MID, this);
}

// Definition of static function to set the loadScreen function
void RoutineHeroWatchFace::setLoadScreenFunction(RoutineHeroWatchFace::LoadScreenFunction func) {
  loadScreenFunc = func;
}

// Static member initialization
RoutineHeroWatchFace::LoadScreenFunction RoutineHeroWatchFace::loadScreenFunc = nullptr;

void RoutineHeroWatchFace::InitLvgl() {
  // RoutineHeroWatchFace::cbuf = canvasBuffer;

  dateTimeController.sAngle = -1;
  // dateTimeController.SetAngle(-1);
  dateTimeController.sSlice = 255;
  sHour = -1;
  sMin = -1;

  pie = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_size(pie, 240, 240);
  lv_obj_set_pos(pie, 0, 0);

  label_time = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(label_time, true);

  minor_scales = lv_linemeter_create(lv_scr_act(), nullptr);
  lv_linemeter_set_scale(minor_scales, 354, 60);
  lv_linemeter_set_angle_offset(minor_scales, 3);
  lv_obj_set_size(minor_scales, 240, 240);
  lv_obj_set_style_local_bg_opa(minor_scales, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_scale_width(minor_scales, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 4);
  lv_obj_set_style_local_scale_end_line_width(minor_scales, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 1);
  lv_obj_set_style_local_scale_end_color(minor_scales, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GAINSBORO);
  // lv_obj_set_click(minor_scales, false);

  major_scales = lv_linemeter_create(lv_scr_act(), nullptr);
  lv_linemeter_set_scale(major_scales, 330, 12);
  lv_linemeter_set_angle_offset(major_scales, 15);
  lv_obj_set_size(major_scales, 240, 240);
  lv_obj_align(major_scales, nullptr, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_bg_opa(major_scales, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_scale_width(major_scales, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_scale_end_line_width(major_scales, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 2);
  lv_obj_set_style_local_scale_end_color(major_scales, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  // lv_obj_set_click(major_scales, false);

  // major_line = lv_line_create(lv_scr_act(), nullptr);
  // lv_obj_set_style_local_line_color(major_line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GAINSBORO);
  // lv_obj_set_style_local_line_width(major_line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 1);
  // lv_obj_set_click(major_line, false);
  // major_line_points[0] = {119, 0};
  // major_line_points[1] = {119, 120};
  // lv_line_set_points(major_line, major_line_points, 2);

  for (i = 0; i < sizeof(numbers) / sizeof(numbers[0]); ++i) {
    lv_obj_t* number = lv_label_create(lv_scr_act(), nullptr);
    lv_label_set_text(number, std::to_string(i + 1).c_str());
    // lv_coord_t number_height2 = 12; // 23
    lv_obj_set_pos(number, numbers[i].x - lv_obj_get_width(number) / 2, numbers[i].y - lv_obj_get_height(number) / 2);
    lv_obj_set_style_local_text_color(number, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  }

  batteryIcon.Create(lv_scr_act());
  lv_obj_align(batteryIcon.GetObject(), nullptr, LV_ALIGN_IN_TOP_RIGHT, 0, 0);
  lv_obj_set_hidden(batteryIcon.GetObject(), true);

  plugIcon = lv_label_create(lv_scr_act(), nullptr);
  // lv_label_set_text_static(plugIcon, Symbols::plug);
  lv_label_set_text_static(plugIcon, LV_SYMBOL_CHARGE);
  lv_obj_align(plugIcon, nullptr, LV_ALIGN_IN_TOP_RIGHT, 0, 0);
  lv_obj_set_hidden(plugIcon, true);

  bleIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(bleIcon, "");
  lv_obj_align(bleIcon, nullptr, LV_ALIGN_IN_TOP_RIGHT, 30, 0);
  lv_obj_set_style_local_text_color(bleIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_SILVER);

  // mainIcon = lv_img_create(lv_scr_act(), nullptr);
  // lv_img_set_auto_size(mainIcon, false);
  // lv_obj_set_size(mainIcon, 24, 24);
  // lv_obj_align(mainIcon, nullptr, LV_ALIGN_IN_TOP_RIGHT, 0, 0);
  // lv_obj_set_style_local_image_recolor(mainIcon, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);

  canvas = lv_canvas_create(lv_scr_act(), nullptr);
  // lv_obj_set_click(canvas, false);
  lv_obj_set_hidden(canvas, true);
  if (cbuf == nullptr)
    cbuf = new uint8_t[520](); //(64+1) * 64 / 8
  lv_canvas_set_buffer(canvas, cbuf, CANVAS_WIDTH, CANVAS_WIDTH, LV_IMG_CF_INDEXED_1BIT);
  lv_canvas_set_palette(canvas, 0, LV_COLOR_TRANSP);
  lv_canvas_set_palette(canvas, 1, LV_COLOR_WHITE);
}

RoutineHeroWatchFace::~RoutineHeroWatchFace() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void RoutineHeroWatchFace::Refresh() {

  // ICONS UPDATE
  if (state != DisplayApp::States::Idle) {
    isCharging = batteryController.IsCharging();
    if (isCharging.IsUpdated()) {
      if (isCharging.Get()) {
        lv_obj_set_hidden(batteryIcon.GetObject(), true);
        lv_obj_set_hidden(plugIcon, false);
      } else {
        lv_obj_set_hidden(batteryIcon.GetObject(), false);
        lv_obj_set_hidden(plugIcon, true);
        SetBatteryIcon();
      }
    }
    if (!isCharging.Get()) {
      batteryPercentRemaining = batteryController.PercentRemaining();
      if (batteryPercentRemaining.IsUpdated()) {
        SetBatteryIcon();
      }
    }

    bleState = bleController.IsConnected();
    if (bleState.IsUpdated()) {
      if (bleState.Get()) {
        // lv_label_set_text_static(bleIcon, Symbols::bluetooth);
        lv_label_set_text_static(bleIcon, LV_SYMBOL_BLUETOOTH);
      } else {
        lv_label_set_text_static(bleIcon, "");
      }
    }
  }

  // STOP IF TIME NOT UPDATED
  currentDateTime = dateTimeController.CurrentDateTime();
  if (!currentDateTime.IsUpdated())
    return;

  uint8_t hour24 = dateTimeController.Hours();
  uint8_t min = dateTimeController.Minutes();
  uint16_t angle720 = hour24 * 12 + min / 5;

  if (angle720 == dateTimeController.sAngle)
    return;
  dateTimeController.sAngle = angle720;

  std::vector<Data> dataList;
  bool clocksFile = settingsController.LoadClocksFromFile(dataList);
  // std::cout << "dataList.size(): " << dataList.size() << std::endl;

  // Handle empty dataList with a Welcome
  if (dataList.empty()) {
    wasEmpty = true;

    // COMPENSATE NO ICON LOADED:
    brightnessController.Set(Controllers::BrightnessController::Levels::High);

    // lv_obj_set_hidden(canvas, true);
    lv_obj_clean(pie);

    // Create label
    if (!clocksFile) {
      lv_label_set_text_static(label_time, "Welcome!\nDownload & connect\nthe RoutineHero app!\n ");
    } else {
      lv_label_set_text_static(label_time, "Error\nCorrupted schedule\nUpdate the watch!\n ");
    }
    lv_obj_set_style_local_text_line_space(label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 9);
    lv_label_set_align(label_time, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(label_time, nullptr, LV_ALIGN_CENTER, 0, -20);

    // Make sure canvas is visible
    lv_obj_set_hidden(canvas, false);
    lv_canvas_fill_bg(canvas, LV_COLOR_BLACK, 0); // Invalid write of size 8
    lv_canvas_transform(canvas, (lv_img_dsc_t*) &hearticon, 0, LV_IMG_ZOOM_NONE, 0, 0, 64, 64, false);
    lv_obj_set_pos(canvas, 90, 140);

    return;
  }

  if (wasEmpty) {
    lv_obj_align(label_time, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    wasEmpty = false;
  }

  Controllers::DateTime::Days day_of_week = dateTimeController.DayOfWeek();

  std::vector<Data> result;
  for (const auto& data : dataList) {

    bool activeToday = false;

    switch (day_of_week) {
      case Controllers::DateTime::Days::Monday:
        activeToday = data.monday;
        break;
      case Controllers::DateTime::Days::Tuesday:
        activeToday = data.tuesday;
        break;
      case Controllers::DateTime::Days::Wednesday:
        activeToday = data.wednesday;
        break;
      case Controllers::DateTime::Days::Thursday:
        activeToday = data.thursday;
        break;
      case Controllers::DateTime::Days::Friday:
        activeToday = data.friday;
        break;
      case Controllers::DateTime::Days::Saturday:
        activeToday = data.saturday;
        break;
      case Controllers::DateTime::Days::Sunday:
        activeToday = data.sunday;
        break;
      default:
        // Optionally handle or ignore
        break;
    }

    if (activeToday) {
      result.push_back(data);
    }
  }

  // SCHEDULE DATA WITHOUT RESULTS
  // std::cout << "result.size(): " << result.size() << std::endl;
  if (result.size() == 0) {
    slices = dataList[0].slices;
  } else {
    slices = result[0].slices;
  }

  // std::cout << "slices.size(): " << slices.size() << std::endl;

  // SCHEDULE DATA WITHOUT SLICES
  if (slices.size() == 0)
    return;

  uint8_t sliceIndex = 255;
  for (i = 0; i < slices.size(); ++i) {
    if (angle720 >= slices[i].start && angle720 < slices[i].end) {
      sliceIndex = i;
      break;
    }
  }

  // VIBRATE ALTHOUGH IS IDLE BUT NOT IF THE SAME ANGLE
  asyncVibrate(angle720, sliceIndex);

  if (state == DisplayApp::States::Idle)
    return;

  DrawTime(hour24, min);

  slice = &slices[sliceIndex];

  if (0 == slice->icon || 1 == slice->icon) {
    brightnessController.Set(Controllers::BrightnessController::Levels::Low);
  } else {
    brightnessController.Set(Controllers::BrightnessController::Levels::High);
  }

  bool sleep = slice->icon == 0 || slice->icon == 1;
  bool black = slice->red == 0 && slice->green == 0 && slice->blue == 0;
  if (sleep && black && hour24 >= 12) {
    lv_obj_set_hidden(canvas, true);
    lv_obj_clean(pie);
    lv_obj_t* imgFlag = lv_img_create(pie, nullptr);
    // lv_obj_set_click(imgFlag, false);
    lv_img_set_auto_size(imgFlag, false);
    lv_obj_set_size(imgFlag, 24, 24);
    lv_img_set_src(imgFlag, "F:/images/1.bin");
    lv_obj_set_pos(imgFlag, 108, 108);
    lv_obj_set_style_local_image_recolor(imgFlag, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_SILVER);
    dateTimeController.sSlice = sliceIndex;
    return;
  }

  DrawPie(angle720, slices);
  CanvasReset(angle720 * 2.5);
  DrawArrow(angle720 * 2.5);
}

void RoutineHeroWatchFace::SetBatteryIcon() {
  auto batteryPercent = batteryPercentRemaining.Get();
  batteryIcon.SetBatteryPercentage(batteryPercent);
}

void RoutineHeroWatchFace::DrawTime(uint8_t hour, uint8_t min) {
  if (sHour != hour || sMin != min) {
    sHour = hour;
    sHour = min;
    lv_label_set_text_fmt(label_time, "#cccccc %02d:%02d", hour, min); //:::
  }
}

// angle720: Angle of the hour pointer from 0 to 720 (2 full circles)
void RoutineHeroWatchFace::DrawPie(int16_t angle720, std::vector<IntervalColor> slices) {

  // 360 OFFSET IF >12h
  uint8_t hour = dateTimeController.Hours();
  uint16_t offset = 0;
  if (hour >= 12) {
    offset = (uint16_t) 144;
  }

  // GET SLICES FOR THE CURRENT 12h CLOCK
  clockSlices = std::vector<IntervalColor>();
  for (i = 0; i < slices.size(); ++i) {
    if ((slices[i].start > offset && slices[i].start < offset + 144) || (slices[i].end > offset && slices[i].end < offset + 144)) {
      clockSlices.push_back(slices[i]);
    }
  }

  // GET CURRENT SLICE INDEX
  uint8_t sliceIndex = 254; //-2
  for (i = 0; i < clockSlices.size(); ++i) {
    if (angle720 >= clockSlices[i].start && angle720 < clockSlices[i].end) {
      sliceIndex = i;
      break;
    }
  }

  // UPDATE CLOCK IF CURRENT SLICE CHANGED
  if (dateTimeController.sSlice != sliceIndex) {
    dateTimeController.sSlice = sliceIndex;

    // CLEAN pie IF NOT FIRST SLICE LOADED
    // if (255 != dateTimeController.sSlice) //-1
    lv_obj_clean(pie);

    // DRAW SLICES:
    for (IntervalColor slice : clockSlices) {
      DrawSlice(slice, angle720);
    }

    // DRAW ICONS:
    for (IntervalColor slice : clockSlices) {
      DrawIcon(slice, angle720);
    }
  }
}

void RoutineHeroWatchFace::DrawSlice(IntervalColor slice, int16_t angle720) {
  if (slice.red == 0 && slice.green == 0 && slice.blue == 0)
    return;

  uint8_t hour = dateTimeController.Hours();
  uint16_t start = slice.start;
  uint16_t end = slice.end;

  // AFTERNOON START AT 12, PREVENT COLLISION WITH SLEEP TIME
  if (hour >= 12 && start < 144)
    start = 144;

  // IF BEFORE 6:00 DON'T SHOW SLICES TRASPASSING 12:00 (SHOW NIGHT)
  if (hour < 6 && end > 144)
    end = 144;

  if (hour >= 12 && end > 144) {
    start -= 144;
    end -= 144;
  }

  // uint can't handle negatives, do += 144 instead of -= 144
  if (start > 144) {
    start += 144;
    end += 144;
  }

  lv_obj_t* p = lv_arc_create(pie, nullptr);
  // lv_obj_set_click(p, false);

  if (angle720 >= slice.start && angle720 < slice.end && 0 != slice.icon) {
    lv_arc_set_radius(p, CANVAS_CENTER);
    color = lv_color_make((uint8_t) (slice.red * 0.9), (uint8_t) (slice.green * 0.9), (uint8_t) (slice.blue * 0.9));
  } else if (slice.end < angle720) {
    color = lv_color_make((uint8_t) (slice.red * 0.3), (uint8_t) (slice.green * 0.3), (uint8_t) (slice.blue * 0.3));
  } else {
    color = lv_color_make((uint8_t) (slice.red * 0.6), (uint8_t) (slice.green * 0.6), (uint8_t) (slice.blue * 0.6));
  }
  lv_obj_set_style_local_line_color(p, LV_ARC_PART_BG, LV_STATE_DEFAULT, color);

  lv_arc_set_bg_angles(p, start * 2.5, end * 2.5);
}

void RoutineHeroWatchFace::DrawIcon(IntervalColor slice, int16_t angle720) {
  if (slice.start == slice.end)
    return;

  icon = slice.icon;
  if (icon == 0) {
    // icon = 1;
    return;
  }

  uint8_t hour = dateTimeController.Hours();
  uint16_t start = slice.start;
  uint16_t end = slice.end;

  // AFTERNOON START AT 12, PREVENT COLLISION WITH SLEEP TIME
  if (hour >= 12 && start < 144)
    start = 144;

  // IF BEFORE 6:00 DON'T SHOW SLICES TRASPASSING 12:00 (SHOW NIGHT)
  if (hour < 6 && end > 144)
    end = 144;

  if (hour >= 12 && end > 144) {
    start -= 144;
    end -= 144;
  }

  // uint do not have negatives
  if (start > 144) {
    start += 144;
    end += 144;
  }

  bool black = slice.red == 0 && slice.green == 0 && slice.blue == 0;
  bool current = (angle720 >= slice.start && angle720 < slice.end) || (slices[dateTimeController.sSlice].icon == 0 && black);

  lv_color_t color = LV_COLOR_ICONS;
  if (current) {
    color = LV_COLOR_WHITE;
  } else if (slice.end < angle720) {
    color = LV_COLOR_GRAY;
  }

  lv_obj_t* imgFlag = lv_img_create(pie, nullptr);
  lv_obj_set_click(imgFlag, false);
  lv_img_set_auto_size(imgFlag, false);
  lv_obj_set_size(imgFlag, 24, 24);

  lv_img_set_src(imgFlag, ("F:/images/" + std::to_string(icon) + ".bin").c_str());

  position_image_on_circle(imgFlag, CANVAS_CENTER, CANVAS_CENTER, 81, (start + (end - start) / 2) * 2.5 - 90);

  lv_obj_set_style_local_image_recolor(imgFlag, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, color);
  // lv_obj_set_style_local_image_recolor_opa(imgFlag, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, 200);

  if (angle720 >= slice.start && angle720 < slice.end && 0 != slice.icon) {
    lv_obj_set_click(imgFlag, true);
    lv_obj_set_event_cb(imgFlag, lvEventCb);
  }
}

void RoutineHeroWatchFace::lvEventCb(lv_obj_t* obj, lv_event_t event) {
  if (loadScreenFunc)
    loadScreenFunc(obj, event);
}

void RoutineHeroWatchFace::CanvasReset(int16_t angle) {
  int8_t img_x, img_y;
  polar_to_cartesian(angle - 90, 28, &img_x, &img_y);
  lv_obj_set_pos(canvas, 88 + img_x, 88 + img_y);
  // lv_canvas_fill_bg(canvas, LV_COLOR_BLACK, 0);
}

void RoutineHeroWatchFace::DrawArrow(int16_t angle) {
  lv_canvas_fill_bg(canvas, LV_COLOR_BLACK, 0); // Invalid write of size 8
  lv_canvas_transform(canvas, (lv_img_dsc_t*) &arrowicon, angle * 10, LV_IMG_ZOOM_NONE, 24, 0, 8, 32, false);
  lv_obj_set_hidden(canvas, false);
}

void RoutineHeroWatchFace::position_image_on_circle(lv_obj_t* img, uint8_t center_x, uint8_t center_y, uint8_t radius, int16_t angle_deg) {
  int8_t img_x, img_y;
  polar_to_cartesian(angle_deg, radius, &img_x, &img_y);
  // lv_obj_set_pos(img, center_x + img_x - lv_obj_get_width(img) / 2, center_y + img_y - lv_obj_get_height(img) / 2);
  lv_obj_set_pos(img, center_x + img_x - 12, center_y + img_y - 12);
}

void RoutineHeroWatchFace::polar_to_cartesian(int16_t angle_deg, uint8_t radius, int8_t* x, int8_t* y) {
  float angle_rad = angle_deg * 0.017453293; // 3.14159265358979323846 / 180.0;
  *x = radius * cos(angle_rad);
  *y = radius * sin(angle_rad);
}

void RoutineHeroWatchFace::asyncVibrate(int16_t angle720, uint8_t sliceIndex) {

  if (254 == sliceIndex)
    return;
  if (255 == sliceIndex)
    return;
  if (0 == sliceIndex)
    return;

  // ONLY VIBRATE AT THE START OF THE SLICE
  if (angle720 != slices[sliceIndex].start)
    return;

  if (dateTimeController.sSlice < 254 && dateTimeController.sSlice != sliceIndex) {
    xTimerStart(vibrationTimer, 0);
  }
}

void RoutineHeroWatchFace::Vibrate(TimerHandle_t /* xTimer */) {
  nrf_gpio_pin_clear(PinMap::Motor);
  vTaskDelay(100);
  nrf_gpio_pin_set(PinMap::Motor);
  vTaskDelay(500);
  nrf_gpio_pin_clear(PinMap::Motor);
  vTaskDelay(100);
  nrf_gpio_pin_set(PinMap::Motor);
}

// void RoutineHeroWatchFace::LoadSettingsFromFile() {
//   ClockData bufferSettings;
//   lfs_file_t settingsFile;

//   if (filesystem.FileOpen(&settingsFile, "/settings.dat", LFS_O_RDONLY) != LFS_ERR_OK) {
//     return;
//   }
//   filesystem.FileRead(&settingsFile, reinterpret_cast<uint8_t*>(&bufferSettings), sizeof(settings));
//   filesystem.FileClose(&settingsFile);
// }

// void RoutineHeroWatchFace::SaveSettingsToFile() {
//   lfs_file_t settingsFile;

//   filesystem.FileDelete("/settings.dat");
//   if (filesystem.FileOpen(&settingsFile, "/settings.dat", LFS_O_WRONLY | LFS_O_CREAT) != LFS_ERR_OK) {
//     return;
//   }
//   settings.clock[1].midday = 1;
//   filesystem.FileWrite(&settingsFile, reinterpret_cast<uint8_t*>(&settings), sizeof(settings));
//   filesystem.FileClose(&settingsFile);
// }

// void RoutineHeroWatchFace::LoadSettingsFromFile() const {
//   // ClockData bufferSettings;
//   // lfs_file_t settingsFile;
//   // if (filesystem.FileOpen(&settingsFile, "/clocks.json", LFS_O_RDONLY) != LFS_ERR_OK) {
//   //   return;
//   // }
//   // filesystem.FileRead(&settingsFile, reinterpret_cast<uint8_t*>(&bufferSettings), sizeof(settings));
//   // filesystem.FileClose(&settingsFile);

//   // std::ifstream file("data.txt");
//   // std::string line;
//   // std::vector<Data> dataList;

//   // if (file.is_open()) {
//   //   while (std::getline(file, line)) {
//   //     if (!line.empty()) {
//   //       Data data = parseLine(line);
//   //       dataList.push_back(data);
//   //     }
//   //   }
//   //   file.close();
//   // } else {
//   //   printf("Unable to open file %d\n", 1);
//   //   return;
//   // }

//   std::string bufferSettings;
//   lfs_file_t settingsFile;
//   if (filesystem.FileOpen(&settingsFile, "/clocks.txt", LFS_O_RDONLY) != LFS_ERR_OK) {
//     return;
//   }
//   filesystem.FileRead(&settingsFile, &bufferSettings, 4294967295);
//   filesystem.FileClose(&settingsFile);

//   // Now parse bufferSettings into dataList
//   std::istringstream iss(bufferSettings);
//   std::string line;
//   while (std::getline(iss, line)) {
//     if (!line.empty()) {
//       Data data = parseLine(line); // Assuming parseLine function exists
//       dataList.push_back(data);
//     }
//   }

//   // Example output
//   for (const auto& data : dataList) {
//     printf("year %d\n", data.year);
//     printf("month %d\n", data.month);
//     printf("day %d\n", data.day);
//     printf("weekend %d\n", data.weekend);
//     printf("am_pm %d\n", data.am_pm);
//     for (const auto& slice : data.slices) {
//       for (int num : slice) {
//         printf("num %d\n", num);
//       }
//       printf("%d\n", 1);
//     }
//     printf("------------ %d\n", 1);
//   }
// }

// // void RoutineHeroWatchFace::SaveSettingsToFile() {
// //   lfs_file_t settingsFile;

// //   filesystem.FileDelete("/settings.dat");
// //   if (filesystem.FileOpen(&settingsFile, "/settings.dat", LFS_O_WRONLY | LFS_O_CREAT) != LFS_ERR_OK) {
// //     return;
// //   }
// //   settings.clock[1].midday = 1;
// //   filesystem.FileWrite(&settingsFile, reinterpret_cast<uint8_t*>(&settings), sizeof(settings));
// //   filesystem.FileClose(&settingsFile);
// // }

// std::vector<int> RoutineHeroWatchFace::parseSlice(const std::string& sliceStr) const {
//   std::vector<int> slice;
//   std::stringstream ss(sliceStr);
//   std::string number;
//   while (std::getline(ss, number, ',')) {
//     slice.push_back(std::stoi(number));
//   }
//   return slice;
// }

// std::vector<std::vector<int>> RoutineHeroWatchFace::parseSlices(const std::string& slicesStr) const {
//   std::vector<std::vector<int>> slices;
//   std::stringstream ss(slicesStr);
//   std::string sliceStr;
//   while (std::getline(ss, sliceStr, ';')) {
//     slices.push_back(parseSlice(sliceStr));
//   }
//   return slices;
// }

// Data RoutineHeroWatchFace::parseLine(const std::string& line) const {
//   Data data;
//   std::stringstream ss(line);
//   std::string part;

//   // Read year, month, day, weekend, am_pm
//   std::getline(ss, part, '|');
//   data.year = std::stoi(part);
//   std::getline(ss, part, '|');
//   data.month = std::stoi(part);
//   std::getline(ss, part, '|');
//   data.day = std::stoi(part);
//   std::getline(ss, part, '|');
//   data.weekend = std::stoi(part);
//   std::getline(ss, part, '|');
//   data.am_pm = std::stoi(part);

//   // Read slices
//   std::getline(ss, part, '|');
//   data.slices = parseSlices(part);

//   return data;
// }
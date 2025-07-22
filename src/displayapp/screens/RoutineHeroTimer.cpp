#include "displayapp/screens/RoutineHeroTimer.h"
#include <lvgl/lvgl.h>
#include <cstring>
#include <math.h> //sqrt
#include "displayapp/screens/BatteryIcon.h"
#include "displayapp/screens/BleIcon.h"
#include "displayapp/icons/arrow/arrowtimer.c"
#include "displayapp/InfiniTimeTheme.h"
#include "displayapp/DisplayApp.h" //state
#include <hal/nrf_gpio.h>          //nrf_gpio_pin_clear(PinMap::Motor);
#include "drivers/PinMap.h"        //nrf_gpio_pin_clear(PinMap::Motor);

using namespace Pinetime::Applications::Screens;

uint8_t* RoutineHeroTimer::cbuf;

const TimerNumbers RoutineHeroTimer::numbers[12] = {
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

RoutineHeroTimer::RoutineHeroTimer(Controllers::DateTime& dateTimeController,
                                   const Controllers::Battery& batteryController,
                                   const Controllers::Ble& bleController,
                                   Controllers::Settings& settingsController,
                                   Controllers::BrightnessController& brightnessController,
                                   DisplayApp::States& state)
  : state {state},
    batteryIcon(true),
    dateTimeController {dateTimeController},
    batteryController {batteryController},
    bleController {bleController},
    settingsController {settingsController},
    brightnessController {brightnessController},
    currentDateTime {{}} {
  vibrationTimer = xTimerCreate("vibrationTimer", 1, pdTRUE, this, Vibrate);

  init = false;
  sUpdate = -1;
  sHour = -1;
  sMin = -1;
  sSlice = 255;

  // red: 254,44,7
  // orange: 255,126,2
  // yellow: 255,199,0
  // green: 91,185,1
  // cyan: 1,207,255
  // blue: 2,113,255
  // purple: 128,66,209

  //

  pie = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_size(pie, 240, 240);
  lv_obj_set_pos(pie, 0, 0);

  // p2 = lv_arc_create(pie, nullptr);
  // lv_arc_set_bg_angles(p2, 270, 90);
  // lv_obj_set_hidden(p2, true);
  pDone = lv_arc_create(pie, nullptr);
  lv_arc_set_radius(pDone, CANVAS_CENTER);
  lv_obj_set_hidden(pDone, true);

  p = lv_arc_create(pie, nullptr);
  lv_arc_set_radius(p, CANVAS_CENTER);

  minor_scales = lv_linemeter_create(lv_scr_act(), nullptr);
  lv_linemeter_set_scale(minor_scales, 354, 60);
  lv_linemeter_set_angle_offset(minor_scales, 3);
  lv_obj_set_size(minor_scales, 240, 240);
  lv_obj_set_style_local_bg_opa(minor_scales, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_scale_width(minor_scales, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 4);
  lv_obj_set_style_local_scale_end_line_width(minor_scales, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 1);
  lv_obj_set_style_local_scale_end_color(minor_scales, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GAINSBORO);
  lv_obj_set_click(minor_scales, false);

  major_scales = lv_linemeter_create(lv_scr_act(), nullptr);
  lv_linemeter_set_scale(major_scales, 330, 12);
  lv_linemeter_set_angle_offset(major_scales, 15);
  lv_obj_set_size(major_scales, 240, 240);
  lv_obj_align(major_scales, nullptr, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_bg_opa(major_scales, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_scale_width(major_scales, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_scale_end_line_width(major_scales, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 2);
  lv_obj_set_style_local_scale_end_color(major_scales, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_obj_set_click(major_scales, false);

  major_line = lv_line_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_line_color(major_line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GAINSBORO);
  lv_obj_set_style_local_line_width(major_line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 1);
  lv_obj_set_click(major_line, false);
  major_line_points[0] = {119, 0};
  major_line_points[1] = {119, 120};
  lv_line_set_points(major_line, major_line_points, 2);
  lv_obj_set_hidden(major_line, true);

  for (i = 0; i < sizeof(numbers) / sizeof(numbers[0]) - 1; ++i) {
    lv_obj_t* number = lv_label_create(lv_scr_act(), nullptr);
    uint8_t n = (i + 1) * 5;
    if (n == 60)
      n = 0;
    lv_label_set_text(number, std::to_string(n).c_str());
    lv_obj_set_pos(number, 240 - numbers[i].x - lv_obj_get_width(number) / 2, numbers[i].y - lv_obj_get_height(number) / 2);
    lv_obj_set_style_local_text_color(number, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_text_font(number, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_12);
  }

  number0 = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(number0, "0");
  lv_obj_set_pos(number0, numbers[11].x - lv_obj_get_width(number0) / 2, numbers[11].y - lv_obj_get_height(number0) / 2);
  lv_obj_set_style_local_text_color(number0, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_obj_set_style_local_text_font(number0, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_12);
  lv_obj_set_hidden(number0, true);

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

  //

  label_time = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(label_time, "");
  lv_label_set_recolor(label_time, true);

  currentIcon = lv_img_create(lv_scr_act(), nullptr);
  // lv_obj_set_hidden(currentIcon, true);
  lv_img_set_auto_size(currentIcon, false);
  lv_obj_set_size(currentIcon, 24, 24);

  nextIcon = lv_img_create(lv_scr_act(), nullptr);
  // lv_obj_set_hidden(nextIcon, true);
  lv_img_set_auto_size(nextIcon, false);
  lv_obj_set_size(nextIcon, 24, 24);
  lv_obj_set_style_local_image_recolor(nextIcon, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
  position_image_on_circle(nextIcon, CANVAS_CENTER, CANVAS_CENTER, 65, 22 - 90);

  canvas = lv_canvas_create(lv_scr_act(), nullptr);
  lv_obj_set_click(canvas, false);
  lv_obj_set_hidden(canvas, true);
  if (cbuf == nullptr)
    cbuf = new uint8_t[520](); //(64+1) * 64 / 8
  lv_canvas_set_buffer(canvas, cbuf, CANVAS_WIDTH, CANVAS_WIDTH, LV_IMG_CF_INDEXED_1BIT);
  lv_canvas_set_palette(canvas, 0, LV_COLOR_TRANSP);
  lv_canvas_set_palette(canvas, 1, LV_COLOR_GAINSBORO);
  lv_obj_set_pos(canvas, 88, 88);

  line = lv_line_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_line_color(line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
  lv_obj_set_style_local_line_width(line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 4);
  lv_obj_set_click(line, false);
  lv_obj_set_hidden(line, true);

  taskRefresh = lv_task_create(RefreshTaskCallback, 500, LV_TASK_PRIO_MID, this);
}

RoutineHeroTimer::~RoutineHeroTimer() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void RoutineHeroTimer::Refresh() {

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
  uint16_t angle720 = hour24 * 30 + min / 2;

  Controllers::DateTime::Days day_of_week = dateTimeController.DayOfWeek();

  settingsController.LoadClocksFromFile(dataList);

  if (dataList.empty()) {
    return;
  }

  result = std::vector<Data>();
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

  if (result.size() == 0) {
    slices = dataList[0].slices;
  } else {
    slices = result[0].slices;
  }

  uint8_t sliceIndex = 255;
  for (i = 0; i < slices.size(); ++i) {
    if (angle720 >= slices[i].start * 2.5 && angle720 < slices[i].end * 2.5) {
      sliceIndex = i;
      break;
    }
  }

  asyncVibrate(sliceIndex);

  if (state == DisplayApp::States::Idle)
    return;

  DrawTime(hour24, min);

  uint16_t currSec10 = (hour24 * 3600 + min * 60 + dateTimeController.Seconds()) / 10;
  if (currSec10 == sUpdate)
    return;
  sUpdate = currSec10;

  uint8_t icon = 0;
  uint16_t start = 0;
  uint16_t end = slices[0].start;
  bool black = true;

  if (sliceIndex < slices.size()) {
    slice = &slices[sliceIndex];
    icon = slice->icon;
    start = slice->start;
    end = slice->end;
    black = slice->red == 0 && slice->green == 0 && slice->blue == 0;
  }

  if (0 == icon || 1 == icon) {
    brightnessController.Set(Controllers::BrightnessController::Levels::Low);
  } else {
    brightnessController.Set(Controllers::BrightnessController::Levels::High);
  }

  // secondsLeft = (slice.end * 120) - s;
  // angleLeft = secondsLeft / 10;
  angleLeft = (end * 30) - currSec10;

  if (black && (hour24 > 12 || angleLeft >= 360)) { // SLEEP ICON AT THE CENTER, HIDE EVERITHING ELSE
    lv_obj_set_hidden(p, true);
    lv_obj_set_hidden(pDone, true);
    lv_obj_set_hidden(canvas, true);
    lv_obj_set_hidden(line, true);
    lv_obj_set_hidden(major_line, true);
    lv_obj_set_hidden(number0, false);
    lv_obj_set_hidden(nextIcon, true);
    lv_img_set_src(currentIcon, "F:/images/1.bin");
    lv_obj_set_pos(currentIcon, 108, 108);
    lv_obj_set_style_local_image_recolor(currentIcon, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_SILVER);
    return;
  }

  // ELSE
  lv_obj_set_style_local_image_recolor(currentIcon, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_obj_set_hidden(p, false);
  lv_obj_set_hidden(canvas, false);
  lv_obj_set_hidden(line, false);
  lv_obj_set_hidden(major_line, false);
  lv_obj_set_hidden(number0, true);

  angleTotal = (end - start) * 30;
  if (angleTotal <= 315 && angleTotal > angleLeft) {
    DrawSlice2(angleLeft, angleTotal, sliceIndex);
    lv_obj_set_hidden(pDone, false);
  } else {
    lv_obj_set_hidden(pDone, true);
  }

  DrawArrow(angleLeft);
  DrawSlice(angleLeft, sliceIndex);
  DrawIcon(angleLeft, sliceIndex);
}

void RoutineHeroTimer::asyncVibrate(uint8_t sliceIndex) {
  if (sSlice < 254 && sSlice != sliceIndex) {
    xTimerStart(vibrationTimer, 0);
  }
}

void RoutineHeroTimer::Vibrate(TimerHandle_t /* xTimer */) {
  nrf_gpio_pin_clear(PinMap::Motor);
  vTaskDelay(100);
  nrf_gpio_pin_set(PinMap::Motor);
  vTaskDelay(500);
  nrf_gpio_pin_clear(PinMap::Motor);
  vTaskDelay(100);
  nrf_gpio_pin_set(PinMap::Motor);
}

void RoutineHeroTimer::DrawSlice(uint16_t angle, uint8_t sliceIndex) {

  // .h VARIABLE DECLARATION TO PREVENT OVERRIDE?
  uint8_t red = 0;
  uint8_t green = 0;
  uint8_t blue = 0;

  if (sliceIndex < slices.size()) {
    red = slices[sliceIndex].red;
    green = slices[sliceIndex].green;
    blue = slices[sliceIndex].blue;
  }

  if (angle < 360 && 0 == red && 0 == green && 0 == blue) {
    red = 50;
    green = 50;
    blue = 50;
  }

  currColor = lv_color_make((uint8_t) (red * 0.9), (uint8_t) (green * 0.9), (uint8_t) (blue * 0.9));
  lv_obj_set_style_local_line_color(p, LV_ARC_PART_BG, LV_STATE_DEFAULT, currColor);

  if (angle > 359) {
    angle = 359;
  }

  if (angle < 1) {
    angle = 1;
  }

  lv_arc_set_bg_angles(p, 360 - angle, 0);
}

void RoutineHeroTimer::DrawSlice2(uint16_t angle1, uint16_t angle2, uint8_t sliceIndex) {
  if (sliceIndex >= slices.size())
    return;
  slice2 = &slices[sliceIndex];
  currColor2 = lv_color_make((uint8_t) (slice2->red * 0.4), (uint8_t) (slice2->green * 0.4), (uint8_t) (slice2->blue * 0.4));
  lv_obj_set_style_local_line_color(pDone, LV_ARC_PART_BG, LV_STATE_DEFAULT, currColor2);
  lv_arc_set_bg_angles(pDone, 360 - angle2, 360 - angle1);
}

void RoutineHeroTimer::DrawIcon(uint16_t angle, uint8_t sliceIndex) {

  // NEXT ACTIVITY ICON
  if (angle < 315) {
    uint8_t nextIndex = sliceIndex + 1; // ALLOW FROM 255 TO 0

    // IF NEXT SLICE NOT EXISTS (LAST SLICE), SHOW SLEEP
    nextIconIndex = 1;
    if (nextIndex < slices.size())
      nextIconIndex = slices[nextIndex].icon;

    lv_img_set_src(nextIcon, ("F:/images/" + std::to_string(nextIconIndex) + ".bin").c_str());
    lv_obj_set_hidden(nextIcon, false);
  } else {
    lv_obj_set_hidden(nextIcon, true);
  }

  // CURRENT ACTIVITY ICON
  uint8_t currentIconIndex = 1;
  if (sliceIndex < slices.size())
    currentIconIndex = slices[sliceIndex].icon;
  if (0 == currentIconIndex)
    currentIconIndex = 1;
  lv_img_set_src(currentIcon, ("F:/images/" + std::to_string(currentIconIndex) + ".bin").c_str());

  if (angle >= 360) {
    angle = 360;
  } else if (angle > 180) {
    angle = 180;
  } else if (angle < 44) {
    angle = 44;
  }

  position_image_on_circle(currentIcon, CANVAS_CENTER, CANVAS_CENTER, 65, -angle / 2 - 90);
}

void RoutineHeroTimer::DrawArrow(int16_t angle) {
  if (angle > 360)
    angle = 360;
  lv_canvas_fill_bg(canvas, LV_COLOR_BLACK, 0); // Invalid write of size 8
  lv_canvas_transform(canvas, (lv_img_dsc_t*) &arrowtimer, (-angle + 45) * 10, LV_IMG_ZOOM_NONE, 8, 8, 24, 24, false);

  int8_t x0, y0;
  polar_to_cartesian(-angle - 90, 5, &x0, &y0);
  line_points[0] = {(lv_coord_t) (x0 + 120), (lv_coord_t) (y0 + 120)};
  int8_t x, y;
  polar_to_cartesian(-angle - 90, 26, &x, &y);
  line_points[1] = {(lv_coord_t) (x + 120), (lv_coord_t) (y + 120)};
  lv_line_set_points(line, line_points, 2);
}

void RoutineHeroTimer::polar_to_cartesian(int16_t angle_deg, uint8_t radius, int8_t* x, int8_t* y) {
  float angle_rad = angle_deg * 0.017453293; // 3.14159265358979323846 / 180.0;
  *x = radius * cos(angle_rad);
  *y = radius * sin(angle_rad);
}

void RoutineHeroTimer::position_image_on_circle(lv_obj_t* img, uint8_t center_x, uint8_t center_y, uint8_t radius, int16_t angle_deg) {
  int8_t img_x, img_y;
  polar_to_cartesian(angle_deg, radius, &img_x, &img_y);
  // lv_obj_set_pos(img, center_x + img_x - lv_obj_get_width(img) / 2, center_y + img_y - lv_obj_get_height(img) / 2);
  lv_obj_set_pos(img, center_x + img_x - 12, center_y + img_y - 12);
}

void RoutineHeroTimer::DrawTime(uint8_t hour, uint8_t min) {
  if (sHour != hour || sMin != min) {
    sHour = hour;
    sHour = min;
    lv_label_set_text_fmt(label_time, "#cccccc %02d:%02d", hour, min);
  }
}

void RoutineHeroTimer::SetBatteryIcon() {
  auto batteryPercent = batteryPercentRemaining.Get();
  batteryIcon.SetBatteryPercentage(batteryPercent);
}
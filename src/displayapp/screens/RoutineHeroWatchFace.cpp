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
#include "displayapp/icons/routinehero/routinehero.c"
#include "displayapp/DisplayApp.h"

#include "displayapp/icons/routinehero/play_store_badge.c"
#include "displayapp/icons/routinehero/apple_store_badge.c"
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

  this->clocksFile = settingsController.LoadClocksFromFile(this->dataList);

  InitLvgl();

  DrawTime(dateTimeController.Hours(), dateTimeController.Minutes());
  DrawName();

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

  label_name = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(label_name, "");
  lv_obj_set_auto_realign(label_name, true);
  lv_obj_align(label_name, nullptr, LV_ALIGN_IN_BOTTOM_LEFT, 0, 1);
  lv_obj_set_style_local_text_color(label_name, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xCCCCCC));

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
    numbers_labels[i] = lv_label_create(lv_scr_act(), nullptr);
    lv_label_set_text(numbers_labels[i], std::to_string(i + 1).c_str());
    // lv_coord_t number_height2 = 12; // 23
    lv_obj_set_pos(numbers_labels[i], numbers[i].x - lv_obj_get_width(numbers_labels[i]) / 2, numbers[i].y - lv_obj_get_height(numbers_labels[i]) / 2);
    lv_obj_set_style_local_text_color(numbers_labels[i], LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
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
  if (cbuf != nullptr) {
    delete[] cbuf;
    cbuf = nullptr;
  }
  if (vibrationTimer != nullptr) {
    xTimerDelete(vibrationTimer, 0);
  }
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

  uint8_t year = static_cast<uint8_t>(dateTimeController.Year() - 2000);
  uint8_t month = static_cast<uint8_t>(dateTimeController.Month());
  uint8_t day = dateTimeController.Day();

  uint8_t hour24 = dateTimeController.Hours();
  uint8_t min = dateTimeController.Minutes();
  uint16_t angle1440 = hour24 * 60 + min;

  if (angle1440 == dateTimeController.sAngle)
    return;

  // GEMINI: If sAngle is -1, it means we came from a BLE RELOAD command
  if (dateTimeController.sAngle == (uint16_t)-1) {
    lv_img_cache_invalidate_src(NULL); // Clear the entire image cache
  }

  dateTimeController.sAngle = angle1440;

  // Handle empty dataList with a Welcome
  if (this->dataList.empty()) {
    wasEmpty = true;

    // COMPENSATE NO ICON LOADED:
    brightnessController.Set(Controllers::BrightnessController::Levels::High);

    printf("GEMINI: Refresh() entered. dataList is empty!\n");
    printf("GEMINI: minor_scales hidden status before: %d\n", lv_obj_get_hidden(minor_scales));
    // Hide analog clock scales and numbers
    lv_obj_set_hidden(pie, true);
    lv_obj_set_hidden(minor_scales, true);
    lv_obj_set_hidden(major_scales, true);
    for (int j = 0; j < 12; ++j) {
      if (numbers_labels[j] != nullptr) {
        lv_obj_set_hidden(numbers_labels[j], true);
      }
    }
    printf("GEMINI: minor_scales hidden status after: %d\n", lv_obj_get_hidden(minor_scales));

    // lv_obj_set_hidden(canvas, true);
    lv_obj_clean(pie);

    // Create label
    if (!clocksFile) {
      lv_label_set_text_static(label_time, "RoutineHero");
    } else {
      lv_label_set_text_static(label_time, "Error\nCorrupted schedule\nUpdate the watch!\n ");
    }
    lv_obj_set_style_local_text_font(label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_24);
    lv_obj_set_style_local_text_line_space(label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 9);
    lv_label_set_align(label_time, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(label_time, nullptr, LV_ALIGN_IN_TOP_MID, 0, 82);

    // Make sure canvas is visible
    lv_obj_set_hidden(canvas, false);
    lv_canvas_fill_bg(canvas, LV_COLOR_BLACK, 0); // Invalid write of size 8
    lv_canvas_transform(canvas, (lv_img_dsc_t*) &routinehero, 0, LV_IMG_ZOOM_NONE, 0, 0, 64, 64, false);
    lv_obj_set_pos(canvas, 88, 15);

    // Lazy-create store badge images if they don't exist yet
    if (img_play == nullptr) {
      img_play = lv_img_create(lv_scr_act(), nullptr);
      lv_img_set_src(img_play, &play_store_icon);
      lv_obj_set_style_local_image_recolor(img_play, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    }
    if (img_ios == nullptr) {
      img_ios = lv_img_create(lv_scr_act(), nullptr);
      lv_img_set_src(img_ios, &apple_store_icon);
      lv_obj_set_style_local_image_recolor(img_ios, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    }

    // Position and show store logos & labels (stacked layout)
    lv_obj_set_pos(img_play, 32, 138);
    lv_obj_set_hidden(img_play, false);

    lv_obj_set_pos(img_ios, 32, 191);
    lv_obj_set_hidden(img_ios, false);

    return;
  }

  if (wasEmpty) {
    lv_obj_align(label_time, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    if (img_play != nullptr) {
      lv_obj_set_hidden(img_play, true);
    }
    if (img_ios != nullptr) {
      lv_obj_set_hidden(img_ios, true);
    }
    
    // Restore analog clock scales and numbers
    lv_obj_set_hidden(pie, false);
    lv_obj_set_hidden(minor_scales, false);
    lv_obj_set_hidden(major_scales, false);
    for (int j = 0; j < 12; ++j) {
      if (numbers_labels[j] != nullptr) {
        lv_obj_set_hidden(numbers_labels[j], false);
      }
    }
    wasEmpty = false;
  }

  Controllers::DateTime::Days day_of_week = dateTimeController.DayOfWeek();

  // printf("\nday %d", day);
  // printf("\nmonth %d", month);
  // printf("\nyear %d", year);

  std::vector<Data> result;
  for (const auto& data : dataList) {

    // Loop through all dates for this data
    if (!data.dates.empty()) {
      for (const auto& date : data.dates) {

        // printf("\nday %d", date.day);
        // printf("\nmonth %d", date.month);
        // printf("\nyear %d", date.year);

        if (date.day == day && date.month == month && date.year == year) {
          // Found a matching date
          result.push_back(data);
          break; // No need to check other dates
        }
      }
      if (result.size() > 0) break;
    }
  }

  if (result.size() == 0) {
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
        break;
      }
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
    if (angle1440 >= slices[i].start * 5 && angle1440 < slices[i].end * 5) {
      sliceIndex = i;
      break;
    }
  }

  // VIBRATE ALTHOUGH IS IDLE BUT NOT IF THE SAME ANGLE
  asyncVibrate(angle1440, sliceIndex);

  if (state == DisplayApp::States::Idle)
    return;

  DrawTime(hour24, min);

  uint8_t icon = 1; // Asumimos icono de dormir por defecto
  uint16_t end = slices[0].start; // El fin de la noche es el inicio del primer evento de mañana
  bool black = true; // Si no hay evento asignado, la pantalla es negra (dormir)

  if (sliceIndex < slices.size()) {
    slice = &slices[sliceIndex];
    icon = slice->icon;
    end = slice->end;
    black = (slice->red == 0 && slice->green == 0 && slice->blue == 0);
  }

  if (0 == icon || 1 == icon) {
    settingsController.SetBrightness(Controllers::BrightnessController::Levels::Low);
  } else {
    settingsController.SetBrightness(Controllers::BrightnessController::Levels::High);
  }

  uint16_t currSec10 = (hour24 * 3600 + min * 60 + dateTimeController.Seconds()) / 10;
  uint16_t angleLeft = (end * 30) - currSec10;

  if (black && (hour24 > 12 || angleLeft >= 360)) { // SLEEP ICON AT THE CENTER, HIDE EVERITHING ELSE
    lv_obj_set_hidden(canvas, true);
    lv_obj_clean(pie);
    lv_obj_t* imgFlag = lv_img_create(pie, nullptr);
    // lv_obj_set_click(imgFlag, false);
    lv_img_set_auto_size(imgFlag, false);
    lv_obj_set_size(imgFlag, 24, 24);
    lv_img_set_src(imgFlag, "F:/1.bin");
    lv_obj_set_pos(imgFlag, 108, 108);
    lv_obj_set_style_local_image_recolor(imgFlag, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_SILVER);
    dateTimeController.sSlice = sliceIndex;
    return;
  }

  uint16_t angle360 = angle1440;
  if (hour24 >= 12)
    angle360 = angle360 - 720;
  angle360 = angle360 / 2;

  DrawPie(angle1440 / 5, slices);
  CanvasReset(angle360);
  DrawArrow(angle360);
}

void RoutineHeroWatchFace::SetBatteryIcon() {
  auto batteryPercent = batteryPercentRemaining.Get();
  batteryIcon.SetBatteryPercentage(batteryPercent);
}

void RoutineHeroWatchFace::DrawTime(uint8_t hour, uint8_t min) {
  if (sHour != hour || sMin != min) {
    sHour = hour;
    sMin = min;
    lv_label_set_text_fmt(label_time, "#cccccc %02d:%02d", hour, min); //:::
  }
}

void RoutineHeroWatchFace::DrawName() {
  std::string name;
  if (settingsController.GetFileContent("/name.txt", name)) {
    const size_t max_length = 7;

    if (name.length() > max_length) {
      // Cut to 7 characters and add a dot
      name = name.substr(0, max_length) + ".";
    }

    // Update the label text
    // We use %s because 'name' is now a string/text
    lv_label_set_text(label_name, name.c_str());
  }
}

void RoutineHeroWatchFace::DrawPie(int16_t angle288, std::vector<IntervalColor> slices) {

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
    if (angle288 >= clockSlices[i].start && angle288 < clockSlices[i].end) {
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

    int16_t wakeup288 = 0;
    for (i = 0; i < slices.size(); ++i) {
      if (slices[i].icon > 1) {
        wakeup288 = slices[i].start;
        break;
      }
    }

    // DRAW SLICES:
    for (IntervalColor slice : clockSlices) {
      DrawSlice(slice, angle288, wakeup288);
    }

    // DRAW ICONS:
    for (IntervalColor slice : clockSlices) {
      DrawIcon(slice, angle288, wakeup288);
    }
  }
}

void RoutineHeroWatchFace::DrawSlice(IntervalColor slice, int16_t angle288, int16_t wakeup288) {
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

  if (angle288 < wakeup288){
    color = lv_color_make((uint8_t) (slice.red * 0.2), (uint8_t) (slice.green * 0.2), (uint8_t) (slice.blue * 0.2));
  } else if (angle288 >= slice.start && angle288 < slice.end && 0 != slice.icon) { //DOING
    lv_arc_set_radius(p, CANVAS_CENTER);
    color = lv_color_make((uint8_t) (slice.red * 0.9), (uint8_t) (slice.green * 0.9), (uint8_t) (slice.blue * 0.9));
  } else if (slice.end <= angle288) { //DONE
    color = lv_color_make((uint8_t) (slice.red * 0.3), (uint8_t) (slice.green * 0.3), (uint8_t) (slice.blue * 0.3));
  } else { //TO DO
    color = lv_color_make((uint8_t) (slice.red * 0.65), (uint8_t) (slice.green * 0.65), (uint8_t) (slice.blue * 0.65));
  }
  lv_obj_set_style_local_line_color(p, LV_ARC_PART_BG, LV_STATE_DEFAULT, color);

  lv_arc_set_bg_angles(p, start * 2.5, end * 2.5);
}

void RoutineHeroWatchFace::DrawIcon(IntervalColor slice, int16_t angle288, int16_t wakeup288) {
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
  bool current = (angle288 >= slice.start && angle288 < slice.end) || (slices[dateTimeController.sSlice].icon == 0 && black);

  lv_color_t color = LV_COLOR_ICONS;
  if (current) {
    color = LV_COLOR_WHITE;
  } else if (angle288 < wakeup288) {
    color = LV_COLOR_MAKE(0x70, 0x70, 0x70);
  } else if (slice.end < angle288) {
    color = LV_COLOR_GRAY;
  }

  lv_obj_t* imgFlag = lv_img_create(pie, nullptr);
  lv_obj_set_click(imgFlag, false);
  lv_img_set_auto_size(imgFlag, false);
  lv_obj_set_size(imgFlag, 24, 24);

  lv_img_set_src(imgFlag, ("F:/" + std::to_string(icon) + ".bin").c_str());

  position_image_on_circle(imgFlag, CANVAS_CENTER, CANVAS_CENTER, 81, (start + (end - start) / 2) * 2.5 - 90);

  lv_obj_set_style_local_image_recolor(imgFlag, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, color);
  // lv_obj_set_style_local_image_recolor_opa(imgFlag, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, 200);

  if (angle288 >= slice.start && angle288 < slice.end && 0 != slice.icon) {
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
  *x = (radius * _lv_trigo_sin(angle_deg + 90)) >> 15; 
  *y = (radius * _lv_trigo_sin(angle_deg)) >> 15;
}

void RoutineHeroWatchFace::asyncVibrate(int16_t angle1440, uint8_t sliceIndex) {

  // IGNORE IF NO PREVIUS DATA
  if (dateTimeController.sSlice >= 254)
    return;

  // IGNORE MISSING SLICES
  if (sliceIndex >= 254)
    return;

  // IGNORE IF COMES FROM SLEEPING TIME
  if (dateTimeController.sSlice < 2)
    return;

  // IGNORE IF GOES TO SLEEP (IF KID ALREADY SPEEPING?)
  if (sliceIndex < 2)
    return;

  // ONLY VIBRATE AT THE START OF THE SLICE
  if (angle1440 != slices[sliceIndex].start * 5)
    return;

if (dateTimeController.sSlice != sliceIndex) {
    vibrationStep = 0; // 1. Reiniciamos el estado de la vibración
    xTimerChangePeriod(vibrationTimer, pdMS_TO_TICKS(10), 0); // 2. Arrancamos el timer casi de inmediato
  }
}

void RoutineHeroWatchFace::Vibrate(TimerHandle_t xTimer) {
  // Recuperamos la instancia de nuestra clase para acceder a las variables
  auto* watchface = static_cast<RoutineHeroWatchFace*>(pvTimerGetTimerID(xTimer));

  switch (watchface->vibrationStep) {
    case 0: 
      // Primer pulso: Encender motor
      nrf_gpio_pin_clear(PinMap::Motor); 
      // Programamos el siguiente evento en 100ms
      xTimerChangePeriod(xTimer, pdMS_TO_TICKS(100), 0); 
      watchface->vibrationStep = 1;
      break;

    case 1: 
      // Pausa: Apagar motor
      nrf_gpio_pin_set(PinMap::Motor);
      // Programamos el siguiente evento en 500ms
      xTimerChangePeriod(xTimer, pdMS_TO_TICKS(500), 0); 
      watchface->vibrationStep = 2;
      break;

    case 2: 
      // Segundo pulso: Encender motor
      nrf_gpio_pin_clear(PinMap::Motor);
      // Programamos el apagado final en 100ms
      xTimerChangePeriod(xTimer, pdMS_TO_TICKS(100), 0); 
      watchface->vibrationStep = 3;
      break;

    case 3: 
      // Fin de la secuencia: Apagar motor y detener el timer
      nrf_gpio_pin_set(PinMap::Motor);
      xTimerStop(xTimer, 0);
      break;
  }
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
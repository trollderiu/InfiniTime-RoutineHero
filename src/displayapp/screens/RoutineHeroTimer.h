#pragma once

#include "displayapp/screens/Screen.h"
#include <lvgl/src/lv_core/lv_obj.h>
#include "FreeRTOS.h"
#include <vector>
#include "components/datetime/DateTimeController.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "displayapp/screens/BatteryIcon.h"
#include "utility/DirtyValue.h"
#include "displayapp/DisplayApp.h"

#define CANVAS_WIDTH 64
#define CANVAS_CENTER 120
// #define m 30
#define m 0

struct TimerNumbers {
  uint8_t x;
  uint8_t y;
};

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class RoutineHeroTimer : public Screen {
      public:
        RoutineHeroTimer(Controllers::DateTime& dateTimeController, const Controllers::Battery& batteryController, const Controllers::Ble& bleController, Controllers::Settings& settingsController, Controllers::BrightnessController& brightnessController, DisplayApp::States& state);
        ~RoutineHeroTimer() override;

        void Refresh() override;
        void asyncRing();

      private:
        DisplayApp::States& state;
        BatteryIcon batteryIcon;
        Controllers::DateTime& dateTimeController;
        const Controllers::Battery& batteryController;
        const Controllers::Ble& bleController;
        Controllers::Settings& settingsController;
        Controllers::BrightnessController& brightnessController;

        uint8_t sSlice;
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>> currentDateTime;

        void DrawTime(uint8_t hour, uint8_t min);

        uint8_t sUpdate = -1;

        // DrawTime
        uint8_t sHour = -1;
        uint8_t sMin = -1;
        // uint8_t sSec = -1;
        lv_obj_t* label_time;

        uint16_t nextIconIndex;
        lv_obj_t* currentIcon;
        lv_obj_t* nextIcon;

        // Refresh
        bool init;
        std::vector<IntervalColor> slices;
        uint8_t i;
        IntervalColor* slice;
        IntervalColor* slice2;
        uint16_t end;
        uint16_t angleLeft;
        uint16_t angleTotal;

        // draw_line
        uint8_t di;

        void print_memory_content(const void *addr, size_t size);
        void print_colors(const lv_color32_t *colors, size_t num_colors);

        TimerHandle_t vibrationTimer;
        void asyncVibrate(uint8_t sliceIndex);
        static void Vibrate(TimerHandle_t xTimer);
        
        lv_task_t* taskRefresh;

        lv_obj_t* minor_scales;
        lv_obj_t* major_scales;
        lv_obj_t* major_line;
        lv_point_t major_line_points[2];
        lv_obj_t* pie;
        lv_obj_t* p;
        lv_obj_t* p2;
        lv_obj_t* pDone;
        uint8_t red;
        uint8_t green;
        uint8_t blue;
        lv_color_t currColor;
        lv_color_t currColor2;
        lv_color_t color;
        lv_obj_t* canvas;
        lv_obj_t* line;
        lv_point_t line_points[2];
        static uint8_t* cbuf;
        void DrawSlice(uint16_t angle, uint8_t index);
        void DrawSlice2(uint16_t angle1, uint16_t angle2, uint8_t index);
        void DrawIcon(uint16_t angle, uint8_t index);
        void DrawArrow(int16_t angle);
        void polar_to_cartesian(int16_t angle_deg, uint8_t radius, int8_t* x, int8_t* y);
        void position_image_on_circle(lv_obj_t* img, uint8_t center_x, uint8_t center_y, uint8_t radius, int16_t angle_deg);
      
        static const TimerNumbers numbers[12];

        Utility::DirtyValue<uint8_t> batteryPercentRemaining {0};
        Utility::DirtyValue<bool> isCharging {};
        Utility::DirtyValue<bool> bleState {};
        lv_obj_t* plugIcon;
        lv_obj_t* bleIcon;
        void SetBatteryIcon();

        lv_obj_t* number0;

        std::vector<Data> dataList;
        std::vector<Data> result;
      };
    }
  }
}

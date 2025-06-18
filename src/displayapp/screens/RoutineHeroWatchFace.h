#pragma once

#include <lvgl/src/lv_core/lv_obj.h>
#include <chrono>
#include <cstdint>
#include <memory>
#include "displayapp/screens/Screen.h"
#include "components/datetime/DateTimeController.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/brightness/BrightnessController.h"
#include "displayapp/screens/BatteryIcon.h"
#include "utility/DirtyValue.h"
#include "displayapp/DisplayApp.h"
#include <functional> //using LoadScreenFunction = std::function..

#define CANVAS_CENTER 120
#define CANVAS_WIDTH  64
#define ARROW_SIZE    136

struct Numbers {
  uint8_t x;
  uint8_t y;
};

namespace Pinetime {
  namespace Controllers {
    class Settings;
    class Battery;
    class Ble;
    // class NotificationManager;
  }

  namespace Applications {
    namespace Screens {

      class RoutineHeroWatchFace : public Screen {
      public:
        using LoadScreenFunction = std::function<void(lv_obj_t*, lv_event_t)>;

        RoutineHeroWatchFace(Controllers::DateTime& dateTimeController,
                        const Controllers::Battery& batteryController,
                        const Controllers::Ble& bleController,
                        Controllers::Settings& settingsController,
                        Controllers::BrightnessController& brightnessController,
                        // Controllers::FS& filesystem
                        DisplayApp::States& state
        );
        ~RoutineHeroWatchFace() override;

        void Refresh() override;

        // static const uint8_t arrowicon_map[ARROW_SIZE];
        static const Numbers numbers[12];
        DisplayApp* app;

        static LoadScreenFunction loadScreenFunc;
        static void setLoadScreenFunction(LoadScreenFunction func);

      private:
        DisplayApp::States& state;
        BatteryIcon batteryIcon;

        Controllers::DateTime& dateTimeController;
        const Controllers::Battery& batteryController;
        const Controllers::Ble& bleController;
        // Controllers::NotificationManager& notificationManager;
        Controllers::Settings& settingsController;
        Controllers::BrightnessController& brightnessController;
        // Controllers::FS& filesystem;

        // Methods
        void InitLvgl();
        void DrawTime(uint8_t hour, uint8_t min);
        int16_t GetAngle(uint8_t hour, uint8_t min);
        void DrawPie(int16_t angle);
        void DrawSlice(IntervalColor a, int16_t angle);
        static void lvEventCb(lv_obj_t* obj, lv_event_t event);
        void DrawIcon(IntervalColor a, int16_t angle);
        void CanvasReset(int16_t angle);
        void DrawArrow(int16_t angle);
        void SetBatteryIcon();
        void polar_to_cartesian(int16_t angle_deg, uint8_t radius, int8_t* x, int8_t* y);
        void position_image_on_circle(lv_obj_t* img, uint8_t center_x, uint8_t center_y, uint8_t radius, int16_t angle_deg);
        void asyncVibrate(int16_t angle720, uint8_t index);
        static void Vibrate(TimerHandle_t xTimer);

        // Main
        TimerHandle_t vibrationTimer;
        lv_task_t* taskRefresh;

        // InitLvgl
        uint8_t i, sHour, sMin;
        lv_obj_t* pie;
        lv_obj_t* label_time;
        lv_obj_t* minor_scales;
        lv_obj_t* major_scales;
        // lv_obj_t* major_line;
        // lv_point_t major_line_points[2];
        lv_obj_t* plugIcon;
        lv_obj_t* bleIcon;
        // lv_obj_t* mainIcon;

        lv_obj_t* canvas;
        static uint8_t* cbuf;

        // Refresh
        int16_t angle;
        Utility::DirtyValue<bool> isCharging {};
        Utility::DirtyValue<uint8_t> batteryPercentRemaining {0};
        Utility::DirtyValue<bool> bleState {};
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>> currentDateTime;

        // UpdateClock
        uint8_t index;
        IntervalColor* slice;

        // DrawPie
        // uint8_t hour;
        // bool clock;
        bool sClock2;
        std::vector<IntervalColor> slices;
        std::vector<IntervalColor> clockSlices;

        // DrawSlice
        lv_color_t color;

        // DrawIcon
        uint8_t icon;

        // struct Slice {
        //   uint8_t start, end; // 0-255 -> 0-360
        //   uint8_t r, g, b;
        //   uint8_t icon;
        // };

        // struct Clock {
        //   // bool midday;     // 0: 0-12, 1: 12-24
        //   // bool weekday[7]; // max 7 days of week 1111100, 0000011..
        //   uint8_t midday;
        //   // date; //TODO
        //   // Slice slice[10]; // max 10 slices per clock face
        //   uint8_t index;
        // };

        // struct ClockData {
        //   Clock clock[10]; // max 10 variations of clocks
        // };

        // ClockData settings;
        // void LoadSettingsFromFile();
        // void SaveSettingsToFile();
        // void LoadSettingsFromFile() const;
        // void SaveSettingsToFile() const;

        // std::vector<int> parseSlice(const std::string &sliceStr) const;
        // std::vector<std::vector<int>> parseSlices(const std::string &slicesStr) const;
        // Data parseLine(const std::string& line) const;

        std::vector<Data> dataList;
        std::vector<Data> result;
      };
    }

    template <>
    struct WatchFaceTraits<WatchFace::Analog> {

      static constexpr WatchFace watchFace = WatchFace::Analog;
      // static constexpr const char* name = "Analog face";

      static Screens::Screen* Create(AppControllers& controllers) {

        auto loadScreenFunction = [&controllers](lv_obj_t* p, lv_event_t event) {
          if (event == LV_EVENT_CLICKED) {
            // controllers.displayApp->LoadScreen(Apps::CurrentActivity, DisplayApp::FullRefreshDirections::None);
            controllers.displayApp->LoadScreen(Apps::Timer, DisplayApp::FullRefreshDirections::None);
          }
        };
        Screens::RoutineHeroWatchFace::setLoadScreenFunction(loadScreenFunction);

        return new Screens::RoutineHeroWatchFace(controllers.dateTimeController,
                                            controllers.batteryController,
                                            controllers.bleController,
                                            controllers.settingsController,
                                            controllers.brightnessController,
                                            // controllers.filesystem
                                            controllers.displayApp->state //Pass state as a reference
                                            // controllers.displayApp->LoadScreen // Assuming LoadScreen is a member function
        );
      };
    };
  }
}

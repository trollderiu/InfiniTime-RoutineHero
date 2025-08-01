#pragma once
#include <cstdint>
#include <bitset>
#include "components/brightness/BrightnessController.h"
#include "components/fs/FS.h"
#include "displayapp/apps/Apps.h"
#include <vector> //GetSchedule()

struct Date {
  uint8_t day;
  uint8_t month;
  uint8_t year;
};

struct IntervalColor {
  int16_t start;
  int16_t end;
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  uint8_t icon;
};

struct Data {
  bool monday;
  bool tuesday;
  bool wednesday;
  bool thursday;
  bool friday;
  bool saturday;
  bool sunday;

  // int year;
  // int month;
  // int day;
  // int weekend;
  std::vector<Date> dates;

  // std::vector<std::vector<int>> slices;
  std::vector<IntervalColor> slices;
};

namespace Pinetime {
  namespace Controllers {
    class Settings {
    public:
      enum class ClockType : uint8_t { H24, H12 };
      // enum class WeatherFormat : uint8_t { Metric, Imperial };
      enum class Notification : uint8_t { On, Off, Sleep };
      enum class ChimesOption : uint8_t { None, Hours, HalfHours };
      enum class WakeUpMode : uint8_t { SingleTap = 0, DoubleTap = 1, RaiseWrist = 2, Shake = 3, LowerWrist = 4 };
      // enum class Colors : uint8_t {
      //   White,
      //   Silver,
      //   Gray,
      //   Black,
      //   Red,
      //   Maroon,
      //   Yellow,
      //   Olive,
      //   Lime,
      //   Green,
      //   Cyan,
      //   Teal,
      //   Blue,
      //   Navy,
      //   Magenta,
      //   Purple,
      //   Orange,
      //   Pink
      // };
      enum class PTSGaugeStyle : uint8_t { Full, Half, Numeric };
      // enum class PTSWeather : uint8_t { On, Off };

      // struct PineTimeStyle {
      //   Colors ColorTime = Colors::Teal;
      //   Colors ColorBar = Colors::Teal;
      //   Colors ColorBG = Colors::Black;
      //   PTSGaugeStyle gaugeStyle = PTSGaugeStyle::Full;
      //   PTSWeather weatherEnable = PTSWeather::Off;
      // };

      // struct WatchFaceInfineat {
      //   bool showSideCover = true;
      //   int colorIndex = 0;
      // };

      Settings(Pinetime::Controllers::FS& fs);

      Settings(const Settings&) = delete;
      Settings& operator=(const Settings&) = delete;
      Settings(Settings&&) = delete;
      Settings& operator=(Settings&&) = delete;

      void Init();
      // void SaveSettings();

      // void SetWatchFace(Pinetime::Applications::WatchFace face) {
      //   if (face != settings.watchFace) {
      //     settingsChanged = true;
      //   }
      //   settings.watchFace = face;
      // };

      // Pinetime::Applications::WatchFace GetWatchFace() const {
      //   return settings.watchFace;
      // };

      // void SetChimeOption(ChimesOption chimeOption) {
      //   if (chimeOption != settings.chimesOption) {
      //     settingsChanged = true;
      //   }
      //   settings.chimesOption = chimeOption;
      // };

      ChimesOption GetChimeOption() const {
        return settings.chimesOption;
      };

      // void SetPTSColorTime(Colors colorTime) {
      //   if (colorTime != settings.PTS.ColorTime)
      //     settingsChanged = true;
      //   settings.PTS.ColorTime = colorTime;
      // };

      // Colors GetPTSColorTime() const {
      //   return settings.PTS.ColorTime;
      // };

      // void SetPTSColorBar(Colors colorBar) {
      //   if (colorBar != settings.PTS.ColorBar)
      //     settingsChanged = true;
      //   settings.PTS.ColorBar = colorBar;
      // };

      // Colors GetPTSColorBar() const {
      //   return settings.PTS.ColorBar;
      // };

      // void SetPTSColorBG(Colors colorBG) {
      //   if (colorBG != settings.PTS.ColorBG)
      //     settingsChanged = true;
      //   settings.PTS.ColorBG = colorBG;
      // };

      // Colors GetPTSColorBG() const {
      //   return settings.PTS.ColorBG;
      // };

      // void SetInfineatShowSideCover(bool show) {
      //   if (show != settings.watchFaceInfineat.showSideCover) {
      //     settings.watchFaceInfineat.showSideCover = show;
      //     settingsChanged = true;
      //   }
      // };

      // bool GetInfineatShowSideCover() const {
      //   return settings.watchFaceInfineat.showSideCover;
      // };

      // void SetInfineatColorIndex(int index) {
      //   if (index != settings.watchFaceInfineat.colorIndex) {
      //     settings.watchFaceInfineat.colorIndex = index;
      //     settingsChanged = true;
      //   }
      // };

      // int GetInfineatColorIndex() const {
      //   return settings.watchFaceInfineat.colorIndex;
      // };

      // void SetPTSGaugeStyle(PTSGaugeStyle gaugeStyle) {
      //   if (gaugeStyle != settings.PTS.gaugeStyle)
      //     settingsChanged = true;
      //   settings.PTS.gaugeStyle = gaugeStyle;
      // };

      // PTSGaugeStyle GetPTSGaugeStyle() const {
      //   return settings.PTS.gaugeStyle;
      // };

      // void SetPTSWeather(PTSWeather weatherEnable) {
      //   if (weatherEnable != settings.PTS.weatherEnable)
      //     settingsChanged = true;
      //   settings.PTS.weatherEnable = weatherEnable;
      // };

      // PTSWeather GetPTSWeather() const {
      //   return settings.PTS.weatherEnable;
      // };

      // void SetAppMenu(uint8_t menu) {
      //   appMenu = menu;
      // };

      // uint8_t GetAppMenu() const {
      //   return appMenu;
      // };

      // void SetSettingsMenu(uint8_t menu) {
      //   settingsMenu = menu;
      // };

      // uint8_t GetSettingsMenu() const {
      //   return settingsMenu;
      // };

      // void SetClockType(ClockType clocktype) {
      //   if (clocktype != settings.clockType) {
      //     settingsChanged = true;
      //   }
      //   settings.clockType = clocktype;
      // };

      ClockType GetClockType() const {
        return settings.clockType;
      };

      // void SetWeatherFormat(WeatherFormat weatherFormat) {
      //   if (weatherFormat != settings.weatherFormat) {
      //     settingsChanged = true;
      //   }
      //   settings.weatherFormat = weatherFormat;
      // };

      // WeatherFormat GetWeatherFormat() const {
      //   return settings.weatherFormat;
      // };

      // void SetNotificationStatus(Notification status) {
      //   if (status != settings.notificationStatus) {
      //     settingsChanged = true;
      //   }
      //   settings.notificationStatus = status;
      // };

      Notification GetNotificationStatus() const {
        return settings.notificationStatus;
      };

      // void SetScreenTimeOut(uint32_t timeout) {
      //   if (timeout != settings.screenTimeOut) {
      //     settingsChanged = true;
      //   }
      //   settings.screenTimeOut = timeout;
      // };

      uint32_t GetScreenTimeOut() const {
        return settings.screenTimeOut;
      };

      // bool GetAlwaysOnDisplay() const {
      //   return settings.alwaysOnDisplay && GetNotificationStatus() != Notification::Sleep;
      // };

      // void SetAlwaysOnDisplaySetting(bool state) {
      //   if (state != settings.alwaysOnDisplay) {
      //     settingsChanged = true;
      //   }
      //   settings.alwaysOnDisplay = state;
      // }

      // bool GetAlwaysOnDisplaySetting() const {
      //   return settings.alwaysOnDisplay;
      // }

      void SetShakeThreshold(uint16_t thresh) {
        if (settings.shakeWakeThreshold != thresh) {
          settings.shakeWakeThreshold = thresh;
          settingsChanged = true;
        }
      }

      int16_t GetShakeThreshold() const {
        return settings.shakeWakeThreshold;
      }

      // void setWakeUpMode(WakeUpMode wakeUp, bool enabled) {
      //   if (enabled != isWakeUpModeOn(wakeUp)) {
      //     settingsChanged = true;
      //   }
      //   settings.wakeUpMode.set(static_cast<size_t>(wakeUp), enabled);
      //   // Handle special behavior
      //   if (enabled) {
      //     switch (wakeUp) {
      //       case WakeUpMode::SingleTap:
      //         settings.wakeUpMode.set(static_cast<size_t>(WakeUpMode::DoubleTap), false);
      //         break;
      //       case WakeUpMode::DoubleTap:
      //         settings.wakeUpMode.set(static_cast<size_t>(WakeUpMode::SingleTap), false);
      //         break;
      //       default:
      //         break;
      //     }
      //   }
      // };

      std::bitset<5> getWakeUpModes() const {
        return settings.wakeUpMode;
      }

      bool isWakeUpModeOn(const WakeUpMode mode) const {
        return getWakeUpModes()[static_cast<size_t>(mode)];
      }

      void SetBrightness(Controllers::BrightnessController::Levels level) {
        if (level != settings.brightLevel) {
          settingsChanged = true;
        }
        settings.brightLevel = level;
      };

      Controllers::BrightnessController::Levels GetBrightness() const {
        return settings.brightLevel;
      };

      // void SetStepsGoal(uint32_t goal) {
      //   if (goal != settings.stepsGoal) {
      //     settingsChanged = true;
      //   }
      //   settings.stepsGoal = goal;
      // };

      uint32_t GetStepsGoal() const {
        return settings.stepsGoal;
      };

      // void SetBleRadioEnabled(bool enabled) {
      //   bleRadioEnabled = enabled;
      // };

      bool GetBleRadioEnabled() const {
        return bleRadioEnabled;
      };

      bool LoadClocksFromFile(std::vector<Data>& dataList) const;

    private:
      Pinetime::Controllers::FS& fs;

      static constexpr uint32_t settingsVersion = 0x0008;

      struct SettingsData {
        uint32_t version = settingsVersion;
        uint32_t stepsGoal = 10000;
        // uint32_t screenTimeOut = 15000;
        uint32_t screenTimeOut = 30000;

        // bool alwaysOnDisplay = false;

        ClockType clockType = ClockType::H24;
        // WeatherFormat weatherFormat = WeatherFormat::Metric;
        Notification notificationStatus = Notification::On;

        Pinetime::Applications::WatchFace watchFace = Pinetime::Applications::WatchFace::Analog;
        ChimesOption chimesOption = ChimesOption::None;

        // PineTimeStyle PTS;

        // WatchFaceInfineat watchFaceInfineat;

        std::bitset<5> wakeUpMode {0};
        uint16_t shakeWakeThreshold = 150;

        Controllers::BrightnessController::Levels brightLevel = Controllers::BrightnessController::Levels::High;
      };

      SettingsData settings;
      bool settingsChanged = false;

      // uint8_t appMenu = 0;
      // uint8_t settingsMenu = 0;
      // uint8_t watchFacesMenu = 0;
      /* ble state is intentionally not saved with the other watch settings and initialized
       * to off (false) on every boot because we always want ble to be enabled on startup
       */
      bool bleRadioEnabled = true;

      void LoadSettingsFromFile();
      // void SaveSettingsToFile();

      // std::vector<int> parseSlice(const std::string& sliceStr) const;
      // IntervalColor parseSlice(const std::string& sliceStr) const;
      bool parseSlice(const std::string& sliceStr, IntervalColor& slice) const;
      // std::vector<std::vector<int>> parseSlices(const std::string& slicesStr) const;
      std::vector<IntervalColor> parseSlices(const std::string& slicesStr) const;
      bool parseLine(const std::string& line, Data& outData) const;
      std::vector<std::string> splitString(const std::string& str, char delimiter) const;
      void processBufferSettings(const std::string &bufferSettings, std::vector<Data> &dataList) const;
      // bool isValidNumber(const std::string& str);
    };
  }
}

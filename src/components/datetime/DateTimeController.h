#pragma once

#include <cstdint>
#include <chrono>
#include <ctime>
#include <string>
#include "components/settings/Settings.h"
#include <FreeRTOS.h>
#include <semphr.h>

// #include <vector> //GetSchedule()

// struct IntervalColor {
//   int16_t start;
//   int16_t end;
//   uint8_t red;
//   uint8_t green;
//   uint8_t blue;
//   uint8_t icon;
// };

// struct Data {
//   int year;
//   int month;
//   int day;
//   int weekend;
//   int am_pm;
//   std::vector<std::vector<int>> slices;
// };


namespace Pinetime {
  namespace System {
    class SystemTask;
  }

  namespace Controllers {
    class DateTime {
    public:
      DateTime(Controllers::Settings& settingsController);
      enum class Days : uint8_t { Unknown, Monday, Tuesday, Wednesday, Thursday, Friday, Saturday, Sunday };
      enum class Months : uint8_t {
        Unknown,
        January,
        February,
        March,
        April,
        May,
        June,
        July,
        August,
        September,
        October,
        November,
        December
      };

      void SetTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);

      /*
       * setter corresponding to the BLE Set Local Time characteristic.
       *
       * used to update difference between utc and local time (see UtcOffset())
       *
       * parameters are in quarters of an hour. Following the BLE CTS specification,
       * timezone is expected to be constant over DST which will be reported in
       * dst field.
       */
      void SetTimeZone(int8_t timezone, int8_t dst);

      uint16_t Year() const {
        return 1900 + localTime.tm_year;
      }

      Months Month() const {
        return static_cast<Months>(localTime.tm_mon + 1);
      }

      uint8_t Day() const {
        return localTime.tm_mday;
      }

      Days DayOfWeek() const {
        int daysSinceSunday = localTime.tm_wday;
        if (daysSinceSunday == 0) {
          return Days::Sunday;
        }
        return static_cast<Days>(daysSinceSunday);
      }

      int DayOfYear() const {
        return localTime.tm_yday + 1;
      }

      uint8_t Hours() const {
        return localTime.tm_hour;
      }

      uint8_t Minutes() const {
        return localTime.tm_min;
      }

      uint8_t Seconds() const {
        return localTime.tm_sec;
      }

      /*
       * returns the offset between local time and UTC in quarters of an hour
       *
       * Availability of this field depends on wether the companion app
       * supports the BLE CTS Local Time Characteristic. Expect it to be 0
       * if not.
       */
      int8_t UtcOffset() const {
        return tzOffset + dstOffset;
      }

      /*
       * returns the offset between the (dst independent) local time zone and UTC
       * in quarters of an hour
       *
       * Availability of this field depends on wether the companion app
       * supports the BLE CTS Local Time Characteristic. Expect it to be 0
       * if not.
       */
      int8_t TzOffset() const {
        return tzOffset;
      }

      /*
       * returns the offset between the local time zone and local time
       * in quarters of an hour
       * if != 0, DST is in effect, if == 0 not.
       *
       * Availability of this field depends on wether the companion app
       * supports the BLE CTS Local Time Characteristic. Expect it to be 0
       * if not.
       */
      int8_t DstOffset() const {
        return dstOffset;
      }

      const char* MonthShortToString() const;
      const char* DayOfWeekShortToString() const;
      static const char* MonthShortToStringLow(Months month);
      static const char* DayOfWeekShortToStringLow(Days day);

      std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> CurrentDateTime();

      std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> UTCDateTime() {
        return CurrentDateTime() - std::chrono::seconds((tzOffset + dstOffset) * 15 * 60);
      }

      std::chrono::seconds Uptime() const {
        return uptime;
      }

      void Register(System::SystemTask* systemTask);
      void SetCurrentTime(std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> t);
      std::string FormattedTime();

      // std::vector<std::vector<IntervalColor>> GetClocks() const;
      // std::vector<IntervalColor> GetSchedule(bool clock) const;
      // mutable std::vector<std::vector<IntervalColor>> clocks;
      // void SetAngle(uint16_t angle) const;
      mutable uint16_t sAngle = -1;
      mutable uint16_t sSlice = -1;

    private:
      void UpdateTime(uint32_t systickCounter, bool forceUpdate);

      std::tm localTime;
      int8_t tzOffset = 0;
      int8_t dstOffset = 0;

      SemaphoreHandle_t mutex = nullptr;

      uint32_t previousSystickCounter = 0;
      std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> currentDateTime;
      std::chrono::seconds uptime {0};

      bool isMidnightAlreadyNotified = false;
      bool isHourAlreadyNotified = true;
      bool isHalfHourAlreadyNotified = true;
      System::SystemTask* systemTask = nullptr;
      Controllers::Settings& settingsController;

      // void LoadSettingsFromFile() const;
      // void SaveSettingsToFile() const;

      // std::vector<int> parseSlice(const std::string &sliceStr) const;
      // std::vector<std::vector<int>> parseSlices(const std::string &slicesStr) const;
      // Data parseLine(const std::string& line) const;
    };
  }
}

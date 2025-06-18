#include "components/datetime/DateTimeController.h"
#include <libraries/log/nrf_log.h>
#include <systemtask/SystemTask.h>
#include <hal/nrf_rtc.h>
#include "nrf_assert.h"

using namespace Pinetime::Controllers;

namespace {
  constexpr const char* const DaysStringShort[] = {"--", "MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN"};
  constexpr const char* const DaysStringShortLow[] = {"--", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
  constexpr const char* const MonthsString[] = {"--", "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
  constexpr const char* const MonthsStringLow[] =
    {"--", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

  constexpr int compileTimeAtoi(const char* str) {
    int result = 0;
    while (*str >= '0' && *str <= '9') {
      result = result * 10 + *str - '0';
      str++;
    }
    return result;
  }
}

DateTime::DateTime(Controllers::Settings& settingsController) : settingsController {settingsController} {
  mutex = xSemaphoreCreateMutex();
  ASSERT(mutex != nullptr);
  xSemaphoreGive(mutex);

  // __DATE__ is a string of the format "MMM DD YYYY", so an offset of 7 gives the start of the year
  SetTime(compileTimeAtoi(&__DATE__[7]), 1, 1, 0, 0, 0);
}

void DateTime::SetCurrentTime(std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> t) {
  xSemaphoreTake(mutex, portMAX_DELAY);
  this->currentDateTime = t;
  UpdateTime(previousSystickCounter, true); // Update internal state without updating the time
  xSemaphoreGive(mutex);
}

void DateTime::SetTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second) {
  std::tm tm = {
    /* .tm_sec  = */ second,
    /* .tm_min  = */ minute,
    /* .tm_hour = */ hour,
    /* .tm_mday = */ day,
    /* .tm_mon  = */ month - 1,
    /* .tm_year = */ year - 1900,
  };

  NRF_LOG_INFO("%d %d %d ", day, month, year);
  NRF_LOG_INFO("%d %d %d ", hour, minute, second);

  tm.tm_isdst = -1; // Use DST value from local time zone

  xSemaphoreTake(mutex, portMAX_DELAY);
  currentDateTime = std::chrono::system_clock::from_time_t(std::mktime(&tm));
  UpdateTime(previousSystickCounter, true);
  xSemaphoreGive(mutex);

  if (systemTask != nullptr) {
    systemTask->PushMessage(System::Messages::OnNewTime);
  }
}

void DateTime::SetTimeZone(int8_t timezone, int8_t dst) {
  tzOffset = timezone;
  dstOffset = dst;
}

std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> DateTime::CurrentDateTime() {
  xSemaphoreTake(mutex, portMAX_DELAY);
  UpdateTime(nrf_rtc_counter_get(portNRF_RTC_REG), false);
  xSemaphoreGive(mutex);
  return currentDateTime;
}

void DateTime::UpdateTime(uint32_t systickCounter, bool forceUpdate) {
  // Handle systick counter overflow
  uint32_t systickDelta = 0;
  if (systickCounter < previousSystickCounter) {
    systickDelta = static_cast<uint32_t>(portNRF_RTC_MAXTICKS) - previousSystickCounter;
    systickDelta += systickCounter + 1;
  } else {
    systickDelta = systickCounter - previousSystickCounter;
  }

  auto correctedDelta = systickDelta / configTICK_RATE_HZ;
  // If a second hasn't passed, there is nothing to do
  // If the time has been changed, set forceUpdate to trigger internal state updates
  if (correctedDelta == 0 && !forceUpdate) {
    return;
  }
  auto rest = systickDelta % configTICK_RATE_HZ;
  if (systickCounter >= rest) {
    previousSystickCounter = systickCounter - rest;
  } else {
    previousSystickCounter = static_cast<uint32_t>(portNRF_RTC_MAXTICKS) - (rest - systickCounter - 1);
  }

  currentDateTime += std::chrono::seconds(correctedDelta);
  uptime += std::chrono::seconds(correctedDelta);

  std::time_t currentTime = std::chrono::system_clock::to_time_t(currentDateTime);
  localTime = *std::localtime(&currentTime);

  auto minute = Minutes();
  auto hour = Hours();

  if (minute == 0 && !isHourAlreadyNotified) {
    isHourAlreadyNotified = true;
    if (systemTask != nullptr) {
      systemTask->PushMessage(System::Messages::OnNewHour);
    }
  } else if (minute != 0) {
    isHourAlreadyNotified = false;
  }

  if ((minute == 0 || minute == 30) && !isHalfHourAlreadyNotified) {
    isHalfHourAlreadyNotified = true;
    if (systemTask != nullptr) {
      systemTask->PushMessage(System::Messages::OnNewHalfHour);
    }
  } else if (minute != 0 && minute != 30) {
    isHalfHourAlreadyNotified = false;
  }

  // Notify new day to SystemTask
  if (hour == 0 and not isMidnightAlreadyNotified) {
    isMidnightAlreadyNotified = true;
    if (systemTask != nullptr)
      systemTask->PushMessage(System::Messages::OnNewDay);
  } else if (hour != 0) {
    isMidnightAlreadyNotified = false;
  }
}

const char* DateTime::MonthShortToString() const {
  return MonthsString[static_cast<uint8_t>(Month())];
}

const char* DateTime::DayOfWeekShortToString() const {
  return DaysStringShort[static_cast<uint8_t>(DayOfWeek())];
}

const char* DateTime::MonthShortToStringLow(Months month) {
  return MonthsStringLow[static_cast<uint8_t>(month)];
}

const char* DateTime::DayOfWeekShortToStringLow(Days day) {
  return DaysStringShortLow[static_cast<uint8_t>(day)];
}

void DateTime::Register(Pinetime::System::SystemTask* systemTask) {
  this->systemTask = systemTask;
}

using ClockType = Pinetime::Controllers::Settings::ClockType;

std::string DateTime::FormattedTime() {
  auto hour = Hours();
  auto minute = Minutes();
  // Return time as a string in 12- or 24-hour format
  char buff[9];
  if (settingsController.GetClockType() == ClockType::H12) {
    uint8_t hour12;
    const char* amPmStr;
    if (hour < 12) {
      hour12 = (hour == 0) ? 12 : hour;
      amPmStr = "AM";
    } else {
      hour12 = (hour == 12) ? 12 : hour - 12;
      amPmStr = "PM";
    }
    snprintf(buff, sizeof(buff), "%i:%02i %s", hour12, minute, amPmStr);
  } else {
    snprintf(buff, sizeof(buff), "%02i:%02i", hour, minute);
  }
  return std::string(buff);
}

// void DateTime::SetAngle(uint16_t angle) const {
//   this->sAngle = angle;
// }

// std::vector<std::vector<IntervalColor>> DateTime::GetClocks() const {
//   return {{
//             // {30, 31, 50, 50, 50, 0},
//             // {36, 38, 50, 50, 50, 0},
//             // {42, 45, 50, 50, 50, 0},
//             // {48, 52, 50, 50, 50, 0},
//             // {54, 59, 50, 50, 50, 0},
//             {60, 210, 0, 0, 0, 1},      //, "sleep"
//                                         //
//             {210, 232, 255, 44, 0, 2},  // 7 - 7.75 , "bowl" -> red
//             {232, 250, 255, 199, 0, 3}, // 7.75 - 8.33 , "clothes" -> yellow
//             {250, 270, 91, 185, 0, 4},  // 8.33 - 9 , "bus" -> green
//             {270, 360, 0, 113, 255, 5}, // 9 - 12 , "school" > blue
//             {0, 210, 0, 0, 0, 0}        // sleep
//           },
//           {
//             {0, 30, 255, 44, 0, 6},      // eat -> red
//             {30, 60, 50, 50, 50, 1},     // sleep
//                                          // {60, 150, 255, 126, 0, 5},   // school -> orange
//             {60, 150, 255, 166, 0, 5},   // school -> orange
//             {150, 165, 255, 199, 0, 4},  // bus -> yellow
//             {165, 195, 91, 185, 0, 7},   // toys -> green
//             {195, 210, 0, 207, 255, 8},  // shower -> cyan
//             {210, 240, 0, 113, 255, 6},  // eat -> blue
//             {240, 255, 128, 66, 209, 9}, // book -> purple
//             {270, 300, 0, 0, 0, 1},      // sleep
//             {270, 570, 0, 0, 0, 0},      // sleep
//                                          //
//                                          //  {301, 306, 50, 50, 50, 0},
//                                          //  {308, 312, 50, 50, 50, 0},
//                                          //  {315, 318, 50, 50, 50, 0},
//                                          //  {322, 324, 50, 50, 50, 0},
//                                          //  {329, 330, 50, 50, 50, 0}
//           },
//           {
//             // {30, 31, 50, 50, 50, 0},
//             // {36, 38, 50, 50, 50, 0},
//             // {42, 45, 50, 50, 50, 0},
//             // {48, 52, 50, 50, 50, 0},
//             // {54, 59, 50, 50, 50, 0},
//             {60, 210, 0, 0, 0, 1},      //, "sleep"
//                                         // //
//             {210, 232, 255, 44, 0, 2},  // 7 - 7.75 , "bowl"
//             {232, 250, 255, 199, 0, 3}, // 7.75 - 8.33 , "clothes"
//             {250, 360, 91, 185, 0, 7},  // 9 - 12 , "toys"
//             {0, 210, 0, 0, 0, 0},
//           },
//           {
//             {0, 30, 255, 44, 0, 6},      // eat -> red
//             {30, 60, 50, 50, 50, 1},     // sleep
//                                          // {60, 90, 255, 126, 0, 10},   // tv -> orange
//             {60, 90, 255, 166, 0, 10},   // tv -> orange
//             {90, 195, 91, 185, 0, 7},    // toys -> green
//             {195, 210, 0, 207, 255, 8},  // shower -> cyan
//             {210, 240, 0, 113, 255, 6},  // eat -> blue
//             {240, 255, 128, 66, 209, 9}, // book -> purple
//             {270, 300, 0, 0, 0, 1},      // sleep
//             {270, 570, 0, 0, 0, 0},      // sleep
//                                          //
//                                          //  {301, 306, 50, 50, 50, 0},
//                                          //  {308, 312, 50, 50, 50, 0},
//                                          //  {315, 318, 50, 50, 50, 0},
//                                          //  {322, 324, 50, 50, 50, 0},
//                                          //  {329, 330, 50, 50, 50, 0}
//           }};
// }

// std::vector<IntervalColor> DateTime::GetSchedule(bool clock) const {
//   if (clocks.empty())
//     clocks = GetClocks();

//   Days day_of_week = DayOfWeek();
//   bool weekend = (day_of_week == Controllers::DateTime::Days::Sunday || day_of_week == Controllers::DateTime::Days::Saturday) ? 1 : 0;

//   // amazon.es/Cander-Berlin-MNU-11930-aprendizaje/dp/B09L1N3NK9/ref=asc_df_B09L1N3NK9
//   // red: e72e34 (231, 46, 52) -> 0xe166
//   // orange: f18711 (241, 235, 17) -> 0xec22
//   // yellow: f5e015 (245, 224, 21) -> 0xf6e3
//   // lime: a8c825 (168, 200, 37) -> 0xa624
//   // green: 35aa25 (53, 170, 37) -> 0x3544
//   // turq: 0a8f64 (10, 143, 100) -> 0x0c6c
//   // cyan: 0a6b7e (10, 107, 126) -> 0x0b4f
//   // blue: 194b88 (25, 75, 136) -> 0x1a71
//   // dark blue: 292e92 (41, 46, 146) -> 0x2972
//   // purple: 563294 (86, 50, 148) -> 0x5192
//   // magenta: 931a84 (147, 26, 132) -> 0x90d0
//   // pink: c20853 (194, 8, 83) -> 0xc04a

//   // alamy.com/stock-photo-simple-7-color-rainbow-element-on-white-133334596.html
//   // red: 254,44,7
//   // orange: 255,126,2
//   // yellow: 255,199,0
//   // green: 91,185,1
//   // cyan: 1,207,255
//   // blue: 2,113,255
//   // purple: 128,66,209

//   // std::string jsonString = R"([
//   //   {
//   //     "m": 0,
//   //     "w": [0,1,2,3,4],
//   //     "p": [
//   //       [0, 360, 255, 255, 255, 0],
//   //       [0, 360, 255, 255, 255, 1],
//   //       [0, 360, 255, 255, 255, 2]
//   //     ]
//   //   },
//   //   {
//   //     "m": 1,
//   //     "w": [5,6],
//   //     "p": [
//   //       [0, 360, 255, 255, 255, 3],
//   //       [0, 360, 255, 255, 255, 4],
//   //       [0, 360, 255, 255, 255, 5]
//   //     ]
//   //   }
//   // ])";
//   // DateTime::SaveSettingsToFile();

//   if (0 == weekend) {
//     if (clock == 0) {
//       return clocks[0];
//     } else {
//       return clocks[1];
//     }
//   } else {
//     if (clock == 0) {
//       return clocks[2];
//     } else {
//       return clocks[3];
//     }
//   }

//   return clocks[0];
// }

// void DateTime::LoadSettingsFromFile() const {
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

// // void DateTime::SaveSettingsToFile() {
// //   lfs_file_t settingsFile;

// //   filesystem.FileDelete("/settings.dat");
// //   if (filesystem.FileOpen(&settingsFile, "/settings.dat", LFS_O_WRONLY | LFS_O_CREAT) != LFS_ERR_OK) {
// //     return;
// //   }
// //   settings.clock[1].midday = 1;
// //   filesystem.FileWrite(&settingsFile, reinterpret_cast<uint8_t*>(&settings), sizeof(settings));
// //   filesystem.FileClose(&settingsFile);
// // }

// std::vector<int> DateTime::parseSlice(const std::string& sliceStr) const {
//   std::vector<int> slice;
//   std::stringstream ss(sliceStr);
//   std::string number;
//   while (std::getline(ss, number, ',')) {
//     slice.push_back(std::stoi(number));
//   }
//   return slice;
// }

// std::vector<std::vector<int>> DateTime::parseSlices(const std::string& slicesStr) const {
//   std::vector<std::vector<int>> slices;
//   std::stringstream ss(slicesStr);
//   std::string sliceStr;
//   while (std::getline(ss, sliceStr, ';')) {
//     slices.push_back(parseSlice(sliceStr));
//   }
//   return slices;
// }

// Data DateTime::parseLine(const std::string& line) const {
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
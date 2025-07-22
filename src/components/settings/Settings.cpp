#include "components/settings/Settings.h"
#include <cstdlib>
#include <cstring>
#include <vector> //GetSchedule()
// #include <iostream>
#include <algorithm>

using namespace Pinetime::Controllers;

Settings::Settings(Pinetime::Controllers::FS& fs) : fs {fs} {
}

// void Settings::SaveSettingsToFile() {
//   lfs_file_t settingsFile;

//   fs.FileDelete("/settings.dat");
//   if (fs.FileOpen(&settingsFile, "/settings.dat", LFS_O_WRONLY | LFS_O_CREAT) != LFS_ERR_OK) {
//     return;
//   }
//   settings.clock[1].midday = 1;
//   fs.FileWrite(&settingsFile, reinterpret_cast<uint8_t*>(&settings), sizeof(settings));
//   fs.FileClose(&settingsFile);
// }

void Settings::Init() {

  // Load default settings from Flash
  LoadSettingsFromFile();
}

// void Settings::SaveSettings() {

//   // verify if is necessary to save
//   if (settingsChanged) {
//     SaveSettingsToFile();
//   }
//   settingsChanged = false;
// }

void Settings::LoadSettingsFromFile() {
  SettingsData bufferSettings;
  lfs_file_t settingsFile;

  if (fs.FileOpen(&settingsFile, "/settings.dat", LFS_O_RDONLY) != LFS_ERR_OK) {
    return;
  }
  fs.FileRead(&settingsFile, reinterpret_cast<uint8_t*>(&bufferSettings), sizeof(settings));
  fs.FileClose(&settingsFile);
  if (bufferSettings.version == settingsVersion) {
    settings = bufferSettings;
  }
}

// void Settings::SaveSettingsToFile() {
//   lfs_file_t settingsFile;

//   if (fs.FileOpen(&settingsFile, "/settings.dat", LFS_O_WRONLY | LFS_O_CREAT) != LFS_ERR_OK) {
//     return;
//   }
//   fs.FileWrite(&settingsFile, reinterpret_cast<uint8_t*>(&settings), sizeof(settings));
//   fs.FileClose(&settingsFile);
// }

// std::vector<Data> Settings::LoadClocksFromFile() const {
//   // printf("LoadClocksFromFile %d\n", 1);

//   // ClockData bufferSettings;
//   // lfs_file_t settingsFile;
//   // if (fs.FileOpen(&settingsFile, "/clocks.json", LFS_O_RDONLY) != LFS_ERR_OK) {
//   //   return;
//   // }
//   // fs.FileRead(&settingsFile, reinterpret_cast<uint8_t*>(&bufferSettings), sizeof(settings));
//   // fs.FileClose(&settingsFile);

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
//   std::vector<Data> dataList;

//   if (fs.FileOpen(&settingsFile, "/clocks.txt", LFS_O_RDONLY) != LFS_ERR_OK) {
//     // printf("no clocks.txt %d\n", 1);
//     return dataList;
//   }

//   constexpr size_t bufferSize = 4096; // Adjust buffer size as needed
//   uint8_t readBuffer[bufferSize];

//   size_t bytesRead = fs.FileRead(&settingsFile, readBuffer, bufferSize);

//   fs.FileClose(&settingsFile);

//   bufferSettings.assign(readBuffer, readBuffer + bytesRead);

//   // Now parse bufferSettings into dataList
//   std::istringstream iss(bufferSettings);
//   std::string line;
//   while (std::getline(iss, line)) {
//     if (!line.empty()) {
//       Data data = parseLine(line); // Assuming parseLine function exists
//       dataList.push_back(data);
//     }
//   }

//   // printf("dataList.size() %d\n", dataList.size());

//   // // Example output
//   // for (const auto& data : dataList) {
//   //   printf("year %d\n", data.year);
//   //   printf("month %d\n", data.month);
//   //   printf("day %d\n", data.day);
//   //   printf("weekend %d\n", data.weekend);
//   //   printf("am_pm %d\n", data.am_pm);
//   //   for (const auto& slice : data.slices) {
//   //     for (int num : slice) {
//   //       printf("num %d\n", num);
//   //     }
//   //     printf("%d\n", 1);
//   //   }
//   //   printf("------------ %d\n", 1);
//   // }

//   return dataList;
// }

// // std::vector<int> Settings::parseSlice(const std::string& sliceStr) const {
// IntervalColor Settings::parseSlice(const std::string& sliceStr) const {
//   std::vector<int> values;
//   std::stringstream ss(sliceStr);
//   std::string number;
//   while (std::getline(ss, number, ',')) {
//     values.push_back(std::stoi(number));
//   }

//   IntervalColor slice;
//   slice.start = static_cast<int16_t>(values[0]);
//   slice.end = static_cast<int16_t>(values[1]);
//   slice.red = static_cast<uint8_t>(values[2]);
//   slice.green = static_cast<uint8_t>(values[3]);
//   slice.blue = static_cast<uint8_t>(values[4]);
//   slice.icon = static_cast<uint8_t>(values[5]);

//   return slice;
// }

// // std::vector<std::vector<int>> Settings::parseSlices(const std::string& slicesStr) const {
// std::vector<IntervalColor> Settings::parseSlices(const std::string& slicesStr) const {
//   // std::vector<std::vector<int>> slices;
//   std::vector<IntervalColor> slices;
//   std::stringstream ss(slicesStr);
//   std::string sliceStr;
//   while (std::getline(ss, sliceStr, ';')) {
//     slices.push_back(parseSlice(sliceStr));
//   }
//   return slices;
// }

// Data Settings::parseLine(const std::string& line) const {
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

bool Settings::LoadClocksFromFile(std::vector<Data>& dataList) const {
  std::string bufferSettings;

  const char* filepath = "/clocks.txt";

  // lfs_file_t clocksFile;
  // if (fs.FileOpen(&clocksFile, "/clocks.txt", LFS_O_RDONLY) == LFS_ERR_OK) {
  //   fs.FileRead(&clocksFile, reinterpret_cast<uint8_t*>(&bufferSettings), sizeof(settings));
  //   fs.FileClose(&clocksFile);
  //   if (bufferSettings.version == settingsVersion) {
  //     settings = bufferSettings;
  //   }
  // }

  lfs_info info = {0};
  if (fs.Stat(filepath, &info) != LFS_ERR_NOENT) {

    lfs_file_t clocksFile;

    if (fs.FileOpen(&clocksFile, filepath, LFS_O_RDONLY) == LFS_ERR_OK) {
      // int openRes = fs.FileOpen(&clocksFile, filepath, LFS_O_RDONLY);
      // if (openRes == 0) {

      size_t bufferSize = info.size; // Adjust buffer size as needed

      // uint8_t readBuffer[bufferSize];
      // size_t bytesRead = fs.FileRead(&clocksFile, readBuffer, bufferSize);
      // bufferSettings.assign(readBuffer, readBuffer + bytesRead);

      std::vector<uint8_t> readBuffer(bufferSize);
      size_t bytesRead = fs.FileRead(&clocksFile, readBuffer.data(), bufferSize);
      bufferSettings.assign(readBuffer.begin(), readBuffer.begin() + bytesRead);

      fs.FileClose(&clocksFile);
    } else {
      return false;
    }
  } else {
    return false;
  }

  if (bufferSettings.empty()) {
    return false;
  }

  size_t pos = 0;
  size_t newLinePos;
  while ((newLinePos = bufferSettings.find('\n', pos)) != std::string::npos) {
    std::string line = bufferSettings.substr(pos, newLinePos - pos);
    if (!line.empty()) {
      Data data;
      if (parseLine(line, data)) {
        dataList.push_back(data);
      }
    }
    pos = newLinePos + 1;
  }

  // Handle the last line if it doesn't end with a newline character
  if (pos < bufferSettings.size()) {
    std::string line = bufferSettings.substr(pos);
    if (!line.empty()) {
      Data data;
      if (parseLine(line, data)) {
        dataList.push_back(data);
      }
    }
  }

  bufferSettings.clear();

  return true;
}

// IntervalColor Settings::parseSlice(const std::string& sliceStr) const {
//   IntervalColor slice;
//   size_t pos = 0;
//   size_t commaPos = sliceStr.find(',');

//   slice.start = static_cast<int16_t>(std::stoi(sliceStr.substr(pos, commaPos - pos)));
//   pos = commaPos + 1;
//   commaPos = sliceStr.find(',', pos);

//   slice.end = static_cast<int16_t>(std::stoi(sliceStr.substr(pos, commaPos - pos)));
//   pos = commaPos + 1;
//   commaPos = sliceStr.find(',', pos);

//   slice.red = static_cast<uint8_t>(std::stoi(sliceStr.substr(pos, commaPos - pos)));
//   pos = commaPos + 1;
//   commaPos = sliceStr.find(',', pos);

//   slice.green = static_cast<uint8_t>(std::stoi(sliceStr.substr(pos, commaPos - pos)));
//   pos = commaPos + 1;
//   commaPos = sliceStr.find(',', pos);

//   slice.blue = static_cast<uint8_t>(std::stoi(sliceStr.substr(pos, commaPos - pos)));
//   pos = commaPos + 1;
//   commaPos = sliceStr.find(',', pos);

//   slice.icon = static_cast<uint8_t>(std::stoi(sliceStr.substr(pos, commaPos - pos)));

//   return slice;
// }
bool Settings::parseSlice(const std::string& sliceStr, IntervalColor& slice) const {
  size_t pos = 0;
  size_t commaPos;
  std::vector<std::string> tokens;

  while ((commaPos = sliceStr.find(',', pos)) != std::string::npos) {
    tokens.push_back(sliceStr.substr(pos, commaPos - pos));
    pos = commaPos + 1;
  }
  tokens.push_back(sliceStr.substr(pos)); // Last element

  if (tokens.size() != 6)
    return false;

  // auto isValidNumber = [](const std::string& str, bool allowNegative = false) {
  //   if (str.empty())
  //     return false;
  //   size_t start = 0;
  //   if (allowNegative && str[0] == '-') {
  //     if (str.size() == 1)
  //       return false;
  //     start = 1;
  //   }
  //   // for (size_t i = start; i < str.size(); ++i) {
  //   //   if (!std::isdigit(static_cast<unsigned char>(str[i])))
  //   //     return false;
  //   // }
  //   return true;
  // };

  // if (!isValidNumber(tokens[0]) || !isValidNumber(tokens[1]) || !isValidNumber(tokens[2]) || !isValidNumber(tokens[3]) ||
  //     !isValidNumber(tokens[4]) || !isValidNumber(tokens[5])) {
  //   return false;
  // }

  slice.start = static_cast<int16_t>(std::stoi(tokens[0]));
  slice.end = static_cast<int16_t>(std::stoi(tokens[1]));
  slice.red = static_cast<uint8_t>(std::stoi(tokens[2]));
  slice.green = static_cast<uint8_t>(std::stoi(tokens[3]));
  slice.blue = static_cast<uint8_t>(std::stoi(tokens[4]));
  slice.icon = static_cast<uint8_t>(std::stoi(tokens[5]));

  return true;
}

std::vector<IntervalColor> Settings::parseSlices(const std::string& slicesStr) const {
  std::vector<IntervalColor> slices;
  size_t pos = 0;
  size_t semicolonPos;

  while ((semicolonPos = slicesStr.find(';', pos)) != std::string::npos) {
    std::string sliceStr = slicesStr.substr(pos, semicolonPos - pos);
    IntervalColor slice;
    if (parseSlice(sliceStr, slice)) {
      slices.push_back(slice);
    }
    pos = semicolonPos + 1;
  }

  // Handle the last slice
  if (pos < slicesStr.size()) {
    IntervalColor slice;
    if (parseSlice(slicesStr.substr(pos), slice)) {
      slices.push_back(slice);
    }
  }

  return slices;
}

// Data Settings::parseLine(const std::string& line) const {
//   // printf("parseLine: %s\n", line.c_str());

//   Data data;
//   size_t pos = 0;
//   size_t firstPipe = line.find('|');

//   // First value: 8-bit day mask
//   uint8_t value = static_cast<uint8_t>(std::stoi(line.substr(0, firstPipe)));

//   data.monday = (value >> 0) & 1;
//   data.tuesday = (value >> 1) & 1;
//   data.wednesday = (value >> 2) & 1;
//   data.thursday = (value >> 3) & 1;
//   data.friday = (value >> 4) & 1;
//   data.saturday = (value >> 5) & 1;
//   data.sunday = (value >> 6) & 1;
//   // data.extraFlag = (value >> 7) & 1;

//   // Find last pipe
//   size_t lastPipe = line.rfind('|');

//   // Extract the middle substring (between firstPipe+1 and lastPipe-1)
//   std::string datesStr = line.substr(firstPipe + 1, lastPipe - firstPipe - 1);

//   // Split middle section by '|'
//   std::vector<std::string> datesStrings = splitString(datesStr, '|');
//   std::vector<Date> dates;

//   for (const std::string& d : datesStrings) {
//     if (d.size() != 3) {
//       // handle error: invalid date length
//       continue;
//     }

//     Date date;
//     date.day = static_cast<uint8_t>(d[0]);
//     date.month = static_cast<uint8_t>(d[1]);
//     date.year = 2000 + static_cast<uint8_t>(d[2]);

//     dates.push_back(date);
//   }

//   data.dates = dates;

//   // Extract last part (after last pipe) — semicolon-separated slices
//   std::string lastSection = line.substr(lastPipe + 1);
//   data.slices = parseSlices(lastSection);

//   return data;
// }
bool Settings::parseLine(const std::string& line, Data& outData) const {
  // printf("parseLine: %s\n", line.c_str());

  Data data;
  size_t firstPipe = line.find('|');
  size_t lastPipe = line.rfind('|'); // Find last pipe

  // First value: 8-bit day mask
  // uint8_t value = static_cast<uint8_t>(std::stoi(line.substr(0, firstPipe)));
  std::string dayMaskStr = line.substr(0, firstPipe);

  // if (!std::all_of(dayMaskStr.begin(), dayMaskStr.end(), ::isdigit)) {
  //   return false;
  // }
  for (char c : dayMaskStr) {
    if (!std::isdigit(static_cast<unsigned char>(c))) {
      return false;
    }
  }

  int temp = std::stoi(dayMaskStr);
  if (temp < 0 || temp > 255) {
    return false;
  }
  uint8_t value = static_cast<uint8_t>(temp);

  data.monday = (value >> 0) & 1;
  data.tuesday = (value >> 1) & 1;
  data.wednesday = (value >> 2) & 1;
  data.thursday = (value >> 3) & 1;
  data.friday = (value >> 4) & 1;
  data.saturday = (value >> 5) & 1;
  data.sunday = (value >> 6) & 1;
  // data.extraFlag = (value >> 7) & 1;

  // Extract the middle substring (between firstPipe+1 and lastPipe-1)
  std::string datesStr = line.substr(firstPipe + 1, lastPipe - firstPipe - 1);

  // Split middle section by '|'
  std::vector<std::string> datesStrings = splitString(datesStr, '|');
  std::vector<Date> dates;

  for (const std::string& d : datesStrings) {
    // if (d.size() != 3) {
    //   // handle error: invalid date length
    //   continue;
    // }
    if (!isdigit(d[0]) || !isdigit(d[1]) || !isdigit(d[2]))
      continue;

    Date date;

    // date.day = static_cast<uint8_t>(d[0]);
    // date.month = static_cast<uint8_t>(d[1]);
    // date.year = 2000 + static_cast<uint8_t>(d[2]);
    date.day = d[0] - '0';
    date.month = d[1] - '0';
    date.year = 2000 + (d[2] - '0');

    dates.push_back(date);
  }

  data.dates = dates;

  // Extract last part (after last pipe) — semicolon-separated slices
  std::string lastSection = line.substr(lastPipe + 1);
  data.slices = parseSlices(lastSection);
  if (data.slices.size() == 0) {
    // std::cout << "data.slices.size(): " << data.slices.size() << std::endl;
    // std::cout << "lastSection: " << lastSection << std::endl;
    // std::cout << "firstPipe: " << firstPipe << std::endl;
    // std::cout << "lastPipe: " << lastPipe << std::endl;
    // std::cout << "line: " << line << std::endl;
    return false;
  }

  outData = data;
  return true;
}

std::vector<std::string> Settings::splitString(const std::string& str, char delimiter) const {
  std::vector<std::string> result;
  size_t start = 0;
  size_t end = str.find(delimiter);

  while (end != std::string::npos) {
    result.push_back(str.substr(start, end - start));
    start = end + 1;
    end = str.find(delimiter, start);
  }

  // Add the last token
  if (start < str.size()) {
    result.push_back(str.substr(start));
  }

  return result;
}

// bool Settings::isValidNumber(const std::string& str) {
//   if (str.empty())
//     return false;
//   size_t start = 0;
//   for (size_t i = start; i < str.size(); ++i) {
//     if (!std::isdigit(static_cast<unsigned char>(str[i])))
//       return false;
//   }
//   return true;
// }
#include "components/settings/Settings.h"
#include <cstdlib>
#include <cstring>
#include <vector> //GetSchedule()
// #include <sstream>

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

void Settings::SaveSettings() {

  // verify if is necessary to save
  if (settingsChanged) {
    SaveSettingsToFile();
  }
  settingsChanged = false;
}

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

void Settings::SaveSettingsToFile() {
  lfs_file_t settingsFile;

  if (fs.FileOpen(&settingsFile, "/settings.dat", LFS_O_WRONLY | LFS_O_CREAT) != LFS_ERR_OK) {
    return;
  }
  fs.FileWrite(&settingsFile, reinterpret_cast<uint8_t*>(&settings), sizeof(settings));
  fs.FileClose(&settingsFile);
}

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

std::vector<Data> Settings::LoadClocksFromFile() const {
  std::string bufferSettings;
  std::vector<Data> dataList;

  // lfs_file_t clocksFile;
  // if (fs.FileOpen(&clocksFile, "/clocks.txt", LFS_O_RDONLY) == LFS_ERR_OK) {
  //   fs.FileRead(&clocksFile, reinterpret_cast<uint8_t*>(&bufferSettings), sizeof(settings));
  //   fs.FileClose(&clocksFile);
  //   if (bufferSettings.version == settingsVersion) {
  //     settings = bufferSettings;
  //   }
  // }

  lfs_info info = {0};
  if (fs.Stat("/clocks.txt", &info) != LFS_ERR_NOENT) {
    // printf("info.size %d\n", info.size);

    lfs_file_t clocksFile;

    if (fs.FileOpen(&clocksFile, "/clocks.txt", LFS_O_RDONLY) == LFS_ERR_OK) {
      // int openRes = fs.FileOpen(&clocksFile, "/clocks.txt", LFS_O_RDONLY);
      // if (openRes == 0) {

      // printf("FileOpen %d\n", 1);

      size_t bufferSize = info.size; // Adjust buffer size as needed

      // uint8_t readBuffer[bufferSize];
      // size_t bytesRead = fs.FileRead(&clocksFile, readBuffer, bufferSize);
      // bufferSettings.assign(readBuffer, readBuffer + bytesRead);

      std::vector<uint8_t> readBuffer(bufferSize);
      size_t bytesRead = fs.FileRead(&clocksFile, readBuffer.data(), bufferSize);
      bufferSettings.assign(readBuffer.begin(), readBuffer.begin() + bytesRead);

      fs.FileClose(&clocksFile);
    }
  }

  if (bufferSettings.empty()) {
    // bufferSettings =
    // "0|0|0|0|0|60,210,0,0,0,1;210,232,255,44,0,2;232,250,255,199,0,3;250,270,91,185,0,4;270,360,0,113,255,5;0,210,0,0,0,0\n0|0|0|0|1|0,30,255,44,0,6;30,60,50,50,50,1;60,150,255,166,0,5;150,165,255,199,0,4;165,195,91,185,0,7;195,210,0,207,255,8;210,240,0,113,255,6;240,255,128,66,209,9;270,300,0,0,0,1;270,570,0,0,0,0\n0|0|0|1|0|60,210,0,0,0,1;210,232,255,44,0,2;232,250,255,199,0,3;250,360,91,185,0,7;0,210,0,0,0,0\n0|0|0|1|1|0,30,255,44,0,6;30,60,50,50,50,1;60,90,255,166,0,10;90,195,91,185,0,7;195,210,0,207,255,8;210,240,0,113,255,6;240,255,128,66,209,9;270,300,0,0,0,1;270,570,0,0,0,0";
    bufferSettings =
      "0|0|0|0|60,210,0,0,0,1;210,232,255,44,0,2;232,250,255,199,0,1;250,270,91,185,0,1;270,360,0,113,255,1;0,210,0,0,0,0;0,"
      "30,255,44,0,2;30,60,50,50,50,1;60,150,255,166,0,1;150,165,255,199,0,1;165,195,91,185,0,1;195,210,0,207,255,1;210,240,0,113,255,1;"
      "240,255,128,66,209,1;270,300,0,0,0,1;270,570,0,0,0,0\n0|0|0|1|60,210,0,0,0,1;210,232,255,44,0,1;232,250,255,199,0,1;250,360,91,"
      "185,0,1;0,210,0,0,0,0;0,30,255,44,0,1;30,60,50,50,50,1;60,90,255,166,0,1;90,195,91,185,0,1;195,210,0,207,255,1;210,240,0,"
      "113,255,1;240,255,128,66,209,1;270,300,0,0,0,1;270,570,0,0,0,0";
  }

  size_t pos = 0;
  size_t newLinePos;
  while ((newLinePos = bufferSettings.find('\n', pos)) != std::string::npos) {
    std::string line = bufferSettings.substr(pos, newLinePos - pos);
    if (!line.empty()) {
      Data data = parseLine(line); // Assuming parseLine function exists
      dataList.push_back(data);
    }
    pos = newLinePos + 1;
  }

  // Handle the last line if it doesn't end with a newline character
  if (pos < bufferSettings.size()) {
    std::string line = bufferSettings.substr(pos);
    if (!line.empty()) {
      Data data = parseLine(line); // Assuming parseLine function exists
      dataList.push_back(data);
    }
  }

  bufferSettings.clear();

  return dataList;
}

IntervalColor Settings::parseSlice(const std::string& sliceStr) const {
  IntervalColor slice;
  size_t pos = 0;
  size_t commaPos = sliceStr.find(',');

  slice.start = static_cast<int16_t>(std::stoi(sliceStr.substr(pos, commaPos - pos)));
  pos = commaPos + 1;
  commaPos = sliceStr.find(',', pos);

  slice.end = static_cast<int16_t>(std::stoi(sliceStr.substr(pos, commaPos - pos)));
  pos = commaPos + 1;
  commaPos = sliceStr.find(',', pos);

  slice.red = static_cast<uint8_t>(std::stoi(sliceStr.substr(pos, commaPos - pos)));
  pos = commaPos + 1;
  commaPos = sliceStr.find(',', pos);

  slice.green = static_cast<uint8_t>(std::stoi(sliceStr.substr(pos, commaPos - pos)));
  pos = commaPos + 1;
  commaPos = sliceStr.find(',', pos);

  slice.blue = static_cast<uint8_t>(std::stoi(sliceStr.substr(pos, commaPos - pos)));
  pos = commaPos + 1;
  commaPos = sliceStr.find(',', pos);

  slice.icon = static_cast<uint8_t>(std::stoi(sliceStr.substr(pos, commaPos - pos)));

  return slice;
}

std::vector<IntervalColor> Settings::parseSlices(const std::string& slicesStr) const {
  std::vector<IntervalColor> slices;
  size_t pos = 0;
  size_t semicolonPos;
  while ((semicolonPos = slicesStr.find(';', pos)) != std::string::npos) {
    std::string sliceStr = slicesStr.substr(pos, semicolonPos - pos);
    slices.push_back(parseSlice(sliceStr));
    pos = semicolonPos + 1;
  }

  // Handle the last slice
  if (pos < slicesStr.size()) {
    slices.push_back(parseSlice(slicesStr.substr(pos)));
  }

  return slices;
}

Data Settings::parseLine(const std::string& line) const {
  // printf("parseLine: %s\n", line.c_str());

  Data data;
  size_t pos = 0;
  size_t pipePos;

  pipePos = line.find('|', pos);
  data.year = std::stoi(line.substr(pos, pipePos - pos));
  pos = pipePos + 1;

  pipePos = line.find('|', pos);
  data.month = std::stoi(line.substr(pos, pipePos - pos));
  pos = pipePos + 1;

  pipePos = line.find('|', pos);
  data.day = std::stoi(line.substr(pos, pipePos - pos));
  pos = pipePos + 1;

  pipePos = line.find('|', pos);
  data.weekend = std::stoi(line.substr(pos, pipePos - pos));
  pos = pipePos + 1;

  // printf("line.substr(pos): %s\n", line.substr(pos).c_str());

  data.slices = parseSlices(line.substr(pos));

  return data;
}
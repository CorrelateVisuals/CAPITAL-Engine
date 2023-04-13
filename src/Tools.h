#pragma once

#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>

void logStringToFile(const std::string& message) {
  // Get the current time as a string
  auto now = std::chrono::system_clock::now();
  std::time_t now_c = std::chrono::system_clock::to_time_t(now);
  char now_str[26];
  struct tm timeinfo;
  localtime_s(&timeinfo, &now_c);
  std::strftime(now_str, sizeof(now_str), "[%Y-%m-%d %H:%M:%S]", &timeinfo);

  // Open the log file for appending
  std::ofstream logFile("log.txt", std::ios::app);
  if (!logFile.is_open()) {
    std::cerr << "Error: Could not open log file for writing." << std::endl;
    return;
  }

  // Write the current time and message to the log file
  logFile << now_str << " " << message << std::endl;

  // Close the log file
  logFile.close();
}

//
// Created by Eric Saturn on 2019/12/12.
//

#include "debug_tools/print_tools.h"

using std::string;

namespace Net {
namespace PrintTools {

void print_buff(uint8_t *const buf, size_t size);

void debugPrintError(const string error_info, ...) {
  va_list args;
  va_start(args, error_info);
  string output = "\033[31m[Error] ";
  output += error_info + string("\033[0m\n");
  vprintf(output.c_str(), args);
}

void debugPrintWarning(const string warning_info, ...) {
  va_list args;
  va_start(args, warning_info);
  string output = "\033[33m[Warning] ";
  output += warning_info + string("\033[0m\n");
  vprintf(output.c_str(), args);
}

void debugPrintSuccess(const string success_info, ...) {
  va_list args;
  va_start(args, success_info);
  string output = "\033[32m[Success] ";
  output += success_info + string("\033[0m\n");
  vprintf(output.c_str(), args);
}

void printInfo(const string &info, const string &tag) {
  printf("[DEBUG INFO] %s ", info.data());
  if (!tag.empty())
    printf("{ %s }\n", tag.data());
}

void printInfoFormal(const string &title, initializer_list<FormalItem> body) {
  printf("\n>>>\n {%s}\n", title.data());
  printf(">-------------------------------------\n");
  for (auto &item : body) {
    printf("[%s] : \"%s\"; \n", item.first.data(), item.second.data());
  }
  printf("----------------------------------<\n<<<\n\n");
}

void print_buff(uint8_t *const buf, size_t size) {

  printf("\n[DEBUG INFO (BUFFER)]\n");
  printf(">----------------------------------------------\n");
  auto *p_i = buf;
  auto *p_e = (uint8_t *const) & buf[size - 1];
  for (int c = 0; p_i < p_e; ++p_i, ++c) {
    if (!(c % 16) && c)
      printf("\n");
    printf("%02x ", *p_i);
  }
  printf("\n");
  printf("----------------------------------------------<\n");
}

void printInfoBuffer(const string &info, const string &tag) {
  print_buff((uint8_t *)&info[0], info.size());
  if (!tag.empty())
    printf("{ %s }\n\n", tag.data());
}

void printInfoBuffer(const vector<char> &info, const string &tag) {
  print_buff((uint8_t *)&info[0], info.size());
  if (!tag.empty())
    printf("{ %s }\n\n", tag.data());
}

} // namespace PrintTools
} // namespace Net

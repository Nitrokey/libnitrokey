#include <string>

void clear_string(std::string &s) {
  std::fill(s.begin(), s.end(), ' ');
}
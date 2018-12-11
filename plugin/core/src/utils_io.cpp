
#include <iterator>
#include "core/include/utils_io.h"

namespace feasst {

// thanks to http://www.cplusplus.com/forum/beginner/87238/
std::vector<std::string> split(const std::string str) {
  std::istringstream buffer(str);
  std::istream_iterator<std::string> begin(buffer), end;
  std::vector<std::string> tokens(begin, end);
  return tokens;
}

std::string trim(const char* specialchr, const char* phrase, int from_left) {
  std::string fs(phrase);

  // find position of last spchr
  std::string spchr(specialchr);
  std::size_t found = fs.find(spchr);
  std::size_t foundprev = 0;
  while (found != std::string::npos) {
    foundprev = found;
    found = fs.find(spchr, found + 1);
  }

  // erase all characters up to spchr
  if (int(foundprev) != 0) {
    if (from_left == 1) {
      fs.erase(fs.begin(), fs.begin() + foundprev + spchr.size());
    } else {
      fs.erase(fs.begin() + foundprev + spchr.size(), fs.end());
    }
  }

  return fs;
}
std::string trim(const char* specialchr, std::string phrase, int from_left) {
  return trim(specialchr, phrase.c_str(), from_left);
}

}  // namespace feasst

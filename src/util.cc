#include "util.h"

// Since for big searches, the pattern string will be looked up repeatedly,
//   we can cache the failure array;
std::string last_matched_string;
std::vector<int> last_failure_array;

bool fuzzy_match(std::string& needle, std::string& haystack) {
  // KMP Search Algorithm with:
  // - cached needle failure array
  // - case insensitive
  // - ignores non alphanumerical characteres
  if (needle.empty()) {
    return true;
  }

  if (needle != last_matched_string) {
    // Generate failure array
    last_failure_array.clear();
    last_failure_array.resize(needle.size(), 0);
     for (size_t i = 1, k = 0; i < needle.size(); ++i) {
      while (k && std::toupper(needle[k]) != std::toupper(needle[i])) {
        k = last_failure_array[k - 1];
      }
      if (std::toupper(needle[k]) == std::toupper(needle[i])) {
        k += 1;
      }
      last_failure_array[i] = k;
    }
    last_matched_string = needle;
  }

  for (size_t i = 0, k = 0; i < haystack.size(); ++i) {
    while (k && std::toupper(needle[k]) != std::toupper(haystack[i]) &&
           std::isalnum(haystack[i])) {
      k = last_failure_array[k - 1];
    }
    if (std::toupper(needle[k]) == std::toupper(haystack[i])) {
      k += 1;
    }
    if (k == needle.size()) {
      return i - k + 1 >= 0;
    }
  }

  return false;
}

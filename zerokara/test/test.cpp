#include <string>
#include <iostream>
using std::string;
using std::string_view;
int main(void) {
  string s("stuff:blah;");
  string_view sv(s);
  size_t colon_pos = sv.find(':');
  string parsed = string(sv.data(), colon_pos);
  std::cout << parsed << std::endl;
  return 0;
}

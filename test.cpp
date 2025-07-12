#include <assert.h>
#include <stdio.h>
#include <string>
const char *test = "3.5";
int main(void) {
  double parsed;
  int successes = sscanf(test, "%lf", &parsed);
  if (successes != 1) { assert(0 && "sscanf is fucked"); }
  printf("%lf\n", parsed);

  double parsed_cpp = std::stod(test);
  printf("%lf\n", parsed_cpp);
  return 0;
}

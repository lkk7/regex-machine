#include "regex-machine/regex-machine.hpp"

#include "testutils.hpp"

int main() {
  auto const result = name();

  return result == "regex-machine" ? 0 : 1;
}

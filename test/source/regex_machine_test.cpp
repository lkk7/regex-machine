#include "regex_machine/regex_machine.hpp"

#include <catch2/catch.hpp>

TEST_CASE("Test case works") {
  auto result = name();
  REQUIRE_THAT(result, Catch::Matchers::Equals("regex-machine"));
}

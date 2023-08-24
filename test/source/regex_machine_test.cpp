#include "regex_machine.hpp"

#include <catch2/catch_all.hpp>

using RM::Matcher;

TEST_CASE("Matcher::match") {
  SECTION("a") {
    Matcher matcher{"a"};
    REQUIRE(matcher.match("a"));
    REQUIRE(!matcher.match("b"));
    REQUIRE(!matcher.match(""));
  }

  SECTION("ab") {
    Matcher matcher{"ab"};
    REQUIRE(matcher.match("ab"));
    REQUIRE(!matcher.match("a"));
    REQUIRE(!matcher.match("b"));
    REQUIRE(!matcher.match("c"));
  }

  SECTION("a|b") {
    Matcher matcher{"a|b"};
    REQUIRE(matcher.match("a"));
    REQUIRE(matcher.match("b"));
    REQUIRE(!matcher.match("ab"));
    REQUIRE(!matcher.match("ba"));
    REQUIRE(!matcher.match("ba"));
  }

  SECTION("(xy)*") {
    Matcher matcher{"(xy)*"};
    REQUIRE(matcher.match("xy"));
    REQUIRE(matcher.match("xyxy"));
    REQUIRE(matcher.match("xyxyxyxy"));
    REQUIRE(!matcher.match("xyxyx"));
    REQUIRE(!matcher.match("xyxyy"));
  }

  SECTION("(x|y)*") {
    Matcher matcher{"(x|y)*"};
    REQUIRE(matcher.match(""));
    REQUIRE(matcher.match("xy"));
    REQUIRE(matcher.match("xyxy"));
    REQUIRE(matcher.match("xyxyxyxy"));
    REQUIRE(!matcher.match("xyxyz"));
  }

  SECTION("(a|b|c)(xyz)*") {
    Matcher matcher{"(a|b|c)(xyz)*"};
    REQUIRE(matcher.match("a"));
    REQUIRE(matcher.match("b"));
    REQUIRE(matcher.match("c"));
    REQUIRE(matcher.match("axyz"));
    REQUIRE(matcher.match("bxyzxyz"));
    REQUIRE(matcher.match("cxyzxyzxyz"));
    REQUIRE(!matcher.match("ab"));
    REQUIRE(!matcher.match(""));
  }

  SECTION("(a|b)(x|y)") {
    Matcher matcher{"(a|b)(x|y)*"};
    REQUIRE(matcher.match("ax"));
    REQUIRE(matcher.match("ay"));
    REQUIRE(matcher.match("bx"));
    REQUIRE(matcher.match("by"));
    REQUIRE(!matcher.match("ab"));
    REQUIRE(!matcher.match("xy"));
    REQUIRE(!matcher.match(""));
    REQUIRE(1 == 1);
  }

  SECTION("errors") {
    // Errors are tested in detail in parsing_test.cpp
    Matcher matcher{"(a"};
    REQUIRE(!matcher.err_msg.empty());
    matcher = Matcher{"()"};
    REQUIRE(!matcher.err_msg.empty());
    matcher = Matcher{"(())"};
    REQUIRE(!matcher.err_msg.empty());
    matcher = Matcher{"()()"};
    REQUIRE(!matcher.err_msg.empty());
    matcher = Matcher{""};
    REQUIRE(!matcher.err_msg.empty());
  }
}

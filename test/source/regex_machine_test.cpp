#include "regex_machine.hpp"

#include <catch2/catch_all.hpp>

using RM::Matcher;

TEST_CASE("Matcher::match") {
  SECTION("a") {
    const Matcher matcher{"a"};
    REQUIRE(matcher.err_msg.empty());
    REQUIRE(matcher.match("a"));
    REQUIRE(!matcher.match("b"));
    REQUIRE(!matcher.match(""));
  }

  SECTION("ab") {
    const Matcher matcher{"ab"};
    REQUIRE(matcher.err_msg.empty());
    REQUIRE(matcher.match("ab"));
    REQUIRE(!matcher.match("a"));
    REQUIRE(!matcher.match("b"));
    REQUIRE(!matcher.match("c"));
  }

  SECTION("a|b") {
    const Matcher matcher{"a|b"};
    REQUIRE(matcher.err_msg.empty());
    REQUIRE(matcher.match("a"));
    REQUIRE(matcher.match("b"));
    REQUIRE(!matcher.match("ab"));
    REQUIRE(!matcher.match("ba"));
    REQUIRE(!matcher.match("ba"));
  }

  SECTION("(xy)*") {
    const Matcher matcher{"(xy)*"};
    REQUIRE(matcher.err_msg.empty());
    REQUIRE(matcher.match("xy"));
    REQUIRE(matcher.match("xyxy"));
    REQUIRE(matcher.match("xyxyxyxy"));
    REQUIRE(!matcher.match("xyxyx"));
    REQUIRE(!matcher.match("xyxyy"));
  }

  SECTION("(x|y)*") {
    const Matcher matcher{"(x|y)*"};
    REQUIRE(matcher.err_msg.empty());
    REQUIRE(matcher.match(""));
    REQUIRE(matcher.match("xy"));
    REQUIRE(matcher.match("xyxy"));
    REQUIRE(matcher.match("xyxyxyxy"));
    REQUIRE(!matcher.match("xyxyz"));
  }

  SECTION("(a|b|c)(xyz)*") {
    const Matcher matcher{"(a|b|c)(xyz)*"};
    REQUIRE(matcher.err_msg.empty());
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
    const Matcher matcher{"(a|b)(x|y)*"};
    REQUIRE(matcher.err_msg.empty());
    REQUIRE(matcher.match("ax"));
    REQUIRE(matcher.match("ay"));
    REQUIRE(matcher.match("bx"));
    REQUIRE(matcher.match("by"));
    REQUIRE(!matcher.match("ab"));
    REQUIRE(!matcher.match("xy"));
    REQUIRE(!matcher.match(""));
  }

  SECTION("a?") {
    const Matcher matcher{"a?"};
    REQUIRE(matcher.err_msg.empty());
    REQUIRE(matcher.match(""));
    REQUIRE(matcher.match("a"));
    REQUIRE(!matcher.match("b"));
    REQUIRE(!matcher.match("aa"));
  }

  SECTION("(ab)?c*") {
    const Matcher matcher{"(ab)?c*"};
    REQUIRE(matcher.err_msg.empty());
    REQUIRE(matcher.match(""));
    REQUIRE(matcher.match("c"));
    REQUIRE(matcher.match("cc"));
    REQUIRE(matcher.match("abccccc"));
    REQUIRE(!matcher.match("aba"));
    REQUIRE(!matcher.match("d"));
  }

  SECTION("a+") {
    const Matcher matcher{"a+"};
    REQUIRE(matcher.err_msg.empty());
    REQUIRE(matcher.match("a"));
    REQUIRE(matcher.match("aa"));
    REQUIRE(!matcher.match(""));
    REQUIRE(!matcher.match("b"));
    REQUIRE(!matcher.match("ba"));
  }

  SECTION("(ab)+c*") {
    const Matcher matcher{"(ab)+c*"};
    REQUIRE(matcher.err_msg.empty());
    REQUIRE(matcher.match("ab"));
    REQUIRE(matcher.match("abc"));
    REQUIRE(matcher.match("ababccccc"));
    REQUIRE(!matcher.match(""));
    REQUIRE(!matcher.match("ababd"));
    REQUIRE(!matcher.match("aba"));
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

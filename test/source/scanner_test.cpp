// NOLINTBEGIN(
//   bugprone-easily-swappable-parameters,
// )
#include "internal/scanner.hpp"

#include <catch2/catch_all.hpp>
#include <unordered_set>

using RM::Impl::Scanner;

void REQUIRE_SCANNER_EQ(std::string&& input, std::string&& regex,
                        size_t node_charcount, int paren_balance,
                        std::unordered_set<size_t> escapes) {
  Scanner result{input};
  REQUIRE(result.regex == regex);
  REQUIRE(result.node_charcount == node_charcount);
  REQUIRE(result.paren_balance == paren_balance);
  REQUIRE(result.escapes == escapes);
}

TEST_CASE("Scanner::peek_token") {
  SECTION("standard string") {
    Scanner scanner{"abc"};
    REQUIRE(scanner.peek() == 'a');
    scanner.pop();
    REQUIRE(scanner.peek() == '.');
    scanner.pop();
    REQUIRE(scanner.peek() == 'b');
    scanner.pop();
    REQUIRE(scanner.peek() == '.');
    scanner.pop();
    REQUIRE(scanner.peek() == 'c');
    scanner.pop();
    REQUIRE(scanner.peek() == '\0');
  }

  SECTION("empty string") {
    Scanner scanner{""};
    REQUIRE(scanner.peek() == '\0');
  }
}

TEST_CASE("Scanner::pop_token") {
  SECTION("standard string") {
    Scanner scanner{"abc"};
    REQUIRE(scanner.pop() == 'a');
    REQUIRE(scanner.pop() == '.');
    REQUIRE(scanner.pop() == 'b');
    REQUIRE(scanner.pop() == '.');
    REQUIRE(scanner.pop() == 'c');
    REQUIRE(scanner.pop() == '\0');
  }

  SECTION("empty string") {
    Scanner scanner{""};
    REQUIRE(scanner.pop() == '\0');
  }
}

TEST_CASE("Scanner::Scanner") {
  // Non-escaped strings
  REQUIRE_SCANNER_EQ("", "", 0, 0, {});
  REQUIRE_SCANNER_EQ("a", "a", 1, 0, std::unordered_set<size_t>{});
  REQUIRE_SCANNER_EQ("ab", "a.b", 3, 0, std::unordered_set<size_t>{});
  REQUIRE_SCANNER_EQ("abc", "a.b.c", 5, 0, std::unordered_set<size_t>{});
  REQUIRE_SCANNER_EQ("abcd", "a.b.c.d", 7, 0, std::unordered_set<size_t>{});
  REQUIRE_SCANNER_EQ("a|bc", "a|b.c", 5, 0, std::unordered_set<size_t>{});
  REQUIRE_SCANNER_EQ("ab|c", "a.b|c", 5, 0, std::unordered_set<size_t>{});
  REQUIRE_SCANNER_EQ("a*b*c?d|e", "a*.b*.c?.d|e", 12, 0,
                     std::unordered_set<size_t>{});
  REQUIRE_SCANNER_EQ("(a?b*)(c)(def)?gh|iabc",
                     "(a?.b*).(c).(d.e.f)?.g.h|i.a.b.c", 26, 0,
                     std::unordered_set<size_t>{});
  REQUIRE_SCANNER_EQ("(a", "(a", 1, 1, std::unordered_set<size_t>{});
  REQUIRE_SCANNER_EQ("a)", "a)", 1, -1, std::unordered_set<size_t>{});

  // Escaped strings
  REQUIRE_SCANNER_EQ("a\\)", "a.)", 3, 0, {2});
  REQUIRE_SCANNER_EQ("\\(a", "(.a", 3, 0, {0});
  REQUIRE_SCANNER_EQ("a\\*", "a.*", 3, 0, {2});
  REQUIRE_SCANNER_EQ("a\\|b", "a.|.b", 5, 0, {2});
  REQUIRE_SCANNER_EQ("((\\*))", "((*))", 1, 0, {2});
  REQUIRE_SCANNER_EQ("((a\\*))", "((a.*))", 3, 0, {4});
  REQUIRE_SCANNER_EQ("\\((ab)?\\)", "(.(a.b)?.)", 8, 0, {0, 9});
}

// NOLINTEND(
//   bugprone-easily-swappable-parameters,
// )
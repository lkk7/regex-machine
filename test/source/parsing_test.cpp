// NOLINTBEGIN(
//   bugprone-easily-swappable-parameters,
//   cppcoreguidelines-avoid-magic-numbers,
//   readability-magic-numbers
// )
#include "regex_machine/parsing.hpp"

#include <catch2/catch.hpp>
#include <string>

using RegexMachine::Node;
using RegexMachine::Parser;
using RegexMachine::Scanner;

void REQUIRE_SCANNER_EQ(std::string&& input, std::string&& regex,
                        size_t node_charcount) {
  Scanner result{input};
  REQUIRE(result.regex == regex);
  REQUIRE(result.node_charcount == node_charcount);
}

void REQUIRE_NODE_EQ(Node result, Node::index left, Node::index right,
                     Node::NodeType type, char character) {
  REQUIRE(result.left == left);
  REQUIRE(result.right == right);
  REQUIRE(result.type == type);
  REQUIRE(result.character == character);
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
  REQUIRE_SCANNER_EQ("", "", 0);
  REQUIRE_SCANNER_EQ("a", "a", 1);
  REQUIRE_SCANNER_EQ("ab", "a.b", 3);
  REQUIRE_SCANNER_EQ("abc", "a.b.c", 5);
  REQUIRE_SCANNER_EQ("abcd", "a.b.c.d", 7);
  REQUIRE_SCANNER_EQ("a|bc", "a|b.c", 5);
  REQUIRE_SCANNER_EQ("ab|c", "a.b|c", 5);
  REQUIRE_SCANNER_EQ("a*b*c?d|e", "a*.b*.c?.d|e", 12);
  REQUIRE_SCANNER_EQ("(a?b*)(c)(def)?gh|i", "(a?.b*).(c).(d.e.f)?.g.h|i", 20);
}

TEST_CASE("Parser::parse") {
  SECTION("empty string") {
    auto result = Parser{""}.parse().nodes;
    REQUIRE(result.empty());
  }

  SECTION("one character") {
    auto result = Parser{"1"}.parse().nodes;
    REQUIRE(result.size() == 1);
    REQUIRE_NODE_EQ(result[0], -1, -1, Node::NodeType::CHAR, '1');
  }

  SECTION("single altered pair") {
    auto result = Parser{"a|b"}.parse().nodes;
    REQUIRE(result.size() == 3);
    REQUIRE_NODE_EQ(result[0], -1, -1, Node::NodeType::CHAR, 'a');
    REQUIRE_NODE_EQ(result[1], -1, -1, Node::NodeType::CHAR, 'b');
    REQUIRE_NODE_EQ(result[2], 0, 1, Node::NodeType::OR, '\0');
  }

  SECTION("single concatenated pair") {
    auto result = Parser{"ab"}.parse().nodes;
    REQUIRE(result.size() == 3);
    REQUIRE_NODE_EQ(result[0], -1, -1, Node::NodeType::CHAR, 'a');
    REQUIRE_NODE_EQ(result[1], -1, -1, Node::NodeType::CHAR, 'b');
    REQUIRE_NODE_EQ(result[2], 0, 1, Node::NodeType::CONCAT, '\0');
  }

  SECTION("zero or more of a single character") {
    auto result = Parser{"1*"}.parse().nodes;
    REQUIRE(result.size() == 2);
    REQUIRE_NODE_EQ(result[0], -1, -1, Node::NodeType::CHAR, '1');
    REQUIRE_NODE_EQ(result[1], 0, -1, Node::NodeType::ZERO_OR_MORE, '\0');
  }

  SECTION("optional character") {
    auto result = Parser{"1?"}.parse().nodes;
    REQUIRE(result.size() == 2);
    REQUIRE_NODE_EQ(result[0], -1, -1, Node::NodeType::CHAR, '1');
    REQUIRE_NODE_EQ(result[1], 0, -1, Node::NodeType::OPTIONAL, '\0');
  }

  SECTION("heavily parenthesized character") {
    auto result = Parser{"(((1)))"}.parse().nodes;
    REQUIRE(result.size() == 1);
    REQUIRE_NODE_EQ(result[0], -1, -1, Node::NodeType::CHAR, '1');
  }

  SECTION("parenthesises combined with other operators") {
    auto result = Parser{"(ab)*|1"}.parse().nodes;
    REQUIRE(result.size() == 6);
    REQUIRE_NODE_EQ(result[0], -1, -1, Node::NodeType::CHAR, 'a');
    REQUIRE_NODE_EQ(result[1], -1, -1, Node::NodeType::CHAR, 'b');
    REQUIRE_NODE_EQ(result[2], 0, 1, Node::NodeType::CONCAT, '\0');
    REQUIRE_NODE_EQ(result[3], 2, -1, Node::NodeType::ZERO_OR_MORE, '\0');
    REQUIRE_NODE_EQ(result[4], -1, -1, Node::NodeType::CHAR, '1');
    REQUIRE_NODE_EQ(result[5], 3, 4, Node::NodeType::OR, '\0');
  }
}

// NOLINTEND(
//   bugprone-easily-swappable-parameters,
//   cppcoreguidelines-avoid-magic-numbers,
//   readability-magic-numbers
// )
// NOLINTBEGIN(
//   bugprone-easily-swappable-parameters,
//   cppcoreguidelines-avoid-magic-numbers,
//   readability-magic-numbers
// )
#include "regex_machine/parsing.hpp"

#include <catch2/catch_all.hpp>
#include <string>

using RegexMachine::Node;
using RegexMachine::Parser;
using RegexMachine::Scanner;

void REQUIRE_SCANNER_EQ(std::string&& input, std::string&& regex,
                        size_t node_charcount, int paren_balance) {
  Scanner result{input};
  REQUIRE(result.regex == regex);
  REQUIRE(result.node_charcount == node_charcount);
  REQUIRE(result.paren_balance == paren_balance);
}

void REQUIRE_NODE_EQ(Node result, Node::index left, Node::index right,
                     Node::NodeType type, char character) {
  REQUIRE(result.left == left);
  REQUIRE(result.right == right);
  REQUIRE(result.type == type);
  REQUIRE(result.character == character);
}

void REQUIRE_NODE_ERR(std::string&& input, std::string&& err_msg) {
  REQUIRE(Parser{input}.parse().err_msg == err_msg);
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
  REQUIRE_SCANNER_EQ("", "", 0, 0);
  REQUIRE_SCANNER_EQ("a", "a", 1, 0);
  REQUIRE_SCANNER_EQ("ab", "a.b", 3, 0);
  REQUIRE_SCANNER_EQ("abc", "a.b.c", 5, 0);
  REQUIRE_SCANNER_EQ("abcd", "a.b.c.d", 7, 0);
  REQUIRE_SCANNER_EQ("a|bc", "a|b.c", 5, 0);
  REQUIRE_SCANNER_EQ("ab|c", "a.b|c", 5, 0);
  REQUIRE_SCANNER_EQ("a*b*c?d|e", "a*.b*.c?.d|e", 12, 0);
  REQUIRE_SCANNER_EQ("(a?b*)(c)(def)?gh|iabc",
                     "(a?.b*).(c).(d.e.f)?.g.h|i.a.b.c", 26, 0);
  REQUIRE_SCANNER_EQ("(a", "(a", 1, 1);
  REQUIRE_SCANNER_EQ("a)", "a)", 1, -1);
}

TEST_CASE("Parser::parse") {
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
    REQUIRE_NODE_EQ(result[1], 0, -1, Node::NodeType::KLEENE_STAR, '\0');
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
    REQUIRE_NODE_EQ(result[3], 2, -1, Node::NodeType::KLEENE_STAR, '\0');
    REQUIRE_NODE_EQ(result[4], -1, -1, Node::NodeType::CHAR, '1');
    REQUIRE_NODE_EQ(result[5], 3, 4, Node::NodeType::OR, '\0');
  }

  SECTION("logically empty regexes") {
    REQUIRE_NODE_ERR("", "empty regex");
    REQUIRE_NODE_ERR("()", "empty regex");
    REQUIRE_NODE_ERR("((()))", "empty regex");
    // Here, the error is different because the initial scanner doesn't detect
    // emptiness due to concatenation of two parenthesized nodes "().()".
    // However, the parser detects the unexpected closing right after opening
    // the parenthesis.
    REQUIRE_NODE_ERR("()()", "unexpected character ')' with value 41");
  }

  SECTION("unbalanced parens") {
    REQUIRE_NODE_ERR("(a", "unbalanced parens");
    REQUIRE_NODE_ERR("a((", "unbalanced parens");
    REQUIRE_NODE_ERR("((a", "unbalanced parens");
    REQUIRE_NODE_ERR("(", "unbalanced parens");
    REQUIRE_NODE_ERR("abc)", "unbalanced parens");
  }

  SECTION("unexpected characters") {
    REQUIRE_NODE_ERR("\1", "unexpected character '\1' with value 1");
    // Here, ')' is an unexpected character because the parser doesn't expect
    // the closing parenthesis immediately.
    REQUIRE_NODE_ERR("a(bcd())", "unexpected character ')' with value 41");
  }
}

// NOLINTEND(
//   bugprone-easily-swappable-parameters,
//   cppcoreguidelines-avoid-magic-numbers,
//   readability-magic-numbers
// )
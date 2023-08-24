// NOLINTBEGIN(
//   bugprone-easily-swappable-parameters,
// )
#include "internal/parsing.hpp"

#include <catch2/catch_all.hpp>
#include <string>

using RM::Impl::ParseNode;
using RM::Impl::Parser;
using RM::Impl::Scanner;

void REQUIRE_SCANNER_EQ(std::string&& input, std::string&& regex,
                        size_t node_charcount, int paren_balance) {
  Scanner result{input};
  REQUIRE(result.regex == regex);
  REQUIRE(result.node_charcount == node_charcount);
  REQUIRE(result.paren_balance == paren_balance);
}

void REQUIRE_NODE_EQ(ParseNode result, ParseNode::index left,
                     ParseNode::index right, ParseNode::NodeType type,
                     char character) {
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
    auto parsed = Parser{"1"}.parse();
    REQUIRE(parsed.first_node == 0);
    auto result = parsed.nodes;
    REQUIRE(result.size() == 1);
    REQUIRE_NODE_EQ(result[0], -1, -1, ParseNode::NodeType::CHAR, '1');
  }

  SECTION("single altered pair") {
    auto parsed = Parser{"a|b"}.parse();
    REQUIRE(parsed.first_node == 2);
    auto result = parsed.nodes;
    REQUIRE(result.size() == 3);
    REQUIRE_NODE_EQ(result[0], -1, -1, ParseNode::NodeType::CHAR, 'a');
    REQUIRE_NODE_EQ(result[1], -1, -1, ParseNode::NodeType::CHAR, 'b');
    REQUIRE_NODE_EQ(result[2], 0, 1, ParseNode::NodeType::OR, '\0');
  }

  SECTION("single concatenated pair") {
    auto parsed = Parser{"ab"}.parse();
    REQUIRE(parsed.first_node == 2);
    auto result = parsed.nodes;
    REQUIRE(result.size() == 3);
    REQUIRE_NODE_EQ(result[0], -1, -1, ParseNode::NodeType::CHAR, 'a');
    REQUIRE_NODE_EQ(result[1], -1, -1, ParseNode::NodeType::CHAR, 'b');
    REQUIRE_NODE_EQ(result[2], 0, 1, ParseNode::NodeType::CONCAT, '\0');
  }

  SECTION("zero or more of a single character") {
    auto parsed = Parser{"1*"}.parse();
    REQUIRE(parsed.first_node == 1);
    auto result = parsed.nodes;
    REQUIRE(result.size() == 2);
    REQUIRE_NODE_EQ(result[0], -1, -1, ParseNode::NodeType::CHAR, '1');
    REQUIRE_NODE_EQ(result[1], 0, -1, ParseNode::NodeType::KLEENE_STAR, '\0');
  }

  SECTION("optional character") {
    auto parsed = Parser{"1?"}.parse();
    REQUIRE(parsed.first_node == 1);
    auto result = parsed.nodes;
    REQUIRE(result.size() == 2);
    REQUIRE_NODE_EQ(result[0], -1, -1, ParseNode::NodeType::CHAR, '1');
    REQUIRE_NODE_EQ(result[1], 0, -1, ParseNode::NodeType::OPTIONAL, '\0');
  }

  SECTION("heavily parenthesized character") {
    auto parsed = Parser{"(((1)))"}.parse();
    REQUIRE(parsed.first_node == 0);
    auto result = parsed.nodes;
    REQUIRE(result.size() == 1);
    REQUIRE_NODE_EQ(result[0], -1, -1, ParseNode::NodeType::CHAR, '1');
  }

  SECTION("parenthesises combined with other operators") {
    auto parsed = Parser{"(ab)*|1"}.parse();
    REQUIRE(parsed.first_node == 5);
    auto result = parsed.nodes;
    REQUIRE(result.size() == 6);
    REQUIRE_NODE_EQ(result[0], -1, -1, ParseNode::NodeType::CHAR, 'a');
    REQUIRE_NODE_EQ(result[1], -1, -1, ParseNode::NodeType::CHAR, 'b');
    REQUIRE_NODE_EQ(result[2], 0, 1, ParseNode::NodeType::CONCAT, '\0');
    REQUIRE_NODE_EQ(result[3], 2, -1, ParseNode::NodeType::KLEENE_STAR, '\0');
    REQUIRE_NODE_EQ(result[4], -1, -1, ParseNode::NodeType::CHAR, '1');
    REQUIRE_NODE_EQ(result[5], 3, 4, ParseNode::NodeType::OR, '\0');
  }

  SECTION("logically empty regexes") {
    REQUIRE_NODE_ERR("", "empty regex");
    REQUIRE_NODE_ERR("()", "empty regex");
    REQUIRE_NODE_ERR("(())", "empty regex");
  }

  SECTION("unbalanced parens") {
    REQUIRE_NODE_ERR("(a", "unbalanced parens");
    REQUIRE_NODE_ERR("a((", "unbalanced parens");
    REQUIRE_NODE_ERR("((a", "unbalanced parens");
    REQUIRE_NODE_ERR("(", "unbalanced parens");
    REQUIRE_NODE_ERR("abc)", "unbalanced parens");
  }

  SECTION("empty () expression") {
    // The parser does not detect a logically empty regex because of
    // concatenation of two nodes (it's seen as "().()"), but it detects empty
    // parentheses.
    REQUIRE_NODE_ERR("()()", "empty () expression");
    REQUIRE_NODE_ERR("(()())", "empty () expression");
    REQUIRE_NODE_ERR("()(())", "empty () expression");
    REQUIRE_NODE_ERR("a(bcd())", "empty () expression");
  }
}

// NOLINTEND(
//   bugprone-easily-swappable-parameters,
// )
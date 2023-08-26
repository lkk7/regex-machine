// NOLINTBEGIN(
//   bugprone-easily-swappable-parameters,
// )
#include "internal/parser.hpp"

#include <catch2/catch_all.hpp>
#include <string>

using RM::Impl::ParseNode;
using RM::Impl::Parser;

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

  SECTION("one or more of a single character") {
    auto parsed = Parser{"1+"}.parse();
    REQUIRE(parsed.first_node == 1);
    auto result = parsed.nodes;
    REQUIRE(result.size() == 2);
    REQUIRE_NODE_EQ(result[0], -1, -1, ParseNode::NodeType::CHAR, '1');
    REQUIRE_NODE_EQ(result[1], 0, -1, ParseNode::NodeType::ONE_OR_MORE, '\0');
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
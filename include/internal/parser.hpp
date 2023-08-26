#pragma once

#include <string>
#include <vector>

#include "scanner.hpp"

namespace RM::Impl {

/** A binary tree node with indices instead of pointers.
 * Describes a character or a binary/unary operator.
 * When an index is meaningless (e.g. right index for unary "a*"), it's -1.
 * When a character is meaningless (so, for every operator), it's '\0'.
 */
struct ParseNode {
  using index = int;
  enum class NodeType {
    CHAR,
    CONCAT,
    KLEENE_STAR,
    ONE_OR_MORE,
    OPTIONAL,
    OR,
  };

  index left;
  index right;
  NodeType type;
  char character;
};

/** A Parser that accepts any regex string.
 *
 * The EBNF-style representation of the grammar is:
 * <or> ::= <concat> ("|" <or>)?
 * <concat> ::= <repeat> ("." <concat>)?
 * <repeat> ::= <paren> ("*" | "?" | "+")?
 * <paren> ::= <char> | "(" <or> ")"
 * <char> ::= (any alphanumeric char)
 */
class Parser {
 public:
  using index = int;
  static_assert(std::is_same<index, ParseNode::index>::value);
  // The nodes are stored in a vector and point to each other via indices for
  // memory contiguity and safety.
  struct ParseResult {
    std::vector<ParseNode> nodes;
    std::string err_msg;
    index first_node;
  };

  Parser() = delete;
  explicit Parser(const std::string& regex) : scanner{regex} {}

  ParseResult parse() {
    if (scanner.paren_balance != 0) {
      return {.nodes = {}, .err_msg = "unbalanced parens"};
    }
    if (scanner.node_charcount == 0) {
      return {.nodes = {}, .err_msg = "empty regex"};
    }

    ParseResult parse_result{
        .nodes = std::vector<ParseNode>(scanner.node_charcount),
        .err_msg = "",
    };

    // Launch parsing by calling the "outermost" grammar rule
    parse_result.first_node = get_or(parse_result);
    return parse_result;
  }

 private:
  index get_or(ParseResult& result) {
    const index left = get_concat(result);
    if (left == -1) [[unlikely]] {
      return -1;
    }
    if (scanner.peek() != '|') {
      return left;
    }
    scanner.pop();
    const index right = get_or(result);
    if (right == -1) [[unlikely]] {
      return -1;
    }
    return set_next_node({.left = left,
                          .right = right,
                          .type = ParseNode::NodeType::OR,
                          .character = '\0'},
                         result);
  }

  index get_concat(ParseResult& result) {
    const index left = get_repeat(result);
    if (left == -1) [[unlikely]] {
      return -1;
    }
    if (scanner.peek() != '.') {
      return left;
    }
    scanner.pop();
    const index right = get_concat(result);
    if (right == -1) [[unlikely]] {
      return -1;
    }
    return set_next_node({.left = left,
                          .right = right,
                          .type = ParseNode::NodeType::CONCAT,
                          .character = '\0'},
                         result);
  }

  index get_repeat(ParseResult& result) {
    using NodeType = ParseNode::NodeType;
    const index paren = get_paren(result);
    if (paren == -1) [[unlikely]] {
      return -1;
    }

    const char symbol = scanner.peek();
    if (symbol != '*' && symbol != '?' && symbol != '+') {
      return paren;
    }
    scanner.pop();
    NodeType type = NodeType::ONE_OR_MORE;
    if (symbol == '*') {
      type = NodeType::KLEENE_STAR;
    }
    if (symbol == '?') {
      type = NodeType::OPTIONAL;
    }
    return set_next_node(
        {.left = paren, .right = -1, .type = type, .character = '\0'}, result);
  }

  index get_paren(ParseResult& result) {
    if (scanner.peek() != '(') {
      return get_char(result);
    }

    scanner.pop();
    if (scanner.peek() == ')') [[unlikely]] {
      result.err_msg = "empty () expression";
      return -1;
    }
    const index or_expr = get_or(result);
    if (or_expr == -1) [[unlikely]] {
      return -1;
    }
    if (const char c = scanner.pop(); c != ')') [[unlikely]] {
      result.err_msg = "')' expected, got char with code" +
                       std::to_string(static_cast<int>(c));
      return -1;
    }
    return or_expr;
  }

  index get_char(ParseResult& result) {
    return set_next_node(
        {
            .left = -1,
            .right = -1,
            .type = ParseNode::NodeType::CHAR,
            .character = scanner.pop(),
        },
        result);
  }

  index set_next_node(ParseNode&& node, ParseResult& result) {
    result.nodes[static_cast<size_t>(node_counter)] = node;
    return node_counter++;
  }

  Scanner scanner;
  index node_counter = 0;
};

}  // namespace RM::Impl

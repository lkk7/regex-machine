#pragma once

#include <cctype>
#include <sstream>
#include <string>
#include <vector>

namespace RM::Impl {

/** Reader and preprocessor of regex input.
 * Adds concatenation: "abc" -> "a.b.c".
 * Checks for problems such as no meaningful content: "(()())"
 * or unbalanced parentheses: "((ab)".
 */
class Scanner {
 public:
  explicit Scanner(const std::string& input) {
    if (input.empty()) {
      return;
    }

    node_charcount = input.size();
    auto is_paren = [this](char chr) {
      if (chr == '(') {
        ++paren_balance;
        return 1U;
      }
      if (chr == ')') {
        --paren_balance;
        return 1U;
      }
      return 0U;
    };

    for (size_t i = 0; i < input.size() - 1; ++i) {
      const char c = input[i];
      const char next = input[i + 1];
      regex.push_back(c);
      node_charcount -= is_paren(c);
      const bool is_left_concatable = static_cast<bool>(std::isalnum(c)) ||
                                      c == ')' || c == '*' || c == '?';
      const bool is_right_concatable =
          next != ')' && next != '|' && next != '*' && next != '?';
      if (is_left_concatable && is_right_concatable) {
        regex.push_back('.');
        ++node_charcount;
      }
    }
    node_charcount -= is_paren(*(input.cend() - 1));
    regex.push_back(*(input.cend() - 1));
  }

  char peek() const { return index >= regex.size() ? '\0' : regex[index]; }
  char pop() { return index >= regex.size() ? '\0' : regex[index++]; }

  size_t node_charcount = 0;
  int paren_balance = 0;
  std::string regex;

 private:
  size_t index = 0;
};

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
    OPTIONAL,
    OR,
    KLEENE_STAR,
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
 * <repeat> ::= <paren> ("*" | "?")?
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
    const index paren = get_paren(result);
    if (paren == -1) [[unlikely]] {
      return -1;
    }
    const char symbol = scanner.peek();

    if (symbol == '*' || symbol == '?') {
      scanner.pop();
      const ParseNode::NodeType type = (symbol == '*')
                                           ? ParseNode::NodeType::KLEENE_STAR
                                           : ParseNode::NodeType::OPTIONAL;
      return set_next_node(
          {.left = paren, .right = -1, .type = type, .character = '\0'},
          result);
    }

    return paren;
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
      std::stringstream strs;
      strs << "')' expected, got char with code " << static_cast<int>(c);
      result.err_msg = strs.str();
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

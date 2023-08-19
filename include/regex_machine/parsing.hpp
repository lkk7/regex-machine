#pragma once

#include <cctype>
#include <sstream>
#include <string>
#include <vector>

namespace RegexMachine {

struct Node {
  using index = int;
  enum class NodeType {
    CHAR,
    CONCAT,
    OPTIONAL,
    OR,
    KLEENE_STAR,
  };
  bool operator==(const Node&) const = default;

  index left;
  index right;
  NodeType type;
  char character;
};

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

    for (auto it = input.cbegin(); it != input.cend() - 1; ++it) {
      regex.push_back(*it);
      node_charcount -= is_paren(*it);
      const bool is_left_concatable = static_cast<bool>(std::isalnum(*it)) ||
                                      *it == ')' || *it == '*' || *it == '?';
      const bool is_right_concatable = *(it + 1) != ')' && *(it + 1) != '|' &&
                                       *(it + 1) != '*' && *(it + 1) != '?';
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

  std::string regex;
  size_t node_charcount = 0;
  int paren_balance = 0;

 private:
  size_t index = 0;
};

/** A Parser that accepts any regex string.
 * The EBNF-style representation of the grammar is:
 * <or> ::= <concat> ("|" <or>)?
 * <concat> ::= <repeat> ("." <concat>)?
 * <repeat> ::= <paren> ("*" | "?")?
 * <paren> ::= <char> | "(" <or> ")"
 * <char> ::= (any alphanumeric char)
 */
class Parser {
 public:
  Parser() = delete;
  explicit Parser(const std::string& regex) : scanner{regex} {}

  struct ParseResult {
    std::vector<Node> nodes;
    std::string err_msg;
  };

  ParseResult parse() {
    if (scanner.paren_balance != 0) {
      return {.nodes = {}, .err_msg = "unbalanced parens"};
    }
    if (scanner.node_charcount == 0) {
      return {.nodes = {}, .err_msg = "empty regex"};
    }

    ParseResult parse_result{
        .nodes = std::vector<Node>(scanner.node_charcount),
        .err_msg = "",
    };

    // Launch parsing by calling the "outermost" grammar rule
    get_or(parse_result);
    return parse_result;
  }

 private:
  using index = int;
  static_assert(std::is_same<index, Node::index>::value);

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
                          .type = Node::NodeType::OR,
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
                          .type = Node::NodeType::CONCAT,
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
      const Node::NodeType type = (symbol == '*') ? Node::NodeType::KLEENE_STAR
                                                  : Node::NodeType::OPTIONAL;
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
    const index or_expr = get_or(result);
    if (or_expr == -1) [[unlikely]] {
      return -1;
    }
    if (scanner.pop() != ')') [[unlikely]] {
      result.err_msg = "unterminated left parenthesis";
      return -1;
    }
    return or_expr;
  }

  index get_char(ParseResult& result) {
    const char token = scanner.peek();

    if (!static_cast<bool>(std::isalnum(token)) && token != '\0') [[unlikely]] {
      std::stringstream strs;
      strs << "unexpected character '" << token << "' with value "
           << static_cast<int>(token);
      result.err_msg = strs.str();
      return -1;
    }
    return set_next_node(
        {
            .left = -1,
            .right = -1,
            .type = Node::NodeType::CHAR,
            .character = scanner.pop(),
        },
        result);
  }

  index set_next_node(Node&& node, ParseResult& result) {
    result.nodes[static_cast<size_t>(node_counter)] = node;
    return node_counter++;
  }

  Scanner scanner;
  index node_counter = 0;
};

}  // namespace RegexMachine

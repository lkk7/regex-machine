#pragma once

#include <string>

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
    const size_t n = node_charcount;
    for (size_t i = 0; i < n; ++i) {
      const char c = input[i];
      const auto [balance, is_paren] = char_check(c);
      paren_balance -= balance;
      node_charcount -= static_cast<size_t>(is_paren);
      regex.push_back(c);
      if ((i + 1 < n) && is_left_concat(c) && is_right_concat(input[i + 1])) {
        regex.push_back('.');
        ++node_charcount;
      }
    }
  }

  char peek() const { return index >= regex.size() ? '\0' : regex[index]; }
  char pop() { return index >= regex.size() ? '\0' : regex[index++]; }

  size_t node_charcount = 0;
  int paren_balance = 0;
  std::string regex;

 private:
  struct check_res {
    int paren_balance;
    bool is_paren;
  };

  static constexpr check_res char_check(char chr) {
    if (chr == '(') {
      return check_res{-1, true};
    }
    if (chr == ')') {
      return check_res{1, true};
    }
    return check_res{0, false};
    ;
  }

  static constexpr bool is_left_concat(char c) {
    return static_cast<bool>(std::isalnum(c)) || c == ')' || c == '*' ||
           c == '?' || c == '+';
  }
  static constexpr bool is_right_concat(char c) {
    return c != ')' && c != '|' && c != '*' && c != '?' && c != '+';
  }

  size_t index = 0;
};

}  // namespace RM::Impl
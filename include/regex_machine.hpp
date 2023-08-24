#pragma once

#include <string>

#include "internal/nfa.hpp"
#include "internal/parsing.hpp"

namespace RM {

class Matcher {
 public:
  explicit Matcher(std::string&& input) : nfa{create_nfa(std::move(input))} {}

  bool match(std::string&& input) {
    if (!err_msg.empty()) {
      return false;
    }
    return nfa.match(input);
  }

  std::string err_msg;

 private:
  Impl::NFA create_nfa(std::string&& str) {
    Impl::Parser::ParseResult parsed = Impl::Parser{str}.parse();
    if (!parsed.err_msg.empty()) {
      err_msg = parsed.err_msg;
    }
    Impl::NFA result = Impl::create_from_parse(std::move(parsed));
    if (nfa.error != Impl::NFA::err_state::OK && err_msg.empty()) {
      err_msg = std::to_string(static_cast<int>(nfa.error));
    }
    return result;
  }

  Impl::NFA nfa;
};

}  // namespace RM

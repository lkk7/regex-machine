#pragma once

#include <string>

#include "internal/nfa.hpp"
#include "internal/parsing.hpp"

namespace RM {

class Matcher {
 public:
  explicit Matcher(std::string&& input)
      : nfa{Impl::create_from_str(std::move(input), err_msg)} {}

  bool match(std::string&& input) const {
    return err_msg.empty() ? nfa.match(input) : false;
  }

  std::string err_msg;

 private:
  Impl::NFA nfa;
};

}  // namespace RM

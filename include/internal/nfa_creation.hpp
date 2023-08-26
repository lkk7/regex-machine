#pragma once

#include "nfa.hpp"

namespace RM::Impl {

constexpr char EPS = static_cast<char>(NFA::input::EPS);

inline NFA create_err(NFA::err_state error) {
  NFA result{0, {0, 0}};
  result.error = error;
  return result;
}

inline NFA create_basic(char input) {
  NFA result(2, {0, 1});
  result.add_transition({0, 1}, input);
  return result;
}

inline NFA create_concat(NFA&& nfa1, NFA&& nfa2) {
  nfa2.shift_states(nfa1.size - 1);
  NFA result{nfa2};
  result.fill_states_from(nfa1);
  result.initial_state = nfa1.initial_state;
  return result;
}

inline NFA create_kleene_star(NFA&& nfa) {
  nfa.shift_states(1);
  nfa.push_empty_state();
  nfa.add_transition({0, nfa.initial_state}, EPS);
  nfa.add_transition({0, nfa.size - 1}, EPS);
  nfa.add_transition({nfa.final_state, nfa.initial_state}, EPS);
  nfa.add_transition({nfa.final_state, nfa.size - 1}, EPS);
  nfa.initial_state = 0;
  nfa.final_state = nfa.size - 1;
  return nfa;
}

inline NFA create_one_or_more(NFA&& nfa) {
  nfa.push_empty_state();
  nfa.add_transition({nfa.final_state, nfa.size - 1}, EPS);
  nfa.add_transition({nfa.size - 1, nfa.initial_state}, EPS);
  nfa.final_state = nfa.size - 1;
  return nfa;
}

inline NFA create_optional(NFA&& nfa) {
  nfa.shift_states(1);
  nfa.push_empty_state();
  nfa.add_transition({0, nfa.initial_state}, EPS);
  nfa.add_transition({0, nfa.size - 1}, EPS);
  nfa.add_transition({nfa.final_state, nfa.size - 1}, EPS);
  nfa.initial_state = 0;
  nfa.final_state = nfa.size - 1;
  return nfa;
}

inline NFA create_or(NFA&& nfa1, NFA&& nfa2) {
  nfa1.shift_states(1);
  nfa2.shift_states(nfa1.size);

  NFA result{nfa2};
  result.fill_states_from(nfa1);
  result.add_transition({0, nfa1.initial_state}, EPS);
  result.add_transition({0, nfa2.initial_state}, EPS);
  result.initial_state = 0;
  result.push_empty_state();
  result.final_state = result.size - 1;
  result.add_transition({nfa1.final_state, result.final_state}, EPS);
  result.add_transition({nfa2.final_state, result.final_state}, EPS);
  return result;
}

inline NFA create_from_parse(Parser::ParseResult&& parsed) {
  if (!parsed.err_msg.empty()) [[unlikely]] {
    return create_err(NFA::err_state::BAD_PARSE);
  }

  const auto recursive_build = [&](ParseNode::index i, auto& f) -> NFA {
    const ParseNode node = parsed.nodes[static_cast<size_t>(i)];
    const auto [left, right, type, character] = node;

    switch (type) {
      case ParseNode::NodeType::CHAR:
        return create_basic(character);
      case ParseNode::NodeType::OR:
        return create_or(f(left, f), f(right, f));
      case ParseNode::NodeType::CONCAT:
        return create_concat(f(left, f), f(right, f));
      case ParseNode::NodeType::KLEENE_STAR:
        return create_kleene_star(f(left, f));
      case ParseNode::NodeType::ONE_OR_MORE:
        return create_one_or_more(f(left, f));
      case ParseNode::NodeType::OPTIONAL:
        return create_optional(f(left, f));
      default:
        return create_err(NFA::err_state::BAD_PARSE);
    }
  };

  return recursive_build(parsed.first_node, recursive_build);
}

inline NFA create_from_str(std::string&& str, std::string& err_msg) {
  Parser::ParseResult parsed = Impl::Parser{str}.parse();
  if (!parsed.err_msg.empty()) {
    err_msg = parsed.err_msg;
  }
  Impl::NFA result = Impl::create_from_parse(std::move(parsed));
  if (result.error != Impl::NFA::err_state::OK && err_msg.empty()) {
    err_msg = std::to_string(static_cast<int>(result.error));
  }
  return result;
}

}  // namespace RM::Impl
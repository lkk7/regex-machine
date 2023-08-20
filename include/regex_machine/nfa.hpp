#pragma once

#include <iostream>
#include <set>
#include <stdexcept>
#include <vector>

#include "parsing.hpp"

namespace RegexMachine {

class NFA {
 public:
  using state = size_t;
  using state_pair = std::pair<state, state>;
  using trans_vec = std::vector<std::vector<char>>;
  enum class input : char { EPS = -1, NONE = 0 };
  enum class err_state : char {
    OK,
    BAD_PARSE,
    BAD_INIT,
    BAD_FINAL,
    BAD_FROM,
    BAD_TO
  };

  explicit NFA(size_t size, state_pair start_and_end)
      : size{size},
        initial_state{start_and_end.first},
        final_state{start_and_end.second} {
    if (initial_state >= size) [[unlikely]] {
      error = err_state::BAD_INIT;
      return;
    } else if (final_state >= size) [[unlikely]] {
      error = err_state::BAD_FINAL;
      return;
    }
    transitions = trans_vec(size, std::vector<char>(size, '\0'));
  }

  void add_transition(state_pair from_to, char input_char) {
    const auto [from, to] = from_to;
    if (from >= size) [[unlikely]] {
      error = err_state::BAD_FROM;
      return;
    } else if (to >= size) [[unlikely]] {
      error = err_state::BAD_TO;
      return;
    }
    transitions[from][to] = input_char;
    if (static_cast<input>(input_char) != input::EPS) {
      inputs.insert(input_char);
    }
  }

  void fill_states_from(const NFA& other) {
    for (state i = 0; i < other.size; ++i) {
      for (state j = 0; j < other.size; ++j) {
        transitions[i][j] = other.transitions[i][j];
      }
    }

    for (const char input : other.inputs) {
      inputs.insert(input);
    }
  }

  void shift_states(size_t n) {
    if (n == 0) {
      return;
    }
    const size_t new_size = size + n;

    trans_vec new_transitions(new_size, std::vector<char>(new_size, '\0'));
    for (state i = 0; i < size; ++i) {
      for (state j = 0; j < size; ++j) {
        new_transitions[i + n][j + n] = transitions[i][j];
      }
    }

    size = new_size;
    initial_state += n;
    final_state += n;
    transitions = new_transitions;
  }

  void push_empty_state() {
    for (state i = 0; i < size; ++i) {
      transitions[i].push_back('\0');
    }
    transitions.emplace_back(++size, '\0');
  }

  std::set<state> get_reachable_states(std::vector<state>&& states,
                                       char input) {
    std::set<state> result;
    for (const state s : states) {
      for (size_t i = 0; i < transitions[s].size(); ++i) {
        if (transitions[s][i] == input) {
          result.insert(i);
        }
      }
    }
    return result;
  }

  std::vector<std::vector<char>> transitions;
  std::set<char> inputs;
  size_t size;
  state initial_state;
  state final_state;
  err_state error = err_state::OK;
};

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

inline NFA create_or(NFA&& nfa1, NFA&& nfa2) {
  nfa1.shift_states(1);
  nfa2.shift_states(nfa1.size);

  NFA result{nfa2};
  result.fill_states_from(nfa1);
  result.add_transition({0, nfa1.initial_state},
                        static_cast<char>(NFA::input::EPS));
  result.add_transition({0, nfa2.initial_state},
                        static_cast<char>(NFA::input::EPS));
  result.initial_state = 0;
  result.push_empty_state();
  result.final_state = result.size - 1;
  result.add_transition({nfa1.final_state, result.final_state},
                        static_cast<char>(NFA::input::EPS));
  result.add_transition({nfa2.final_state, result.final_state},
                        static_cast<char>(NFA::input::EPS));
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
  nfa.add_transition({nfa.final_state, nfa.initial_state},
                     static_cast<char>(NFA::input::EPS));
  nfa.add_transition({0, nfa.initial_state},
                     static_cast<char>(NFA::input::EPS));
  nfa.add_transition({nfa.final_state, nfa.size - 1},
                     static_cast<char>(NFA::input::EPS));
  nfa.add_transition({0, nfa.size - 1}, static_cast<char>(NFA::input::EPS));
  nfa.initial_state = 0;
  nfa.final_state = nfa.size - 1;
  return nfa;
}

inline NFA create_from_parse(const Parser::ParseResult& parsed) {
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
      default:
        return create_err(NFA::err_state::BAD_PARSE);
    }
  };

  return recursive_build(parsed.first_node, recursive_build);
}

}  // namespace RegexMachine

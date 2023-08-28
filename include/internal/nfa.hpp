#pragma once

#include <concepts>
#include <iostream>
#include <stdexcept>
#include <unordered_set>
#include <vector>

#include "parser.hpp"

namespace RM::Impl {

class NFA {
 public:
  using state = size_t;
  using state_pair = std::pair<state, state>;
  using state_set = std::unordered_set<state>;
  using trans_vec = std::vector<std::vector<char>>;
  enum class input : char { EPS = -1, NONE = 0 };
  enum class err_state : char {
    OK = 0,
    BAD_PARSE,
    BAD_INIT,
    BAD_FINAL,
    BAD_FROM,
    BAD_TO
  };

  explicit NFA(size_t nfa_size, state_pair start_and_end)
      : size{nfa_size},
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
    for (const char in : other.inputs) {
      inputs.insert(in);
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

  state_set get_reachable_states(const state_set& states, char c) const {
    std::unordered_set<state> result;
    for (const state s : states) {
      for (size_t i = 0; i < transitions[s].size(); ++i) {
        if (transitions[s][i] == c) {
          result.insert(i);
        }
      }
    }
    return result;
  }

  /**
   * Compute the epsilon-closure of a set of NFA states.
   *
   * The epsilon-closure
   * of a set of states is the set of states that can be reached from the input
   * states by following epsilon transitions alone.
   */
  state_set eps_closure(state_set&& states) const {
    if (states.empty()) {
      return {};
    }
    std::vector<state> stack(states.begin(), states.end());
    state_set result(states.begin(), states.end());

    while (!stack.empty()) {
      const state s = stack.back();
      stack.pop_back();

      for (size_t i = 0; i < transitions[s].size(); ++i) {
        const char c = transitions[s][i];
        if (c != static_cast<char>(input::EPS)) {
          continue;
        }
        if (!result.contains(i)) {
          result.emplace(i);
          stack.push_back(i);
        }
      }
    }

    return result;
  }

  bool match(const std::string& s) const {
    state_set reachable = eps_closure({initial_state});
    for (const char c : s) {
      if (!inputs.contains(c)) {
        return false;
      }
      reachable = eps_closure(get_reachable_states(reachable, c));
      if (reachable.empty()) {
        return false;
      }
    }
    return reachable.contains(final_state);
  }

  std::vector<std::vector<char>> transitions;
  std::unordered_set<char> inputs;
  size_t size{};
  state initial_state{};
  state final_state{};
  err_state error = err_state::OK;
};

}  // namespace RM::Impl

#include "regex_machine/nfa.hpp"

#include <catch2/catch_all.hpp>
#include <string>
#include <vector>

#include "regex_machine/parsing.hpp"

using RegexMachine::NFA, RegexMachine::create_basic, RegexMachine::create_or,
    RegexMachine::create_basic, RegexMachine::create_concat,
    RegexMachine::create_kleene_star, RegexMachine::create_from_parse,
    RegexMachine::Parser;

TEST_CASE("NFA::nfa") {
  SECTION("Basic NFA") {
    NFA nfa{2, {0, 1}};
    REQUIRE(nfa.size == 2);
    REQUIRE(nfa.initial_state == 0);
    REQUIRE(nfa.final_state == 1);
    REQUIRE(nfa.inputs.empty());
    REQUIRE(nfa.transitions == NFA::trans_vec{{'\0', '\0'}, {'\0', '\0'}});
    REQUIRE(nfa.error == NFA::err_state::OK);
  }

  SECTION("Bad NFA initial state") {
    NFA nfa{2, {2, 0}};
    REQUIRE(nfa.error == NFA::err_state::BAD_INIT);
  }

  SECTION("Bad NFA final state") {
    NFA nfa{2, {0, 2}};
    REQUIRE(nfa.error == NFA::err_state::BAD_FINAL);
  }
}

TEST_CASE("NFA::add_transition") {
  SECTION("Basic case") {
    NFA nfa{3, {0, 2}};

    REQUIRE(nfa.transitions == NFA::trans_vec{
                                   {'\0', '\0', '\0'},
                                   {'\0', '\0', '\0'},
                                   {'\0', '\0', '\0'},
                               });
    nfa.add_transition({0, 1}, 'A');
    nfa.add_transition({1, 2}, 'B');
    nfa.add_transition({2, 0}, 'C');
    REQUIRE(nfa.transitions == NFA::trans_vec{
                                   {'\0', 'A', '\0'},
                                   {'\0', '\0', 'B'},
                                   {'C', '\0', '\0'},
                               });
    REQUIRE(nfa.inputs == std::set<char>{'A', 'B', 'C'});
    REQUIRE(nfa.error == NFA::err_state::OK);
  }

  SECTION("Bad NFA `from` state") {
    NFA nfa{2, {0, 1}};
    nfa.add_transition({2, 1}, 'a');
    REQUIRE(nfa.error == NFA::err_state::BAD_FROM);
  }

  SECTION("Bad NFA `to` state") {
    NFA nfa{2, {0, 1}};
    nfa.add_transition({0, 2}, 'a');
    REQUIRE(nfa.error == NFA::err_state::BAD_TO);
  }
}

TEST_CASE("NFA::filL_states_from") {
  NFA nfa1{3, {0, 2}};
  NFA nfa2{2, {0, 1}};
  nfa2.add_transition({0, 1}, 'a');
  nfa2.add_transition({1, 0}, 'b');
  nfa1.fill_states_from(nfa2);
  REQUIRE(nfa1.transitions == NFA::trans_vec{
                                  {'\0', 'a', '\0'},
                                  {'b', '\0', '\0'},
                                  {'\0', '\0', '\0'},
                              });
  REQUIRE(nfa1.inputs == std::set<char>{'a', 'b'});
}

TEST_CASE("NFA::shift_states") {
  NFA nfa{2, {0, 1}};
  nfa.add_transition({0, 1}, 'a');
  nfa.add_transition({1, 0}, 'b');
  nfa.shift_states(2);
  REQUIRE(nfa.size == 4);
  REQUIRE(nfa.initial_state == 2);
  REQUIRE(nfa.final_state == 3);
  REQUIRE(nfa.transitions == NFA::trans_vec{
                                 {'\0', '\0', '\0', '\0'},
                                 {'\0', '\0', '\0', '\0'},
                                 {'\0', '\0', '\0', 'a'},
                                 {'\0', '\0', 'b', '\0'},
                             });
}

TEST_CASE("NFA::push_empty_state") {
  NFA nfa{3, {0, 2}};
  nfa.push_empty_state();
  REQUIRE(nfa.size == 4);
  REQUIRE(nfa.transitions == NFA::trans_vec{
                                 {'\0', '\0', '\0', '\0'},
                                 {'\0', '\0', '\0', '\0'},
                                 {'\0', '\0', '\0', '\0'},
                                 {'\0', '\0', '\0', '\0'},
                             });
}

TEST_CASE("NFA::get_movable_states") {
  NFA nfa{3, {0, 2}};
  nfa.add_transition({0, 1}, 'a');
  nfa.add_transition({1, 2}, 'b');
  nfa.add_transition({2, 1}, 'c');
  REQUIRE(nfa.get_reachable_states({0, 1}, 'b') == std::set<NFA::state>{2});
}

TEST_CASE("NFA::create_basic") {
  NFA result = create_basic('t');
  REQUIRE(result.error == NFA::err_state::OK);
  REQUIRE(result.size == 2);
  REQUIRE(result.initial_state == 0);
  REQUIRE(result.final_state == 1);
  REQUIRE(result.inputs == std::set<char>{'t'});
  REQUIRE(result.transitions == NFA::trans_vec{
                                    {'\0', 't'},
                                    {'\0', '\0'},
                                });
}

TEST_CASE("NFA::create_alter") {
  NFA result = create_or(create_basic('a'), create_basic('b'));
  REQUIRE(result.error == NFA::err_state::OK);
  REQUIRE(result.size == 6);
  REQUIRE(result.initial_state == 0);
  REQUIRE(result.final_state == 5);
  REQUIRE(result.inputs == std::set<char>{'a', 'b'});
  REQUIRE(result.transitions == NFA::trans_vec{
                                    {'\0', -1, '\0', -1, '\0', '\0'},
                                    {'\0', '\0', 'a', '\0', '\0', '\0'},
                                    {'\0', '\0', '\0', '\0', '\0', -1},
                                    {'\0', '\0', '\0', '\0', 'b', '\0'},
                                    {'\0', '\0', '\0', '\0', '\0', -1},
                                    {'\0', '\0', '\0', '\0', '\0', '\0'},
                                });
}

TEST_CASE("NFA::create_concat") {
  NFA result = create_concat(create_basic('a'), create_basic('b'));
  REQUIRE(result.error == NFA::err_state::OK);
  REQUIRE(result.size == 3);
  REQUIRE(result.initial_state == 0);
  REQUIRE(result.final_state == 2);
  REQUIRE(result.inputs == std::set<char>{'a', 'b'});
  REQUIRE(result.transitions == NFA::trans_vec{
                                    {'\0', 'a', '\0'},
                                    {'\0', '\0', 'b'},
                                    {'\0', '\0', '\0'},
                                });
}

TEST_CASE("NFA::create_kleene_star") {
  NFA result = create_kleene_star(create_basic('x'));
  REQUIRE(result.error == NFA::err_state::OK);
  REQUIRE(result.size == 4);
  REQUIRE(result.initial_state == 0);
  REQUIRE(result.final_state == 3);
  REQUIRE(result.inputs == std::set<char>{'x'});
  REQUIRE(result.transitions == NFA::trans_vec{
                                    {'\0', -1, '\0', -1},
                                    {'\0', '\0', 'x', '\0'},
                                    {'\0', -1, '\0', -1},
                                    {'\0', '\0', '\0', '\0'},
                                });
}

TEST_CASE("NFA::create_from_parse") {
  const Parser::ParseResult parsed = Parser{"a|b"}.parse();
  auto result = create_from_parse(parsed);
  REQUIRE(result.error == NFA::err_state::OK);
  REQUIRE(result.size == 6);
  REQUIRE(result.initial_state == 0);
  REQUIRE(result.final_state == 5);
  REQUIRE(result.inputs == std::set<char>{'a', 'b'});
  REQUIRE(result.transitions == NFA::trans_vec{
                                    {'\0', -1, '\0', -1, '\0', '\0'},
                                    {'\0', '\0', 'a', '\0', '\0', '\0'},
                                    {'\0', '\0', '\0', '\0', '\0', -1},
                                    {'\0', '\0', '\0', '\0', 'b', '\0'},
                                    {'\0', '\0', '\0', '\0', '\0', -1},
                                    {'\0', '\0', '\0', '\0', '\0', '\0'},
                                });
}

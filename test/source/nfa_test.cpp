#include "internal/nfa.hpp"

#include <catch2/catch_all.hpp>

#include "internal/nfa_creation.hpp"
#include "internal/parser.hpp"

using RM::Impl::NFA, RM::Impl::create_basic, RM::Impl::create_concat,
    RM::Impl::create_kleene_star, RM::Impl::create_basic, RM::Impl::create_or;

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
    REQUIRE(nfa.inputs == std::unordered_set<char>{'A', 'B', 'C'});
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

TEST_CASE("fill_states_from") {
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
  REQUIRE(nfa1.inputs == std::unordered_set<char>{'a', 'b'});
}

TEST_CASE("shift_states") {
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

TEST_CASE("push_empty_state") {
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

TEST_CASE("NFA::get_reachable_states") {
  NFA nfa{3, {0, 2}};
  nfa.add_transition({0, 1}, 'a');
  nfa.add_transition({1, 2}, 'b');
  nfa.add_transition({2, 1}, 'c');
  REQUIRE(nfa.get_reachable_states({0, 1}, 'b') ==
          std::unordered_set<NFA::state>{2});
}

TEST_CASE("NFA::eps_closure") {
  using set = std::unordered_set<NFA::state>;

  SECTION("ab") {
    const NFA nfa = create_concat(create_basic('a'), create_basic('b'));

    REQUIRE(nfa.eps_closure({0}) == set{0});
    REQUIRE(nfa.eps_closure({1}) == set{1});
    REQUIRE(nfa.eps_closure({2}) == set{2});
    REQUIRE(nfa.eps_closure({0, 1, 2}) == set{0, 1, 2});
  }

  SECTION("(a|b)*a") {
    NFA a_or_b_star =
        create_kleene_star(create_or(create_basic('a'), create_basic('b')));
    const NFA nfa = create_concat(std::move(a_or_b_star), create_basic('a'));

    REQUIRE(nfa.eps_closure({0}) == set{0, 1, 2, 4, 7});
    REQUIRE(nfa.eps_closure({1}) == set{1, 2, 4});
    REQUIRE(nfa.eps_closure({2}) == set{2});
    REQUIRE(nfa.eps_closure({3}) == set{1, 2, 3, 4, 6, 7});
    REQUIRE(nfa.eps_closure({4}) == set{4});
    REQUIRE(nfa.eps_closure({5}) == set{1, 2, 4, 5, 6, 7});
    REQUIRE(nfa.eps_closure({6}) == set{1, 2, 4, 6, 7});
    REQUIRE(nfa.eps_closure({7}) == set{7});
    REQUIRE(nfa.eps_closure({8}) == set{8});
    REQUIRE(nfa.eps_closure({}).empty());
    REQUIRE(nfa.eps_closure({1, 7}) == set{1, 2, 4, 7});
  }
}

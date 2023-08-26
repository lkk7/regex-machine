#include "internal/nfa_creation.hpp"

#include <catch2/catch_all.hpp>

#include "internal/nfa.hpp"

using RM::Impl::Parser, RM::Impl::NFA, RM::Impl::create_basic,
    RM::Impl::create_concat, RM::Impl::create_from_parse,
    RM::Impl::create_kleene_star, RM::Impl::create_one_or_more,
    RM::Impl::create_optional, RM::Impl::create_basic, RM::Impl::create_or;

TEST_CASE("create_err") {
  NFA result = create_err(NFA::err_state::BAD_PARSE);
  REQUIRE(result.error == NFA::err_state::BAD_PARSE);
  REQUIRE(result.size == 0);
}

TEST_CASE("create_basic") {
  NFA result = create_basic('t');
  REQUIRE(result.error == NFA::err_state::OK);
  REQUIRE(result.size == 2);
  REQUIRE(result.initial_state == 0);
  REQUIRE(result.final_state == 1);
  REQUIRE(result.inputs == std::unordered_set<char>{'t'});
  REQUIRE(result.transitions == NFA::trans_vec{
                                    {'\0', 't'},
                                    {'\0', '\0'},
                                });
}

TEST_CASE("create_concat") {
  NFA result = create_concat(create_basic('a'), create_basic('b'));
  REQUIRE(result.error == NFA::err_state::OK);
  REQUIRE(result.size == 3);
  REQUIRE(result.initial_state == 0);
  REQUIRE(result.final_state == 2);
  REQUIRE(result.inputs == std::unordered_set<char>{'a', 'b'});
  REQUIRE(result.transitions == NFA::trans_vec{
                                    {'\0', 'a', '\0'},
                                    {'\0', '\0', 'b'},
                                    {'\0', '\0', '\0'},
                                });
}

TEST_CASE("create_kleene_star") {
  NFA result = create_kleene_star(create_basic('x'));
  REQUIRE(result.error == NFA::err_state::OK);
  REQUIRE(result.size == 4);
  REQUIRE(result.initial_state == 0);
  REQUIRE(result.final_state == 3);
  REQUIRE(result.inputs == std::unordered_set<char>{'x'});
  REQUIRE(result.transitions == NFA::trans_vec{
                                    {'\0', -1, '\0', -1},
                                    {'\0', '\0', 'x', '\0'},
                                    {'\0', -1, '\0', -1},
                                    {'\0', '\0', '\0', '\0'},
                                });
}

TEST_CASE("create_one_or_more") {
  NFA result = create_one_or_more(create_basic('a'));
  REQUIRE(result.error == NFA::err_state::OK);
  REQUIRE(result.size == 3);
  REQUIRE(result.initial_state == 0);
  REQUIRE(result.final_state == 2);
  REQUIRE(result.inputs == std::unordered_set<char>{'a'});
  REQUIRE(result.transitions == NFA::trans_vec{
                                    {'\0', 'a', '\0'},
                                    {'\0', '\0', -1},
                                    {-1, '\0', '\0'},
                                });
}

TEST_CASE("create_optional") {
  NFA result = create_optional(create_basic('a'));
  REQUIRE(result.error == NFA::err_state::OK);
  REQUIRE(result.size == 4);
  REQUIRE(result.initial_state == 0);
  REQUIRE(result.final_state == 3);
  REQUIRE(result.inputs == std::unordered_set<char>{'a'});
  REQUIRE(result.transitions == NFA::trans_vec{
                                    {'\0', -1, '\0', -1},
                                    {'\0', '\0', 'a', '\0'},
                                    {'\0', '\0', '\0', -1},
                                    {'\0', '\0', '\0', '\0'},
                                });
}

TEST_CASE("create_or") {
  NFA result = create_or(create_basic('a'), create_basic('b'));
  REQUIRE(result.error == NFA::err_state::OK);
  REQUIRE(result.size == 6);
  REQUIRE(result.initial_state == 0);
  REQUIRE(result.final_state == 5);
  REQUIRE(result.inputs == std::unordered_set<char>{'a', 'b'});
  REQUIRE(result.transitions == NFA::trans_vec{
                                    {'\0', -1, '\0', -1, '\0', '\0'},
                                    {'\0', '\0', 'a', '\0', '\0', '\0'},
                                    {'\0', '\0', '\0', '\0', '\0', -1},
                                    {'\0', '\0', '\0', '\0', 'b', '\0'},
                                    {'\0', '\0', '\0', '\0', '\0', -1},
                                    {'\0', '\0', '\0', '\0', '\0', '\0'},
                                });
}

TEST_CASE("create_from_parse") {
  auto result = create_from_parse(Parser{"a|b"}.parse());
  REQUIRE(result.error == NFA::err_state::OK);
  REQUIRE(result.size == 6);
  REQUIRE(result.initial_state == 0);
  REQUIRE(result.final_state == 5);
  REQUIRE(result.inputs == std::unordered_set<char>{'a', 'b'});
  REQUIRE(result.transitions == NFA::trans_vec{
                                    {'\0', -1, '\0', -1, '\0', '\0'},
                                    {'\0', '\0', 'a', '\0', '\0', '\0'},
                                    {'\0', '\0', '\0', '\0', '\0', -1},
                                    {'\0', '\0', '\0', '\0', 'b', '\0'},
                                    {'\0', '\0', '\0', '\0', '\0', -1},
                                    {'\0', '\0', '\0', '\0', '\0', '\0'},
                                });
}

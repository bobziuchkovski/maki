//Copyright Florian Goujeon 2021.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/fgfsm

#include <fgfsm/detail/tlu/get_index.hpp>
#include <catch2/catch.hpp>
#include <tuple>

TEST_CASE("detail::tlu::get_index")
{
    using type_list = std::tuple<char, short, int, long>;

    REQUIRE(fgfsm::detail::tlu::get_index<type_list, char> == 0);
    REQUIRE(fgfsm::detail::tlu::get_index<type_list, short> == 1);
    REQUIRE(fgfsm::detail::tlu::get_index<type_list, int> == 2);
    REQUIRE(fgfsm::detail::tlu::get_index<type_list, long> == 3);
}

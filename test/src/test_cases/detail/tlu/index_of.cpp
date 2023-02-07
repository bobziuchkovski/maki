//Copyright Florian Goujeon 2021 - 2023.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/awesm

#include <awesm/detail/tlu/index_of.hpp>
#include "../../common.hpp"
#include <tuple>

TEST_CASE("detail::tlu::index_of")
{
    using type_list = std::tuple<char, short, int, long>;

    REQUIRE(awesm::detail::tlu::index_of_v<type_list, char> == 0);
    REQUIRE(awesm::detail::tlu::index_of_v<type_list, short> == 1);
    REQUIRE(awesm::detail::tlu::index_of_v<type_list, int> == 2);
    REQUIRE(awesm::detail::tlu::index_of_v<type_list, long> == 3);
}

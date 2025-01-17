//Copyright Florian Goujeon 2021 - 2023.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/maki

#ifndef COMMON_EMPTY_STATE_HPP
#define COMMON_EMPTY_STATE_HPP

#define EMPTY_STATE(name) \
    struct name \
    { \
        [[maybe_unused]] static constexpr auto conf = maki::default_state_conf; \
    }

#endif

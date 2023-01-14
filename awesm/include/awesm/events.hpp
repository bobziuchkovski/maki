//Copyright Florian Goujeon 2021 - 2022.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/awesm

#ifndef AWESM_EVENTS_HPP
#define AWESM_EVENTS_HPP

namespace awesm::events
{

//Default event sent to sm::start()
struct start{};

//Default event sent to sm::stop()
struct stop{};

//State transition completion
struct comp{};

} //namespace

#endif
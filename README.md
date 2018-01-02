# gsf_sample

### timer sample
> * gsf 时钟相关的用例, cpp实现位于cpp_test.h, lua实现位于script/test_timer.lua
| App        | Module           | Event  |
| --------   | :-----           | :----  |
|test_timer  |TimerModule       |eid::base::get_module          |
|            |TestCaseLuaModule |eid::timer::delay_milliseconds |
|            |PathModule        |eid::timer::delay_day          |
|            |TestCaseCppModule |eid::timer::remove_timer       |
|            |                  |eid::timer::timer_arrive       |
|            |                  |eid::lua_proxy::create         |
|            |                  |eid::lua_proxy::destroy        |

### echo sample
> *
| App        | Module           | Event  |
| --------   | :-----           | :----  |

### db sample
> *
| App        | Module           | Event  |
| --------   | :-----           | :----  |

### distributed sample
> *
| App        | Module           | Event  |
| --------   | :-----           | :----  |
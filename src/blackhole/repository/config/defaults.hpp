#pragma once

#include "log.hpp"

namespace blackhole {

namespace repository {

namespace config {

static inline log_config_t trivial() {
    formatter_config_t formatter("string");
    formatter["pattern"] = "[%(timestamp)s] [%(severity)s]: %(message)s";

    sink_config_t sink("files");
    sink["path"] = "/dev/stdout";
    sink["autoflush"] = true;

    frontend_config_t frontend = { formatter, sink };
    return log_config_t{ "trivial", { frontend } };
}

} // namespace config

} // namespace repository

} // namespace blackhole

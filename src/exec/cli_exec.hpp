#pragma once

#include "../CLI.hpp"

namespace cli_exec
{
enum builtin_t
{
    eq,
    pwd,
    cat,
    wc,
    echo,
    ext,
    els
};

void execute(cli::exec_list exc, cli::env_t &env);
} // namespace cli_exec
#pragma once

#include "../CLI.hpp"
#include <optional>

namespace cli_lexer
{

std::optional<cli::exec_list> lexparse(std::string str, cli::env_t &env);

}; // namespace cli_lexer

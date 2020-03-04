#include "../CLI.hpp"
#include "impl/cli_lexer_impl.hpp"

namespace cli_lexer
{
using namespace cli;
using namespace cli_lexer_impl;

std::optional<exec_list> lexparse(std::string str, env_t &env)
{
    return split_exec(preprocess(tokenize(str), env));
};

} // namespace cli_lexer

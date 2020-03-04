#include "CLI.hpp"
#include "lexparse/cli_lexer.hpp"
#include "exec/cli_exec.hpp"
#include <optional>
#include <iostream>

using namespace cli_lexer;

int main()
{
    cli::env_t env;

    while (true)
    {
        std::cout << ">";

        std::string input;
        std::getline(std::cin, input);

        std::optional<cli::exec_list> exc = cli_lexer::lexparse(input, env);

        if (!exc)
        {
            continue;
        }

        cli_exec::execute(exc.value(), env);
    }
}

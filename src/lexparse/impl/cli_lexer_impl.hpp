#pragma once

#include "../../CLI.hpp"
#include <optional>
#include <string>
#include <vector>

namespace cli_lexer_impl
{

std::optional<cli::tokens> tokenize(const std::string in);

std::optional<cli::tokens> preprocess(std::optional<cli::tokens> toks, cli::env_t &env);

std::optional<cli::exec_list> split_exec(std::optional<cli::tokens> toks);

std::optional<cli::tkn_t> process_dbraces(cli::tkn_t tok, cli::env_t &env);

std::optional<std::pair<cli::token, cli::rest>> satisfy(const std::string str, bool (*f)(char));
std::optional<std::pair<cli::token, cli::rest>> satisfy1(const std::string str, bool (*f)(char));

std::optional<std::pair<cli::token, cli::rest>> read_dbraces(const std::string str);
std::optional<std::pair<cli::token, cli::rest>> read_sbraces(const std::string str);
std::optional<std::pair<cli::token, cli::rest>> read_word(const std::string str);

cli::symb_type symbol_type(char x);
bool is_special(char x);

} // namespace cli_lexer_impl
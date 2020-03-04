#pragma once

#include <vector>
#include <string>
#include <unordered_map>

namespace cli
{

enum symb_type
{
    eq,
    pipe,
    white,
    var,
    sbrace,
    dbrace,
    els
};

struct tkn_t
{
    std::string tok;
    symb_type tp;
};

typedef std::string rest;
typedef std::string token;

typedef std::vector<tkn_t> tokens;
typedef std::vector<token> exec_unit;
typedef std::vector<exec_unit> exec_list;
typedef std::unordered_map<std::string, std::string> env_t;

} // namespace cli

#include "lexparse/cli_lexer.hpp"
#include "exec/cli_exec.hpp"
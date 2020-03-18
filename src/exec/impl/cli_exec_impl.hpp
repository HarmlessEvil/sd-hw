#pragma once

#include "../../CLI.hpp"
namespace cli_exec
{

void cli_cat(cli::exec_unit eunit);
void cli_cat_stdin(cli::exec_unit eunit);
void add_env(cli::exec_unit eunit, cli::env_t &env);
void cli_pwd(cli::exec_unit eunit);
void cli_echo(cli::exec_unit eunit);
void exec_pipe(cli::exec_unit from, cli::exec_unit to);
void cli_cat_stdin(cli::exec_unit eunit);
void cli_wc(cli::exec_unit eunit);
void cli_wc_stdin(cli::exec_unit eunit);
void exec_np(cli::exec_unit from);
void cli_cd(cli::exec_unit eunit);
void cli_ls(cli::exec_unit eunit);

builtin_t builtin(cli::token tok);

} // namespace cli_exec
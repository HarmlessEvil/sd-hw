#include "../CLI.hpp"
#include "cli_exec.hpp"
#include "impl/cli_exec_impl.hpp"
#include <string.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory>
#include <filesystem>

namespace cli_exec
{

// General execute function, forwards calls appropriately.
void execute(cli::exec_list exc, cli::env_t &env)
{
    if (exc.empty() || exc.size() > 2)
    {
        std::cout << "Invalid input" << std::endl;
        return;
    }

    if (exc.size() == 1)
    {
        cli::exec_unit eunit = exc.front();

        switch (builtin(eunit[0]))
        {
        case eq:
            add_env(eunit, env);
            break;
        case cat:
            cli_cat(eunit);
            break;
        case wc:
            cli_wc(eunit);
            break;
        case echo:
            cli_echo(eunit);
            break;
        case pwd:
            cli_pwd(eunit);
            break;
        case cd:
            cli_cd(eunit);
            break;
        case ls:
            cli_ls(eunit);
            break;
        case ext:
            exit(0);
            break;
        default:
            exec_np(eunit);
            return;
        }
    }
    else if (exc.size() == 2)
    {
        cli::exec_unit from = exc.front();
        cli::exec_unit to = exc.back();

        exec_pipe(from, to);
    }
}

// "cat" taking arguments
void cli_cat(cli::exec_unit eunit)
{
    if (eunit.empty() || eunit.front() != "cat" || eunit.size() == 1)
    {
        throw std::logic_error("cat fail");
    }

    for (size_t i = 1; i < eunit.size(); ++i)
    {
        std::ifstream in(eunit[i]);
        char c;

        if (in.is_open())
        {
            while (in.good())
            {
                in.get(c);
                std::cout << c;
            }
        }

        if (!in.eof() && in.fail())
            std::cout << "cat " + eunit[i] + " fail";

        std::cout << std::endl;
        in.close();
    }

    return;
}

void cli_cat_stdin(cli::exec_unit eunit)
{
    if (eunit.empty() || eunit.front() != "cat")
    {
        throw std::logic_error("cat call fail");
    }

    if (eunit.size() > 1)
    {
        cli_cat(eunit);
    }
    else
    {
        char c;
        while (std::cin.get(c))
        {
            std::cout << c;
        }
        std::cout << std::endl;
    }

    return;
}

void add_env(cli::exec_unit eunit, cli::env_t &env)
{
    if (eunit.empty() || eunit.front() != "=" || eunit.size() != 3)
    {
        throw std::logic_error("= fail");
    }

    env[eunit[1]] = eunit[2];
    return;
}

void cli_pwd(cli::exec_unit eunit)
{
    if (eunit.empty() || eunit.front() != "pwd")
    {
        throw std::logic_error("pwd call fail");
    }
    size_t buf_size = 1024;
    auto buf = std::make_unique<char[]>(buf_size);
    if (getcwd(buf.get(), buf_size) != nullptr)
    {
        std::cout << buf.get() << std::endl;
    };

    return;
}

void cli_cd(cli::exec_unit eunit)
{
    if (eunit.empty() || eunit.front() != "cd")
    {
        throw std::logic_error("cd call fail");
    }

    if (eunit.size() == 1)
    {
        return;
    }

    if (eunit.size() > 2)
    {
        std::cout << "invalid input to cd" << std::endl;
    }

    if (chdir(eunit[1].c_str()) != 0)
    {
        std::cout << "no such directory " + eunit[1] << std::endl;
    }

    return;
}

void cli_ls(cli::exec_unit eunit)
{
    if (eunit.empty() || eunit.front() != "ls")
    {
        throw std::logic_error("ls call fail");
    }

    if (eunit.size() > 2)
    {
        std::cout << "invalid input to ls" << std::endl;
    }

    std::string path = eunit.size() == 2 ? eunit[1] : ".";
    for (const auto &entry : std::filesystem::directory_iterator(path)) {
        std::cout << entry.path().stem().string() << std::endl;
    }
}

void cli_echo(cli::exec_unit eunit)
{
    if (eunit.empty() || eunit.front() != "echo")
    {
        throw std::logic_error("echo call fail");
    }

    for (size_t i = 1; i < eunit.size(); ++i)
    {
        std::cout << eunit[i] << " ";
    }

    std::cout << std::endl;
}

void cli_wc(cli::exec_unit eunit)
{
    if (eunit.empty() || eunit.front() != "wc")
    {
        throw std::logic_error("wc call fail");
    }

    int ttl_lines = 0;
    int ttl_bytes = 0;

    for (size_t i = 1; i < eunit.size(); ++i)
    {

        int lines = 0;
        int bytes = 0;

        std::ifstream in(eunit[i]);
        std::string c;

        if (in.is_open())
        {
            while (in.good())
            {
                getline(in, c);
                ++lines;
                bytes += c.size();
            }
        }

        if (!in.eof() && in.fail())
        {
            std::cout << "wc " + eunit[i] + " fail" << std::endl;
        }
        else
        {
            std::cout << lines << " " << bytes << " " << eunit[i] << std::endl;
            ttl_lines += lines;
            ttl_bytes += bytes;
        }

        in.close();
    }

    std::cout << ttl_lines << " " << ttl_bytes << std::endl;
}

void cli_wc_stdin(cli::exec_unit eunit)
{
    if (eunit.empty() || eunit.front() != "wc" || eunit.size() != 1)
    {
        throw std::logic_error("wc call fail");
    }

    std::string s;
    int lines = 0;
    int bytes = 0;

    while (getline(std::cin, s))
    {
        ++lines;
        bytes += s.size();
    }

    std::cout << lines << " " << bytes << std::endl;
}

void exec_np(cli::exec_unit from)
{
    std::vector<char *> from_args;
    for (int index = 0; index < from.size(); ++index)
    {
        from_args.push_back(const_cast<char *>(from[index].c_str()));
    }
    from_args.push_back(nullptr);

    if (fork() == 0)
    {
        if (execvp(from_args[0], &from_args[0]))
        {
            std::cout << "No such command" << std::endl;
            exit(1);
        };
    }

    wait(NULL);
}

void exec_pipe(cli::exec_unit from, cli::exec_unit to)
{
    std::vector<char *> from_args;
    for (int index = 0; index < from.size(); ++index)
    {
        from_args.push_back(const_cast<char *>(from[index].c_str()));
    }
    from_args.push_back(nullptr);

    std::vector<char *> to_args;
    for (int index = 0; index < to.size(); ++index)
    {
        to_args.push_back(const_cast<char *>(to[index].c_str()));
    }
    to_args.push_back(nullptr);

    int pipes[2], pid;

    pipe(pipes);

    if (fork() == 0)
    {

        dup2(pipes[1], 1);

        close(pipes[0]);
        close(pipes[1]);

        switch (builtin(from[0]))
        {
        case cat:
            cli_cat(from);
            break;
        case wc:
            cli_wc(from);
            break;
        case echo:
            cli_echo(from);
            break;
        case pwd:
            cli_pwd(from);
            break;
        case cd:
            cli_cd(from);
            break;
        case ls:
            cli_ls(from);
            break;
        case ext:
            exit(0);
        default:
            execvp(from_args[0], &from_args[0]);
        }
        exit(0);
    }
    else
    {
        if (fork() == 0)
        {
            dup2(pipes[0], 0);

            close(pipes[0]);
            close(pipes[1]);

            switch (builtin(to[0]))
            {
            case cat:
                cli_cat_stdin(to);
                break;
            case wc:
                cli_wc_stdin(to);
                break;
            case cd:
                cli_cd(from);
                break;
            case ls:
                cli_ls(from);
                break;
            case echo:
                cli_echo(to);
                break;
            case pwd:
                cli_pwd(to);
                break;
            case ext:
                exit(0);
                break;
            default:
                execvp(to_args[0], &to_args[0]);
            }
            exit(0);
        }
    }

    close(pipes[0]);
    close(pipes[1]);

    wait(NULL);
    wait(NULL);
}

builtin_t builtin(cli::token tok)
{
    if (tok == "wc")
    {
        return wc;
    }
    else if (tok == "=")
    {
        return eq;
    }
    else if (tok == "pwd")
    {
        return pwd;
    }
    else if (tok == "cat")
    {
        return cat;
    }
    else if (tok == "echo")
    {
        return echo;
    }
    else if (tok == "exit")
    {
        return ext;
    }
    else if (tok == "cd")
    {
        return cd;
    }
    else if (tok == "ls")
    {
        return ls;
    }

    return els;
}

} // namespace cli_exec
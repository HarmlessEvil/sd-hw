#include "../../CLI.hpp"
#include "cli_lexer_impl.hpp"
#include "../cli_lexer.hpp"
#include <string>
#include <numeric>
#include <algorithm>

namespace cli_lexer_impl
{

using namespace cli;

// Splits string into tokens.
std::optional<tokens> tokenize(const std::string in)
{
    tokens toks;
    std::string string = in;

    while (!string.empty())
    {
        symb_type symb = symbol_type(string[0]);

        switch (symb)
        {
        case sbrace:
        {
            std::optional<std::pair<token, rest>> opt = read_sbraces(string);

            if (!opt)
            {
                return std::nullopt;
            }

            toks.push_back({opt->first, symb});
            string = opt->second;
            break;
        }
        case dbrace:
        {
            std::optional<std::pair<token, rest>> opt = read_dbraces(string);

            if (!opt)
            {
                return std::nullopt;
            }

            toks.push_back({opt->first, symb});

            string = opt->second;
            break;
        }
        case eq:
        {
            std::optional<std::pair<token, rest>> opt = satisfy1(string, [](char x) { return x == '='; });

            if (!opt)
            {
                return std::nullopt;
            }

            if (toks.empty() || toks.back().tp != els)
            {
                return std::nullopt;
            }

            std::optional<std::pair<token, rest>> opt2 = read_word(opt->second);

            if (!opt2 || opt2->first.empty())
            {
                return std::nullopt;
            }

            tkn_t tok = toks.back();
            toks.pop_back();
            toks.push_back({opt->first, symb});
            toks.push_back(tok);
            toks.push_back({opt2->first, els});

            string = opt2->second;

            break;
        }
        case pipe:
        {
            std::optional<std::pair<token, rest>> opt = satisfy1(string, [](char x) { return x == '|'; });

            // Fail if there's nothing to parse or to pipe
            if (!opt || toks.empty() || toks.back().tp == pipe)
            {
                return std::nullopt;
            }

            toks.push_back({opt->first, symb});

            string = opt->second;

            break;
        }
        case white:
        {
            std::optional<std::pair<token, rest>> opt = satisfy(string, [](char x) { return isspace(x) != 0; });

            if (!opt)
            {
                return std::nullopt;
            }

            toks.push_back({opt->first, symb});

            string = opt->second;

            break;
        }
        case var:
        {
            std::optional<std::pair<token, rest>> opt = satisfy1(string, [](char x) { return x == '$'; });

            if (!opt)
            {
                return std::nullopt;
            }

            std::optional<std::pair<token, rest>> opt2 = read_word(opt->second);

            if (!opt2 || opt2->first.empty())
            {
                return std::nullopt;
            }

            toks.push_back({opt2->first, symb});

            string = opt2->second;
            break;
        }
        case els:
        {
            std::optional<std::pair<token, rest>> opt = read_word(string);

            if (!opt)
            {
                return std::nullopt;
            }

            toks.push_back({opt->first, symb});

            string = opt->second;
            break;
        }
        default:
            return std::nullopt;
        }
    }

    return toks;
};

// Delete white space, replace variables with values from environment.
std::optional<tokens> preprocess(std::optional<tokens> toks, env_t &env)
{
    if (!toks)
    {
        return std::nullopt;
    }

    tokens vec;

    for (size_t i = 0; i < toks->size(); ++i)
    {
        switch (toks->at(i).tp)
        {
        case var:
        {
            if (env.find(toks->at(i).tok) != env.end())
            {
                token tok = env[toks->at(i).tok];
                if (!vec.empty() && vec.back().tp == els)
                {
                    tok = vec.back().tok + tok;
                    vec.pop_back();
                }
                vec.push_back({tok, els});
            }
            break;
        }
        case dbrace:
        {
            auto op = process_dbraces(toks->at(i), env);

            if (!op)
            {
                return std::nullopt;
            }

            vec.push_back({op.value().tok, els});
            break;
        }
        default:
            vec.push_back(toks->at(i));
        }
    }
    tokens ret;
    std::copy_if(vec.begin(), vec.end(), std::back_inserter(ret), [](auto x) { return (x.tp != white); });

    return ret;
    // return vec;
}

// Split tokens into execution units. (Splits at pipe.)
std::optional<exec_list> split_exec(std::optional<tokens> toks)
{
    if (!toks)
    {
        return std::nullopt;
    }

    std::vector<std::string> vec;
    std::vector<std::vector<std::string>> ret;

    for (size_t i = 0; i < toks->size(); ++i)
    {
        if (toks->at(i).tp == pipe)
        {
            if (vec.empty() || i == (toks->size() - 1) || toks->at(i + 1).tp == pipe)
            {
                return std::nullopt;
            }

            ret.push_back(vec);
            vec.clear();
        }
        else
        {
            vec.push_back(toks->at(i).tok);
        }
    }

    if (vec.empty())
    {
        return ret;
    }

    ret.push_back(vec);

    return ret;
}

// Substitute variables inside a string double quoted string.
std::optional<tkn_t> process_dbraces(tkn_t tok, env_t &env)
{
    auto opt = tokenize(tok.tok);

    if (!opt)
    {
        return std::nullopt;
    }

    std::vector<std::string> vec;

    for (size_t i = 0; i < opt->size(); ++i)
    {
        if (opt->at(i).tp == var)
        {
            if (env.find(opt->at(i).tok) != env.end())
            {
                vec.push_back(env[opt->at(i).tok]);
            }
            continue;
        }
        vec.push_back(opt->at(i).tok);
    }

    std::string brace_insides = std::accumulate(vec.begin(), vec.end(), std::string(""));

    tkn_t expanded = {brace_insides, dbrace};

    return expanded;
};

// Parse sequence of characters that satisfies the given predicate.
// Returns parsed sequence and the rest of the string.
std::optional<std::pair<token, rest>> satisfy(const std::string str, bool (*f)(char))
{
    auto it = str.begin();

    for (; it != str.end(); ++it)
    {
        if (!f(*it))
        {
            break;
        }
    }

    std::pair<token, rest> ret = {{str.begin(), it}, {it, str.end()}};

    return ret;
}

// Parse one character that satisfies the given predicate.
// Returns parsed character and the rest of the string.
std::optional<std::pair<token, rest>> satisfy1(const std::string str, bool (*f)(char))
{
    auto it = str.begin();

    if (!f(*it))
    {
        return std::nullopt;
    }
    ++it;

    std::pair<token, rest> ret = {{str.begin(), it}, {it, str.end()}};

    return ret;
}

symb_type symbol_type(char x)
{
    if (x == '=')
    {
        return eq;
    }
    else if (x == '|')
    {
        return pipe;
    }
    else if (x == '$')
    {
        return var;
    }
    else if (x == '\'')
    {
        return sbrace;
    }
    else if (x == '"')
    {
        return dbrace;
    }
    else if (isspace(x))
    {
        return white;
    }
    return els;
}

bool is_special(char x)
{
    return symbol_type(x) != els;
}

// Parse insides of a double quoted string.
std::optional<std::pair<token, rest>> read_dbraces(const std::string str)
{
    auto op = satisfy1(str, [](char x) { return x == '"'; });

    if (!op)
    {
        return std::nullopt;
    }

    auto isnt_brace = [](char x) { return x != '"'; };

    auto opt = satisfy(op->second, isnt_brace);

    if (opt->second.empty())
    {
        return std::nullopt;
    }

    return make_pair(opt->first, std::string{++opt->second.begin(), opt->second.end()});
}

// Parse insides of a single quoted string.
std::optional<std::pair<token, rest>> read_sbraces(const std::string str)
{
    auto op = satisfy1(str, [](char x) { return x == '\''; });

    if (!op)
    {
        return std::nullopt;
    }

    auto isnt_brace = [](char x) { return x != '\''; };

    auto opt = satisfy(op->second, isnt_brace);

    if (opt->second.empty())
    {
        return std::nullopt;
    }

    return make_pair(opt->first, std::string{++opt->second.begin(), opt->second.end()});
}

// Parse sequence of non-special, non-white characters
std::optional<std::pair<token, rest>> read_word(const std::string str)
{
    auto pred = [](char x) { return !is_special(x) && !isspace(x); };
    return satisfy(str, pred);
}
}; // namespace cli_lexer_impl
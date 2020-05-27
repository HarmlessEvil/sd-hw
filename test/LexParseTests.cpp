#include "../src/CLI.hpp"
#include "../src/lexparse/cli_lexer.hpp"
#include "../src/lexparse/impl/cli_lexer_impl.hpp"
#include "../src/exec/cli_exec.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sstream>

TEST(LexParseTest, SanityCheckTest)
{
    using namespace cli;
    using namespace cli_lexer;
    env_t mp;

    mp["ab"] = "hello";
    mp["a"] = "ec";
    mp["b"] = "ho";

    std::string enter = "$a $b";

    auto ret = lexparse(enter, mp);

    EXPECT_TRUE(ret);

    enter = "$a $b | abksd | \"$a$b\" | \' hello \' | $ab ";

    exec_list vec = {{"ec", "ho"},
                     {"abksd"},
                     {"echo"},
                     {" hello "},
                     {"hello"}};

    ret = lexparse(enter, mp);
    EXPECT_TRUE(ret);

    for (size_t i = 0; i < ret->size(); ++i)
    {
        EXPECT_EQ(vec[i], ret->at(i));
    }

    enter = "||||";

    ret = lexparse(enter, mp);
    EXPECT_FALSE(ret);

    enter = "$a$b | $ab ";

    vec = {
        {"echo"},
        {"hello"},
    };

    ret = lexparse(enter, mp);
    EXPECT_TRUE(ret);

    for (size_t i = 0; i < ret->size(); ++i)
    {
        EXPECT_EQ(vec[i], ret->at(i));
    }
}

TEST(SatisfyTest, DoesEvenItWork)
{
    using namespace cli;
    using namespace cli_lexer_impl;

    std::optional<std::pair<cli::token, rest>> opt = satisfy("hello", [](char x) { return false; });

    EXPECT_TRUE("" == opt->first) << opt->first;
    EXPECT_TRUE("hello" == opt->second) << opt->second;

    opt = satisfy("hello", [](char x) { return true; });
    EXPECT_TRUE("" == opt->second) << opt->second;
    EXPECT_TRUE("hello" == opt->first) << opt->first;

    opt = satisfy("hello", [](char x) { return x != 'l'; });
    EXPECT_TRUE("he" == opt->first) << opt->first;
    EXPECT_TRUE("llo" == opt->second) << opt->second;

    opt = satisfy("hello", [](char x) { return x != 'h'; });
    EXPECT_TRUE("" == opt->first) << opt->first;
    EXPECT_TRUE("hello" == opt->second) << opt->second;

    opt = satisfy("", [](char x) { return true; });
    EXPECT_TRUE("" == opt->first) << opt->first;
    EXPECT_TRUE("" == opt->second) << opt->second;

    opt = satisfy("", [](char x) { return false; });
    EXPECT_TRUE("" == opt->first) << opt->first;
    EXPECT_TRUE("" == opt->second) << opt->second;
}

TEST(ReadBraceTest, ReadSBraces)
{
    using namespace cli;
    using namespace cli_lexer_impl;

    std::optional<std::pair<cli::token, std::string>> opt = read_sbraces("\'hello\' ");

    EXPECT_TRUE(opt);
    EXPECT_EQ("hello", opt->first);
    EXPECT_EQ(" ", opt->second);

    opt = read_sbraces("\'\'   ");

    EXPECT_TRUE(opt);
    EXPECT_EQ("", opt->first);
    EXPECT_EQ("   ", opt->second);

    opt = read_sbraces("\'   \"    \"       \'   ");

    EXPECT_TRUE(opt);
    EXPECT_EQ("   \"    \"       ", opt->first);
    EXPECT_EQ("   ", opt->second);
}

TEST(ReadBraceTest, ReadDBraces)
{
    using namespace cli;
    using namespace cli_lexer_impl;

    std::optional<std::pair<cli::token, std::string>> opt = read_dbraces("\"hello\" ");

    EXPECT_TRUE(opt);
    EXPECT_EQ("hello", opt->first);
    EXPECT_EQ(" ", opt->second);

    opt = read_dbraces("\"hello\"  \'  ");

    EXPECT_TRUE(opt);
    EXPECT_EQ("hello", opt->first);
    EXPECT_EQ("  \'  ", opt->second);

    opt = read_dbraces("\"\"");

    EXPECT_TRUE(opt);
    EXPECT_EQ("", opt->first);
    EXPECT_EQ("", opt->second);

    opt = read_dbraces("\"             ");

    EXPECT_FALSE(opt);

    opt = read_dbraces("   \"             ");

    EXPECT_FALSE(opt);

    opt = read_dbraces("\"   \'    \"       \'   ");

    EXPECT_TRUE(opt);
    EXPECT_EQ("   \'    ", opt->first);
    EXPECT_EQ("       \'   ", opt->second);
}

TEST(ReadWordTest, ReadWord)
{
    using namespace cli;
    using namespace cli_lexer_impl;

    std::optional<std::pair<cli::token, std::string>> opt = read_word("\"hello\" ");

    EXPECT_TRUE(opt);
    EXPECT_EQ("", opt->first);
    EXPECT_EQ("\"hello\" ", opt->second);

    opt = read_word("hello\"  \'  ");

    EXPECT_TRUE(opt);
    EXPECT_EQ("hello", opt->first);
    EXPECT_EQ("\"  \'  ", opt->second);

    opt = read_word("\"\"");

    EXPECT_TRUE(opt);
    EXPECT_EQ("", opt->first);
    EXPECT_EQ("\"\"", opt->second);
}

TEST(TokenizeTest, SanityCheck)
{
    using namespace cli;
    using namespace cli_lexer_impl;

    auto opt = tokenize("\"hello\" ");

    EXPECT_TRUE(opt);

    opt = tokenize("cd \\home\\user");
    EXPECT_TRUE(opt);

    opt = tokenize("cd \' this is a test \'   \" other \"");
    EXPECT_TRUE(opt);

    opt = tokenize(" alsjdf dsalfjdsl | skksks sssss kkkkk |a | sdlfjsdj");
    EXPECT_TRUE(opt);

    opt = tokenize(" alsjdf dsalfjdsl | skk\"sks ssss\'s kk\"kkk | a | sdl\'fjsd\'j");
    EXPECT_TRUE(opt);

    opt = tokenize(" alsjdf dsal$fjd sl | skk\"sks ssss\'s kk\"kkk | a | sdl\'fjsd\'j");
    EXPECT_TRUE(opt);

    opt = tokenize(" a=b ");
    EXPECT_TRUE(opt);
}

TEST(VarSub, VarSub_DBracesSub_Test)
{
    using namespace cli;
    using namespace cli_lexer_impl;

    cli::env_t mp;

    mp["ab"] = "hello";

    tkn_t tok = {"  $ab   ", dbrace};

    auto op = process_dbraces(tok, mp);

    EXPECT_TRUE(op);
    EXPECT_EQ("  hello   ", op->tok);

    mp["a"] = "ec";
    mp["b"] = "ho";

    tok = {"  $a$b   ", dbrace};

    op = process_dbraces(tok, mp);

    EXPECT_TRUE(op);
    EXPECT_EQ("  echo   ", op->tok);
}

TEST(VarSub, VarSub_variable_sub_Test)
{
    using namespace cli;
    using namespace cli_lexer_impl;

    cli::env_t mp;
    mp["ab"] = "hello";

    mp["a"] = "ec";
    mp["b"] = "ho";

    std::string enter = "$a $b";

    auto toks = tokenize(enter);

    EXPECT_TRUE(toks);

    auto res = preprocess(toks, mp);

    EXPECT_TRUE(res);

    enter = "echo $a | echo";

    toks = tokenize(enter);

    EXPECT_TRUE(toks);
    res = preprocess(toks, mp);
    std::vector<std::string> vec = {"echo", "ec", "|", "echo"};
    EXPECT_TRUE(res);

    for (size_t i = 0; i < res->size(); ++i)
    {
        EXPECT_EQ(vec[i], res->at(i).tok);
    }
}

TEST(LexPrs, PipeTests)
{
    using namespace cli;
    using namespace std;
    using namespace cli_lexer;

    cli::env_t mp;
    mp["ab"] = "hello";

    mp["a"] = "ec";
    mp["b"] = "ho";

    string enter = "   | ";
    auto opt = lexparse(enter, mp);

    EXPECT_FALSE(opt);

    // enter = " alsjdf dsalfjdsl | skksks sssss kkkkk | | sdlfjsdj"; -- quetionable
    enter = "echo |";

    opt = lexparse(enter, mp);
    EXPECT_FALSE(opt);

    enter = "echo |  ";
    opt = lexparse(enter, mp);
    EXPECT_FALSE(opt);

    enter = " |  ech";
    opt = lexparse(enter, mp);
    EXPECT_FALSE(opt);

    enter = " |  ech | ";
    opt = lexparse(enter, mp);
    EXPECT_FALSE(opt);

    enter = " ech |   | ";
    opt = lexparse(enter, mp);
    EXPECT_FALSE(opt);

    enter = " ech |   |";
    opt = lexparse(enter, mp);
    EXPECT_FALSE(opt);
}

struct cout_redirect {
    explicit cout_redirect(std::streambuf* new_buffer) : old(std::cout.rdbuf(new_buffer)) {}

    ~cout_redirect() {
        std::cout.rdbuf(old);
    }

private:
    std::streambuf* old;
};

// Cannot test ls without cd, because it's undefined, which files will be in working directory
TEST(CDLSTest, SmokeTest)
{
    std::stringstream buffer;

    {
        cout_redirect cout_redirect{buffer.rdbuf()};

        std::filesystem::create_directory("tmp");
        {
            std::ofstream tmp_file("tmp/tmp_file");
        }

        cli::env_t env{};
        cli_exec::execute({{"cd", "tmp"}}, env);
        cli_exec::execute({{"ls"}}, env);
    }
    EXPECT_EQ("tmp_file\n", buffer.str());

    std::filesystem::remove_all("tmp");
}
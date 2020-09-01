#include <cstdlib>
#include <fstream>

#include <argparse.h>

#include <snek/config.hpp>
#include <snek/cst.hpp>
#include <snek/interpreter.hpp>
#include <snek/lexer.hpp>

#if defined(SNEK_HAS_UNISTD_H)
# include <unistd.h>
#endif
#if defined(SNEK_HAS_SYSEXITS_H)
# include <sysexits.h>
#endif
#if !defined(EX_USAGE)
# define EX_USAGE 64
#endif

using namespace argparse;
using namespace snek;

namespace snek::repl { void loop(); }

static ArgumentParser
parse_arguments(int argc, const char** argv)
{
  ArgumentParser parser(argv[0], "Snek interpreter");

  parser
    .add_argument()
    .names({"--cst"})
    .description("Prints out CST of the script.");

  parser
    .add_argument()
    .names({"-f", "--file"})
    .description("Script file to execute")
    .position(ArgumentParser::Argument::Position::LAST);

  parser.enable_help();

  if (const auto error = parser.parse(argc, argv))
  {
    std::cerr << error << std::endl;
    std::exit(EX_USAGE);
  }

  return parser;
}

static inline void
die(const Error& error)
{
  std::cerr << error << std::endl;
  std::exit(EXIT_FAILURE);
}

static inline std::string
make_indent_str(int level)
{
  std::string result;

  result.reserve(level);
  for (int i = 0; i < level; ++i)
  {
    result.append(1, ' ');
    result.append(1, ' ');
  }

  return result;
}

static void
execute(
  const std::u32string& source,
  const std::u32string& file,
  ArgumentParser& args
)
{
  if (args.exists("--cst"))
  {
    using peelo::unicode::encoding::utf8::encode;
    const auto tokens = lexer::lex(source, file);
    std::size_t indent = 0;

    if (!tokens)
    {
      die(tokens.error());
    }
    for (const auto& token : tokens.value())
    {
      if (token.kind() == cst::Kind::Indent)
      {
        ++indent;
      }
      else if (indent > 0 && token.kind() == cst::Kind::Dedent)
      {
        --indent;
      }
      std::cout << make_indent_str(indent)
                << encode(cst::to_string(token.kind()))
                << std::endl;
    }
  } else {
    Interpreter interpreter;

    if (const auto error = interpreter.exec(source, file))
    {
      die(*error);
    }
  }
}

static inline bool
is_interactive_console()
{
#if defined(SNEK_HAS_ISATTY)
  return isatty(fileno(stdin));
#else
  return false;
#endif
}

int
main(int argc, const char** argv)
{
  auto args = parse_arguments(argc, argv);

  if (args.exists("help"))
  {
    args.print_help();
    std::exit(EXIT_SUCCESS);
  }

  if (args.exists("file"))
  {
    const auto path = args.get<std::string>("file");
    std::ifstream is(path, std::ios_base::in);

    if (is.good())
    {
      const std::string source = std::string(
        std::istreambuf_iterator<char>(is),
        std::istreambuf_iterator<char>()
      );

      is.close();
      execute(
        peelo::unicode::encoding::utf8::decode(source),
        peelo::unicode::encoding::utf8::decode(path),
        args
      );
    } else {
      std::cerr << "Error: Unable to open `"
                << path
                << "' for reading."
                << std::endl;
      std::exit(EXIT_FAILURE);
    }
  }
  else if (is_interactive_console())
  {
    repl::loop();
  } else {
    const std::string source = std::string(
      std::istreambuf_iterator<char>(std::cin),
      std::istreambuf_iterator<char>()
    );

    execute(
      peelo::unicode::encoding::utf8::decode(source),
      U"<stdin>",
      args
    );
  }

  return EXIT_SUCCESS;
}

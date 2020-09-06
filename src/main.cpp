#include <cstdlib>
#include <fstream>
#include <iomanip>

#include <cxxopts.hpp>

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

using namespace snek;

namespace snek::repl { void loop(); }

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
do_print_cst(const std::u32string& source, const std::u32string& file)
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
    std::cout << std::setw(2)
              << std::setfill(' ')
              << token.position().line
              << ':'
              << std::setw(2)
              << std::setfill(' ')
              << token.position().column
              << ':'
              << ' '
              << make_indent_str(indent)
              << encode(cst::to_string(token.kind()))
              << std::endl;
  }
}

static void
do_execute(const std::u32string& source, const std::u32string& file)
{
  const auto tokens = lexer::lex(source, file);
  Interpreter interpreter;

  if (const auto error = interpreter.exec(source, file))
  {
    die(*error);
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
main(int argc, char** argv)
{
  cxxopts::Options options(argv[0]);

  options
    .custom_help("[switches]")
    .positional_help("[file]")
    .add_options()
      ("print-cst", "Prints out CST of the script.")
      ("h,help", "Shows this page.")
      ("file", "Script to execute.", cxxopts::value<std::string>());
  options.parse_positional({"file", "args"});

  try
  {
    const auto result = options.parse(argc, argv);

    if (result.count("help"))
    {
      std::cout << options.help();
      std::exit(EXIT_SUCCESS);
    }
    else if (result.count("file"))
    {
      const auto file = result["file"].as<std::string>();
      std::ifstream is(file, std::ios_base::in);

      if (is.good())
      {
        const std::string source = std::string(
          std::istreambuf_iterator<char>(is),
          std::istreambuf_iterator<char>()
        );

        is.close();
        if (result.count("print-cst"))
        {
          do_print_cst(
            peelo::unicode::encoding::utf8::decode(source),
            peelo::unicode::encoding::utf8::decode(file)
          );
        } else {
          do_execute(
            peelo::unicode::encoding::utf8::decode(source),
            peelo::unicode::encoding::utf8::decode(file)
          );
        }
      } else {
        std::cerr << "Error: Unable to open `"
                  << file
                  << "' for reading."
                  << std::endl;
        std::exit(EXIT_FAILURE);
      }
    } else {
      if (!result.count("print-cst") && is_interactive_console())
      {
        repl::loop();
      } else {
        const auto source = std::string(
          std::istreambuf_iterator<char>(std::cin),
          std::istreambuf_iterator<char>()
        );

        if (result.count("print-cst"))
        {
          do_print_cst(
            peelo::unicode::encoding::utf8::decode(source),
            U"<stdin>"
          );
        } else {
          do_execute(
            peelo::unicode::encoding::utf8::decode(source),
            U"<stdin>"
          );
        }
      }
    }
  }
  catch (cxxopts::OptionException& ex)
  {
    std::cerr << options.help();
    std::exit(EX_USAGE);
  }

  return EXIT_SUCCESS;
}

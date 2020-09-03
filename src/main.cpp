#include <cstdlib>
#include <fstream>
#include <iomanip>

#include <clipp.h>

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
  std::string file;
  bool print_help = false;
  bool print_cst = false;
  std::vector<std::string> unrecognized_args;
  auto cli = (
    clipp::with_prefix("--",
      clipp::option("--print-cst")
        .set(print_cst)
        .doc("Prints out CST of the script."),
      clipp::any_other(unrecognized_args)
    ),
    clipp::with_prefix("-",
      clipp::any_other(unrecognized_args)
    ),
    clipp::option("--help", "-h")
      .set(print_help)
      .doc("Shows this page."),
    clipp::opt_value("file", file)
      .doc("Script file to execute.")
  );

  if (!clipp::parse(argc, argv, cli) || !unrecognized_args.empty())
  {
    std::cerr << clipp::make_man_page(cli, argv[0]);
    std::exit(EX_USAGE);
  }
  else if (print_help)
  {
    std::cout << clipp::make_man_page(cli, argv[0]);
    std::exit(EXIT_SUCCESS);
  }
  else if (file.empty())
  {
    if (!print_cst && is_interactive_console())
    {
      repl::loop();
    } else {
      const auto source = std::string(
        std::istreambuf_iterator<char>(std::cin),
        std::istreambuf_iterator<char>()
      );

      if (print_cst)
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
  } else {
    std::ifstream is(file, std::ios_base::in);

    if (is.good())
    {
      const std::string source = std::string(
        std::istreambuf_iterator<char>(is),
        std::istreambuf_iterator<char>()
      );

      is.close();
      if (print_cst)
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
  }

  return EXIT_SUCCESS;
}

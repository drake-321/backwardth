//
// some very wip forth REPL
//


#include <iostream>
#include <string>
#include <vector>
#include <string_view>
#include <map>
#include <sstream>
#include <stack>
#include <exception>
#include <algorithm>

class Interpreter
{
public:
  Interpreter( )
  {
    stack = std::stack<int>( );
  }

  ~Interpreter( ) = default;

  void execute_line( std::string_view line )
  {
    std::stringstream ss;

    ss << line;

    std::string tmp;
    while ( !ss.eof( ) )
    {
      ss >> tmp;

      /*
       * words
       */

      // compile mode
      if ( tmp == ":" )
      {
        std::stringstream word;

        // store word, reading everything up to ';'
        ss >> tmp;
        auto word_name = tmp;

        ss >> tmp;
        while ( tmp != ";" )
        {
          word << tmp << ' ';
          ss >> tmp;
        }

        word << ';';

        words[word_name] = word.str( );
      }
      // ignore
      else if ( tmp == ";" ) { }
      // execute a word
      else if ( words.find( tmp ) != words.end( ) )
      {
        execute_line( words[tmp] );
      }
      // see what a word does
      else if ( tmp == "see" )
      {
        ss >> tmp;
        std::cout << words[tmp] << std::endl;
      }
      // push number to stack
      else if ( auto num = parse_num( tmp )  )
      {
        stack.push( num );
        continue;
      }

      /*
       * variables 
       */

      // push variable to map
      else if ( tmp == "variable" )
      {
        ss >> tmp;

        variables[tmp] = 0;
      }

      // this is a variable, operate on it
      else if ( variables.find( tmp ) != variables.end( ) )
      {
        std::string next_tok;
        ss >> next_tok;

        auto tok = get_var_token( next_tok );

        if ( tok == var_tok::INVALID )
        {
          std::cout << "invalid variable operation" << std::endl;
        }
        // assignment
        else if ( tok == var_tok::ASSIGN )
        {
          variables[tmp] = stack.top( );
        }
        // read to stack
        else if ( tok == var_tok::READ )
        {
          stack.push( variables[tmp] );
        }
        // read to stack then print
        else if ( tok == var_tok::READPRINT )
        {
          stack.push( variables[tmp] );
          std::cout << stack.top( ) << std::endl;
          stack.pop( );
        }
      }

      // execute arithmetic op
      else if ( get_arith_token( tmp ) != arith_tok::INVALID )
      {
        auto tok = get_arith_token( tmp );

        execute_arith( tok );
      }

      // execute stack manipulation op
      else if ( get_manip_token( tmp ) != manip_tok::INVALID )
      {
        auto tok = get_manip_token( tmp );

        execute_manip( tok );
      }

      // pop and print to screen
      else if ( tmp == "." )
      {
        if ( stack.empty( ) )
        {
          std::cout << "stack is empty" << std::endl;
          continue;
        }

        std::cout << stack.top( ) << std::endl;
        stack.pop( );
      }

      // print whole stack
      else if ( tmp == ".s" )
      {
        auto tmp_stack = stack;
        std::vector<int> out{ };

        while ( !tmp_stack.empty( ) )
        {
          out.push_back( tmp_stack.top( ) );
          tmp_stack.pop( );
        }

        std::reverse( out.begin( ), out.end( ) );

        std::cout << "[ ";
        for ( const int val : out )
        {
          std::cout << val << " ";
        }
        std::cout << ']' << std::endl;
      }
      else if ( tmp == ".quit" )
      {
        std::exit( 0 );
      }
      else
      {
        std::cout << "invalid input" << std::endl;
      }

    }
  }

private:
  enum class arith_tok
  {
    INVALID,
    ADD,
    SUB,
    DIV,
    MUL,
    // ...
  };

  enum class manip_tok
  {
    INVALID,
    DUP,
    SWAP,
    ROT,
    DROP,
    // ...
  };

  enum class var_tok
  {
    INVALID,
    ASSIGN,
    READ,
    READPRINT
  };

  const std::map<std::string, arith_tok> arith_tok_map = {
    { "+", arith_tok::ADD },
    { "-", arith_tok::SUB },
    { "/", arith_tok::DIV },
    { "*", arith_tok::MUL },
  };

  const std::map<std::string, manip_tok> manip_tok_map = {
    { "dup", manip_tok::DUP },
    { "swap", manip_tok::SWAP },
    // ...
  };

  const std::map<std::string, var_tok> var_tok_map = {
    { "!", var_tok::ASSIGN },
    { "@", var_tok::READ },
    { "?", var_tok::READPRINT }
  };

  inline int parse_num( std::string s )
  {
    return std::strtol(
      s.data( ),
      reinterpret_cast<char **>((s.data( ) + s.length( ))),
      10
    );
  }

  void execute_arith( arith_tok tok )
  {
    auto a = stack.top( );
    stack.pop( );
    auto b = stack.top( );
    stack.pop( );

    switch ( tok )
    {
    case arith_tok::ADD:
    {
      stack.push( a + b );
      break;
    }
    case arith_tok::SUB:
    {
      stack.push( a - b );
      break;
    }
    case arith_tok::DIV:
    {
      stack.push( a / b );
      break;
    }
    case arith_tok::MUL:
    {
      stack.push( a * b );
      break;
    }
    default:
      throw "cannot compute arithmetic operation";
    }
  }

  void execute_manip( manip_tok tok )
  {
    switch ( tok )
    {
    case manip_tok::DUP:
    {
      stack.push( stack.top( ) );
      break;
    }
    default:
      throw "cannot compute stack manipulation";
    }
  }

  inline arith_tok get_arith_token( const std::string& s )
  {
    try
    {
      return arith_tok_map.at( s );
    }
    catch ( std::exception e )
    {
      return arith_tok::INVALID;
    }
  }

  manip_tok get_manip_token( const std::string& s )
  {
    try
    {
      return manip_tok_map.at( s );
    }
    catch ( std::exception e )
    {
      return manip_tok::INVALID;
    }
  }

  var_tok get_var_token( const std::string &s )
  {
    try
    {
      return var_tok_map.at( s );
    }
    catch ( std::exception e )
    {
      return var_tok::INVALID;
    }
  }

  std::stack<int> stack;
  std::map<std::string, int> variables;
  std::map<std::string, std::string> words;
};

int main( int argc, char *argv[] )
{
  auto interpreter = Interpreter( );

  std::string input;

  std::cout << "> ";
  while ( std::getline( std::cin, input ) )
  {
    interpreter.execute_line( input );
    std::cout << "> ";
  }

  return 0;
}
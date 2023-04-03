//----------------------------------------------------------------------
// FILE: lexer.cpp
// DATE: CPSC 326, Spring 2023
// NAME: Ian Myers
// DESC: Takes input and puts them into tokens
//----------------------------------------------------------------------

#include "lexer.h"
#include <iostream>

using namespace std;


Lexer::Lexer(istream& input_stream)
  : input {input_stream}, column {0}, line {1}
{}


char Lexer::read()
{
  ++column;
  return input.get();
}


char Lexer::peek()
{
  return input.peek();
}


void Lexer::error(const string& msg, int line, int column) const
{
  throw MyPLException::LexerError(msg + " at line " + to_string(line) +
                                  ", column " + to_string(column));
}


Token Lexer::next_token()
{
  char ch = read(); // takes in input
  string lexeme = ""; // string to build up input if needed
  if(ch == EOF) // if end of file returns end of file token
      return Token(TokenType::EOS, "end-of-stream", line, column);
  else if(ch == '\n') // if new line character updates line, resets column and returns next_token()
  {
    line++; 
    column = 0;
    return next_token();
  }
  else if(isspace(ch)) 
    return next_token();
  else if(ch == '#') // if there is a comment reads until it finds a new line character of EOF
  {
    while((ch != '\n') && (ch != EOF))
      ch = read();
    if(ch == '\n') // if new line character updates line, resets column and returns next_token()
    {
      line++;
      column = 0;
      if(peek() == EOF) // if the input after is EOF returns end of file token
      {
        ch = read();
        return Token(TokenType::EOS, "end-of-stream", line, column);
      }
      return next_token();
    }
    else 
    {
      return Token(TokenType::EOS, "end-of-stream", line, column);
    }
  }
  else if(ch == '=') // checks for = 
  {
    if(peek() == '=') // checks for ==
    {
      ch = read();
      return Token(TokenType::EQUAL, "==", line, (column - 1));
    }
    else
      return Token(TokenType::ASSIGN, "=", line, column);
  }
  // checks for punctuation
  else if(ch == '.') 
    return Token(TokenType::DOT, ".", line, column);
  else if(ch == ',')
    return Token(TokenType::COMMA, ",", line, column);
  else if(ch == '(')
    return Token(TokenType::LPAREN, "(", line, column);
  else if(ch == ')')
    return Token(TokenType::RPAREN, ")", line, column);
  else if(ch == '{')
    return Token(TokenType::LBRACE, "{", line, column);
  else if(ch == '}')
    return Token(TokenType::RBRACE, "}", line, column);
  else if(ch == ';')
    return Token(TokenType::SEMICOLON, ";", line, column);
  else if(ch == '[')
    return Token(TokenType::LBRACKET, "[", line, column);
  else if(ch == ']')
    return Token(TokenType::RBRACKET, "]", line, column);
  // checks for operators
  else if(ch == '+')
    return Token(TokenType::PLUS, "+", line, column);
  else if(ch == '-')
    return Token(TokenType::MINUS, "-", line, column);
  else if(ch == '*')
    return Token(TokenType::TIMES, "*", line, column);
  else if(ch == '/')
    return Token(TokenType::DIVIDE, "/", line, column);
  // checks for comparators
  else if(ch == '<')
  {
    if(peek() == '=') // checks if it is <=
    {
      ch = read();
      return Token(TokenType::LESS_EQ, "<=", line, (column -1));
    }
    else
      return Token(TokenType::LESS, "<", line, column);
  }
  else if(ch == '>')
  {
    if(peek() == '=') // checks if it is >=
    {
      ch = read();
      return Token(TokenType::GREATER_EQ, ">=", line, (column - 1));
    }
    else
      return Token(TokenType::GREATER, ">", line, column);
  }
  else if(ch == '!')
  {
    if(peek() == '=') // checks if it is !=
    {
      ch = read();
      return Token(TokenType::NOT_EQUAL, "!=", line, (column - 1));
    }
    else // finds the values for error to print
    {
      int start_equal = column;
      string issue_equal = "";
      while(!isspace(ch) && (ch != EOF))
      {
        issue_equal += ch;
        ch = read();
      }
      error("expecting '!=' found '" + issue_equal + "'", line, start_equal);
    }
  }
  // checks for chars
  else if(ch == '\'')
  {
    int start_char = column;
    if(peek() == '\'') // looks for empty character
      error("empty character", line, (column + 1));
    else
    {
      ch = read();
      lexeme += ch;
      if(ch == '\\') // searchs for backslash for special characters
      {
        if((peek() == 'n') || (peek() == 't') || (peek() == '0'))
        {
          ch = read(); 
          lexeme += ch;
          ch = read();
          return Token(TokenType::CHAR_VAL, lexeme, line, start_char);
        }
        else
          return Token(TokenType::CHAR_VAL, lexeme, line, start_char);
      }
      else if(ch == EOF)
        error("found end-of-file in character", line, column);
      else if(ch == '\n')
        error("found end-of-line in character", line, column);
      else if(peek() == '\'') // tokenizes the char
      {
        ch = read();
        return Token(TokenType::CHAR_VAL, lexeme, line, start_char);
      }
      else // prints out error message for invalid char
      {
        ch = read();
        string issue = "";
        issue += ch;
        error("expecting ' found " + issue, line, column);
      }
    }
      
  }
  // checks for strings
  else if(ch == '"')
  {
    int start_string = column;
    ch = read();
    while((ch != '"') && (ch != EOF) && (ch != '\n')) // builds up lexeme until end of string, EOF or new line
    {
      lexeme = lexeme + ch;
      ch = read();
    }
    if(ch == EOF) 
      error("found end-of-file in string", line, column);
    if(ch == '\n')
      error("found end-of-line in string", line, column);
    return Token(TokenType::STRING_VAL, lexeme, line, start_string);
  }
  // checks for ints and double vals
  else if(isdigit(ch))
  {
    int start_num = column;
    if((ch == '0') && (isdigit(peek()))) // checks for ints with a leading 0
      error("leading zero in number", line, column);
    lexeme = lexeme + ch;
    while(isdigit(peek())) // adds digits to lexeme until .
    {
      ch = read();
      lexeme = lexeme + ch;
    }
    if(peek() == '.')  
    {
      ch = read();
      lexeme = lexeme + ch;
      if(!isdigit(peek())) // prints an error if no digit after .
        error("missing digit in '" + lexeme + "'", line, (column + 1));
      while(isdigit(peek())) // adds digits to lexeme until it is not digit
      {
        ch = read();
        lexeme = lexeme + ch;
      }
      return Token(TokenType::DOUBLE_VAL, lexeme, line, start_num);
    }
    else
      return Token(TokenType::INT_VAL, lexeme, line, start_num);
  }
  // checks for reserve words, primitive types, bool vals and IDs
  else if(isalpha(ch))
  {
    int start_word = column;
    lexeme = lexeme + ch;
    while(isdigit(peek()) || isalpha(peek()) || (peek() == '_')) // builds up lexeme until invalid input for lexeme
    {
      ch = read();
      lexeme = lexeme + ch;
    }
    // checks lexeme against all disclosed types
    if(lexeme.compare("struct") == 0)
      return Token(TokenType::STRUCT, lexeme, line, start_word);
    else if(lexeme.compare("array") == 0)
      return Token(TokenType::ARRAY, lexeme, line, start_word);
    else if(lexeme.compare("delete") == 0)
      return Token(TokenType::DELETE, lexeme, line, start_word);
    else if(lexeme.compare("for") == 0)
      return Token(TokenType::FOR, lexeme, line, start_word);
    else if(lexeme.compare("while") == 0)
      return Token(TokenType::WHILE, lexeme, line, start_word);
    else if(lexeme.compare("if") == 0)
      return Token(TokenType::IF, lexeme, line, start_word);
    else if(lexeme.compare("elseif") == 0)
      return Token(TokenType::ELSEIF, lexeme, line, start_word);
    else if(lexeme.compare("else") == 0)
      return Token(TokenType::ELSE, lexeme, line, start_word);
    else if(lexeme.compare("and") == 0)
      return Token(TokenType::AND, lexeme, line, start_word);
    else if(lexeme.compare("or") == 0)
      return Token(TokenType::OR, lexeme, line, start_word);
    else if(lexeme.compare("not") == 0)
      return Token(TokenType::NOT, lexeme, line, start_word);
    else if(lexeme.compare("new") == 0)
      return Token(TokenType::NEW, lexeme, line, start_word);
    else if(lexeme.compare("return") == 0)
      return Token(TokenType::RETURN, lexeme, line, start_word);
    else if(lexeme.compare("and") == 0)
      return Token(TokenType::AND, lexeme, line, start_word);
    else if(lexeme.compare("null") == 0)
      return Token(TokenType::NULL_VAL, lexeme, line, start_word);
    else if(lexeme.compare("true") == 0)
      return Token(TokenType::BOOL_VAL, lexeme, line, start_word);
    else if(lexeme.compare("false") == 0)
      return Token(TokenType::BOOL_VAL, lexeme, line, start_word);
    else if(lexeme.compare("bool") == 0)
      return Token(TokenType::BOOL_TYPE, lexeme, line, start_word);
    else if(lexeme.compare("int") == 0)
      return Token(TokenType::INT_TYPE, lexeme, line, start_word);
    else if(lexeme.compare("double") == 0)
      return Token(TokenType::DOUBLE_TYPE, lexeme, line, start_word);
    else if(lexeme.compare("char") == 0)
      return Token(TokenType::CHAR_TYPE, lexeme, line, start_word);
    else if(lexeme.compare("string") == 0)
      return Token(TokenType::STRING_TYPE, lexeme, line, start_word);
    else if(lexeme.compare("void") == 0)
      return Token(TokenType::VOID_TYPE, lexeme, line, start_word);
    else // if nothing is found it is marked as an ID
      return Token(TokenType::ID, lexeme, line, start_word);
  }
  else // returns and error for the input that was not picked up by the rest of the if statement
  {
    int start_else = column;
    string issue_else = "";
    while(!isspace(ch) && (ch != EOF))
    {
      issue_else += ch;
      ch = read();
    }
    error("unexpected character '" + issue_else + "'", line, start_else);
  }
    
    


      
}

  


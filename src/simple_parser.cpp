//----------------------------------------------------------------------
// FILE: simple_parser.cpp
// DATE: CPSC 326, Spring 2023
// AUTH: Ian Myers
// DESC: Looks through code tokens for any syntatic errors
//----------------------------------------------------------------------

#include "simple_parser.h"


SimpleParser::SimpleParser(const Lexer& a_lexer)
  : lexer {a_lexer}
{}


void SimpleParser::advance()
{
  curr_token = lexer.next_token();
}


void SimpleParser::eat(TokenType t, const std::string& msg)
{
  if (!match(t))
    error(msg);
  advance();
}


bool SimpleParser::match(TokenType t)
{
  return curr_token.type() == t;
}


bool SimpleParser::match(std::initializer_list<TokenType> types)
{
  for (auto t : types)
    if (match(t))
      return true;
  return false;
}


void SimpleParser::error(const std::string& msg)
{
  std::string s = msg + " found '" + curr_token.lexeme() + "' ";
  s += "at line " + std::to_string(curr_token.line()) + ", ";
  s += "column " + std::to_string(curr_token.column());
  throw MyPLException::ParserError(s);
}


bool SimpleParser::bin_op()
{
  return match({TokenType::PLUS, TokenType::MINUS, TokenType::TIMES,
      TokenType::DIVIDE, TokenType::AND, TokenType::OR, TokenType::EQUAL,
      TokenType::LESS, TokenType::GREATER, TokenType::LESS_EQ,
      TokenType::GREATER_EQ, TokenType::NOT_EQUAL});
}

// Takes in the function or struct and calls smaller methods to check they are correct
void SimpleParser::parse()
{
  advance();
  while (!match(TokenType::EOS)) {
    if (match(TokenType::STRUCT))
      struct_def();
    else
      fun_def();
  }
  eat(TokenType::EOS, "expecting end-of-file");
}

// Takes in the struct and calls smaller method for its fields
void SimpleParser::struct_def()
{
  eat(TokenType::STRUCT, "Expecting Structure");
  eat(TokenType::ID, "Expecting ID");
  eat(TokenType::LBRACE, "Expecting LBRACE");
  fields();
  eat(TokenType::RBRACE, "Expecting RBRACE");
}

// Finds the given fields for the struct
void SimpleParser::fields()
{
  if(!match(TokenType::RBRACE))
  {
    data_type();
    eat(TokenType::ID, "Expecting ID");
    while(match(TokenType::COMMA))
    {
      advance();
      data_type();
      eat(TokenType::ID, "Expecting ID");
    }
  }
}

// Checks what kind of data type is being used
void SimpleParser::data_type()
{
  if(match(TokenType::ID))
    advance();
  else if(match(TokenType::ARRAY))
  {
    advance();
    if(match(TokenType::ID))
      advance();
    else
      base_type();
  }
  else
    base_type();
}

// Checks the type of base being used
void SimpleParser::base_type()
{
  if(match(TokenType::INT_TYPE) || match(TokenType::DOUBLE_TYPE) || match(TokenType::STRING_TYPE) || match(TokenType::CHAR_TYPE) || match(TokenType::BOOL_TYPE))
    advance();
  else
    error("Expecting Base Type");
}

// Finds what kind of statement is being used and calls the appropriate sub function
void SimpleParser::stmt()
{
  if(match(TokenType::INT_TYPE) || match(TokenType::DOUBLE_TYPE) || match(TokenType::STRING_TYPE) || match(TokenType::CHAR_TYPE) || match(TokenType::BOOL_TYPE) || match(TokenType::ARRAY))
    vdecl_stmt();
  else if(match(TokenType::ID))// Since 3 statements start with ID it must advance to differentiate them
  {
    advance();
    if(match(TokenType::LPAREN))
      call_expr();
    else if(match(TokenType::ID))
      vdecl_stmt();
    else
      assign_stmt();
  }
  else if(match(TokenType::IF))
    if_stmt();
  else if(match(TokenType::WHILE))
    while_stmt();
  else if(match(TokenType::FOR))
    for_stmt();
  else if(match(TokenType::RETURN))
    ret_stmt();
  else if(match(TokenType::DELETE))
    delete_stmt();
  else
    error("Expecting stmnt");
}

// Checks for a proper declaration of variable statement 
void SimpleParser::vdecl_stmt()
{
  if(!match(TokenType::ID))
  {
    data_type();
  }
  eat(TokenType::ID, "Expecting ID");
  eat(TokenType::ASSIGN, "Expecting ASSIGN");
  expr();
}

void SimpleParser::delete_stmt()
{
  eat(TokenType::DELETE, "Expecting Delete");
  expr();
}

// Checks for a proper assign statement
void SimpleParser::assign_stmt()
{
  lvalue();
  eat(TokenType::ASSIGN, "Expecting ASSIGN");
  expr();
}

// Finds what kind of lvalue is occuring
void SimpleParser::lvalue()
{
  while(match(TokenType::DOT) || match(TokenType::LBRACKET))
  {
    if(match(TokenType::DOT))
    {
      advance();
      eat(TokenType::ID, "Expecting ID");
    }
    else
    {
      advance();
      expr();
      eat(TokenType::RBRACKET, "Expecting RBRACKET");
    }
  }
}

// Checks that the if statement is correct
void SimpleParser::if_stmt()
{
  eat(TokenType::IF, "Expecting IF");
  eat(TokenType::LPAREN, "Expecting LPAREN");
  expr();
  eat(TokenType::RPAREN, "Expecting RPAREN");
  eat(TokenType::LBRACE, "Expecting LBRACE");
  while(!match(TokenType::RBRACE))
  {
    stmt();
  }
  advance();
  if_stmt_tail();
}

// Checks any else or else if statements after and if statement
void SimpleParser::if_stmt_tail()
{
  if(match(TokenType::ELSEIF))
  {
    eat(TokenType::ELSEIF, "Expecting ELSEIF");
    eat(TokenType::LPAREN, "Expecting LPAREN");
    expr();
    eat(TokenType::RPAREN, "Expecting RPAREN");
    eat(TokenType::LBRACE, "Expecting LBRACE");
    while(!match(TokenType::RBRACE))
      stmt();
    advance();
    if_stmt_tail();
  }
  else if(match(TokenType::ELSE))
  {
    advance();
    eat(TokenType::LBRACE, "Expecting LBRACE");
    while(!match(TokenType::RBRACE))
      stmt();
    eat(TokenType::RBRACE, "Expecting RBRACE");
  }
}

// Checks through the requirements for a while loop statement
void SimpleParser::while_stmt()
{
  eat(TokenType::WHILE, "Expecting WHILE");
  eat(TokenType::LPAREN, "Expecting LPAREN");
  expr();
  eat(TokenType::RPAREN, "Expecting RPAREN");
  eat(TokenType::LBRACE, "Expecting LBRACE");
  while(!match(TokenType::RBRACE))
    stmt();
  advance();
}

// Checks through the requirements for a for loop statement
void SimpleParser::for_stmt()
{
  eat(TokenType::FOR, "Expecting FOR");
  eat(TokenType::LPAREN, "Expecting LPAREN");
  vdecl_stmt();
  eat(TokenType::SEMICOLON, "Expecting SEMICOLON");
  expr();
  eat(TokenType::SEMICOLON, "Expecting SEMICOLON");
  eat(TokenType::ID, "Expecting ID");
  assign_stmt();
  eat(TokenType::RPAREN, "Expecting RPAREN");
  eat(TokenType::LBRACE, "Expecting LBRACE");
  while(!match(TokenType::RBRACE))
    stmt();
  advance();
}

// Checks for function call expressions
void SimpleParser::call_expr()
{
  eat(TokenType::LPAREN, "Expecting LPAREN");
  if(!match(TokenType::RPAREN))
  {
    expr();
    while(match(TokenType::COMMA))
    {
      advance();
      expr();
    }
  }
  eat(TokenType::RPAREN, "Expecting RPAREN");
}

// Checks for return statement
void SimpleParser::ret_stmt()
{
  eat(TokenType::RETURN, "Expecting RETURN");
  expr();
}

// Finds what what is the first part and second part of the expression is
void SimpleParser::expr()
{
  if(match(TokenType::NOT))
  {
    advance();
    expr();
  }
  else if(match(TokenType::LPAREN))
  {
    advance();
    expr();
    eat(TokenType::RPAREN, "Expecting RPAREN");
  }
  else
    rvalue();

  if(bin_op())
  {
    advance();
    expr();
  }
}

// Finds what kind of rvalue is occuring
void SimpleParser::rvalue()
{
  if(match(TokenType::NULL_VAL))
    advance();
  else if(match(TokenType::NEW))
    new_rvalue();
  else if(match(TokenType::ID))
  {
    advance();
    if(match(TokenType::LPAREN))
      call_expr();
    else 
      var_rvalue();
  }
  else
    base_rvalue();
}

// Parses for new rvalues 
void SimpleParser::new_rvalue()
{
  eat(TokenType::NEW, "Expecting NEW");
  if(match(TokenType::ID))
  {
    eat(TokenType::ID, "Expecting ID");
    if(match(TokenType::LBRACKET))
    {
      eat(TokenType::LBRACKET, "Expecting LBRACKET");
      expr();
      eat(TokenType::RBRACKET, "Expecting RBRACKET");
    } 
  }
  else
  {
    base_type();
    eat(TokenType::LBRACKET, "Expecting LBRACKET");
    expr();
    eat(TokenType::RBRACKET, "Expecting RBRACKET");
  }
}

// Finds the type of rvalue 
void SimpleParser::base_rvalue()
{
  if(match(TokenType::INT_VAL) || match(TokenType::DOUBLE_VAL) || match(TokenType::STRING_VAL) || match(TokenType::CHAR_VAL) || match(TokenType::BOOL_VAL))
    advance();
  else
    error("Expecting Base VAL");  
}

// Sees if anything needs to be added to var's ID in rvalue
void SimpleParser::var_rvalue()
{
  while(match(TokenType::DOT) || match(TokenType::LBRACKET))
  {
    if(match(TokenType::DOT))
    {
      advance();
      eat(TokenType::ID, "Expecting ID");
    }
    else
    {
      advance();
      expr();
      eat(TokenType::RBRACKET, "Expecting RBRACKET");
    }
  }
}

// Takes in the function and calls smaller methods for parameters, output type and statements
void SimpleParser::fun_def()
{
  if(!match(TokenType::VOID_TYPE))
    data_type();
  else
    advance();
  eat(TokenType::ID, "Expecting ID");
  eat(TokenType::LPAREN, "Expecting LPAREN");
  params();
  eat(TokenType::RPAREN, "Expecting RPAREN");
  eat(TokenType::LBRACE, "Expecting LBRACE");
  while(!match(TokenType::RBRACE))
    stmt();
  eat(TokenType::RBRACE, "Expecting RBRACE");
}

// Finds the parameters within a function 
void SimpleParser::params()
{
  if(!match(TokenType::RPAREN))
  {
    data_type();
    eat(TokenType::ID, "Expecting ID");
    while(match(TokenType::COMMA))
    {
      advance();
      data_type();
      eat(TokenType::ID, "Expecting ID");
    }
  }
}




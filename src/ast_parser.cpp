//----------------------------------------------------------------------
// FILE: ast_parser.cpp
// DATE: CPSC 326, Spring 2023
// AUTH:
// DESC:
//----------------------------------------------------------------------

#include "ast_parser.h"

using namespace std;


ASTParser::ASTParser(const Lexer& a_lexer)
  : lexer {a_lexer}
{}


void ASTParser::advance()
{
  curr_token = lexer.next_token();
}


void ASTParser::eat(TokenType t, const string& msg)
{
  if (!match(t))
    error(msg);
  advance();
}


bool ASTParser::match(TokenType t)
{
  return curr_token.type() == t;
}


bool ASTParser::match(initializer_list<TokenType> types)
{
  for (auto t : types)
    if (match(t))
      return true;
  return false;
}


void ASTParser::error(const string& msg)
{
  string s = msg + " found '" + curr_token.lexeme() + "' ";
  s += "at line " + to_string(curr_token.line()) + ", ";
  s += "column " + to_string(curr_token.column());
  throw MyPLException::ParserError(s);
}


bool ASTParser::bin_op()
{
  return match({TokenType::PLUS, TokenType::MINUS, TokenType::TIMES,
      TokenType::DIVIDE, TokenType::AND, TokenType::OR, TokenType::EQUAL,
      TokenType::LESS, TokenType::GREATER, TokenType::LESS_EQ,
      TokenType::GREATER_EQ, TokenType::NOT_EQUAL});
}


Program ASTParser::parse()
{
  Program p;
  advance();
  while (!match(TokenType::EOS)) {
    if (match(TokenType::STRUCT))
      struct_def(p);
    else
      fun_def(p);
  }
  eat(TokenType::EOS, "expecting end-of-file");
  return p;
}


// Takes in the struct and calls smaller method for its fields
void ASTParser::struct_def(Program& p)
{
  StructDef s;
  eat(TokenType::STRUCT, "Expecting Structure");
  s.struct_name = curr_token;
  eat(TokenType::ID, "Expecting ID");
  eat(TokenType::LBRACE, "Expecting LBRACE");
  fields(s); // inputs a structdef object to have its fields added
  eat(TokenType::RBRACE, "Expecting RBRACE");
  p.struct_defs.push_back(s); // pushes the struct onto the program
}

// Takes in the function and calls smaller methods for parameters, output type and statements
void ASTParser::fun_def(Program& p)
{
  FunDef f;
  if(!match(TokenType::VOID_TYPE))
    data_type(f.return_type); 
  else
  {
    f.return_type.type_name = curr_token.lexeme();
    advance();
  }
  f.fun_name = curr_token;
  eat(TokenType::ID, "Expecting ID");
  eat(TokenType::LPAREN, "Expecting LPAREN");
  params(f); // inputs fundef to have parameters added
  eat(TokenType::RPAREN, "Expecting RPAREN");
  eat(TokenType::LBRACE, "Expecting LBRACE");
  while(!match(TokenType::RBRACE))
    stmt(f.stmts); // inputs fundef vector of stmts to have stmts added
  eat(TokenType::RBRACE, "Expecting RBRACE");
  p.fun_defs.push_back(f); // pushes the struct onto the program
}

// Finds the given fields for the struct
void ASTParser::fields(StructDef& s)
{
  VarDef f;
  if(!match(TokenType::RBRACE)) // adds var name and type for each varDef in field
  {
    data_type(f.data_type);
    f.var_name = curr_token;
    eat(TokenType::ID, "Expecting ID");
    s.fields.push_back(f);
    while(match(TokenType::COMMA))
    {
      VarDef f;
      advance();
      data_type(f.data_type);
      f.var_name = curr_token;
      eat(TokenType::ID, "Expecting ID");
      s.fields.push_back(f);
    }
  }
}

// Checks what kind of data type is being used
void ASTParser::data_type(DataType& f)
{
  if(match(TokenType::ID))
  {
    f.type_name = curr_token.lexeme();
     advance();
  }
  else if(match(TokenType::ARRAY))
  {
    f.is_array = true;
    advance();
    if(match(TokenType::ID))
    {
       f.type_name = curr_token.lexeme();
       advance();
    }
    else
    {
      f.type_name = curr_token.lexeme();
      base_type();
    }
  }
  else
  {
    f.type_name = curr_token.lexeme();
    base_type();
  }
}

// Checks the type of base being used
void ASTParser::base_type()
{
  if(match(TokenType::INT_TYPE) || match(TokenType::DOUBLE_TYPE) || match(TokenType::STRING_TYPE) || match(TokenType::CHAR_TYPE) || match(TokenType::BOOL_TYPE))
      advance();
  else
    error("Expecting Base Type");
}

// Finds what kind of statement is being used and calls the appropriate sub function
void ASTParser::stmt(std::vector<std::shared_ptr<Stmt>>& s)
{
  //creates the appropriate type of stmt then adjusts it in the subfunction
  if(match(TokenType::INT_TYPE) || match(TokenType::DOUBLE_TYPE) || match(TokenType::STRING_TYPE) || match(TokenType::CHAR_TYPE) || match(TokenType::BOOL_TYPE) || match(TokenType::ARRAY))
  {
    VarDeclStmt v;
    vdecl_stmt(v);
    s.push_back(std::make_shared<VarDeclStmt>(v));
  } 
  else if(match(TokenType::ID))// Since 3 statements start with ID it must advance to differentiate them
  {
    Token t = curr_token;
    advance();
    if(match(TokenType::LPAREN))
    {
      CallExpr c;
      c.fun_name = t;
      call_expr(c);
      s.push_back(std::make_shared<CallExpr>(c));
    }
    else if(match(TokenType::ID))
    {
      VarDeclStmt v;
      v.var_def.data_type.type_name = t.lexeme();
      vdecl_stmt(v);
      s.push_back(std::make_shared<VarDeclStmt>(v));
    }
    else
    {
      AssignStmt a;
      VarRef v;
      v.var_name = t;
      a.lvalue.push_back(v);
      assign_stmt(a);
      s.push_back(std::make_shared<AssignStmt>(a));
    }
  }
  else if(match(TokenType::IF))
  {
    IfStmt i;
    if_stmt(i);
    s.push_back(std::make_shared<IfStmt>(i));
  }
  else if(match(TokenType::WHILE))
  {
    WhileStmt w;
    while_stmt(w);
    s.push_back(std::make_shared<WhileStmt>(w));
  }
  else if(match(TokenType::FOR))
  {
    ForStmt o;
    for_stmt(o);
    s.push_back(std::make_shared<ForStmt>(o));
  }
  else if(match(TokenType::RETURN))
  {
    ReturnStmt r;
    ret_stmt(r);
    s.push_back(std::make_shared<ReturnStmt>(r));
  }
  else if(match(TokenType::DELETE))
  {
    DeleteStmt d;
    delete_stmt(d);
    s.push_back(std::make_shared<DeleteStmt>(d));
  }
  else
    error("Expecting stmnt");
}

// Checks for a proper declaration of variable statement 
void ASTParser::vdecl_stmt(VarDeclStmt& v)
{
  if(!match(TokenType::ID))
  {
    data_type(v.var_def.data_type);
  }
  v.var_def.var_name = curr_token;
  eat(TokenType::ID, "Expecting ID");
  eat(TokenType::ASSIGN, "Expecting ASSIGN");
  expr(v.expr);
}

// Checks for a proper assign statement
void ASTParser::assign_stmt(AssignStmt& a)
{
  lvalue(a.lvalue);
  eat(TokenType::ASSIGN, "Expecting ASSIGN");
  expr(a.expr);
}

void ASTParser::delete_stmt(DeleteStmt& d)
{
  eat(TokenType::DELETE, "Expecting ASSIGN");
  expr(d.expr);
}

// Finds what kind of lvalue is occuring
void ASTParser::lvalue(std::vector<VarRef>& p)
{
  while(match(TokenType::DOT) || match(TokenType::LBRACKET))
  {
    if(match(TokenType::DOT))
    {
      VarRef l;
      advance();
      l.var_name = curr_token;
      eat(TokenType::ID, "Expecting ID");
      p.push_back(l);
    }
    else
    {
      VarRef r;
      r = p.back();
      p.pop_back();
      advance();
      Expr e;
      expr(e);
      r.array_expr = e;
      p.push_back(r);
      eat(TokenType::RBRACKET, "Expecting RBRACKET");
    }
  }
}

// Checks that the if statement is correct
void ASTParser::if_stmt(IfStmt& i)
{
  eat(TokenType::IF, "Expecting IF");
  eat(TokenType::LPAREN, "Expecting LPAREN");
  BasicIf b;
  expr(b.condition);
  eat(TokenType::RPAREN, "Expecting RPAREN");
  eat(TokenType::LBRACE, "Expecting LBRACE");
  while(!match(TokenType::RBRACE))
  {
    stmt(b.stmts);
  }
  i.if_part = b;
  advance();
  if_stmt_tail(i);
}

// Checks any else or else if statements after and if statement
void ASTParser::if_stmt_tail(IfStmt& i)
{
  if(match(TokenType::ELSEIF))
  {
    BasicIf b;
    eat(TokenType::ELSEIF, "Expecting ELSEIF");
    eat(TokenType::LPAREN, "Expecting LPAREN");
    expr(b.condition);
    eat(TokenType::RPAREN, "Expecting RPAREN");
    eat(TokenType::LBRACE, "Expecting LBRACE");
    while(!match(TokenType::RBRACE))
      stmt(b.stmts);
    i.else_ifs.push_back(b);
    advance();
    if_stmt_tail(i);
  }
  else if(match(TokenType::ELSE))
  {
    advance();
    eat(TokenType::LBRACE, "Expecting LBRACE");
    while(!match(TokenType::RBRACE))
      stmt(i.else_stmts);
    eat(TokenType::RBRACE, "Expecting RBRACE");
  }
}

// Checks through the requirements for a while loop statement
void ASTParser::while_stmt(WhileStmt& w)
{
  eat(TokenType::WHILE, "Expecting WHILE");
  eat(TokenType::LPAREN, "Expecting LPAREN");
  expr(w.condition);
  eat(TokenType::RPAREN, "Expecting RPAREN");
  eat(TokenType::LBRACE, "Expecting LBRACE");
  while(!match(TokenType::RBRACE))
    stmt(w.stmts);
  advance();
}

// Checks through the requirements for a for loop statement
void ASTParser::for_stmt(ForStmt& o)
{
  eat(TokenType::FOR, "Expecting FOR");
  eat(TokenType::LPAREN, "Expecting LPAREN");
  vdecl_stmt(o.var_decl);
  eat(TokenType::SEMICOLON, "Expecting SEMICOLON");
  expr(o.condition);
  eat(TokenType::SEMICOLON, "Expecting SEMICOLON");
  VarRef v;
  v.var_name = curr_token;
  o.assign_stmt.lvalue.push_back(v);
  eat(TokenType::ID, "Expecting ID");
  assign_stmt(o.assign_stmt);
  eat(TokenType::RPAREN, "Expecting RPAREN");
  eat(TokenType::LBRACE, "Expecting LBRACE");
  while(!match(TokenType::RBRACE))
    stmt(o.stmts);
  advance();
}

// Checks for function call expressions
void ASTParser::call_expr(CallExpr& c)
{
  eat(TokenType::LPAREN, "Expecting LPAREN");
  if(!match(TokenType::RPAREN))
  {
    Expr e;
    expr(e);
    c.args.push_back(e);
    while(match(TokenType::COMMA))
    {
      Expr s;
      advance();
      expr(s);
      c.args.push_back(s);
    }
  }
  eat(TokenType::RPAREN, "Expecting RPAREN");
}

// Checks for return statement
void ASTParser::ret_stmt(ReturnStmt& r)
{
  eat(TokenType::RETURN, "Expecting RETURN");
  expr(r.expr);
}

// Finds what what is the first part and second part of the expression is
void ASTParser::expr(Expr& e)
{
  if(match(TokenType::NOT))
  {
    e.negated = true;
    advance();
    expr(e);
  }
  else if(match(TokenType::LPAREN))
  {
    ComplexTerm c;
    advance();
    expr(c.expr);
    eat(TokenType::RPAREN, "Expecting RPAREN");
    e.first = std::make_shared<ComplexTerm>(c);
  }
  else
  {
    SimpleTerm s;
    rvalue(s.rvalue);
    e.first = std::make_shared<SimpleTerm>(s);
  }

  if(bin_op())
  {
    e.op = curr_token;
    advance();
    Expr r;
    expr(r);
    e.rest = std::make_shared<Expr>(r);
  }
}

// Finds what kind of rvalue is occuring
void ASTParser::rvalue(std::shared_ptr<RValue>& r)
{
  if(match(TokenType::NULL_VAL))
  {
    SimpleRValue s;
    s.value = curr_token;
    r = std::make_shared<SimpleRValue>(s);
    advance();
  }
  else if(match(TokenType::NEW))
  {
    NewRValue n;
    new_rvalue(n);
    r = std::make_shared<NewRValue>(n);
  }
  else if(match(TokenType::ID))
  {
    Token t = curr_token;
    advance();
    if(match(TokenType::LPAREN))
    {
      CallExpr c;
      c.fun_name = t;
      call_expr(c);
      r = std::make_shared<CallExpr>(c);
    }
    else 
    {
      VarRValue a;
      VarRef v;
      v.var_name = t;
      a.path.push_back(v);
      var_rvalue(a.path);
      r = std::make_shared<VarRValue>(a);
    }
  }
  else
  {
    SimpleRValue s;
    base_rvalue(s);
    r = std::make_shared<SimpleRValue>(s);
  }
}

// Parses for new rvalues 
void ASTParser::new_rvalue(NewRValue& n)
{
  eat(TokenType::NEW, "Expecting NEW");
  if(match(TokenType::ID))
  {
    n.type = curr_token;
    eat(TokenType::ID, "Expecting ID");
    if(match(TokenType::LBRACKET))
    {
      eat(TokenType::LBRACKET, "Expecting LBRACKET");
      Expr a;
      expr(a);
      n.array_expr = a;
      eat(TokenType::RBRACKET, "Expecting RBRACKET");
    } 
  }
  else
  {
    n.type = curr_token;
    base_type();
    eat(TokenType::LBRACKET, "Expecting LBRACKET");
    Expr a;
    expr(a);
    n.array_expr = a;
    eat(TokenType::RBRACKET, "Expecting RBRACKET");
  }
}

// Finds the type of rvalue 
void ASTParser::base_rvalue(SimpleRValue& r)
{
  if(match(TokenType::INT_VAL) || match(TokenType::DOUBLE_VAL) || match(TokenType::STRING_VAL) || match(TokenType::CHAR_VAL) || match(TokenType::BOOL_VAL))
  {
    r.value = curr_token;
    advance();
  }
  else
    error("Expecting Base VAL");  
}

// Sees if anything needs to be added to var's ID in rvalue
void ASTParser::var_rvalue(std::vector<VarRef>& p)
{
  while(match(TokenType::DOT) || match(TokenType::LBRACKET))
  {
    if(match(TokenType::DOT))
    {
      VarRef l;
      advance();
      l.var_name = curr_token;
      eat(TokenType::ID, "Expecting ID");
      p.push_back(l);
    }
    else
    {
      VarRef r;
      r = p.back();
      p.pop_back();
      advance();
      Expr e;
      expr(e);
      r.array_expr = e;
      p.push_back(r);
      eat(TokenType::RBRACKET, "Expecting RBRACKET");
    }
  }
}

// Finds the parameters within a function 
void ASTParser::params(FunDef& f)
{
  VarDef s;
  if(!match(TokenType::RPAREN))
  {
    data_type(s.data_type);
    s.var_name = curr_token;
    eat(TokenType::ID, "Expecting ID");
    f.params.push_back(s);
    while(match(TokenType::COMMA))
    {
      VarDef s;
      advance();
      data_type(s.data_type);
      s.var_name = curr_token;
      eat(TokenType::ID, "Expecting ID");
      f.params.push_back(s);
    }
  }
}


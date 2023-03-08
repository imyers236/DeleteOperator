//----------------------------------------------------------------------
// FILE: semantic_checker.cpp
// DATE: CPSC 326, Spring 2023
// AUTH: 
// DESC: 
//----------------------------------------------------------------------

#include <unordered_set>
#include "mypl_exception.h"
#include "semantic_checker.h"



using namespace std;

// hash table of names of the base data types and built-in functions
const unordered_set<string> BASE_TYPES {"int", "double", "char", "string", "bool"};
const unordered_set<string> BUILT_INS {"print", "input", "to_string",  "to_int",
  "to_double", "length", "get", "concat"};


// helper functions

optional<VarDef> SemanticChecker::get_field(const StructDef& struct_def,
                                            const string& field_name)
{
  for (const VarDef& var_def : struct_def.fields)
    if (var_def.var_name.lexeme() == field_name)
      return var_def;
  return nullopt;
}


void SemanticChecker::error(const string& msg, const Token& token)
{
  string s = msg;
  s += " near line " + to_string(token.line()) + ", ";
  s += "column " + to_string(token.column());
  throw MyPLException::StaticError(s);
}


void SemanticChecker::error(const string& msg)
{
  throw MyPLException::StaticError(msg);
}


// visitor functions


void SemanticChecker::visit(Program& p)
{
  // record each struct def
  for (StructDef& d : p.struct_defs) {
    string name = d.struct_name.lexeme();
    if (struct_defs.contains(name))
      error("multiple definitions of '" + name + "'", d.struct_name);
    struct_defs[name] = d;
  }
  // record each function def (need a main function)
  bool found_main = false;
  for (FunDef& f : p.fun_defs) {
    string name = f.fun_name.lexeme();
    if (BUILT_INS.contains(name))
      error("redefining built-in function '" + name + "'", f.fun_name);
    if (fun_defs.contains(name))
      error("multiple definitions of '" + name + "'", f.fun_name);
    if (name == "main") {
      if (f.return_type.type_name != "void")
        error("main function must have void type", f.fun_name);
      if (f.params.size() != 0)
        error("main function cannot have parameters", f.params[0].var_name);
      found_main = true;
    }
    fun_defs[name] = f;
  }
  if (!found_main)
    error("program missing main function");
  // check each struct
  for (StructDef& d : p.struct_defs)
    d.accept(*this);
  // check each function
  for (FunDef& d : p.fun_defs)
    d.accept(*this);
}


void SemanticChecker::visit(SimpleRValue& v)
{
  if (v.value.type() == TokenType::INT_VAL)
    curr_type = DataType {false, "int"};
  else if (v.value.type() == TokenType::DOUBLE_VAL)
    curr_type = DataType {false, "double"};    
  else if (v.value.type() == TokenType::CHAR_VAL)
    curr_type = DataType {false, "char"};    
  else if (v.value.type() == TokenType::STRING_VAL)
    curr_type = DataType {false, "string"};    
  else if (v.value.type() == TokenType::BOOL_VAL)
    curr_type = DataType {false, "bool"};    
  else if (v.value.type() == TokenType::NULL_VAL)
    curr_type = DataType {false, "void"};    
}


// TODO: Implement the rest of the visitor functions (stubbed out below)

void SemanticChecker::visit(FunDef& f)
{
  DataType return_type = f.return_type;
  //check return type is a valid type
  if((return_type.type_name != "int") && (return_type.type_name != "double") && (return_type.type_name != "char") && (return_type.type_name != "string") && (return_type.type_name != "bool") && (return_type.type_name != "void"))
  {
    error("invalid return type");
  }
  //check param type is correct
  for(int i = 0; i < f.params.size(); i++)
  {
    if((f.params[i].data_type.type_name != "int") && (f.params[i].data_type.type_name != "double") && (f.params[i].data_type.type_name != "char") && (f.params[i].data_type.type_name != "string") && (f.params[i].data_type.type_name != "bool"))
    {
      if(!symbol_table.name_exists(f.params[i].data_type.type_name) && !(struct_defs.contains(f.params[i].data_type.type_name)))
      {
        error("invalid parameter type '" + f.params[i].data_type.type_name + "'", f.params[i].var_name);
      }
    }
    //check params are different
    for(int j = i + 1; j < f.params.size(); j++)
    {
      if(f.params[i].var_name.lexeme() == f.params[j].var_name.lexeme())
      {
        error("Multiple parameters of name '" + f.params[i].var_name.lexeme() + "'", f.params[i].var_name);
      }
    }
  }
  symbol_table.push_environment();
  symbol_table.add("return", return_type);
  //add parameter name to enviroment 
  for(int i = 0; i < f.params.size(); i++)
  {
    symbol_table.add(f.params[i].var_name.lexeme(), f.params[i].data_type);
  }
  //loop stmts
  for(auto s : f.stmts)
  {
    s->accept(*this);
  }
  //pop environment
  symbol_table.pop_environment();
}


void SemanticChecker::visit(StructDef& s)
{
  for(int i = 0; i < s.fields.size(); i++)
    {
      if((s.fields[i].data_type.type_name != "int") && (s.fields[i].data_type.type_name != "double") && (s.fields[i].data_type.type_name != "char") && (s.fields[i].data_type.type_name != "string") && (s.fields[i].data_type.type_name != "bool"))
      {
        if(!(symbol_table.name_exists(s.fields[i].data_type.type_name)) && !(struct_defs.contains(s.fields[i].data_type.type_name)))
        {
          error("invalid struct type '" + s.fields[i].data_type.type_name + "'", s.fields[i].var_name);
        }
      }
      //check fields are different
      for(int j = i + 1; j < s.fields.size(); j++)
      {
        if(s.fields[i].var_name.lexeme() == s.fields[j].var_name.lexeme())
        {
          error("Multiple structs of name '" + s.fields[i].var_name.lexeme() + "'", s.fields[i].var_name);
        }
      }
    }
  //add fields name to enviroment 
  for(int i = 0; i < s.fields.size(); i++)
  {
    symbol_table.add(s.fields[i].var_name.lexeme(), s.fields[i].data_type);
  }
  //pop environment
  symbol_table.pop_environment();
}


void SemanticChecker::visit(ReturnStmt& s)
{
  s.expr.accept(*this);
}


void SemanticChecker::visit(WhileStmt& s)
{
  s.condition.accept(*this);
  if((curr_type.type_name != "bool") || (curr_type.is_array))
  {
    error("Type mismatch", s.condition.first_token());
  }
  symbol_table.push_environment();
  for(auto t : s.stmts)
  {
    t->accept(*this);
  }
  symbol_table.pop_environment();
}


void SemanticChecker::visit(ForStmt& s)
{
  s.condition.accept(*this);
  if((curr_type.type_name != "bool") || (curr_type.is_array))
  {
    error("Type mismatch", s.condition.first_token());
  }
  symbol_table.push_environment();
  s.var_decl.accept(*this);
  s.assign_stmt.accept(*this);
  for(auto t : s.stmts)
  {
    t->accept(*this);
  }
  symbol_table.pop_environment();
}


void SemanticChecker::visit(IfStmt& s)
{
  symbol_table.push_environment();
  s.if_part.condition.accept(*this);
  for(auto t : s.if_part.stmts)
  {
    t->accept(*this);
  }
  symbol_table.pop_environment();
} 

void SemanticChecker::visit(VarDeclStmt& s)
{
  if((s.var_def.data_type.type_name != "int") && (s.var_def.data_type.type_name != "double") && (s.var_def.data_type.type_name != "char") && (s.var_def.data_type.type_name != "string") && (s.var_def.data_type.type_name != "bool"))
    {
      if(!(symbol_table.name_exists(s.var_def.data_type.type_name)) && !(struct_defs.contains(s.var_def.data_type.type_name)))
        {
          error("invalid variable declaration type '" + s.var_def.data_type.type_name + "'", s.var_def.var_name);
        }
    }
  if(symbol_table.name_exists_in_curr_env(s.var_def.var_name.lexeme()))
        {
          error("Multiple vars of name '" + s.var_def.var_name.lexeme() + "' in current in enviroment", s.var_def.var_name);
        }
  symbol_table.add(s.var_def.var_name.lexeme(), s.var_def.data_type);
  s.expr.accept(*this);
  if(((curr_type.type_name != s.var_def.data_type.type_name) && (curr_type.type_name != "void")) || (curr_type.is_array != s.var_def.data_type.is_array))
    {
      error("Type mismatch", s.var_def.var_name);
    }
}


void SemanticChecker::visit(AssignStmt& s)
{
  s.expr.accept(*this);
  if(s.lvalue.size() < 2)
  {
    DataType lhs = *symbol_table.get(s.lvalue[0].var_name.lexeme()); 
    if((curr_type.type_name != lhs.type_name) || (curr_type.is_array != lhs.is_array))
    {
      error("Type mismatch", s.lvalue[0].var_name);
    }
  }
  else
  {
    //for dots and arrays
  }
}


void SemanticChecker::visit(CallExpr& e)
{
  string fun_name = e.fun_name.lexeme();
  if(fun_name == "print")
  {
    if(!(e.args.size() == 1))
    {
      error("Invalid number of parameters");
    }
    e.args[0].accept(*this);
    if(!((curr_type.type_name == "string") && (curr_type.type_name == "char") && (curr_type.type_name == "char")))
    {
      error("Invalid parameter type");
    }
    /*
    if(!(curr_type == symbol_table.get("return_type")))
    {
      error("Invalid return type");
    }
    */
  }
  if(fun_name == "get")
  {
    if(!(e.args.size() == 2))
    {
      error("Invalid number of parameters");
    }
    e.args[0].accept(*this);
    //if(!(curr_type == symbol_table.get("return_type")))
    {
      error("Invalid return type");
    }
  }
}


void SemanticChecker::visit(Expr& e)
{
  e.first->accept(*this);
  DataType lhs = curr_type;
  if(e.op.has_value())
  {
    e.rest->accept(*this);
    DataType rhs = curr_type;
    if((lhs.type_name != rhs.type_name) || (lhs.is_array != rhs.is_array))
    {
      error("Type mismatch within expression", e.first_token());
    }
    if((e.op.value().lexeme() == "+") || (e.op.value().lexeme() == "-") || (e.op.value().lexeme() == "*") || (e.op.value().lexeme() == "/"))
    {
      if((lhs.type_name != rhs.type_name) && (lhs.type_name != "double") && (lhs.type_name != "int"))
      {
        error("Invalid type cannot use " + lhs.type_name + " with " + e.op.value().lexeme(), e.first_token());
      }
    }
    else if((e.op.value().lexeme() == "==") || (e.op.value().lexeme() == "!="))
    {
      curr_type.type_name = "bool";
    }
    if((e.op.value().lexeme() == "<") || (e.op.value().lexeme() == "<=") || (e.op.value().lexeme() == ">") || (e.op.value().lexeme() == ">="))
    {
      if((lhs.type_name != "double") && (lhs.type_name != "int") && (lhs.type_name != "char") && (lhs.type_name != "string"))
      {
        error("Invalid type cannot use " + lhs.type_name + " with " + e.op.value().lexeme(), e.first_token());
      }
      curr_type.type_name = "bool";
    }
  }

}


void SemanticChecker::visit(SimpleTerm& t)
{
  t.rvalue->accept(*this);
} 


void SemanticChecker::visit(ComplexTerm& t)
{
  t.expr.accept(*this);
}


void SemanticChecker::visit(NewRValue& v)
{
  if((v.type.lexeme() != "int") && (v.type.lexeme() != "double") && (v.type.lexeme() != "char") && (v.type.lexeme() != "string") && (v.type.lexeme() != "bool"))
    {
      if(!(symbol_table.name_exists(v.type.lexeme())) && !(struct_defs.contains(v.type.lexeme())))
        {
          error("invalid New Rvalue Type type '" + v.type.lexeme() + "'", v.type);
        }
    }
}


void SemanticChecker::visit(VarRValue& v)
{

}    


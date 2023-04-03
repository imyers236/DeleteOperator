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


/**
 * inputs each struct definition in the 'struct_defs' map and each function
 * definition in the 'fun_defs' map and checks each struct and function definition
 * 
 * @param p the program to check
 */
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




/**
 * This function checks that the return type and parameter types are valid, that the
 * parameter names are unique, and that the statements in the function are valid
 * 
 * @param f The function definition node
 */
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


/**
 * It checks the structs for duplicates and correct types.
 * 
 * @param s The StructDef node that is being visited.
 */
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
  symbol_table.push_environment();
  //add fields name to enviroment 
  for(int i = 0; i < s.fields.size(); i++)
  {
    symbol_table.add(s.fields[i].var_name.lexeme(), s.fields[i].data_type);
  }
  //pop environment
  symbol_table.pop_environment();
}


/**
 * If the return type of the function is not void, then the return statement must return a value of the
 * same type as the function
 * 
 * @param s The return statement being visited.
 */
void SemanticChecker::visit(ReturnStmt& s)
{
  s.expr.accept(*this);
  DataType expected_type = symbol_table.get("return").value();
  if((symbol_table.get("return")->type_name != curr_type.type_name) && (curr_type.type_name != "void"))
  {
    error("Type mismatch returning " + curr_type.type_name + " when expected " + expected_type.type_name, s.expr.first_token());
  }
}

void SemanticChecker::visit(DeleteStmt& s)
{
  s.expr.accept(*this);
  if(!(struct_defs.contains(curr_type.type_name)) && !(curr_type.is_array))
  {
    error("Invalid type " + curr_type.type_name + " when expected struct or array", s.expr.first_token());
  }
  else if(struct_defs.contains(curr_type.type_name))
  {
    struct_defs[curr_type.type_name] = NULL;
  }
  else
  {
    
  }
}


/**
 * Checks the condition, and then check the statements
 * in the while loop
 * 
 * @param s The WhileStmt object that is being visited.
 */
void SemanticChecker::visit(WhileStmt& s)
{
  symbol_table.push_environment();
  s.condition.accept(*this);
  if((curr_type.type_name != "bool") || (curr_type.is_array))
  {
    error("Type mismatch", s.condition.first_token());
  }
  for(auto t : s.stmts)
  {
    t->accept(*this);
  }
  symbol_table.pop_environment();
}


/**
 * We push a new environment onto the symbol table, visit the variable declaration, condition, and
 * assignment statement, and then visit each statement in the for loop. 
 * 
 * @param s The statement being visited.
 */
void SemanticChecker::visit(ForStmt& s)
{
  symbol_table.push_environment();
  s.var_decl.accept(*this);
  s.condition.accept(*this);
  if((curr_type.type_name != "bool") || (curr_type.is_array))
  {
    error("Type mismatch", s.condition.first_token());
  }
  s.assign_stmt.accept(*this);
  for(auto t : s.stmts)
  {
    t->accept(*this);
  }
  symbol_table.pop_environment();
}


/**
 * Checks the condition, checks the if statements, checks the
 * else ifs and checks the else statements
 * 
 * @param s The if statement that is being visited.
 */
void SemanticChecker::visit(IfStmt& s)
{
  symbol_table.push_environment();
  s.if_part.condition.accept(*this);
  if(curr_type.type_name != "bool" || curr_type.is_array)
  {
    error("Type mismatch must have a bool in if condition");
  }
  for(auto t : s.if_part.stmts)
  {
    t->accept(*this);
  }
  symbol_table.pop_environment();
  for(auto e : s.else_ifs)
  {
    symbol_table.push_environment();
    e.condition.accept(*this);
    if(curr_type.type_name != "bool" || curr_type.is_array)
    {
      error("Type mismatch must have a bool in if condition");
    }
    for(int i = 0; i < e.stmts.size(); i++)
    {
      e.stmts[i]->accept(*this);
    }
    symbol_table.pop_environment();
  }
  symbol_table.push_environment();
  for(auto e : s.else_stmts)
  {
    e->accept(*this);
  }
  symbol_table.pop_environment();
} 

/**
 * It checks if the variable is of a valid type, and if it is not, it throws an error.
 * 
 * @param s The VarDeclStmt being visited
 */
void SemanticChecker::visit(VarDeclStmt& s)
{
  if((s.var_def.data_type.type_name != "int") && (s.var_def.data_type.type_name != "double") && (s.var_def.data_type.type_name != "char") && (s.var_def.data_type.type_name != "string") && (s.var_def.data_type.type_name != "bool"))
    {
      if(!(symbol_table.name_exists(s.var_def.data_type.type_name)) && !(struct_defs.contains(s.var_def.data_type.type_name)))
        {
          error("invalid variable declaration type '" + s.var_def.data_type.type_name + "'", s.var_def.var_name);
        }
    }
  else if(s.var_def.data_type.is_array == true)
  {
    curr_type = DataType {true, s.var_def.data_type.type_name};
  }
  if(symbol_table.name_exists_in_curr_env(s.var_def.var_name.lexeme()))
  {
    error("Multiple vars of name '" + s.var_def.var_name.lexeme() + "' in current in enviroment", s.var_def.var_name);
  }
  symbol_table.add(s.var_def.var_name.lexeme(), s.var_def.data_type);
  s.expr.accept(*this);
  if(((curr_type.type_name != s.var_def.data_type.type_name) && (curr_type.type_name != "void")))
    {
      if(s.var_def.data_type.is_array)
      {
        curr_type.is_array = true;
      }
      else
      {
        error("Type mismatch", s.var_def.var_name);
      }
    }
}


/**
 * We check that the type of the expression on the right hand side of the assignment statement is the
 * same as the type of the variable on the left hand side of the assignment statement
 * 
 * @param s The AssignStmt being visited
 */
void SemanticChecker::visit(AssignStmt& s)
{
  s.expr.accept(*this);
  DataType rhs = curr_type;
  if(s.lvalue.size() < 2)
  {
    DataType lhs = *symbol_table.get(s.lvalue[0].var_name.lexeme()); 
    if((curr_type.type_name != lhs.type_name))
    {
      error("Type mismatch", s.lvalue[0].var_name);
    }
  }
  else
  {
   string var_name = s.lvalue[0].var_name.lexeme();
   if(symbol_table.name_exists(var_name))
   {
    DataType d = symbol_table.get(var_name).value();
    curr_type = DataType(symbol_table.get(var_name)->is_array, symbol_table.get(var_name).value().type_name);
    if(struct_defs.contains(curr_type.type_name))
    {
      for(int i = 1; i < s.lvalue.size(); i++)
      {
        string var_name2 = s.lvalue[i].var_name.lexeme();
        VarDef field = get_field(struct_defs[curr_type.type_name], var_name2).value();
        curr_type = {field.data_type.is_array, field.data_type.type_name};
      }
    }
    if(curr_type.type_name != rhs.type_name)
    {
      error("Type mismatch between " + curr_type.type_name + " and " + rhs.type_name, s.lvalue[0].var_name);
    }
   }
   else
   {
    error("Use before definition");
   }
  }
}


/**
 * If the function is a built-in function, check that the parameters are correct. If the function is a
 * user-defined function, check that the parameters are correct and is defined
 * 
 * @param e The expression being visited
 */
void SemanticChecker::visit(CallExpr& e)
{
  string fun_name = e.fun_name.lexeme();
  if(fun_name == "print")
  {
    if(!(e.args.size() == 1))
    {
      error("Invalid number of parameters", e.first_token());
    }
    e.args[0].accept(*this);
    if(struct_defs.contains(curr_type.type_name))
    {
      error("Cannot print type struct", e.first_token());
    }
    if(curr_type.is_array)
    {
      error("Invalid parameters for argument one cannot have an array", e.first_token());
    }
    curr_type = {false,"void"};
  }
  else if(fun_name == "get")
  {
    if(!(e.args.size() == 2))
    {
      error("Invalid number of parameters", e.first_token());
    }
    e.args[0].accept(*this);
    if((curr_type.type_name != "int") || (curr_type.is_array))
    {
      error("Invalid parameter type cannot have " + curr_type.type_name, e.first_token());
    }
    e.args[1].accept(*this);
    if((curr_type.type_name != "string") || (curr_type.is_array))
    {
      error("Invalid parameter type cannot have " + curr_type.type_name, e.first_token());
    }
    curr_type = {false,"char"};
  }
  else if(fun_name == "to_string")
  {
    if(!(e.args.size() == 1))
    {
      error("Invalid number of parameters", e.first_token());
    }
    e.args[0].accept(*this);
    if((curr_type.type_name == "void") || (curr_type.type_name == "bool") || (curr_type.is_array))
    {
      error("Invalid parameter type cannot have " + curr_type.type_name, e.first_token());
    }
    curr_type = {false,"string"};
  }
  else if(fun_name == "input")
  {
    if(!(e.args.size() == 0))
    {
      error("Invalid number of parameters", e.first_token());
    }
    curr_type = {false,"string"};
  }
  else if(fun_name == "to_int")
  {
    if(!(e.args.size() == 1))
    {
      error("Invalid number of parameters", e.first_token());
    }
    e.args[0].accept(*this);
    if((curr_type.type_name == "void") || (curr_type.type_name == "bool") || (curr_type.is_array) || (curr_type.type_name == "int"))
    {
      error("Invalid parameter type cannot have " + curr_type.type_name, e.first_token());
    }
    curr_type = {false,"int"};
  }
  else if(fun_name == "to_double")
  {
    if(!(e.args.size() == 1))
    {
      error("Invalid number of parameters", e.first_token());
    }
    e.args[0].accept(*this);
    if((curr_type.type_name == "void") || (curr_type.type_name == "bool") || (curr_type.is_array) || (curr_type.type_name == "double"))
    {
      error("Invalid parameter type cannot have " + curr_type.type_name, e.first_token());
    }
    curr_type = {false,"double"};
  }
  else if(fun_name == "length")
  {
    if(!(e.args.size() == 1))
    {
      error("Invalid number of parameters", e.first_token());
    }
    e.args[0].accept(*this);
    if((curr_type.type_name != "string") && !(curr_type.is_array))
    {
      error("Invalid parameter type cannot have " + curr_type.type_name, e.first_token());
    }
    curr_type = {false,"int"};
  }
  else if(fun_name == "concat")
  {
    if(!(e.args.size() == 2))
    {
      error("Invalid number of parameters", e.first_token());
    }
    e.args[0].accept(*this);
    if((curr_type.type_name != "string") || (curr_type.is_array))
    {
      error("Invalid parameter type cannot have " + curr_type.type_name, e.first_token());
    }
    e.args[1].accept(*this);
    if((curr_type.type_name != "string") || (curr_type.is_array))
    {
      error("Invalid parameter type cannot have " + curr_type.type_name, e.first_token());
    }
    curr_type = {false,"string"};
  }
  else if(fun_defs.contains(fun_name))
  {
    FunDef f = fun_defs[fun_name];
    if(e.args.size() != f.params.size())
    {
      error("Invalid number of parameters", e.first_token());
    }
    for(int i = 0; i < e.args.size(); i++)
    {
      DataType param = f.params[i].data_type;
      e.args[i].accept(*this);
      if((curr_type.type_name != param.type_name) || (curr_type.is_array != param.is_array))
      {
        if(curr_type.type_name != "void")
        {
          error("Invalid parameter type cannot have " + curr_type.type_name, e.first_token());
        }
      }
    }
    curr_type = f.return_type;
  }
  else
  {
    error("Function used before defined", e.first_token());
  }

}


/**
 * The function checks the type of the first expression and checks if the type of the second expression
 * is the same as the first expression. If the operator exist it checks that the types are correct for each operator
 * 
 * @param e The expression we are visiting
 */
void SemanticChecker::visit(Expr& e)
{
  e.first->accept(*this);
  DataType lhs = curr_type;
  if(e.op.has_value())
  {
    e.rest->accept(*this);
    DataType rhs = curr_type;
    if((e.op.value().lexeme() == "+") || (e.op.value().lexeme() == "-") || (e.op.value().lexeme() == "*") || (e.op.value().lexeme() == "/"))
    {
      if((lhs.type_name != rhs.type_name) || (lhs.is_array != rhs.is_array))
      {
        error("Type mismatch must have same type for " + e.op.value().lexeme(), e.op.value());
      }
      if((lhs.type_name != "double") && (lhs.type_name != "int") && (rhs.type_name != "double") && (rhs.type_name != "int"))
      {
        error("Invalid type cannot use " + lhs.type_name + " with " + e.op.value().lexeme(), e.first_token());
      }
    }
    else if((e.op.value().lexeme() == "==") || (e.op.value().lexeme() == "!="))
    {
      if((lhs.type_name != rhs.type_name) && (lhs.type_name != "void") && (rhs.type_name != "void"))
      {
        error("Invalid type cannot use " + lhs.type_name + " with " + e.op.value().lexeme(), e.first_token());
      }
      curr_type = DataType {false, "bool"};
    }
    else if((e.op.value().lexeme() == "<") || (e.op.value().lexeme() == "<=") || (e.op.value().lexeme() == ">") || (e.op.value().lexeme() == ">="))
    {
      if((lhs.type_name != rhs.type_name) || (lhs.is_array != rhs.is_array))
      {
        error("Type mismatch must have same type for " + e.op.value().lexeme() + " cannot have type " + lhs.type_name + " with " + rhs.type_name, e.op.value());
      }
      if((lhs.type_name != "double") && (lhs.type_name != "int") && (lhs.type_name != "char") && (lhs.type_name != "string") && (rhs.type_name != "double") && (rhs.type_name != "int") && (rhs.type_name != "char") && (rhs.type_name != "string"))
      {
        error("Invalid type cannot use " + lhs.type_name + " with " + e.op.value().lexeme(), e.first_token());
      }
      curr_type.type_name = "bool";
    }
    else if((e.op.value().lexeme() == "and") || (e.op.value().lexeme() == "or") || (e.op.value().lexeme() == "not"))
    {
      if((lhs.type_name != rhs.type_name) || (lhs.is_array != rhs.is_array))
      {
        error("Type mismatch must have same type for " + e.op.value().lexeme(), e.op.value());
      }
      if((lhs.type_name != "bool") && (rhs.type_name != "bool"))
      {
        error("Invalid type cannot use " + lhs.type_name + " with " + e.op.value().lexeme(), e.first_token());
      }
      curr_type.type_name = "bool";
    }
  }

}


/**
 * Visit the term.
 * 
 * @param t The node being visited.
 */
void SemanticChecker::visit(SimpleTerm& t)
{
  t.rvalue->accept(*this);
} 


/**
 * It checks that the expression in the complex term is valid
 * 
 * @param t the term being visited
 */
void SemanticChecker::visit(ComplexTerm& t)
{
  t.expr.accept(*this);
}


/**
 * If the type of the new expression is not a primitive type, then it must be a struct type that has
 * been defined
 * 
 * @param v the node being visited
 */
void SemanticChecker::visit(NewRValue& v)
{
  if((v.type.lexeme() != "int") && (v.type.lexeme() != "double") && (v.type.lexeme() != "char") && (v.type.lexeme() != "string") && (v.type.lexeme() != "bool"))
    {
      if(!(symbol_table.name_exists(v.type.lexeme())) && !(struct_defs.contains(v.type.lexeme())))
        {
          error("invalid New Rvalue Type type '" + v.type.lexeme() + "'", v.type);
        }
    }
    if(v.array_expr.has_value())
    {
      curr_type = {true, v.type.lexeme()};
    }
    else
    {
      curr_type = {false, v.type.lexeme()};
    }
    
}


/**
 * If the variable exists, then set the current type to the type of the variable
 * 
 * @param v The VarRValue node that we're visiting.
 */
void SemanticChecker::visit(VarRValue& v)
{
  string var_name = v.path[0].var_name.lexeme();
  if(symbol_table.name_exists(var_name))
  {
    DataType d = symbol_table.get(var_name).value();
    curr_type = DataType(symbol_table.get(var_name)->is_array, symbol_table.get(var_name).value().type_name);
    if(struct_defs.contains(curr_type.type_name))
    {
      for(int i = 1; i < v.path.size(); i++)
      {
        string var_name2 = v.path[i].var_name.lexeme();
        VarDef field = get_field(struct_defs[curr_type.type_name], var_name2).value();
        curr_type = {field.data_type.is_array, field.data_type.type_name};
      }
    }
  }
  else
  {
    error("Use before definition", v.first_token());
  }
}

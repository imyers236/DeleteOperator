//----------------------------------------------------------------------
// FILE: print_visitor.cpp
// DATE: CPSC 326, Spring 2023
// AUTH: 
// DESC: 
//----------------------------------------------------------------------

#include "print_visitor.h"

using namespace std;


PrintVisitor::PrintVisitor(ostream& output)
  : out(output)
{
}


void PrintVisitor::inc_indent()
{
  indent += INDENT_AMT;
}


void PrintVisitor::dec_indent()
{
  indent -= INDENT_AMT;
}


void PrintVisitor::print_indent()
{
  out << string(indent, ' ');
}


void PrintVisitor::visit(Program& p)
{
  for (auto struct_def : p.struct_defs)
    struct_def.accept(*this);
  for (auto fun_def : p.fun_defs)
    fun_def.accept(*this);
}

void PrintVisitor::visit(FunDef& f)
{
  out << endl;
  if(f.return_type.is_array)
  {
    out << "array " << f.return_type.type_name << " " << f.fun_name.lexeme() << "(";
  }
  else
  {
    out << f.return_type.type_name << " " << f.fun_name.lexeme() << "(";
  }
  for(int i = 0; i < f.params.size(); i++)
  {
    if(f.params[i].data_type.is_array)
    {
      out << "array ";
    }
    out << f.params[i].data_type.type_name << " " <<f.params[i].var_name.lexeme();
    if(!(f.params.size() == (i+1)))
    {
      out << ", ";
    }
  }
  out << ") {\n";
  inc_indent();
  for(int i = 0; i < f.stmts.size(); i++)
  {
    print_indent();
    f.stmts[i]->accept(*this);
    out << endl;
  }
  dec_indent();
  out << "}\n";

  

}

void PrintVisitor::visit(StructDef& s)
{
  out << endl;
  out << "struct " << s.struct_name.lexeme() << " {" << endl;
  inc_indent();
  for(int i = 0; i < s.fields.size(); i++)
  {
    print_indent();
    if(s.fields[i].data_type.is_array)
    {
      out << "array ";
    }
    out << s.fields[i].data_type.type_name << " " <<s.fields[i].var_name.lexeme();
    if(!(s.fields.size() == (i+1)))
    {
      out << ",\n";
    }
  }
  dec_indent();
  out << "\n}\n";
  
}

void PrintVisitor::visit(ReturnStmt& s)
{
  out << "return ";
  s.expr.accept(*this);
}

void PrintVisitor::visit(DeleteStmt& s)
{
  out << "delete ";
  s.expr.accept(*this);
}

void PrintVisitor::visit(WhileStmt& s)
{
  out << "while (";
  inc_indent();
  s.condition.accept(*this);
  out << ") {\n";
  for(int i = 0; i < s.stmts.size(); i++)
  {
    print_indent();
    s.stmts[i]->accept(*this);
    out << endl;
  }
  dec_indent();
  print_indent();
  out << "}";

}

void PrintVisitor::visit(ForStmt& s)
{
  out << "for (";
  s.var_decl.accept(*this);
  out << "; ";
  s.condition.accept(*this);
  out << "; ";
  s.assign_stmt.accept(*this);
  out << ") {\n";
  inc_indent();
  for(int i = 0; i < s.stmts.size(); i++)
  {
    print_indent();
    s.stmts[i]->accept(*this);
    out << endl;
  }
  dec_indent();
  print_indent();
  out << "}";
}

void PrintVisitor::visit(IfStmt& s)
{
  out << "if (";
  s.if_part.condition.accept(*this);
  out << ") {\n";
  inc_indent();
  for(int i = 0; i < s.if_part.stmts.size(); i++)
  {
    print_indent();
    s.if_part.stmts[i]->accept(*this);
    out << endl;
  }
  dec_indent();
  print_indent();
  out << "}";
  if(!(s.else_ifs.size() == 0))
  {
    out << endl;    
    for(int i = 0; i < s.else_ifs.size(); i++)
    {
      print_indent();
      out << "elseif (";
      s.else_ifs[i].condition.accept(*this);
      out << ") {\n";
      inc_indent();
      for(int j = 0; j < s.else_ifs[i].stmts.size(); j++)
      {
        print_indent();
        s.else_ifs[i].stmts[j]->accept(*this);
        out << endl;
      }
      dec_indent();
      print_indent();
      out << "}\n";
    }
  }
  if(!(s.else_stmts.size() == 0))
  {
    if((s.else_ifs.size() == 0))
    {
      out << endl;
    }
    print_indent();
    out << "else {\n";
    inc_indent();
    for(int i = 0; i < s.else_stmts.size(); i++)
    {
      print_indent();
      s.else_stmts[i]->accept(*this);
      out << endl;
    }
    dec_indent();
    print_indent();
    out << "}";
  }
}

void PrintVisitor::visit(VarDeclStmt& s)
{
  out << s.var_def.data_type.type_name << " " << s.var_def.var_name.lexeme() << " = ";
  s.expr.accept(*this);
}

void PrintVisitor::visit(AssignStmt& s)
{
  for(int i = 0; i < s.lvalue.size(); i++)
  {
    out << s.lvalue[i].var_name.lexeme();
    if(s.lvalue[i].array_expr.has_value())
    {
      out << "[";
      s.lvalue[i].array_expr->accept(*this);
      out << "]";
    }
    if(!(s.lvalue.size() == (i+1)))
    {
      out << ".";
    }
  }
  out << " = ";
  s.expr.accept(*this);
}

void PrintVisitor::visit(CallExpr& e)
{
  out << e.fun_name.lexeme() << "(";
  for(int i = 0; i < e.args.size(); i++)
  {
    e.args[i].accept(*this);
    if(!(e.args.size() == (i+1)))
    {
      out << ", ";
    }
  }
  out << ")";
}

void PrintVisitor::visit(Expr& e)
{
  if(e.negated)
  {
    out << "not ("; 
  }
  e.first->accept(*this);
  if(e.op.has_value())
  {
    out << " ";
    out << e.op.value().lexeme();
    out << " ";
    e.rest->accept(*this);
  }
  if(e.negated)
  {
    out << ")"; 
  }
}

void PrintVisitor::visit(SimpleTerm& t)
{
  t.rvalue->accept(*this);
}

void PrintVisitor::visit(ComplexTerm& t)
{
  out << "(";
  t.expr.accept(*this);
  out << ")";
}

void PrintVisitor::visit(SimpleRValue& v)
{
  if(v.first_token().type() == TokenType::STRING_VAL)
  {
    out << "\"" << v.value.lexeme() << "\"";
  }
  else if(v.first_token().type() == TokenType::CHAR_VAL)
  {
    out << "\'" << v.value.lexeme() << "\'";
  }
  else
  {
    out << v.value.lexeme();
  }
}

void PrintVisitor::visit(NewRValue& v)
{
  out << "new " << v.type.lexeme();
  if(v.array_expr.has_value())
  {
    out << " [";
    v.array_expr->accept(*this);
    out << "]";
  }
}

void PrintVisitor::visit(VarRValue& v)
{
  for(int i = 0; i < v.path.size(); i++)
  {
    out << v.path[i].var_name.lexeme();
    if(v.path[i].array_expr.has_value())
    {
      out << "[";
      v.path[i].array_expr->accept(*this);
      out << "]";
    }
    if(!(v.path.size() == (i+1)))
    {
      out << ".";
    }
  }
}



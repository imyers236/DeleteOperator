//----------------------------------------------------------------------
// FILE: mypl.cpp
// DATE: Spring 2023
// AUTH: Ian Myers
// DESC: Creates a basic skeleton for command line options used later
//----------------------------------------------------------------------

#include <iostream>
#include <fstream>
#include "lexer.h"
#include "simple_parser.h"
#include "ast_parser.h"
#include "print_visitor.h"
#include "semantic_checker.h"

using namespace std;
void usage();// shows the message for help
void parse(istream* input);//  prints the first two characters of the input
void print(istream* input);// prints the first word of the input
void check(istream* input);// prints the first line of the input
void ir(istream* input);// prints the first two lines of the input
void df(istream* input);// prints the entire file(default)



int main(int argc, char* argv[])
{ 
  istream* input = &cin; 

  // changes char inputs to string
  string args[argc];
  for(int i = 0; i < argc; ++i)
  {
	args[i] = string(argv[i]);
  }

 // If statement checks which command it is and if it has a file
  if((args[1] == "--help") || (argc > 3))
	usage();
  else if(args[1] == "--lex")
  {
	if(argc == 3)// checks if it has a file
	{
		input = new ifstream(argv[2]);// sets the file to input
		if(input -> fail())// checks if the file fails
		{
			cout << "ERROR:  Unable to open file '" << argv[2] << "'" << endl;
		}
		else
		{
			try {
					Lexer lexer(*input);
					Token t = lexer.next_token();
					cout << to_string(t) << endl;
					while (t.type() != TokenType::EOS) {
						t = lexer.next_token();
						cout << to_string(t) << endl;
					}
				} catch (MyPLException& ex) {
					cerr << ex.what() << endl;
				}
		}
	}	
	else
	{
		input = &cin;
			try {
					Lexer lexer(*input);
					Token t = lexer.next_token();
					cout << to_string(t) << endl;
					while (t.type() != TokenType::EOS) {
						t = lexer.next_token();
						cout << to_string(t) << endl;
					}
				} catch (MyPLException& ex) {
					cerr << ex.what() << endl;
				}
	}
  }
  else if(args[1] == "--parse")
  {
	if(argc == 3)// checks if it has a file
	{
		input = new ifstream(argv[2]);// sets the file to input
		if(input -> fail())// checks if the file fails
		{
			cout << "ERROR:  Unable to open file '" << argv[2] << "'" << endl;
		}
		else
			try {
					Lexer lexer(*input);
					SimpleParser parser(lexer);
					parser.parse();
				} catch (MyPLException& ex) {
					cerr << ex.what() << endl;
				}
	}	
	else
	{
		input = &cin;
		try {
				Lexer lexer(*input);
				SimpleParser parser(lexer);
				parser.parse();
			} catch (MyPLException& ex) {
				cerr << ex.what() << endl;
			}
	}
  }
  else if(args[1] == "--print")
  {
	if(argc == 3)// checks if it has a file
	{
		input = new ifstream(argv[2]);// sets the file to input
		if(input -> fail())// checks if the file fails
		{
			cout << "ERROR:  Unable to open file '" << argv[2] << "'" << endl;
		}
		else
			try {
					Lexer lexer(*input);
					ASTParser parser(lexer);
					Program p = parser.parse();
					PrintVisitor v(cout);
					p.accept(v);
				} catch (MyPLException& ex) {
					cerr << ex.what() << endl;
				}
	}	
	else
	{
		input = &cin;
		try {
				Lexer lexer(*input);
				ASTParser parser(lexer);
				Program p = parser.parse();
				PrintVisitor v(cout);
				p.accept(v);
			} catch (MyPLException& ex) {
				cerr << ex.what() << endl;
			}
	}
  }
  else if(args[1] == "--check")
  {
	if(argc == 3)// checks if it has a file
	{
		input = new ifstream(argv[2]);// sets the file to input
		if(input -> fail())// checks if the file fails
		{
			cout << "ERROR:  Unable to open file '" << argv[2] << "'" << endl;
		}
		else
			try {
					Lexer lexer(*input);
					ASTParser parser(lexer);
					Program p = parser.parse();
					SemanticChecker v;
					p.accept(v);
				} catch (MyPLException& ex) {
					cerr << ex.what() << endl;
				}
	}	
	else
	{
		input = &cin;
		try {
				Lexer lexer(*input);
				ASTParser parser(lexer);
				Program p = parser.parse();
				SemanticChecker v;
				p.accept(v);
			} catch (MyPLException& ex) {
				cerr << ex.what() << endl;
			}
	}
  }
  else if(args[1] == "--ir")
  {
	if(argc == 3)// checks if it has a file
	{
		input = new ifstream(argv[2]);// sets the file to input
		if(input -> fail())// checks if the file fails
		{
			cout << "ERROR:  Unable to open file '" << argv[2] << "'" << endl;
		}
		else
			ir(input);
	}	
	else
	{
		input = &cin;
		ir(input);
	}
  }
  else
  {
	if(argc == 2)// checks if it has a file
	{
		input = new ifstream(argv[1]);// sets the file to input
		if(input -> fail())// checks if the file fails
		{
			cout << "ERROR:  Unable to open file '" << argv[1] << "'" << endl;
		}
		else
			df(input);
	}	
	else
	{
		input = &cin;
		df(input);
	}
  }
  if(input != &cin)
  	delete input;
}

	void usage()
	{
		cout << "Usage: ./mypl [option] [script-file]" << endl;
		cout << "Options: " << endl;
		cout << " --help		prints this message" << endl;
		cout << " --lex		displays token information" << endl;
		cout << " --parse	checks for syntax errors" << endl;
		cout << " --print	pretty prints program" << endl;
		cout << " --check	statically checks program" << endl;
		cout << " --ir		print intermediate (code) representation" << endl;
	}

	void parse(istream* input)
	{
		cout << "[Parse Mode]" << endl;
		char ch = input -> get();// gets the first character from file
		cout << ch;// prints character
		ch = input -> get();// gets the second character from file
		cout << ch << endl;// prints character
	}

	void print(istream* input)
	{
		cout << "[Print Mode]" << endl;
		char ch = input -> get();// gets the first character from file
		while(!isspace(ch))// prints then gets the character until there is a space
		{
			cout << ch;
			ch = input -> get();
		}
		cout << endl;
	}

	void check(istream* input)
	{
		cout << "[Check Mode]" << endl;
		char ch = input -> get();// gets the first character from file
		while(ch != '\n')// prints then gets the character until there is an end of line
		{
			cout << ch;
			ch = input -> get();
		}
		cout << endl;
	}

	void ir(istream* input)
	{
		cout << "[IR Mode]" << endl;
		int i = 0;
		string out;// string to print at the end
		while(i < 2)// adds characters to string until there are two end of lines
		{
			char ch = input -> get();
			if(ch == '\n')
			{
				++i;
			}
			out = out + ch;
		}
		cout << out;
	}
	void df(istream* input)
	{
		cout << "[Normal Mode]" << endl;
			char ch;
			string out;// string to print at the end
			while(input -> peek() != EOF)// adds to string until the end of file
			{
				ch = input -> get();
				out = out + ch;
			}
			cout << out;
	}




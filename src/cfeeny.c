#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "utils.h"
#include "bytecode.h"
#include "vm.h"
#include "ast.h"

void interpret_bc (char* filename) {  
  Program* p = load_bytecode(filename);
  print_prog(p);
  printf("\n\n");
  initvm(link_program(p));
  runvm();  
}

void interpret_ast (char* filename) {  
  ScopeStmt* s = read_ast(filename);
  print_scopestmt(s);
  printf("\n\n");
  interpret(s);
}

//Usage:
//cfeeny -ast bsearch.ast
//cfeeny -bc bsearch.bc
int main (int argc, char** argvs) {
  //Check number of arguments
  if(argc != 3){
    printf("Expected 2 arguments to commandline.\n");
    exit(-1);
  }
  if(strcmp(argvs[1], "-ast") == 0)
    interpret_ast(argvs[2]);
  else if(strcmp(argvs[1], "-bc") == 0)
    interpret_bc(argvs[2]);
  else{
    printf("Unrecognized flag: %s\n", argvs[1]);
    exit(-1);
  }
  return 0;
}



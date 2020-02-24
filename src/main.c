#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "utils.h"

//============================================================
//====================== LEXER ===============================
//============================================================

typedef enum {
  LPAREN,
  RPAREN,
  INT,
  SYMBOL,
  NO_TOKEN
} TokenTag;

typedef struct {
  TokenTag tag;
} Token;

typedef struct {
  TokenTag tag;
  int value;
} IntToken;

typedef struct {
  TokenTag tag;
  char* name;
} SymToken;

Token* make_leftparen () {
  Token* ret = (Token*)malloc(sizeof(Token));
  ret->tag = LPAREN;
  return ret;
}

Token* make_rightparen () {
  Token* ret = (Token*)malloc(sizeof(Token));
  ret->tag = RPAREN;
  return ret;
}

SymToken* make_symtoken (char* name) {
  SymToken* ret = (SymToken*)malloc(sizeof(SymToken));
  ret->tag = SYMBOL;
  ret->name = name;
  return ret;
}

IntToken* make_inttoken (int value) {
  IntToken* ret = (IntToken*)malloc(sizeof(IntToken));
  ret->tag = INT;
  ret->value = value;
  return ret;
}

char sym_chars[256];
char op_chars[256];
char num_chars[256];
Vector* lexemes;
void init_lex_tables () {
  //Reset
  lexemes = make_vector();
  for(int i=0; i<256; i++){
    sym_chars[i] = 0;
    op_chars[i] = 0;
    num_chars[i] = 0;
  }
  //Symbols
  for(char c = 'a'; c <= 'z'; c++)
    sym_chars[c] = 1;
  for(char c = 'A'; c <= 'Z'; c++)
    sym_chars[c] = 1;
  for(char c = '0'; c <= '9'; c++)
    sym_chars[c] = 1;
  sym_chars['_'] = 1;
  //Operators
  op_chars['-'] = 1;
  op_chars['+'] = 1;
  op_chars['*'] = 1;
  op_chars['/'] = 1;
  op_chars['='] = 1;
  //Numbers
  for(char c = '0'; c <= '9'; c++)
    num_chars[c] = 1;
}

char* lex_buffer;
int lex_index;
char peek_char (){
  return lex_buffer[lex_index];
}

char peek_char_i (int i){
  return lex_buffer[lex_index + i];
}

char* eat_nchars (int n) {
  char* cpy = (char*)malloc(sizeof(char) * n + 1);
  strncpy(cpy, lex_buffer + lex_index, n);
  cpy[n] = (char)0;
  lex_index += n;
  return cpy;  
}

char* eat_chars (int start, char* table) {
  int n = start;
  while(table[lex_buffer[lex_index + n]])
    n++;
  return eat_nchars(n);
}

int eat_symbol () {
  if(sym_chars[peek_char()]){
    char* name = eat_chars(0, sym_chars);
    vector_add(lexemes, make_symtoken(name));
    return 1;
  }
  return 0;
}

int eat_operator () {
  if(op_chars[peek_char()]){
    char* name = eat_nchars(1);
    vector_add(lexemes, make_symtoken(name));
    return 1;
  }
  return 0;
}

int parse_int (char* str) {
  int i;
  sscanf(str, "%d", &i);
  return i;
}

int eat_number () {
  if(peek_char() == '-' && num_chars[peek_char_i(1)]){
    char* num = eat_chars(1, num_chars);
    int i = parse_int(num);
    vector_add(lexemes, make_inttoken(i));
    return 1;
  }
  else if(num_chars[peek_char()]){
    char* num = eat_chars(0, num_chars);
    int i = parse_int(num);
    vector_add(lexemes, make_inttoken(i));
    return 1;
  }
  return 0;
}

int eat_paren () {
  if(peek_char() == '('){
    lex_index++;
    vector_add(lexemes, make_leftparen());
    return 1;
  }
  else if(peek_char() == ')'){
    lex_index++;
    vector_add(lexemes, make_rightparen());
    return 1;
  }
  return 0;
}

void eat_whitespace () {
  while(peek_char() == ' ' || peek_char() == '\n')
    lex_index++;
}

void lex (char* str) {
  lex_buffer = str;
  lex_index = 0;
  vector_clear(lexemes);
  while(peek_char() != 0){
    int ate = eat_number() || eat_symbol() || eat_operator() || eat_paren();
    if(!ate && peek_char() != 0){
      printf("Lex Error: Unexpected character %c.\n", peek_char());
      exit(-1);
    }
    eat_whitespace();
  }
}

//============================================================
//===================== AST ==================================
//============================================================

typedef enum {
  ADD_EXP,
  SUB_EXP,
  MUL_EXP,
  DIV_EXP,
  INT_EXP,
  VAR_EXP
} ExpTag;

typedef struct {
  ExpTag tag;
} Exp;

typedef struct {
  ExpTag tag;
  Exp* a;
  Exp* b;
} AddExp;

typedef struct {
  ExpTag tag;
  Exp* a;
  Exp* b;
} SubExp;

typedef struct {
  ExpTag tag;
  Exp* a;
  Exp* b;
} MulExp;

typedef struct {
  ExpTag tag;
  Exp* a;
  Exp* b;
} DivExp;

typedef struct {
  ExpTag tag;
  int value;
} IntExp;

typedef struct {
  ExpTag tag;
  char* name;
} VarExp;

typedef enum {
  CALC_STMT,
  LET_STMT
} StmtTag;

typedef struct {
  StmtTag tag;
} Stmt;

typedef struct {
  StmtTag tag;
  Exp* exp;
} CalcStmt;

typedef struct {
  StmtTag tag;
  char* name;
  Exp* exp;
} LetStmt;

AddExp* make_addexp(Exp* a, Exp* b){
  AddExp* e = malloc(sizeof(AddExp));
  e->tag = ADD_EXP;
  e->a = a;
  e->b = b;
  return e;
}

SubExp* make_subexp(Exp* a, Exp* b){
  SubExp* e = malloc(sizeof(SubExp));
  e->tag = SUB_EXP;
  e->a = a;
  e->b = b;
  return e;
}

MulExp* make_mulexp(Exp* a, Exp* b){
  MulExp* e = malloc(sizeof(MulExp));
  e->tag = MUL_EXP;
  e->a = a;
  e->b = b;
  return e;
}

DivExp* make_divexp(Exp* a, Exp* b){
  DivExp* e = malloc(sizeof(DivExp));
  e->tag = DIV_EXP;
  e->a = a;
  e->b = b;
  return e;
}

IntExp* make_intexp(int value){
  IntExp* e = (IntExp*)malloc(sizeof(IntExp));
  e->tag = INT_EXP;
  e->value = value;
  return e;
}

VarExp* make_varexp(char* name){
  VarExp* e = (VarExp*)malloc(sizeof(VarExp));
  e->tag = VAR_EXP;
  e->name = name;
  return e;
}

CalcStmt* make_calcstmt(Exp* e){
  CalcStmt* s = (CalcStmt*)malloc(sizeof(CalcStmt));
  s->tag = CALC_STMT;
  s->exp = e;
  return s;
}

LetStmt* make_letstmt(char* name, Exp* e){
  LetStmt* s = (LetStmt*)malloc(sizeof(LetStmt));
  s->tag = LET_STMT;
  s->name = name;
  s->exp = e;
  return s;
}

void print_exp (Exp* e){
  switch(e->tag){
  case ADD_EXP:{
    AddExp* e2 = (AddExp*)e;
    printf("(");
    print_exp(e2->a);
    printf(" + ");
    print_exp(e2->b);
    printf(")");
    break;    
  }
  case SUB_EXP:{
    SubExp* e2 = (SubExp*)e;
    printf("(");
    print_exp(e2->a);
    printf(" - ");
    print_exp(e2->b);
    printf(")");
    break;    
  }
  case MUL_EXP:{
    MulExp* e2 = (MulExp*)e;
    printf("(");
    print_exp(e2->a);
    printf(" * ");
    print_exp(e2->b);
    printf(")");
    break;    
  }
  case DIV_EXP:{
    DivExp* e2 = (DivExp*)e;
    printf("(");
    print_exp(e2->a);
    printf(" / ");
    print_exp(e2->b);
    printf(")");
    break;
  }
  case INT_EXP:{
    IntExp* e2 = (IntExp*)e;
    printf("%d", e2->value);
    break;
  }
  case VAR_EXP:{
    VarExp* e2 = (VarExp*)e;
    printf("%s", e2->name);
    break;
  }
  default:
    printf("Expression with unknown tag %d\n", e->tag);
    exit(-1);
  }  
}

void print_stmt (Stmt* s){
  switch(s->tag){
  case CALC_STMT:{
    CalcStmt* s2 = (CalcStmt*)s;
    print_exp(s2->exp);
    break;
  }
  case LET_STMT:{
    LetStmt* s2 = (LetStmt*)s;
    printf("let %s = ", s2->name);
    print_exp(s2->exp);
    break;
  }
  default:
    printf("Statement with unknown tag %d\n", s->tag);
    exit(-1);
  }
}

//============================================================
//==================== PARSER ================================
//============================================================

Exp* parse_exp ();
Exp* parse_term (int p);
Exp* parse_ops (Exp* a, int p);
Exp* parse_atom ();

int parse_index;
int more_tokens () {
  return parse_index < lexemes->size;
}
int n_more_tokens (int n) {
  return parse_index + n - 1 < lexemes->size;
}
Token* peek_token () {
  return vector_get(lexemes, parse_index);
}
Token* peek_token_i (int i) {
  return vector_get(lexemes, parse_index + i);
}
Token* pop_token () {
  parse_index++;
  return vector_get(lexemes, parse_index-1);
}
TokenTag peek_tag () {
  if(more_tokens())
    return peek_token()->tag;
  return NO_TOKEN;
}

int is_op(int p, char* n) {
  switch(p){
  case 0:
    return strcmp(n,"+") == 0 ||
           strcmp(n,"-") == 0;
  case 1:
    return strcmp(n,"*") == 0 ||
           strcmp(n,"/") == 0;
  default:
    return 0;
  }
}

Exp* parse_exp () {
  return parse_term(0);
}

Exp* parse_term (int p) {
  if(p < 2) return parse_ops(parse_term(p + 1), p);
  else return parse_atom();
}
 
Exp* parse_ops (Exp* a, int p) {
  switch(peek_tag()){
  case SYMBOL:{
    SymToken* t = (SymToken*)peek_token();
    if(is_op(p, t->name)){
      pop_token();
      if(strcmp(t->name, "+") == 0)
        return parse_ops((Exp*)make_addexp(a, parse_term(p + 1)), p);
      else if(strcmp(t->name, "-") == 0)
        return parse_ops((Exp*)make_subexp(a, parse_term(p + 1)), p);
      else if(strcmp(t->name, "*") == 0)
        return parse_ops((Exp*)make_mulexp(a, parse_term(p + 1)), p);
      else if(strcmp(t->name, "/") == 0)
        return parse_ops((Exp*)make_divexp(a, parse_term(p + 1)), p);
    }
    else{
      return a;
    }
  }
  default:
    return a;
  }
}

Exp* parse_atom () {
  if(!more_tokens()){
    printf("Parse error: Unexpected end of input.\n");
    exit(-1);
  }
  
  Token* t = pop_token();
  switch(t->tag){
  case INT:{
    IntToken* t2 = (IntToken*)t;
    return (Exp*)make_intexp(t2->value);
  }
  case SYMBOL:{
    SymToken* t2 = (SymToken*)t;
    if(strlen(t2->name) != 1){
      printf("Invalid variable name: %s\n", t2->name);
      exit(-1);
    }
    char c = t2->name[0];
    if(c < 'a' || c > 'z'){
      printf("Invalid variable name: %s\n", t2->name);
      exit(-1);
    }    
    return (Exp*)make_varexp(t2->name);
  }
  case LPAREN:{
    Exp* e = parse_exp();
    Token* t2 = pop_token();
    if(t2->tag != RPAREN){
      printf("Parse error: Unbalanced parenthesis.\n");
      exit(-1);
    }
    return e;
  }
  default:
    printf("Parse error: Unexpected token with tag %d.\n", t->tag);
    exit(-1);
  }
}

int let_pending () {
  if(n_more_tokens(2)){
    Token* x = peek_token();
    if(x->tag != SYMBOL)
      return 0;    
    Token* t = peek_token_i(1);
    if(t->tag == SYMBOL){
      SymToken* t2 = (SymToken*)t;
      return strcmp(t2->name, "=") == 0;
    }
  }
  return 0;
}

Stmt* parse_stmt () {
  if(let_pending()){
    SymToken* x = (SymToken*)pop_token();
    pop_token();
    return (Stmt*)make_letstmt(x->name, parse_exp());
  }
  else{
    return (Stmt*)make_calcstmt(parse_exp());
  }
}

void ensure_no_more_tokens () {
  if(more_tokens()){
    printf("Parse error: Unexpected token with tag %d.\n", peek_token()->tag);
    exit(-1);
  }
}

Stmt* parse_one_stmt (char* str){
  lex(str);
  parse_index = 0;
  Stmt* s = parse_stmt();
  ensure_no_more_tokens();
  return s;
}

//============================================================
//==================== EVALUATOR =============================
//============================================================

int var_idx (char* name) {
  char c = name[0];
  return c - 'a';
}

int eval_exp (Exp* e, int* env) {
  switch(e->tag){
  case ADD_EXP:{
    AddExp* e2 = (AddExp*)e;
    int a = eval_exp(e2->a, env);
    int b = eval_exp(e2->b, env);
    return a + b;
  }
  case SUB_EXP:{
    SubExp* e2 = (SubExp*)e;
    int a = eval_exp(e2->a, env);
    int b = eval_exp(e2->b, env);
    return a - b;
  }
  case MUL_EXP:{
    MulExp* e2 = (MulExp*)e;
    int a = eval_exp(e2->a, env);
    int b = eval_exp(e2->b, env);
    return a * b;
  }
  case DIV_EXP:{
    DivExp* e2 = (DivExp*)e;
    int a = eval_exp(e2->a, env);
    int b = eval_exp(e2->b, env);
    return a / b;
  }
  case INT_EXP:{
    IntExp* e2 = (IntExp*)e;
    return e2->value;
  }    
  case VAR_EXP:{
    VarExp* e2 = (VarExp*)e;
    return env[var_idx(e2->name)];
  }
  default:
    printf("Unsupported expression with tag %d\n", e->tag);
    exit(-1);
  }
}

void run_stmt (Stmt* s, int* env) {
  switch(s->tag){
  case CALC_STMT:{
    CalcStmt* s2 = (CalcStmt*)s;
    int r = eval_exp(s2->exp, env);
    print_stmt((Stmt*)s2);
    printf(" = ");
    printf("%d\n", r);
    break;
  }
  case LET_STMT:{
    LetStmt* s2 = (LetStmt*)s;
    int r = eval_exp(s2->exp, env);
    env[var_idx(s2->name)] = r;
    print_stmt((Stmt*)s2);
    printf(" = ");
    printf("%d\n", r);
    break;
  }
  default:
    printf("Unsupported statement with tag: %d\n", s->tag);
    exit(-1);
  }
}

int* new_env () {
  return malloc(sizeof(int)*26);
}

//============================================================
//=================== BYTECODE FORMAT ========================
//============================================================
typedef enum {
  LIT_OP,
  ADD_OP,
  SUB_OP,
  MUL_OP,
  DIV_OP,
  GET_OP,
  SET_OP,
  END_OP
} OpCode;

typedef struct {
  char op;
} Ins;

typedef struct {
  char op;
  char b3;
  char b2;
  char b1;
  char b0;
} LitIns;

typedef struct {
  char op;
  char idx;
} GetIns;

typedef struct {
  char op;
  char idx;  
} SetIns;

void print_ins (char* ins) {
  while(1){
    switch(*ins){
    case LIT_OP:{
      LitIns* i = (LitIns*)ins;
      int v = (i->b3 << 3*8) + (i->b2 << 2*8) + (i->b1 << 1*8) + i->b0;
      printf("   LIT[%d]\n", v);
      ins += sizeof(LitIns);
      continue;
    }      
    case GET_OP:{
      GetIns* i = (GetIns*)ins;
      printf("   GET[%c]\n", 'a' + i->idx);
      ins += sizeof(GetIns);
      continue;
    }
    case SET_OP:{
      SetIns* i = (SetIns*)ins;
      printf("   SET[%c]\n", 'a' + i->idx);
      ins += sizeof(SetIns);
      continue;
    }
    case ADD_OP:
      printf("   ADD\n");
      ins++;
      continue;
    case SUB_OP:
      printf("   SUB\n");
      ins++;
      continue;
    case MUL_OP:
      printf("   MUL\n");
      ins++;
      continue;
    case DIV_OP:
      printf("   DIV\n");
      ins++;
      continue;
    case END_OP:
      return;
    }
  }
}

//============================================================
//================= BYTECODE COMPILER ========================
//============================================================
char* push_ins (Exp* e, char* buffer) {
  switch(e->tag){
  case ADD_EXP:{
    AddExp* e2 = (AddExp*)e;
    buffer = push_ins(e2->a, buffer);
    buffer = push_ins(e2->b, buffer);
    buffer[0] = ADD_OP;
    return buffer+1;
  }
  case SUB_EXP:{
    SubExp* e2 = (SubExp*)e;
    buffer = push_ins(e2->a, buffer);
    buffer = push_ins(e2->b, buffer);
    buffer[0] = SUB_OP;
    return buffer+1;
  }
  case MUL_EXP:{
    MulExp* e2 = (MulExp*)e;
    buffer = push_ins(e2->a, buffer);
    buffer = push_ins(e2->b, buffer);
    buffer[0] = MUL_OP;
    return buffer+1;
  }
  case DIV_EXP:{
    DivExp* e2 = (DivExp*)e;
    buffer = push_ins(e2->a, buffer);
    buffer = push_ins(e2->b, buffer);
    buffer[0] = DIV_OP;
    return buffer+1;
  }
  case INT_EXP:{
    LitIns* i = (LitIns*)buffer;
    IntExp* e2 = (IntExp*)e;
    int v = e2->value;
    i->op = LIT_OP;
    i->b3 = (char)(v >> 3*8);
    i->b2 = (char)(v >> 2*8);
    i->b1 = (char)(v >> 1*8);
    i->b0 = (char)(v);
    return buffer + sizeof(LitIns);
  }
  case VAR_EXP:{
    GetIns* i = (GetIns*)buffer;
    VarExp* e2 = (VarExp*)e;
    i->op = GET_OP;
    i->idx = (char)var_idx(e2->name);
    return buffer + sizeof(GetIns);
  }
  default:
    printf("Unknown expression with tag: %d\n", e->tag);
    exit(-1);
  }
}

char* compile (Stmt* s) {
  if(s->tag == CALC_STMT){
    CalcStmt* s2 = (CalcStmt*)s;
    char* ins = malloc(1024 * 1024);
    char* end = push_ins(s2->exp, ins);
    end[0] = END_OP;
    return ins;    
  }
  else if(s->tag == LET_STMT){
    LetStmt* s2 = (LetStmt*)s;
    char* ins = malloc(1024 * 1024);
    char* end = push_ins(s2->exp, ins);
    //Set Ins
    SetIns* i = (SetIns*)end;
    i->op = SET_OP;
    i->idx = (char)var_idx(s2->name);
    end += sizeof(SetIns);
    //End Ins
    end[0] = END_OP;
    return ins;        
  }
  else{
    printf("Unknown statement with tag: %d\n", s->tag);
    exit(-1);
  }
}

//============================================================
//=============== BYTECODE INTERPRETER =======================
//============================================================

int run_ins (char* ins, int* env) {
  int* stack = malloc(sizeof(int) * 1024);
  int* sp = stack;
  int running = 1;
  while(running){
    switch(*ins){
    case LIT_OP:{
      LitIns* i = (LitIns*)ins;
      int v = (i->b3 << 3*8) + (i->b2 << 2*8) + (i->b1 << 1*8) + i->b0;
      sp[0] = v;
      sp++;
      ins += sizeof(LitIns);
      continue;
    }      
    case GET_OP:{
      GetIns* i = (GetIns*)ins;
      sp[0] = env[i->idx];
      sp++;
      ins += sizeof(GetIns);
      continue;
    }
    case SET_OP:{
      SetIns* i = (SetIns*)ins;
      env[i->idx] = sp[-1];
      ins += sizeof(SetIns);
      continue;
    }
    case ADD_OP:{
      sp--;
      sp[-1] = sp[-1] + sp[0];
      ins++;
      continue;
    }
    case SUB_OP:
      sp--;
      sp[-1] = sp[-1] - sp[0];
      ins++;
      continue;
    case MUL_OP:
      sp--;
      sp[-1] = sp[-1] * sp[0];
      ins++;
      continue;
    case DIV_OP:
      sp--;
      sp[-1] = sp[-1] / sp[0];
      ins++;
      continue;
    case END_OP:
      running = 0;
      break;
    }
  }
  int v = sp[-1];
  free(stack);
  return v;
}

void compile_run_stmt (Stmt* s, int* env) {
  char* ins = compile(s);
  int r = run_ins(ins, env);
  printf("Code:\n");
  print_ins(ins);
  if(s->tag == CALC_STMT)
    printf("Result = %d\n", r);
}

//============================================================
//==================== DRIVERS ===============================
//============================================================

void test_eater (char* s) {
  init_lex_tables();
  lex(s);
}

void test_parse (char* str) {
  init_lex_tables();
  Stmt* s = parse_one_stmt(str);
  printf("Parsed: ");
  print_stmt(s);
  printf("\n");
}

char* read_line () {
  char* line;
  int size = 0;
  getline(&line, &size, stdin);
  return line;
}

void test_eval () {
  init_lex_tables();
  int* env = new_env();
  while(1){
    printf("> ");
    fflush(stdout);
    char* line = read_line();
    Stmt* s = parse_one_stmt(line);
    run_stmt(s, env);
  }
}

void test_compile () {
  init_lex_tables();
  int* env = new_env();
  while(1){
    printf("> ");
    fflush(stdout);
    char* line = read_line();
    Stmt* s = parse_one_stmt(line);
    compile_run_stmt(s, env);
  }
}

int main (int argc, char** argvs) {
  if(argc < 2){
    printf("No flags given.\n");
    printf("Use either -eval, or -compile\n");
    exit(-1);
  }
  if(strcmp(argvs[1], "-eval") == 0)
    test_eval();
  else if(strcmp(argvs[1], "-compile") == 0)
    test_compile();
}

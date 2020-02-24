#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "utils.h"
#include "ast.h"

//============================================================
//================= CONSTRUCTORS =============================
//============================================================

Exp* make_IntExp (int value) {
  IntExp* e = malloc(sizeof(IntExp));
  e->tag = INT_EXP;
  e->value = value;
  return (Exp*)e;
}

Exp* make_NullExp () {
  Exp* e = malloc(sizeof(Exp));
  e->tag = NULL_EXP;
  return e;
}

Exp* make_PrintfExp (char* format, int nexps, Exp** exps) {
  PrintfExp* e = malloc(sizeof(PrintfExp));
  e->tag = PRINTF_EXP;
  e->format = format;
  e->nexps = nexps;
  e->exps = exps;
  return (Exp*)e;
}

Exp* make_ArrayExp (Exp* length, Exp* init) {
  ArrayExp* e = malloc(sizeof(ArrayExp));
  e->tag = ARRAY_EXP;
  e->length = length;
  e->init = init;
  return (Exp*)e;
}

Exp* make_ObjectExp (Exp* parent, int nslots, SlotStmt** slots) {
  ObjectExp* e = malloc(sizeof(ObjectExp));
  e->tag = OBJECT_EXP;
  e->parent = parent;
  e->nslots = nslots;
  e->slots = slots;
  return (Exp*)e;
}

Exp* make_SlotExp (char* name, Exp* exp) {
  SlotExp* e = malloc(sizeof(SlotExp));
  e->tag = SLOT_EXP;
  e->name = name;
  e->exp = exp;
  return (Exp*)e;
}

Exp* make_SetSlotExp (char* name, Exp* exp, Exp* value) {
  SetSlotExp* e = malloc(sizeof(SetSlotExp));
  e->tag = SET_SLOT_EXP;
  e->name = name;
  e->exp = exp;
  e->value = value;
  return (Exp*)e;
}

Exp* make_CallSlotExp (char* name, Exp* exp, int nargs, Exp** args) {
  CallSlotExp* e = malloc(sizeof(CallSlotExp));
  e->tag = CALL_SLOT_EXP;
  e->name = name;
  e->exp = exp;
  e->nargs = nargs;
  e->args = args;
  return (Exp*)e;
}

Exp* make_CallExp (char* name, int nargs, Exp** args) {
  CallExp* e = malloc(sizeof(CallExp));
  e->tag = CALL_EXP;
  e->name = name;
  e->nargs = nargs;
  e->args = args;
  return (Exp*)e;
}

Exp* make_SetExp (char* name, Exp* exp) {
  SetExp* e = malloc(sizeof(SetExp));
  e->tag = SET_EXP;
  e->name = name;
  e->exp = exp;
  return (Exp*)e;
}

Exp* make_IfExp (Exp* pred, ScopeStmt* conseq, ScopeStmt* alt) {
  IfExp* e = malloc(sizeof(IfExp));
  e->tag = IF_EXP;
  e->pred = pred;
  e->conseq = conseq;
  e->alt = alt;
  return (Exp*)e;
}

Exp* make_WhileExp (Exp* pred, ScopeStmt* body) {
  WhileExp* e = malloc(sizeof(WhileExp));
  e->tag = WHILE_EXP;
  e->pred = pred;
  e->body = body;
  return (Exp*)e;
}

Exp* make_RefExp (char* name) {
  RefExp* e = malloc(sizeof(RefExp));
  e->tag = REF_EXP;
  e->name = name;
  return (Exp*)e;
}

SlotStmt* make_SlotVar (char* name, Exp* exp) {
  SlotVar* s = malloc(sizeof(SlotVar));
  s->tag = VAR_STMT;
  s->name = name;
  s->exp = exp;
  return (SlotStmt*)s;
}

SlotStmt* make_SlotMethod (char* name, int nargs, char** args, ScopeStmt* body) {
  SlotMethod* s = malloc(sizeof(SlotMethod));
  s->tag = FN_STMT;
  s->name = name;
  s->nargs = nargs;
  s->args = args;
  s->body = body;
  return (SlotStmt*)s;
}

ScopeStmt* make_ScopeVar (char* name, Exp* exp) {
  ScopeVar* s = malloc(sizeof(ScopeVar));
  s->tag = VAR_STMT;
  s->name = name;
  s->exp = exp;
  return (ScopeStmt*)s;
}

ScopeStmt* make_ScopeFn (char* name, int nargs, char** args, ScopeStmt* body) {
  ScopeFn* s = malloc(sizeof(ScopeFn));
  s->tag = FN_STMT;
  s->name = name;
  s->nargs = nargs;
  s->args = args;
  s->body = body;
  return (ScopeStmt*)s;
}

ScopeStmt* make_ScopeSeq (ScopeStmt* a, ScopeStmt* b) {
  ScopeSeq* s = malloc(sizeof(ScopeSeq));
  s->tag = SEQ_STMT;
  s->a = a;
  s->b = b;
  return (ScopeStmt*)s;
}

ScopeStmt* make_ScopeExp (Exp* exp) {
  ScopeExp* s = malloc(sizeof(ScopeExp));
  s->tag = EXP_STMT;
  s->exp = exp;
  return (ScopeStmt*)s;
}

//============================================================
//=================== PRINTING ===============================
//============================================================

void print_exp (Exp* e) {
  switch(e->tag){
  case INT_EXP:{
    IntExp* e2 = (IntExp*)e;
    printf("%d", e2->value);
    break;
  }
  case NULL_EXP:{
    printf("null");
    break;
  }
  case PRINTF_EXP:{
    PrintfExp* e2 = (PrintfExp*)e;
    printf("printf(");
    print_string(e2->format);
    for(int i=0; i<e2->nexps; i++){
      printf(", ");
      print_exp(e2->exps[i]);
    }
    printf(")");
    break;
  }
  case ARRAY_EXP:{
    ArrayExp* e2 = (ArrayExp*)e;
    printf("array(");
    print_exp(e2->length);
    printf(", ");
    print_exp(e2->init);
    printf(")");
    break;
  }
  case OBJECT_EXP:{
    ObjectExp* e2 = (ObjectExp*)e;
    printf("object : (");
    for(int i=0; i<e2->nslots; i++){
      if(i > 0) printf(" ");
      print_slotstmt(e2->slots[i]);
    }
    printf(")");
    break;
  }
  case SLOT_EXP:{
    SlotExp* e2 = (SlotExp*)e;
    print_exp(e2->exp);
    printf(".%s", e2->name);
    break;
  }
  case SET_SLOT_EXP:{
    SetSlotExp* e2 = (SetSlotExp*)e;
    print_exp(e2->exp);
    printf(".%s = ", e2->name);
    print_exp(e2->value);
    break;
  }
  case CALL_SLOT_EXP:{
    CallSlotExp* e2 = (CallSlotExp*)e;
    print_exp(e2->exp);
    printf(".%s(", e2->name);
    for(int i=0; i<e2->nargs; i++){
      if(i > 0) printf(", ");
      print_exp(e2->args[i]);
    }
    printf(")");
    break;
  }
  case CALL_EXP:{
    CallExp* e2 = (CallExp*)e;
    printf("%s(", e2->name);
    for(int i=0; i<e2->nargs; i++){
      if(i > 0) printf(", ");
      print_exp(e2->args[i]);
    }
    printf(")");
    break;
  }
  case SET_EXP:{
    SetExp* e2 = (SetExp*)e;
    printf("%s = ", e2->name);
    print_exp(e2->exp);
    break;
  }
  case IF_EXP:{
    IfExp* e2 = (IfExp*)e;
    printf("if ");
    print_exp(e2->pred);
    printf(" : (");
    print_scopestmt(e2->conseq);
    printf(") else : (");
    print_scopestmt(e2->alt);
    printf(")");
    break;
  }
  case WHILE_EXP:{
    WhileExp* e2 = (WhileExp*)e;
    printf("while ");
    print_exp(e2->pred);
    printf(" : (");
    print_scopestmt(e2->body);
    printf(")");
    break;
  }
  case REF_EXP:{
    RefExp* e2 = (RefExp*)e;
    printf("%s", e2->name);
    break;
  }
  default:
    printf("Unrecognized Expression with tag %d\n", e->tag);
    exit(-1);
  }
}

void print_slotstmt (SlotStmt* s) {
  switch(s->tag){
  case VAR_STMT:{
    SlotVar* s2 = (SlotVar*)s;
    printf("var %s = ", s2->name);
    print_exp(s2->exp);
    break;
  }
  case FN_STMT:{
    SlotMethod* s2 = (SlotMethod*)s;
    printf("method %s (", s2->name);
    for(int i=0; i<s2->nargs; i++){
      if(i>0) printf(", ");
      printf("%s", s2->args[i]);
    }
    printf(") : (");
    print_scopestmt(s2->body);
    printf(")");
    break;
  }
  default:
    printf("Unrecognized slot statement with tag %d\n", s->tag);
    exit(-1);
  }
}

void print_scopestmt (ScopeStmt* s) {
  switch(s->tag){
  case VAR_STMT:{
    ScopeVar* s2 = (ScopeVar*)s;
    printf("var %s = ", s2->name);
    print_exp(s2->exp);
    break;
  }
  case FN_STMT:{
    ScopeFn* s2 = (ScopeFn*)s;
    printf("defn %s (", s2->name);
    for(int i=0; i<s2->nargs; i++){
      if(i>0) printf(", ");
      printf("%s", s2->args[i]);
    }
    printf(") : (");
    print_scopestmt(s2->body);
    printf(")");
    break;
  }
  case SEQ_STMT:{
    ScopeSeq* s2 = (ScopeSeq*)s;
    print_scopestmt(s2->a);
    printf(" ");
    print_scopestmt(s2->b);
    break;
  }
  case EXP_STMT:{
    ScopeExp* s2 = (ScopeExp*)s;
    print_exp(s2->exp);
    break;
  }
  default:
    printf("Unrecognized scope statement with tag %d\n", s->tag);
    exit(-1);
  }
}

//============================================================
//=================== LOADING ================================
//============================================================

static FILE* inputfile;

static char read_byte () {
  int i = fgetc(inputfile);
  if(i < 0) {
    printf("Unexpected end of file.\n");
    exit(-1);
  }
  return (char)i;
}
static int read_int () {
  unsigned char b1 = read_byte();
  unsigned char b2 = read_byte();
  unsigned char b3 = read_byte();
  unsigned char b4 = read_byte();
  return (int)b1 + ((int)b2 << 8) + ((int)b3 << 16) + ((int)b4 << 24);
}
static char* read_string () {
  int len = read_int();
  char* str = malloc(len + 1);
  for(int i=0; i<len; i++)
    str[i] = read_byte();
  str[len] = 0;
  return str;
}

//Lists
static char** read_strings (int n) {
  char** strs = malloc(sizeof(char*)*n);
  for(int i=0; i<n; i++)
    strs[i] = read_string();
  return strs;
}
static Exp** read_exps (int n) {
  Exp** exps = malloc(sizeof(Exp*)*n);
  for(int i=0; i<n; i++)
    exps[i] = read_exp();
  return exps;
}
static SlotStmt** read_slots (int n) {
  SlotStmt** slots = malloc(sizeof(SlotStmt*)*n);
  for(int i=0; i<n; i++)
    slots[i] = read_slot();
  return slots;
}

Exp* read_exp () {
  AstTag tag = read_int();
  switch(tag){
  case INT_EXP:{
    int value = read_int();
    return make_IntExp(value);
  }
  case NULL_EXP:{
    return make_NullExp();
  }
  case PRINTF_EXP:{
    char* format = read_string();
    int nexps = read_int();
    Exp** exps = read_exps(nexps);
    return make_PrintfExp(format, nexps, exps);
  }
  case ARRAY_EXP:{
    Exp* length = read_exp();
    Exp* init = read_exp();
    return make_ArrayExp(length, init);      
  }
  case OBJECT_EXP:{
    Exp* parent = read_exp();
    int nslots = read_int();
    SlotStmt** slots = read_slots(nslots);
    return make_ObjectExp(parent, nslots, slots);
  }
  case SLOT_EXP:{
    char* name = read_string();
    Exp* exp = read_exp();
    return make_SlotExp(name, exp);
  }
  case SET_SLOT_EXP:{
    char* name = read_string();
    Exp* exp = read_exp();
    Exp* value = read_exp();
    return make_SetSlotExp(name, exp, value);
  }
  case CALL_SLOT_EXP:{
    char* name = read_string();
    Exp* exp = read_exp();
    int nargs = read_int();
    Exp** args = read_exps(nargs);
    return make_CallSlotExp(name, exp, nargs, args);
  }
  case CALL_EXP:{
    char* name = read_string();
    int nargs = read_int();
    Exp** args = read_exps(nargs);
    return make_CallExp(name, nargs, args);
  }
  case SET_EXP:{
    char* name = read_string();
    Exp* exp = read_exp();
    return make_SetExp(name, exp);
  }
  case IF_EXP:{
    Exp* pred = read_exp();
    ScopeStmt* conseq = read_scopestmt();
    ScopeStmt* alt = read_scopestmt();
    return make_IfExp(pred, conseq, alt);
  }
  case WHILE_EXP:{
    Exp* pred = read_exp();
    ScopeStmt* body = read_scopestmt();
    return make_WhileExp(pred, body);
  }
  case REF_EXP:{
    char* name = read_string();
    return make_RefExp(name);
  }
  default:{
    printf("Expression with unrecognized tag: %d\n", tag);
    exit(-1);
  }
  }
}

SlotStmt* read_slot () {
  AstTag tag = read_int();
  switch(tag){
  case VAR_STMT:{
    char* name = read_string();
    Exp* exp = read_exp();
    return make_SlotVar(name, exp);
  }
  case FN_STMT:{
    char* name = read_string();
    int nargs = read_int();
    char** args = read_strings(nargs);
    ScopeStmt* body = read_scopestmt();
    return make_SlotMethod(name, nargs, args, body);
  }
  default:{
    printf("Unrecognized slot with tag: %d\n", tag);
    exit(-1);
  }
  }
}

ScopeStmt* read_scopestmt () {
  AstTag tag = read_int();
  switch(tag){
  case VAR_STMT:{
    char* name = read_string();
    Exp* exp = read_exp();
    return make_ScopeVar(name, exp);
  }
  case FN_STMT:{
    char* name = read_string();
    int nargs = read_int();
    char** args = read_strings(nargs);
    ScopeStmt* body = read_scopestmt();
    return make_ScopeFn(name, nargs, args, body);
  }
  case SEQ_STMT:{
    ScopeStmt* a = read_scopestmt();
    ScopeStmt* b = read_scopestmt();
    return make_ScopeSeq(a, b);
  }    
  case EXP_STMT:{
    Exp* e = read_exp();
    return make_ScopeExp(e);
  }
  default:{
    printf("Scope statement with unrecognized tag: %d\n", tag);
    exit(-1);
  }    
  }
  return 0;
}

ScopeStmt* read_ast (char* filename) {
  inputfile = fopen(filename, "r");
  if(!inputfile){
    printf("Could not open file %s\n", filename);
    exit(-1);
  }
  ScopeStmt* s = read_scopestmt();  
  fclose(inputfile);
  return s;
}


//============================================================
//================ INTERPRETER OBJECTS =======================
//============================================================

typedef enum {
  NULL_OBJ,
  INT_OBJ,
  ARRAY_OBJ,
  ENV_OBJ
} ObjTag;

typedef enum {
  VAR_ENTRY,
  CODE_ENTRY
} EntryTag;

typedef struct {
  ObjTag tag;
} EvalObj;

typedef struct {
  ObjTag tag;
  int value;
} IntObj;

typedef struct {
  ObjTag tag;
  int length;
  EvalObj** items;
} ArrayObj;

typedef struct {
  ObjTag tag;
  EvalObj* parent;
  Vector* entries;
} EnvObj;

typedef struct {
  EntryTag tag;
  char* name;
  union{
    EvalObj* value;
    struct {
      int nargs;
      char** args;
      ScopeStmt* body;
    };
  };
} EnvEntry;

EnvEntry* make_VarEntry (char* name, EvalObj* obj) {
  EnvEntry* e = malloc(sizeof(EnvEntry));
  e->tag = VAR_ENTRY;
  e->name = name;
  e->value = obj;
  return e;
}

EnvEntry* make_CodeEntry (char* name, int nargs, char** args, ScopeStmt* body) {
  EnvEntry* e = malloc(sizeof(EnvEntry));
  e->tag = CODE_ENTRY;
  e->name = name;
  e->nargs = nargs;
  e->args = args;
  e->body = body;
  return e;
}

EvalObj* make_NullObj () {
  EvalObj* o = malloc(sizeof(EvalObj));
  o->tag = NULL_OBJ;
  return o;
}

IntObj* make_IntObj (int value) {
  IntObj* e = malloc(sizeof(IntObj));
  e->tag = INT_OBJ;
  e->value = value;
  return e;
}

ArrayObj* make_ArrayObj (int length, EvalObj* x) {
  ArrayObj* o = malloc(sizeof(ArrayObj));
  o->tag = ARRAY_OBJ;
  o->length = length;
  o->items = malloc(sizeof(EvalObj*) * length);
  for(int i=0; i<length; i++)
    o->items[i] = x;
  return o;
}

EnvObj* make_Env (EvalObj* parent) {
  EnvObj* e = malloc(sizeof(EnvObj));
  e->tag = ENV_OBJ;
  e->parent = parent;
  e->entries = make_vector();
  return e;
}

//============================================================
//================= PRINTER ==================================
//============================================================

static void print_obj (EvalObj* o) {
  switch(o->tag){
  case NULL_OBJ:{
    printf("null");
    break;
  }
  case INT_OBJ:{
    IntObj* o2 = (IntObj*)o;
    printf("%d", o2->value);
    break;
  }
  case ARRAY_OBJ:{
    ArrayObj* o2 = (ArrayObj*)o;
    printf("[");
    for(int i=0; i<o2->length; i++){
      if(i>0) printf(" ");
      print_obj(o2->items[i]);
    }
    printf("]");
    break;
  }
  case ENV_OBJ:{
    printf("[Object]");
    break;
  }
  default:{
    printf("Object with unknown tag: %d\n", o->tag);
    exit(-1);
  }
  }
}

//============================================================
//================= ENVIRONMENTS =============================
//============================================================

EnvEntry* lookup_frame (Vector* entries, char* name) {
  for(int i=0; i<entries->size; i++){
    EnvEntry* e = vector_get(entries, i);
    if(strcmp(e->name, name) == 0)
      return e;
  }
  return 0;
}

EnvEntry* lookup_env (EnvObj* env, char* name) {
  if(env->tag == ENV_OBJ){
    EnvEntry* e = lookup_frame(env->entries, name);
    if(e) return e;
    return lookup_env((EnvObj*)env->parent, name);
  }
  else{
    printf("No entry for %s\n", name);
    exit(-1);
  }
}

EnvEntry* lookup_fn (EnvObj* env, char* name) {
  EnvEntry* e = lookup_env(env, name);
  if(e->tag != CODE_ENTRY){
    printf("Entry %s is not a function.\n", name);
    exit(-1);
  }    
  return e;
}

EnvEntry* lookup_var (EnvObj* env, char* name) {
  EnvEntry* e = lookup_env(env, name);
  if(e->tag != VAR_ENTRY){
    printf("Entry %s is not a variable.\n", name);
    exit(-1);
  }    
  return e;
}

//============================================================
//================== INTERPRETER =============================
//============================================================

// Interpreter Values
EvalObj* nullobj;
EvalObj* zeroobj;

// Interpreter Routines
EvalObj* eval_stmt (EnvObj* genv, EnvObj* env, ScopeStmt* stmt);  
EvalObj* eval_exp (EnvObj* genv, EnvObj* env, Exp* exp);

static void init_eval () {
  nullobj = (EvalObj*)make_NullObj();
  zeroobj = (EvalObj*)make_IntObj(0);
}

static void ensure_nargs (int nargs, int desired) {
  if(nargs != desired){
    printf("Incorrect number of arguments. Expected %d but got %d\n", desired, nargs);
    exit(-1);
  }
}

static void ensure_int (IntObj* o) {
  if(o->tag != INT_OBJ){
    printf("Not an integer.\n");
    exit(-1);
  }
}

static void ensure_index (ArrayObj* o, IntObj* i) {
  ensure_int(i);
  if(i->value < 0 || i->value >= o->length){
    printf("Index %d out of bounds.\n", i->value);
    exit(-1);
  }
}

static void ensure_parent (EvalObj* o) {
  if(o->tag != NULL_OBJ && o->tag != ENV_OBJ){
    print_obj(o);
    printf(" is not a legal parent.\n");
    exit(-1);
  }
}

static void ensure_env (EnvObj* o) {
  if(o->tag != ENV_OBJ){
    print_obj((EvalObj*)o);
    printf(" is not an object.\n");
    exit(-1);
  }
}

static EvalObj* bool_obj (int b) {
  if(b) return zeroobj;
  return nullobj;
}

static EvalObj* call_int_slot (EnvObj* genv, EnvObj* env, IntObj* obj, char* name, int nargs, Exp** args) {
  ensure_nargs(nargs, 1);
  IntObj* o = (IntObj*)eval_exp(genv, env, args[0]);
  ensure_int(o);
  if(strcmp(name, "eq") == 0)
    return bool_obj(obj->value == o->value);
  else if(strcmp(name, "lt") == 0)
    return bool_obj(obj->value < o->value);
  else if(strcmp(name, "le") == 0)
    return bool_obj(obj->value <= o->value);
  else if(strcmp(name, "gt") == 0)
    return bool_obj(obj->value > o->value);
  else if(strcmp(name, "ge") == 0)
    return bool_obj(obj->value >= o->value);
  else if(strcmp(name, "add") == 0)
    return (EvalObj*)make_IntObj(obj->value + o->value);
  else if(strcmp(name, "sub") == 0)
    return (EvalObj*)make_IntObj(obj->value - o->value);
  else if(strcmp(name, "mul") == 0)
    return (EvalObj*)make_IntObj(obj->value * o->value);
  else if(strcmp(name, "div") == 0)
    return (EvalObj*)make_IntObj(obj->value / o->value);
  else if(strcmp(name, "mod") == 0)
    return (EvalObj*)make_IntObj(obj->value % o->value);
  
  printf("No slot named %s for Ints.\n", name);
  exit(-1);
}

static EvalObj* call_array_slot (EnvObj* genv, EnvObj* env, ArrayObj* obj, char* name, int nargs, Exp** args) {
  if(strcmp(name, "get") == 0){
    ensure_nargs(nargs, 1);
    IntObj* o = (IntObj*)eval_exp(genv, env, args[0]);
    ensure_index(obj, o);
    return obj->items[o->value];
  }
  else if(strcmp(name, "set") == 0){
    ensure_nargs(nargs, 2);
    IntObj* i = (IntObj*)eval_exp(genv, env, args[0]);
    EvalObj* x = eval_exp(genv, env, args[1]);
    ensure_index(obj, i);
    obj->items[i->value] = x;
    return nullobj;
  }
  else if(strcmp(name, "length") == 0){
    ensure_nargs(nargs, 0);
    return (EvalObj*)make_IntObj(obj->length);
  }
  
  printf("No slot named %s for Array.\n", name);
  exit(-1);
}

EvalObj* call_slot (EnvObj* genv, EnvObj* env, EvalObj* obj, char* name, int nargs, Exp** args) {  
  if(obj->tag == NULL_OBJ){
    printf("No slot named %s for null.\n", name);
    exit(-1);
  }
  else if(obj->tag == INT_OBJ){
    return call_int_slot(genv, env, (IntObj*)obj, name, nargs, args);
  }
  else if(obj->tag == ARRAY_OBJ){
    return call_array_slot(genv, env, (ArrayObj*)obj, name, nargs, args);
  }
  else if(obj->tag == ENV_OBJ){
    EnvEntry* m = lookup_fn((EnvObj*)obj, name);
    ensure_nargs(nargs, m->nargs);
    EnvObj* fenv = make_Env((EvalObj*)genv);
    vector_add(fenv->entries, make_VarEntry("this", obj));
    for(int i=0; i<m->nargs; i++){
      EnvEntry* e = make_VarEntry(m->args[i], eval_exp(genv, env, args[i]));
      vector_add(fenv->entries, e);
    }
    return eval_stmt(genv, fenv, m->body);
  }
  else{
    printf("Unknown object with tag: %d\n", obj->tag);
    exit(-1);
  }
}

static void print_format (char* fmt, int nvs, EvalObj** vs) {  
  int i = 0;
  while(1){
    char c = fmt[0];
    if(c == 0) {
      if(i != nvs){
        printf("Incorrect number of arguments to printf statement.\n");
        exit(-1);
      }
      return;
    }else if(c == '~'){
      if(i >= nvs){
        printf("Incorrect number of arguments to printf statement.\n");
        exit(-1);
      }
      print_obj(vs[i]);
      i++;
    }else{
      printf("%c", c);
    }
    fmt++;
  }
}

EvalObj* eval_exp (EnvObj* genv, EnvObj* env, Exp* exp) {  
  switch(exp->tag){
  case INT_EXP:{
    IntExp* e = (IntExp*)exp;
    return (EvalObj*)make_IntObj(e->value);
  }
    
  case NULL_EXP:{
    return nullobj;
  }
    
  case PRINTF_EXP:{
    PrintfExp* e = (PrintfExp*)exp;
    EvalObj** vs = malloc(sizeof(EvalObj*) * e->nexps);
    for(int i=0; i<e->nexps; i++)
      vs[i] = eval_exp(genv, env, e->exps[i]);
    print_format(e->format, e->nexps, vs);
    return nullobj;
  }
    
  case ARRAY_EXP:{
    ArrayExp* e = (ArrayExp*)exp;
    IntObj* n = (IntObj*)eval_exp(genv, env, e->length);    
    EvalObj* x = eval_exp(genv, env, e->init);
    ensure_int(n);
    return (EvalObj*)make_ArrayObj(n->value, x);
  }
    
  case OBJECT_EXP:{
    ObjectExp* e = (ObjectExp*)exp;
    EvalObj* p = eval_exp(genv, env, e->parent);
    ensure_parent(p);
    EnvObj* o = make_Env(p);
    for(int i=0; i<e->nslots; i++){
      SlotStmt* s = e->slots[i];
      if(s->tag == VAR_STMT){
        SlotVar* s2 = (SlotVar*)s;
        EnvEntry* v = make_VarEntry(s2->name, eval_exp(genv, env, s2->exp));
        vector_add(o->entries, v);
      }else if(s->tag == FN_STMT){
        SlotMethod* s2 = (SlotMethod*)s;
        EnvEntry* m = make_CodeEntry(s2->name, s2->nargs, s2->args, s2->body);
        vector_add(o->entries, m);
      }else{
        printf("Unrecognized slot type: %d\n", s->tag);
        exit(-1);
      }
    }
    return (EvalObj*)o;
  }
    
  case SLOT_EXP:{
    SlotExp* e = (SlotExp*)exp;
    EnvObj* o = (EnvObj*)eval_exp(genv, env, e->exp);
    ensure_env(o);
    EnvEntry* v = lookup_var(o, e->name);
    return v->value;
  }
    
  case SET_SLOT_EXP:{
    SetSlotExp* e = (SetSlotExp*)exp;
    EnvObj* o = (EnvObj*)eval_exp(genv, env, e->exp);
    ensure_env(o);
    EvalObj* v = eval_exp(genv, env, e->value);
    EnvEntry* var = lookup_var(o, e->name);
    var->value = v;
    return nullobj;
  }
    
  case CALL_SLOT_EXP:{
    CallSlotExp* e = (CallSlotExp*)exp;
    EvalObj* o = eval_exp(genv, env, e->exp);
    return call_slot(genv, env, o, e->name, e->nargs, e->args);
  }
    
  case CALL_EXP:{
    CallExp* e = (CallExp*)exp;
    EnvEntry* f = lookup_fn(env, e->name);
    ensure_nargs(e->nargs, f->nargs);
    EnvObj* fenv = make_Env((EvalObj*)genv);
    for(int i=0; i<e->nargs; i++){
      EnvEntry* a = make_VarEntry(f->args[i], eval_exp(genv, env, e->args[i]));
      vector_add(fenv->entries, a);
    }
    return eval_stmt(genv, fenv, f->body);
  }
    
  case SET_EXP:{
    SetExp* e = (SetExp*)exp;
    EvalObj* v = eval_exp(genv, env, e->exp);
    EnvEntry* var = lookup_var(env, e->name);
    var->value = v;
    return nullobj;
  }
    
  case IF_EXP:{
    IfExp* e = (IfExp*)exp;
    EvalObj* p = eval_exp(genv, env, e->pred);
    EnvObj* benv = make_Env((EvalObj*)env);
    if(p->tag != NULL_OBJ)
      return eval_stmt(genv, benv, e->conseq);
    else
      return eval_stmt(genv, benv, e->alt);
  }
    
  case WHILE_EXP:{
    WhileExp* e = (WhileExp*)exp;
    while(1){
      EvalObj* p = eval_exp(genv, env, e->pred);
      if(p->tag == NULL_OBJ)
        break;
      EnvObj* benv = make_Env((EvalObj*)env);
      eval_stmt(genv, benv, e->body);
    }
    return nullobj;
  }

  case REF_EXP:{
    RefExp* e = (RefExp*)exp;
    EnvEntry* v = lookup_var(env, e->name);
    return v->value;
  }
    
  default:{
    printf("Unrecognized expression with tag: %d\n", exp->tag);
    exit(-1);
  }    
  }
}

EvalObj* eval_stmt (EnvObj* genv, EnvObj* env, ScopeStmt* stmt) {
  switch(stmt->tag){
  case VAR_STMT:{
    ScopeVar* s = (ScopeVar*)stmt;
    EvalObj* val = eval_exp(genv, env, s->exp);
    EnvEntry* v = make_VarEntry(s->name, val);
    vector_add(env->entries, v);
    return nullobj;
  }
  case FN_STMT:{
    ScopeFn* s = (ScopeFn*)stmt;
    EnvEntry* f = make_CodeEntry(s->name, s->nargs, s->args, s->body);
    vector_add(env->entries, f);
    return nullobj;
  }
  case SEQ_STMT:{
    ScopeSeq* s = (ScopeSeq*)stmt;
    eval_stmt(genv, env, s->a);
    return eval_stmt(genv, env, s->b);
  }
  case EXP_STMT:{
    ScopeExp* s = (ScopeExp*)stmt;
    return eval_exp(genv, env, s->exp);
  }
  default:
    printf("Unrecognized Statement with tag: %d\n", stmt->tag);
    exit(-1);
  }
}

//============================================================
//================= DRIVER ===================================
//============================================================

void interpret (ScopeStmt* stmt) {
  init_eval();
  EnvObj* genv = make_Env(nullobj);
  eval_stmt(genv, genv, stmt);  
}

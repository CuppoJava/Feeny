#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "utils.h"
#include "bytecode.h"

//============================================================
//==================== FILE READING ==========================
//============================================================

Vector* read_slots ();
Vector* read_values ();
Vector* read_code ();

static FILE* inputfile;

static char read_byte () {
  int i = fgetc(inputfile);
  if(i < 0) {
    printf("Unexpected end of file.\n");
    exit(-1);
  }
  return (char)i;
}
static int read_short () {
  unsigned char b1 = read_byte();
  unsigned char b2 = read_byte();
  return b1 + (b2 << 8);
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

#define RETURN_NEW_INS0()                   \
{                                           \
   ByteIns* o = malloc(sizeof(ByteIns));    \
   o->tag = op;                             \
   return o;                                \
}
#define RETURN_NEW_INS1(TAG, x, xv) \
{                                   \
   TAG* o = malloc(sizeof(TAG));    \
   o->tag = op;                     \
   o->x = xv;                       \
   return (ByteIns*)o;              \
}
#define RETURN_NEW_INS2(TAG, x, xv, y, yv) \
{                                          \
   TAG* o = malloc(sizeof(TAG));           \
   o->tag = op;                            \
   o->x = xv;                              \
   o->y = yv;                              \
   return (ByteIns*)o;                     \
}

ByteIns* read_ins () {
  char op = read_byte();
  switch(op){
  case LABEL_OP:
    RETURN_NEW_INS1(LabelIns,
                    name, read_short());
  case LIT_OP:
    RETURN_NEW_INS1(LitIns,
                    idx, read_short());
  case PRINTF_OP:
    RETURN_NEW_INS2(PrintfIns,
                    format, read_short(),
                    arity, read_byte());
  case ARRAY_OP:
    RETURN_NEW_INS0();
  case OBJECT_OP:
    RETURN_NEW_INS1(ObjectIns,
                    class, read_short());
  case SLOT_OP:
    RETURN_NEW_INS1(SlotIns,
                    name, read_short());
  case SET_SLOT_OP:
    RETURN_NEW_INS1(SetSlotIns,
                    name, read_short());
  case CALL_SLOT_OP:
    RETURN_NEW_INS2(CallSlotIns,
                    name, read_short(),
                    arity, read_byte());
  case CALL_OP:
    RETURN_NEW_INS2(CallIns,
                    name, read_short(),
                    arity, read_byte());
  case SET_LOCAL_OP:
    RETURN_NEW_INS1(SetLocalIns,
                    idx, read_short());
  case GET_LOCAL_OP:
    RETURN_NEW_INS1(GetLocalIns,
                    idx, read_short());
  case SET_GLOBAL_OP:
    RETURN_NEW_INS1(SetGlobalIns,
                    name, read_short());
  case GET_GLOBAL_OP:
    RETURN_NEW_INS1(GetGlobalIns,
                    name, read_short());
  case BRANCH_OP:
    RETURN_NEW_INS1(BranchIns,
                    name, read_short());
  case GOTO_OP:
    RETURN_NEW_INS1(GotoIns,
                    name, read_short());
  case RETURN_OP:
    RETURN_NEW_INS0();
  case DROP_OP:
    RETURN_NEW_INS0();
  default:
    printf("Unrecognized Opcode: %d\n", op);
    exit(-1);
  }
}

#define RETURN_NEW_VAL0() \
{\
   Value* o = malloc(sizeof(Value));\
   o->tag = tag;\
   return o;\
}
#define RETURN_NEW_VAL1(TYPE, x, xv) \
{\
   TYPE* o = malloc(sizeof(TYPE));\
   o->tag = tag;\
   o->x = xv;\
   return (Value*)o;\
}
#define RETURN_NEW_VAL4(TYPE, w, wv, x, xv, y, yv, z, zv) \
{\
   TYPE* o = malloc(sizeof(TYPE));\
   o->tag = tag;\
   o->w = wv;\
   o->x = xv;\
   o->y = yv;\
   o->z = zv;\
   return (Value*)o;\
}
Value* read_value () {
  char tag = read_byte();
  switch(tag){
  case INT_VAL:
    RETURN_NEW_VAL1(IntValue,
                    value, read_int());
  case NULL_VAL:
    RETURN_NEW_VAL0();
  case STRING_VAL:
    RETURN_NEW_VAL1(StringValue,
                    value, read_string());
  case METHOD_VAL:
    RETURN_NEW_VAL4(MethodValue,
                    name, read_short(),
                    nargs, read_byte(),
                    nlocals, read_short(),
                    code, read_code());
  case SLOT_VAL:
    RETURN_NEW_VAL1(SlotValue,
                    name, read_short());
  case CLASS_VAL:
    RETURN_NEW_VAL1(ClassValue,
                    slots, read_slots());
  default:
    printf("Unrecognized value tag: %d\n", tag);
    exit(-1);
  }
}

Vector* read_slots () {
  Vector* v = make_vector();
  int n = read_short();
  for(int i=0; i<n; i++)
    vector_add(v, (void*)read_short());
  return v;
}

Vector* read_values () {
  Vector* v = make_vector();
  int n = read_short();
  for(int i=0; i<n; i++)
    vector_add(v, read_value());
  return v;
}

Vector* read_code () {
  Vector* v = make_vector();
  int n = read_int();
  for(int i=0; i<n; i++)
    vector_add(v, read_ins());
  return v;
}

Program* read_program () {
  Program* p = malloc(sizeof(Program));
  p->values = read_values();
  p->slots = read_slots();
  p->entry = read_short();
  return p;
}

Program* load_bytecode (char* filename) {
  inputfile = fopen(filename, "r");
  if(!inputfile){
    printf("Could not read file %s.\n", filename);
    exit(-1);
  }
  Program* p = read_program();
  fclose(inputfile);
  return p;
}

//============================================================
//===================== PRINTING =============================
//============================================================

void print_value (Value* v);

void print_value (Value* v) {
  switch(v->tag){
  case INT_VAL:{
    IntValue* v2 = (IntValue*)v;
    printf("Int(%d)", v2->value);
    break;
  }
  case NULL_VAL:{
    printf("Null");
    break;
  }
  case STRING_VAL:{
    StringValue* v2 = (StringValue*)v;
    printf("String(");
    print_string(v2->value);
    printf(")");
    break;
  }
  case METHOD_VAL:{
    MethodValue* v2 = (MethodValue*)v;
    printf("Method(#%d, nargs:%d, nlocals:%d) :", v2->name, v2->nargs, v2->nlocals);
    for(int i=0; i<v2->code->size; i++){
      printf("\n      ");
      print_ins(vector_get(v2->code, i));
    }
    break;
  }
  case SLOT_VAL:{
    SlotValue* v2 = (SlotValue*)v;
    printf("Slot(#%d)", v2->name);
    break;
  }
  case CLASS_VAL:{
    ClassValue* v2 = (ClassValue*)v;
    printf("Class(");
    for(int i=0; i<v2->slots->size; i++){
      if(i > 0) printf(", ");
      printf("#%d", (int)vector_get(v2->slots,i));
    }
    printf(")");
    break;
  }
  default:
    printf("Value with unknown tag: %d\n", v->tag);
    exit(-1);
  }
}

void print_ins (ByteIns* ins) {
  switch(ins->tag){
  case LABEL_OP:{
    LabelIns* i = (LabelIns*)ins;
    printf("label #%d", i->name);
    break;
  }
  case LIT_OP:{
    LitIns* i = (LitIns*)ins;
    printf("   lit #%d", i->idx);
    break;
  }
  case PRINTF_OP:{
    PrintfIns* i = (PrintfIns*)ins;
    printf("   printf #%d %d", i->format, i->arity);
    break;
  }
  case ARRAY_OP:{
    printf("   array");
    break;
  }
  case OBJECT_OP:{
    ObjectIns* i = (ObjectIns*)ins;
    printf("   object #%d", i->class);
    break;
  }
  case SLOT_OP:{
    SlotIns* i = (SlotIns*)ins;
    printf("   slot #%d", i->name);
    break;
  }
  case SET_SLOT_OP:{
    SetSlotIns* i = (SetSlotIns*)ins;
    printf("   set-slot #%d", i->name);
    break;
  }
  case CALL_SLOT_OP:{
    CallSlotIns* i = (CallSlotIns*)ins;
    printf("   call-slot #%d %d", i->name, i->arity);
    break;
  }
  case CALL_OP:{
    CallIns* i = (CallIns*)ins;
    printf("   call #%d %d", i->name, i->arity);
    break;
  }
  case SET_LOCAL_OP:{
    SetLocalIns* i = (SetLocalIns*)ins;
    printf("   set local %d", i->idx);
    break;
  }
  case GET_LOCAL_OP:{
    GetLocalIns* i = (GetLocalIns*)ins;
    printf("   get local %d", i->idx);
    break;
  }
  case SET_GLOBAL_OP:{
    SetGlobalIns* i = (SetGlobalIns*)ins;
    printf("   set global #%d", i->name);
    break;
  }
  case GET_GLOBAL_OP:{
    GetGlobalIns* i = (GetGlobalIns*)ins;
    printf("   get global #%d", i->name);
    break;
  }
  case BRANCH_OP:{
    BranchIns* i = (BranchIns*)ins;
    printf("   branch #%d", i->name);
    break;
  }
  case GOTO_OP:{
    GotoIns* i = (GotoIns*)ins;
    printf("   goto #%d", i->name);
    break;
  }
  case RETURN_OP:{
    printf("   return");
    break;
  }
  case DROP_OP:{
    printf("   drop");
    break;
  }
  }
}

void print_prog (Program* p) {
  printf("Constants :");
  Vector* vs = p->values;
  for(int i=0; i<vs->size; i++){
    printf("\n   #%d: ", i);
    print_value(vector_get(vs, i));
  }
  printf("\nGlobals :");
  for(int i=0; i<p->slots->size; i++)
    printf("\n   #%d", (int)vector_get(p->slots, i));
  printf("\nEntry : #%d", p->entry);
}

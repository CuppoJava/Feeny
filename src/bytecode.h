#ifndef BYTECODE_H
#define BYTECODE_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "utils.h"

typedef enum {
  INT_VAL,
  NULL_VAL,
  STRING_VAL,
  METHOD_VAL,
  SLOT_VAL,
  CLASS_VAL
} ValTag;

typedef enum {
  LABEL_OP,
  LIT_OP,
  PRINTF_OP,
  ARRAY_OP,
  OBJECT_OP,
  SLOT_OP,
  SET_SLOT_OP,
  CALL_SLOT_OP,
  CALL_OP,
  SET_LOCAL_OP,
  GET_LOCAL_OP,
  SET_GLOBAL_OP,
  GET_GLOBAL_OP,
  BRANCH_OP,
  GOTO_OP,
  RETURN_OP,
  DROP_OP
} OpCode;

typedef struct {
  ValTag tag;
} Value;

typedef struct {
  ValTag tag;
  int value;
} IntValue;

typedef struct {
  ValTag tag;
  char* value;
} StringValue;

typedef struct {
  ValTag tag;
  int name;
  int nargs;
  int nlocals;
  Vector* code;
} MethodValue;

typedef struct {
  ValTag tag;
  int name;
} SlotValue;

typedef struct {
  ValTag tag;
  Vector* slots;
} ClassValue;

typedef struct {
  OpCode tag;  
} ByteIns;

typedef struct {
  OpCode tag;
  int name;
} LabelIns;

typedef struct {
  OpCode tag;
  int idx;
} LitIns;

typedef struct {
  OpCode tag;
  int format;
  int arity;
} PrintfIns;

typedef struct {
  OpCode tag;
  int class;
} ObjectIns;

typedef struct {
  OpCode tag;
  int name;
} SlotIns;

typedef struct {
  OpCode tag;
  int name;
} SetSlotIns;

typedef struct {
  OpCode tag;
  int name;
  int arity;
} CallSlotIns;

typedef struct {
  OpCode tag;
  int name;
  int arity;
} CallIns;

typedef struct {
  OpCode tag;
  int idx;
} SetLocalIns;

typedef struct {
  OpCode tag;
  int idx;
} GetLocalIns;

typedef struct {
  OpCode tag;
  int name;
} SetGlobalIns;

typedef struct {
  OpCode tag;
  int name;
} GetGlobalIns;

typedef struct {
  OpCode tag;
  int name;
} BranchIns;

typedef struct {
  OpCode tag;
  int name;
} GotoIns;

typedef struct {
  Vector* values;
  Vector* slots;
  int entry;
} Program;

Program* load_bytecode (char* filename);
void print_ins (ByteIns* ins);
void print_prog (Program* p);

#endif



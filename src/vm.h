#ifndef VM_H
#define VM_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "utils.h"
#include "bytecode.h"

typedef enum {
  INT_INS,        //0
  NULL_INS,       //1
  PRINTF_INS,     //2
  ARRAY_INS,      //3
  OBJECT_INS,     //4
  SLOT_INS,       //5
  SET_SLOT_INS,   //6
  CALL_SLOT_INS,  //7
  CALL_INS,       //8
  SET_LOCAL_INS,  //9
  GET_LOCAL_INS,  //a
  SET_GLOBAL_INS, //b
  GET_GLOBAL_INS, //c
  BRANCH_INS,     //d
  GOTO_INS,       //e
  RETURN_INS,     //f
  DROP_INS,       //10
  FRAME_INS       //11
} OpTag;

//Int :
//   tag: char
//   value: int
//Printf :
//   tag: char
//   arity: char
//   format: char*
//Object :
//   tag: char
//   arity: char
//   class: short
//Slot :
//   tag: char
//   name: char*
//SetSlot :
//   tag: char
//   name: char*
//CallSlot :
//   tag: char
//   arity: char
//   name: char*
//Call :
//   tag: char
//   arity: char
//   code: void*
//SetLocal :
//   tag: char
//   idx: short
//GetLocal :
//   tag: char
//   idx: short
//SetGlobal :
//   tag: char
//   idx: short
//GetGlobal :
//   tag: char
//   idx: short
//Branch :
//   tag: char
//   code: void*
//Goto :
//   tag: char
//   code: void*
//Frame :
//   tag: char
//   nargs: char
//   nlocals: short

typedef enum {
  VAR_SLOT,
  CODE_SLOT
} SlotTag;

typedef struct {
  SlotTag tag;
  char* name;
  union {
    int idx;
    void* code;
  };
} LSlot;

typedef struct {
  int nvars;
  int nslots;
  LSlot* slots;
} LClass;

char* link_program (Program* prog);
void initvm (char* entry);
void runvm ();

#endif

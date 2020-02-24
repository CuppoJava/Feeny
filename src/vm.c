#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "utils.h"
#include "bytecode.h"
#include "vm.h"

//============================================================
//===================== LINKER ===============================
//============================================================

//======== CODE BUFFER ===============
char* code;
char* codep;
int code_cap;

void init_codebuffer () {
  code_cap = 1024 * 1024;
  code = malloc(code_cap);
  codep = code;
}

void ensure_code_space () {
  int code_size = codep - code;
  if(code_size + 2*sizeof(long) > code_cap){
    int new_cap = code_size * 2 + 2*sizeof(long);
    char* buf = malloc(new_cap);
    memcpy(buf, code, code_size);
    free(code);
    code = buf;
    code_cap = new_cap;
    codep = code + code_size;
  }
}

//=========== WRITER =============
void align_short () {
  codep = (char*)(((long)codep + 1)&(-2));
}
void align_int () {
  codep = (char*)(((long)codep + 3)&(-4));
}
void align_ptr () {
  codep = (char*)(((long)codep + 7)&(-8));
}

void write_char (char c) {
  ensure_code_space();
  codep[0] = c;
  codep++;
}

void write_short (short s) {
  ensure_code_space();
  align_short();
  ((short*)codep)[0] = s;
  codep += sizeof(short);
}

void write_int (int i) {
  ensure_code_space();
  align_int();
  ((int*)codep)[0] = i;
  codep += sizeof(int);
}

void write_ptr (void* ptr) {
  ensure_code_space();
  align_ptr();
  ((void**)codep)[0] = ptr;
  codep += sizeof(void*);
}

void write_frame (MethodValue* v) {
  ensure_code_space();
  write_char(FRAME_INS);
  write_char(v->nargs);
  write_short(v->nlocals);
}

//======= PATCH BUFFER ===============
typedef enum {
  LABEL_PATCH,
  FUNCTION_PATCH,
  CLASS_TAG_PATCH,
  CLASS_ARITY_PATCH,
  GLOBAL_IDX_PATCH,
} PatchType;

typedef struct {
  PatchType type;
  int pos;
  union{
    char* name;
    int class;
  };
} Patch;

Vector* patches;

void init_patchbuffer () {
  patches = make_vector();
}

void write_class_arity (int class) {
  Patch* p = malloc(sizeof(Patch));
  p->type = CLASS_ARITY_PATCH;
  p->pos = codep - code;
  p->class = class;
  vector_add(patches, p);
  write_char(0);
}

void write_class_tag (int class) {
  align_short();
  Patch* p = malloc(sizeof(Patch));
  p->type = CLASS_TAG_PATCH;
  p->pos = codep - code;
  p->class = class;
  vector_add(patches, p);
  write_short(0);
}

void write_function_ptr (char* name) {
  align_ptr();
  Patch* p = malloc(sizeof(Patch));
  p->type = FUNCTION_PATCH;
  p->pos = codep - code;
  p->name = name;
  vector_add(patches, p);  
  write_ptr(0);
}

void write_global_idx (char* name) {
  align_short();
  Patch* p = malloc(sizeof(Patch));
  p->type = GLOBAL_IDX_PATCH;
  p->pos = codep - code;
  p->name = name;
  vector_add(patches, p);
  write_short(0);
}

void write_label (char* name) {
  align_ptr();
  Patch* p = malloc(sizeof(Patch));
  p->type = LABEL_PATCH;
  p->pos = codep - code;
  p->name = name;
  vector_add(patches, p);
  write_ptr(0);
}

//======= TABLE BUFFER ===============
typedef enum {
  LABEL_ENTRY,
  METHOD_ENTRY,
  FUNCTION_ENTRY,
  CLASS_ENTRY
} EntryType;

typedef struct {
  EntryType type;
  union {
    char* name;
    int idx;
  };
  union {
    int codepos;
    int class;
  };
} TableEntry;

Vector* entries;

void init_tablebuffer () {
  entries = make_vector();
}

void set_label (char* name) {
  TableEntry* e = malloc(sizeof(TableEntry));
  e->type = LABEL_ENTRY;
  e->name = name;
  e->codepos = codep - code;
  vector_add(entries, e);
}

void* get_label (char* name) {
  for(int i=0; i<entries->size; i++){
    TableEntry* e = vector_get(entries, i);
    if(e->type == LABEL_ENTRY)
      if(strcmp(e->name, name) == 0)
        return code + e->codepos;
  }
  printf("No label with name %s.\n", name);
  exit(-1);
}

void set_method_label (int idx) {
  TableEntry* e = malloc(sizeof(TableEntry));
  e->type = METHOD_ENTRY;
  e->idx = idx;
  e->codepos = codep - code;
  vector_add(entries, e);
}

int get_method_pos (int idx) {
  for(int i=0; i<entries->size; i++){
    TableEntry* e = vector_get(entries, i);
    if(e->type == METHOD_ENTRY)
      if(e->idx == idx)
        return e->codepos;
  }
  printf("No method with index %d.\n", idx);
  exit(-1);
}

void* get_method_label (int idx) {
  return code + get_method_pos(idx);
}

void set_function_label (char* name, int idx) {
  TableEntry* e = malloc(sizeof(TableEntry));
  e->type = FUNCTION_ENTRY;
  e->name = name;
  e->codepos = get_method_pos(idx);
  vector_add(entries, e);
}

void* get_function_label (char* name) {
  for(int i=0; i<entries->size; i++){
    TableEntry* e = vector_get(entries, i);
    if(e->type == FUNCTION_ENTRY)
      if(strcmp(e->name, name) == 0)
        return code + e->codepos;    
  }
  printf("No function with name %s.\n", name);
  exit(-1);
}

void set_class_tag (int idx, int class) {
  TableEntry* e = malloc(sizeof(TableEntry));
  e->type = CLASS_ENTRY;
  e->idx = idx;
  e->class = class;
  vector_add(entries, e);
}

int get_class_tag (int idx) {
  for(int i=0; i<entries->size; i++){
    TableEntry* e = vector_get(entries, i);
    if(e->type == CLASS_ENTRY)
      if(e->idx == idx)
        return e->class;
  }
  printf("No class with index %d.\n", idx);
  exit(-1);
}

//========== GLOBALS ===========
Vector* globals;

void init_globals () {
  globals = make_vector();
}

void new_global (char* name) {
  vector_add(globals, name);
}

int get_global_idx (char* name) {
  for(int i=0; i<globals->size; i++){
    char* gname = vector_get(globals, i);
    if(strcmp(gname, name) == 0)
      return i;
  }
  printf("No global with name %s.\n", name);
  exit(-1);
}

//========== LINKER =============
char* link_str (Vector* values, int idx) {
  StringValue* v = vector_get(values, idx);
  return v->value;
}

void print_code_buffer () {
  for(long* cp = (long*)code; cp < (long*)(codep + 8); cp += 1){
    printf("0x%lx: %016lx\n", (long)cp, cp[0]);
  }
}

void link_ins (Vector* values, ByteIns* ins) {  
  switch(ins->tag){
  case LABEL_OP:{
    LabelIns* ins2 = (LabelIns*)ins;
    set_label(link_str(values, ins2->name));
    break;          
  }
  case LIT_OP:{
    LitIns* ins2 = (LitIns*)ins;
    Value* v = vector_get(values, ins2->idx);
    if(v->tag == INT_VAL){
      write_char(INT_INS);
      write_int(((IntValue*)v)->value);
    }else if(v->tag == NULL_VAL){
      write_char(NULL_INS);
    }else{
      printf("Unrecognized Literal: %d\n", v->tag);
      exit(-1);
    }
    break;
  }
  case PRINTF_OP:{
    PrintfIns* ins2 = (PrintfIns*)ins;
    write_char(PRINTF_INS);
    write_char(ins2->arity);
    write_ptr(link_str(values, ins2->format));
    break;
  }
  case ARRAY_OP:{
    write_char(ARRAY_INS);
    break;
  }
  case OBJECT_OP:{
    ObjectIns* ins2 = (ObjectIns*)ins;
    write_char(OBJECT_INS);
    write_class_arity(ins2->class);
    write_class_tag(ins2->class);
    break;
  }
  case SLOT_OP:{
    SlotIns* ins2 = (SlotIns*)ins;
    write_char(SLOT_INS);
    write_ptr(link_str(values, ins2->name));
    break;
  }
  case SET_SLOT_OP:{
    SetSlotIns* ins2 = (SetSlotIns*)ins;
    write_char(SET_SLOT_INS);
    write_ptr(link_str(values, ins2->name));
    break;
  }
  case CALL_SLOT_OP:{
    CallSlotIns* ins2 = (CallSlotIns*)ins;
    write_char(CALL_SLOT_INS);
    write_char(ins2->arity);
    write_ptr(link_str(values, ins2->name));
    break;
  }
  case CALL_OP:{
    CallIns* ins2 = (CallIns*)ins;
    write_char(CALL_INS);
    write_char(ins2->arity);
    write_function_ptr(link_str(values, ins2->name));
    break;
  }
  case SET_LOCAL_OP:{
    SetLocalIns* ins2 = (SetLocalIns*)ins;
    write_char(SET_LOCAL_INS);
    write_short(ins2->idx);
    break;
  }
  case GET_LOCAL_OP:{
    GetLocalIns* ins2 = (GetLocalIns*)ins;
    write_char(GET_LOCAL_INS);
    write_short(ins2->idx);
    break;
  }
  case SET_GLOBAL_OP:{
    SetGlobalIns* ins2 = (SetGlobalIns*)ins;
    write_char(SET_GLOBAL_INS);
    write_global_idx(link_str(values, ins2->name));
    break;
  }
  case GET_GLOBAL_OP:{
    GetGlobalIns* ins2 = (GetGlobalIns*)ins;
    write_char(GET_GLOBAL_INS);
    write_global_idx(link_str(values, ins2->name));
    break;
  }
  case BRANCH_OP:{
    BranchIns* ins2 = (BranchIns*)ins;
    write_char(BRANCH_INS);
    write_label(link_str(values, ins2->name));
    break;
  }
  case GOTO_OP:{
    GotoIns* ins2 = (GotoIns*)ins;
    write_char(GOTO_INS);
    write_label(link_str(values, ins2->name));
    break;
  }
  case RETURN_OP:{
    write_char(RETURN_INS);
    break;
  }
  case DROP_OP:{
    write_char(DROP_INS);
    break;
  }
  default:
    printf("Unknown instruction: %d\n", ins->tag);
    exit(-1);
  }
}

//=========== CLASSES =============
Vector* classes;
int NULL_CLASS_TAG;
int INT_CLASS_TAG;
int ARRAY_CLASS_TAG;

void init_classes () {
  classes = make_vector();
  NULL_CLASS_TAG = classes->size;
  vector_add(classes, 0);
  INT_CLASS_TAG = classes->size;
  vector_add(classes, 0);
  ARRAY_CLASS_TAG = classes->size;
  vector_add(classes, 0);
}

int make_class (Vector* values, ClassValue* class) {
  //Link
  int nvars = 0;
  int nslots = class->slots->size;
  LSlot* slots = malloc(sizeof(LSlot) * nslots);
  for(int i=0; i<nslots; i++){
    int vidx = (int)vector_get(class->slots, i);
    Value* v = vector_get(values, vidx);
    switch(v->tag){
    case SLOT_VAL:{
      SlotValue* v2 = (SlotValue*)v;
      slots[i].tag = VAR_SLOT;
      slots[i].name = link_str(values, v2->name);
      slots[i].idx = nvars;
      nvars++;
      break;
    }
    case METHOD_VAL:{
      MethodValue* v2 = (MethodValue*)v;
      slots[i].tag = CODE_SLOT;
      slots[i].name = link_str(values, v2->name);
      slots[i].code = get_method_label(vidx);
      break;
    }
    default:{
      printf("Invalid class slot: %d\n", v->tag);
      exit(-1);
    }
    }
  }

  //Create Class
  LClass* c = malloc(sizeof(LClass));
  c->nvars = nvars;
  c->nslots = nslots;
  c->slots = slots;
  vector_add(classes, c);
  
  //Return class index
  return classes->size - 1;
}

char* link_program (Program* prog) {
  init_codebuffer();
  init_patchbuffer();
  init_tablebuffer();
  init_globals();
  init_classes();
  
  //Link code
  for(int i=0; i<prog->values->size; i++){
    MethodValue* v = vector_get(prog->values, i);
    if(v->tag == METHOD_VAL){
      set_method_label(i);
      write_frame(v);
      for(int i=0; i<v->code->size; i++)
        link_ins(prog->values, vector_get(v->code, i));      
    }
  }

  //Link Classes
  for(int i=0; i<prog->values->size; i++){
    ClassValue* v = vector_get(prog->values, i);
    if(v->tag == CLASS_VAL){
      int tag = make_class(prog->values, v);
      set_class_tag(i, tag);
    }
  }

  //Link Globals
  for(int i=0; i<prog->slots->size; i++){
    int idx = (int)vector_get(prog->slots, i);
    Value* v = vector_get(prog->values, idx);
    if(v->tag == SLOT_VAL){
      SlotValue* v2 = (SlotValue*)v;
      new_global(link_str(prog->values, v2->name));
    }else if(v->tag == METHOD_VAL){
      MethodValue* v2 = (MethodValue*)v;
      set_function_label(link_str(prog->values, v2->name), idx);
    }else{
      printf("Not a global.\n");
      exit(-1);
    }
  }

  //Run Patches
  for(int i=0; i<patches->size; i++){
    Patch* p = vector_get(patches, i);
    switch(p->type){
    case LABEL_PATCH:{
      void* dst = get_label(p->name);
      ((void**)(code + p->pos))[0] = dst;
      break;
    }
    case FUNCTION_PATCH:{
      void* dst = get_function_label(p->name);
      ((void**)(code + p->pos))[0] = dst;
      break;
    }
    case CLASS_TAG_PATCH:{
      short tag = get_class_tag(p->class);
      ((short*)(code + p->pos))[0] = tag;
      break;
    }
    case CLASS_ARITY_PATCH:{
      short tag = get_class_tag(p->class);
      LClass* c = vector_get(classes, tag);
      (code + p->pos)[0] = c->nvars;
      break;
    }
    case GLOBAL_IDX_PATCH:{
      short idx = get_global_idx(p->name);
      ((short*)(code + p->pos))[0] = idx;
      break;
    }
    default:
      printf("Unrecognized patch type: %d\n", p->type);
      exit(-1);
    }
  }

  //Return Entry
  return get_method_label(prog->entry);
}


//============================================================
//===================== INTERPRETER ==========================
//============================================================

typedef struct {
  long tag;
  long value;
} VMInt;

typedef struct {
  long tag;
  long scratch;
} VMNull;

typedef struct {
  long tag;
  void* parent;
  void* slots[];
} VMObj;

typedef struct {
  long tag;
  long length;
  void* items[];
} VMArray;

LSlot lookup_method (VMObj* obj, char* name);
LSlot lookup_varslot (VMObj* obj, char* name);
void call_array_slot (char* slotname, int n);
void call_int_slot (char* slotname, int n);
void run_gc ();
void print_obj (VMObj* obj);

int heap_sz;
char* heap_mem;
char* heap_top;
char* heap_ptr;
char* free_mem;

char* ip;
int fp;
int n;
Vector* vstack;
Vector* fstack;
void** genv;
VMNull* nullobj;
VMInt* zeroobj;

void init_heap () {
  heap_sz = 1024 * 16;
  heap_mem = malloc(heap_sz);
  free_mem = malloc(heap_sz);
  heap_ptr = heap_mem;
  heap_top = heap_mem + heap_sz;
}

void* halloc (long tag, int sz) {
  if(heap_ptr + sz > heap_top){
    run_gc();
    if(heap_ptr + sz > heap_top){
      printf("Out of Memory.\n");
      exit(-1);
    }
  }
  long* obj = (long*)heap_ptr;
  obj[0] = tag;
  heap_ptr += sz;
  return obj;
}

VMNull* alloc_null () {
  return halloc(NULL_CLASS_TAG, sizeof(VMNull));
}

VMInt* alloc_int (int value) {
  VMInt* i = halloc(INT_CLASS_TAG, sizeof(VMInt));
  i->value = value;
  return i;
}

VMArray* alloc_empty_array (int length) {
  VMArray* o = halloc(ARRAY_CLASS_TAG, sizeof(VMArray) + sizeof(void*) * length);
  o->length = length;
  return o;
}

VMArray* alloc_array (int length, void* x) {
  VMArray* o = alloc_empty_array(length);
  for(int i=0; i<length; i++)
    o->items[i] = x;
  return o;
}

VMObj* alloc_object (int class, int nslots) {
  return halloc(class, sizeof(VMObj) + sizeof(void*) * nslots);
}

//============================================================
//================ GARBAGE COLLECTOR =========================
//============================================================

typedef struct {
  long tag;
  void* forward;
} BrokenHeart;

void* make_forward (void* src, void* dst) {
  BrokenHeart* bh = (BrokenHeart*)src;
  bh->tag = -1;
  bh->forward = dst;
  return dst;
}

int sizeof_obj (VMObj* o) {
  if(o->tag == NULL_CLASS_TAG)
    return sizeof(VMNull);
  else if(o->tag == INT_CLASS_TAG)
    return sizeof(VMInt);
  else if(o->tag == ARRAY_CLASS_TAG){
    VMArray* a = (VMArray*)o;
    return sizeof(VMArray) + sizeof(void*) * a->length;
  }
  else{
    LClass* c = vector_get(classes, o->tag);
    return sizeof(VMObj) + sizeof(void*) * c->nvars;
  }
}

void* link_ptr (void* ptr) {
  long tag = ((long*)ptr)[0];
  if(tag == -1){
    BrokenHeart* bh = (BrokenHeart*)ptr;
    return bh->forward;
  }else{
    void* dst = heap_ptr;
    int sz = sizeof_obj((VMObj*)ptr);
    memcpy(dst, ptr, sz);
    heap_ptr += sz;
    return make_forward(ptr, dst);
  }
}

void scan_obj (VMObj* o) {
  LClass* c = vector_get(classes, o->tag);
  o->parent = link_ptr(o->parent);
  for(int i=0; i<c->nvars; i++)
    o->slots[i] = link_ptr(o->slots[i]);
}

void scan_array (VMArray* o) {
  for(int i=0; i<o->length; i++)
    o->items[i] = link_ptr(o->items[i]);
}

void* scan_next (char* ptr) {
  long tag = ((long*)ptr)[0];
  if(tag == ARRAY_CLASS_TAG)
    scan_array((VMArray*)ptr);
  else if(tag != INT_CLASS_TAG && tag != NULL_CLASS_TAG)
    scan_obj((VMObj*)ptr);
  return ptr + sizeof_obj((VMObj*)ptr);
}

void scan_fstack () {
  int frame_top = fstack->size;
  int frame_bot = fp;
  while(frame_top > 0){
    for(int i = frame_bot+2; i<frame_top; i++){
      void* o = link_ptr(vector_get(fstack, i));
      vector_set(fstack, i, o);
    }
    frame_top = frame_bot;
    frame_bot = (int)vector_get(fstack, frame_bot + 1);
  }
}

void scan_vstack () {
  for(int i=0; i<vstack->size; i++){
    void* o = link_ptr(vector_get(vstack, i));
    vector_set(vstack, i, o);
  }
}

void scan_globals () {
  for(int i=0; i<globals->size; i++)
    genv[i] = link_ptr(genv[i]);
}

void run_gc () {
  //Flip flop heap
  char* swap = heap_mem;
  heap_mem = free_mem;
  free_mem = swap;
  heap_ptr = heap_mem;
  heap_top = heap_ptr + heap_sz;
    
  //Scan roots
  scan_globals();
  scan_fstack();
  scan_vstack();
  nullobj = link_ptr(nullobj);
  zeroobj = link_ptr(zeroobj);

  //Scan heap
  char* p = heap_mem;
  while(p < heap_ptr)
    p = scan_next(p);

  //printf("Garbage Collection\n");
  //printf("Number of bytes used: %ld\n", heap_ptr - heap_mem);
}

//============================================================
//============================================================
               

unsigned char next_char () {
  unsigned char c = ip[0];
  ip++;
  return c;
}

int next_short () {
  ip = (char*)(((long)ip + 1)&(-2));
  int s = ((unsigned short*)ip)[0];  
  ip += 2;
  return s;
}

int next_int () {
  ip = (char*)(((long)ip + 3)&(-4));
  int s = ((int*)ip)[0];  
  ip += 4;
  return s;
}

void* next_ptr () {
  ip = (char*)(((long)ip + 7)&(-8));
  void* s = ((void**)ip)[0];  
  ip += 8;
  return s;
}

void initvm (char* entry) {
  //Initialize State
  ip = entry;
  fp = 0;
  n = 0;
  vstack = make_vector();
  fstack = make_vector();
  genv = malloc(sizeof(void*) * globals->size);
  init_heap();
  nullobj = alloc_null();
  zeroobj = alloc_int(0);
  
  //Initialize globals
  for(int i=0; i<globals->size; i++)
    genv[i] = nullobj;

  //Default Frame
  vector_add(fstack, 0);
  vector_add(fstack, 0);
}

void ensure_arity (int actual, int desired) {
  if(actual != desired){
    printf("Incorrect arity: Expected %d but received %d.\n", desired, actual);
    exit(-1);
  }
}

void ensure_parent (VMObj* o) {
  if(o->tag == INT_CLASS_TAG){
    printf("Int is not a legal parent.\n");
    exit(-1);
  }
  else if(o->tag == ARRAY_CLASS_TAG){
    printf("Array is not a legal parent.\n");
    exit(-1);
  }  
}

void ensure_int (VMInt* o) {
  if(o->tag != INT_CLASS_TAG){
    printf("Not an integer!\n");
    exit(-1);
  }
}

void ensure_index (VMInt* i, VMArray* a) {
  ensure_int(i);
  if(i->value < 0 || i->value >= a->length){
    printf("Index %d is out of bounds.\n", (int)(i->value));
    exit(-1);
  }
}

void print_vstack () {
  printf("[");
  for(int i=0; i<vstack->size; i++){
    VMObj* obj = vector_get(vstack, i);
    if(i > 0) printf(" ");
    printf("%d", (int)(obj->tag));
  }
  printf("]\n");
}

void print_obj (VMObj* obj) {
  if(obj->tag == NULL_CLASS_TAG){
    printf("null");
  }else if(obj->tag == INT_CLASS_TAG){
    VMInt* o = (VMInt*)obj;
    printf("%d", (int)(o->value));
  }else if(obj->tag == ARRAY_CLASS_TAG){
    VMArray* o = (VMArray*)obj;
    printf("[");
    for(int i=0; i<o->length; i++){
      if(i > 0) printf(" ");
      print_obj(o->items[i]);
    }
    printf("]");
  }else{
    printf("[Object %ld]", obj->tag);
  }
}

void print_format (char* format, int n) {
  int i = vstack->size - n;
  while(1){
    char c = format[0];
    if(c == 0) return;
    if(c == '~') {
      print_obj(vector_get(vstack, i));
      i++;
    }else{
      printf("%c", c);
    }
    format++;    
  }
}

void runvm () {  
  while(ip){
    //    printf("IP = 0x%lx\n", ip);
    //    print_vstack();
    
    char tag = next_char();
    switch(tag){
    case INT_INS : {
      int i = next_int();
      //printf("Run Int(%d)\n", i);
      VMInt* v = alloc_int(i);
      vector_add(vstack, v);
      break;
    }
    case NULL_INS : {
      //printf("Run Null\n");
      vector_add(vstack, nullobj);
      break;
    }
    case PRINTF_INS : {
      int n = next_char();
      char* format = next_ptr();
      //printf("Run Printf(");
      //print_string(format);
      //printf(", %d)\n", n);
      print_format(format, n);
      for(int i=0; i<n; i++)
        vector_pop(vstack);
      vector_add(vstack, nullobj);
      break;
    }
    case ARRAY_INS : {
      //printf("Run Array\n");
      VMInt* len = vector_get(vstack, vstack->size - 2);
      ensure_int(len);
      VMArray* a = alloc_empty_array(len->value);
      void* init = vector_pop(vstack);
      vector_pop(vstack);
      for(int i=0; i<len->value; i++)
        a->items[i] = init;
      vector_add(vstack, a);
      break;
    }
    case OBJECT_INS: {
      int arity = next_char();
      int class = next_short();      
      //printf("Run Object(%d,%d)\n", class, arity);
      VMObj* o = alloc_object(class, arity);
      for(int i = arity-1; i>=0; i--)
        o->slots[i] = vector_pop(vstack);
      void* parent = vector_pop(vstack);
      ensure_parent(parent);
      o->parent = parent;
      vector_add(vstack, o);
      break;
    }
    case SLOT_INS: {
      char* name = next_ptr();
      //printf("Run Slot(%s)\n", name);
      VMObj* o = vector_pop(vstack);
      if(o->tag == INT_CLASS_TAG || o->tag == NULL_CLASS_TAG || o->tag == ARRAY_CLASS_TAG){
        printf("No variable slot %s for object ", name);
        print_obj(o);
        printf(".\n");
      }
      LSlot slot = lookup_varslot(o, name);
      vector_add(vstack, o->slots[slot.idx]);
      break;
    }
    case SET_SLOT_INS: {
      char* name = next_ptr();
      //printf("Run SetSlot(%s)\n", name);
      void* x = vector_pop(vstack);
      VMObj* o = vector_pop(vstack);
      if(o->tag == INT_CLASS_TAG || o->tag == NULL_CLASS_TAG || o->tag == ARRAY_CLASS_TAG){
        printf("No variable slot %s for object ", name);
        print_obj(o);
        printf(".\n");
      }
      LSlot slot = lookup_varslot(o, name);
      o->slots[slot.idx] = x;
      vector_add(vstack, x);
      break;
    }
    case CALL_SLOT_INS: {
      n = next_char();
      char* name = next_ptr();
      //printf("Run CallSlot(%s, %d)\n", name, n);
      int sp = vstack->size;
      VMObj* obj = vector_get(vstack, sp - n);
      if(obj->tag == INT_CLASS_TAG){
        call_int_slot(name, n);
        break;
      }
      else if(obj->tag == ARRAY_CLASS_TAG){
        call_array_slot(name, n);
        break;
      }
      else if(obj->tag == NULL_CLASS_TAG){
        printf("No slot named %s for Null.\n", name);
        exit(-1);
      }
      else{
        LSlot m = lookup_method(obj, name);
        int newfp = fstack->size;
        vector_add(fstack, ip);
        vector_add(fstack, (void*)fp);
        fp = newfp;
        ip = m.code;
        break;
      }
    }
    case CALL_INS : {
      n = next_char();
      void* code = next_ptr();
      //printf("Run Call(0x%lx, %d)\n", code, n);      
      int newfp = fstack->size;
      vector_add(fstack, ip);
      vector_add(fstack, (void*)fp);
      fp = newfp;
      ip = code;
      break;
    }
    case SET_LOCAL_INS : {
      int idx = next_short();
      //printf("Run SetLocal(%d)\n", idx);
      void* v = vector_peek(vstack);
      vector_set(fstack, fp + 2 + idx, v);
      break;
    }
    case GET_LOCAL_INS : {
      int idx = next_short();
      //printf("Run GetLocal(%d)\n", idx);
      void* v = vector_get(fstack, fp + 2 + idx);
      vector_add(vstack, v);      
      break;
    }
    case SET_GLOBAL_INS : {
      int idx = next_short();
      //printf("Run SetGlobal(%d)\n", idx);
      genv[idx] = vector_peek(vstack);
      break;
    }
    case GET_GLOBAL_INS : {
      int idx = next_short();
      //printf("Run GetGlobal(%d)\n", idx);
      vector_add(vstack, genv[idx]);
      break;
    }
    case BRANCH_INS : {
      void* code = next_ptr();
      //printf("Run Branch(0x%lx)\n", code);
      VMObj* obj = vector_pop(vstack);
      if(obj->tag != NULL_CLASS_TAG)
        ip = code;
      break;
    }
    case GOTO_INS : {
      void* code = next_ptr();
      //printf("Run Goto(0x%lx)\n", code);
      ip = code;
      break;
    }
    case RETURN_INS : {
      //printf("Run Return\n");
      int oldfp = (int)vector_get(fstack, fp + 1);
      ip = vector_get(fstack, fp);
      vector_set_length(fstack, fp, nullobj);
      fp = oldfp;
      break;
    }
    case DROP_INS : {
      //printf("Run Drop\n");
      vector_pop(vstack);
      break;
    }
    case FRAME_INS : {
      int nargs = next_char();
      int nlocals = next_short();
      //printf("Run Frame(%d,%d)\n", nargs, nlocals);
      ensure_arity(n, nargs);
      vector_set_length(fstack, fp + 2 + nargs + nlocals, nullobj);
      for(int i=n-1; i>=0; i--)
        vector_set(fstack, fp + 2 + i, vector_pop(vstack));
      break;
    }
    default:
      printf("Unknown tag: %d\n", tag);
      exit(-1);
    }
  }  
}

void push_bool (int bool) {
  if(bool)
    vector_add(vstack, zeroobj);
  else
    vector_add(vstack, nullobj);  
}

void push_int (int r) {
  vector_add(vstack, alloc_int(r));
}

void call_int_slot (char* slotname, int n) {
  ensure_arity(n, 2);
  VMInt* y = vector_pop(vstack);
  VMInt* x = vector_pop(vstack);
  ensure_int(y);
  if(strcmp(slotname, "eq") == 0)
    push_bool(x->value == y->value);
  else if(strcmp(slotname, "lt") == 0)
    push_bool(x->value < y->value);
  else if(strcmp(slotname, "le") == 0)
    push_bool(x->value <= y->value);
  else if(strcmp(slotname, "gt") == 0)
    push_bool(x->value > y->value);
  else if(strcmp(slotname, "ge") == 0)
    push_bool(x->value >= y->value);
  else if(strcmp(slotname, "add") == 0)
    push_int(x->value + y->value);
  else if(strcmp(slotname, "sub") == 0)
    push_int(x->value - y->value);
  else if(strcmp(slotname, "mul") == 0)
    push_int(x->value * y->value);
  else if(strcmp(slotname, "div") == 0)
    push_int(x->value / y->value);
  else if(strcmp(slotname, "mod") == 0)
    push_int(x->value % y->value);
  else{
    printf("No slot named %s for Int.\n", slotname);
    exit(-1);
  }
}

void call_array_slot (char* slotname, int n) {
  if(strcmp(slotname, "get") == 0){
    ensure_arity(n, 2);
    VMInt* i = vector_pop(vstack);
    VMArray* a = vector_pop(vstack);
    ensure_index(i, a);
    vector_add(vstack, a->items[i->value]);
  }
  else if(strcmp(slotname, "set") == 0){
    ensure_arity(n, 3);
    void* v = vector_pop(vstack);
    VMInt* i = vector_pop(vstack);
    VMArray* a = vector_pop(vstack);
    ensure_index(i, a);
    a->items[i->value] = v;
    vector_add(vstack, nullobj);
  }
  else if(strcmp(slotname, "length") == 0){
    ensure_arity(n, 1);
    VMArray* a = vector_pop(vstack);
    push_int(a->length);
  }
  else{
    printf("No slot named %s for Array.\n", slotname);
    exit(-1);
  }
}

LSlot lookup_slot (VMObj* obj, char* name) {
  if(obj->tag == NULL_CLASS_TAG){
    printf("No slot %s for Null.\n", name);
    exit(-1);
  }else{
    LClass* c = vector_get(classes, obj->tag);
    for(int i=0; i<c->nslots; i++){
      LSlot s = c->slots[i];
      if(strcmp(s.name, name) == 0)
        return s;
    }
    return lookup_slot(obj->parent, name);
  }
}

LSlot lookup_method (VMObj* obj, char* name) {
  LSlot s = lookup_slot(obj, name);
  if(s.tag != CODE_SLOT){
    printf("Slot %s is not a method slot.\n", name);
    exit(-1);
  }
  return s;
}

LSlot lookup_varslot (VMObj* obj, char* name) {
  LSlot s = lookup_slot(obj, name);
  if(s.tag != VAR_SLOT){
    printf("Slot %s is not a variable slot.\n", name);
    exit(-1);
  }
  return s;
}

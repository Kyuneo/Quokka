// qdef.h
// Defines the structs and types of the main datatypes in Quokka.

#ifndef _qdef
#define _qdef

// Interpreter global variables
int bc_line;
int bc_line_count;

int * makeIntPtrFromStr(char * st)
{
    int * n = malloc(sizeof(int));
    n[0] = strtol(st, (char **)NULL, 10);
    return n;
}

int * makeIntPtr(int i)
{
    int * n = malloc(sizeof(int));
    n[0] = i;
    return n;
}

long long * makeLLPtrFromStr(char * st)
{
    long long * n = malloc(sizeof(long long));
    n[0] = strtoll(st, (char **)NULL, 10);
    return n;
}

long long * makeLLPtr(long long i)
{
    long long * n = malloc(sizeof(long long));
    n[0] = i;
    return n;
}

//
/// Object & Varlist
//

typedef struct __Object_Struct__ Object;
struct __Object_Struct__
{
    char * name;

    char ** names;
    void ** values;
    int value_count;
};

typedef struct __Varlist_Struct__ Varlist;
struct __Varlist_Struct__
{
    int count;

    char ** names;
    int * values;
};

// Function declarations
void interp_init();
void quokka_interpret_line(char * linetext);
void quokka_interpret_line_tokens(char ** line);
void quokka_interpret_tokens(char ** tokens);
void quokka_interpret(char * bytecode);

void resetStack();
Object emptyObject(char * name);
Object makeObject(char * name, void * value);
Object addObjectValue(Object obj, char * name, void * value);
int objectHasAttr(Object obj, char * name);
void * objectGetAttr(Object obj, char * name);
void freeObject(Object obj);

void pushTop(Object obj);
Object popTop();
int pushMem(Object obj);
void pushTrash(void * ptr);

void assignGVar(char * name, int obj_ptr);
void assignVar(char * name, int obj_ptr);
void addGVar(char * name, Object obj);
void addVar(char * name, Object obj);

int getVar(char * name);
int getGVarIndex(char * name);
int getLVarIndex(char * name);
int getVarIndex(char * name);

Object * makeArglist(Object obj);

//
/// Function
//

typedef struct __Function_Struct__ Function;
struct __Function_Struct__
{
    char ** f_code;
    int * argc;
};

// Function declarations
void addFunction(Function funcobj);

//
/// Quokka datatypes
//

// int
Object makeInt(int * value);

// long
Object makeLong(long long * value);

// string
Object makeString(char * value);

// list
Object makeList(int length, int * value, int flipped);

// null
Object makeNull();

// function
Object makeFunction(char ** bytecode, int argmin, int argmax);

#endif

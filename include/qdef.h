// qdef.h
// Defines the structs and types of the main datatypes in Quokka.

#ifndef _qdef
#define _qdef

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
    Object * values;
};

// Function declarations
void resetStack();
Object emptyObject(char * name);
Object makeObject(char * name, void * value);
Object addObjectValue(Object obj, char * name, void * value);
int objectHasAttr(Object obj, char * name);
void * objectGetAttr(Object obj, char * name);
void freeObject(Object obj);
void pushTop(Object obj);
Object popTop();
void addVar(char * name, Object obj);
Object getVar(char * name);
int getVarIndex(char * name);

//
/// Function
//

typedef struct __Function_Struct__ Function;
struct __Function_Struct__
{
    Object (*func)(Object * argv);
    int * is_function;
    int * argc;
};

// Function declarations
void addFunction(Function funcobj);

//
/// Quokka datatypes
//

// int
Object makeInteger(int * value);

// string
Object makeString(char * value);

// list
Object makeList(int length, Object * value, int flipped);

// null
Object makeNull();

// function
Object makeFunction(Object (*func)(Object * argv), int argc);
Object makeMethod(Object (*func)(Object * argv), int * argc);

#endif

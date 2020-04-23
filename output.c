#include <stdio.h>
#include <string.h>
#include <stdlib.h>
char*cptrindex(char**value,int index){return value[index];}
#define ptrindex(value, index) _Generic((value) + (index), char ** : cptrindex)(value, index)
typedef struct {
    void * value;
} pointer;
char readchar(){return getchar();}
int readint(){int out;scanf("%d",&out);return out;}
float readfloat(){float out;scanf("%f",&out);return out;}
int iprint(int value){printf("%d",value);return value;}
long lprint(long value){printf("%ld",value);return value;}
char cprint(char value){printf("%c",value);return value;}
char*sprint(char*value){printf("%s",value);return value;}
#define print(value) _Generic((value), int : iprint, long : lprint, char : cprint, char * : sprint, default : sprint)(value)
int iprintln(int value){printf("%d\n",value);return value;}
long lprintln(long value){printf("%ld\n",value);return value;}
char cprintln(char value){printf("%c\n",value);return value;}
char*sprintln(char*value){printf("%s\n",value);return value;}
#define println(value) _Generic((value), int : iprintln, long : lprintln, char : cprintln, char * : sprintln, default : sprintln)(value)
struct stack{int maxsize;int top;int*items;};struct stack*newStack(int capacity){struct stack*pt=(struct stack*)malloc(sizeof(struct stack));pt->maxsize=capacity;pt->top=-1;pt->items=(int*)malloc(sizeof(int)*capacity);return pt;}int size(struct stack*pt){return pt->top+1;}
int isEmpty(struct stack*pt){return pt->top==-1;}
int isFull(struct stack*pt){return pt->top==pt->maxsize-1;}
void push(struct stack*pt,int x){if(isFull(pt))exit(EXIT_FAILURE);pt->items[++pt->top]=x;}
int peek(struct stack*pt){if (!isEmpty(pt))return pt->items[pt->top];else return 0;}
int pop(struct stack*pt){if(isEmpty(pt))return 0;return pt->items[pt->top--];}
int CONDITIONAL_FLAG=1;
struct stack*pt;
int main(int argc,char**argv){pt=newStack(4096);println(argc);if(argc < 3){return 0;}println(ptrindex(argv,argc-1));}
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

//
/// OS-related definitions
//

#ifdef _WIN32
    #include <direct.h>
    #define chdir(value) _chdir(value)
    #define realpath(first, second) _fullpath(first, second, _MAX_PATH)

    char *strndup(const char *s1, size_t n)
    {
    	char *copy = (char *)malloc(n + 1);
    	memcpy(copy, s1, n);
    	copy[n] = 0;
    	return copy;
    }
#else
    #include <unistd.h>
#endif

//
/// Structs
//

// integer
typedef struct __integer_Struct__ integer;
struct __integer_Struct__
{
    long value;

    integer (*__add__)(integer self, integer other);
    integer (*__sub__)(integer self, integer other);
    integer (*__mul__)(integer self, integer other);
    integer (*__div__)(integer self, integer other);
};

// string
typedef struct __string_Struct__ string;
struct __string_Struct__
{
    char * value;
    integer length;

    string (*__add__)(string self, string other);
    string (*__sub__)(string self, string other);
    string (*__mul__)(string self, string other);
    string (*__div__)(string self, string other);

    string (*lstrip)(string self);
    string (*rstrip)(string self);
    string (*strip)(string self);
    string (*upper)(string self);
    string (*lower)(string self);
    string (*slice)(string self, int start, int stop);
    string (*reorder)(string self, int step);
    string (*replace)(string self, string strep, string repwith);
};

// bool
typedef struct __bool_Struct__ bool;
struct __bool_Struct__
{
    integer value;

    integer (*__add__)(bool self, bool other);
    integer (*__sub__)(bool self, bool other);
    integer (*__mul__)(bool self, bool other);
    integer (*__div__)(bool self, bool other);
};

//
/// Function declarations
//

// arrsize
int carrsize(char * arr[]);
int iarrsize(int * arr);
int sarrsize(string * arr);

// println
int iprintln(int value);
long lprintln(long value);
char cprintln(char value);
char * cpprintln(char * value);
string sprintln(string value);
string * saprintln(string value[]);
integer intprintln(integer value);
bool bprintln(bool value);

// print
int iprint(int value);
long lprint(long value);
char cprint(char value);
char * cpprint(char * value);
string sprint(string value);
string * saprint(string value[]);
bool bprint(bool value);

// integer

// Special functions `__funcname__`
integer __add_integer__(integer self, integer other);
integer __sub_integer__(integer self, integer other);
integer __mul_integer__(integer self, integer other);
integer __div_integer__(integer self, integer other);

// Regular functions `funcname`
// (None yet)


// string

// Special functions `__funcname__`
string __add_string__(string self, string other);

// Regular functions `funcname`
string __lstrip_string__(string st);
string __rstrip_string__(string st);
string __strip_string__(string st);
string __upper_string__(string st);
string __lower_string__(string st);
string __slice_string__(string st, int start, int stop);
string __reorder_string__(string st, int step);
string __replace_string__(string st, string strep, string repwith);


// bool

// Special functions `__funcname__`
integer __add_bool__(bool self, bool other);
integer __sub_bool__(bool self, bool other);
integer __mul_bool__(bool self, bool other);
integer __div_bool__(bool self, bool other);

// Regular functions `funcname`
// (None yet)


// Definitions
#define arrsize(value) _Generic((value),\
    char **  : carrsize,\
    int *    : iarrsize,\
    string * : sarrsize,\
    default  : carrsize)(value)

#define print(value) _Generic((value),\
    int      : iprint,\
    long     : lprint,\
    char     : cprint,\
    char *   : cpprint,\
    string   : sprint,\
    string * : saprint,\
    integer  : intprint,\
    bool     : bprint,\
    default  : cpprint)(value)

#define println(value) _Generic((value),\
    int      : iprintln,\
    long     : lprintln,\
    char     : cprintln,\
    char *   : cpprintln,\
    string   : sprintln,\
    string * : saprintln,\
    integer  : intprintln,\
    bool     : bprintln,\
    default  : cpprintln)(value)

//
/// integer
//

integer IntegerFromInt(int value)
{
    integer self;

    self.value = value;

    self.__add__ = __add_integer__;
    self.__sub__ = __sub_integer__;
    self.__mul__ = __mul_integer__;
    self.__div__ = __div_integer__;

    return self;
}

integer IntegerFromInteger(integer value)
{
    return value;
}

integer IntegerFromLong(long value)
{
    return IntegerFromInt((int)value);
}

integer IntegerFromChar(char value)
{
    return IntegerFromInt((int)value);
}

#define Integer(value) _Generic((value),\
    int     : IntegerFromInt,\
    long    : IntegerFromLong,\
    char    : IntegerFromChar,\
    integer : IntegerFromInteger,\
    default : IntegerFromInt)(value)
    // I need to fill this in with every single C datatype

#define __integer_Constructor__(value) Integer(value)

integer __add_integer__(integer self, integer other)
{
    return Integer(self.value + other.value);
}

integer __sub_integer__(integer self, integer other)
{
    return Integer(self.value - other.value);
}

integer __mul_integer__(integer self, integer other)
{
    return Integer(self.value * other.value);
}

integer __div_integer__(integer self, integer other)
{
    return Integer(self.value / other.value);
}

//
/// string
//

string StringFromString(string value)
{
    return value;
}

string StringFromCharPointer(char * value)
{
    string self;

    self.value = value;
    self.length = Integer(strlen(value));

    self.__add__ = __add_string__;

    self.lstrip = __lstrip_string__;
    self.rstrip = __rstrip_string__;
    self.strip = __strip_string__;
    self.upper = __upper_string__;
    self.lower = __lower_string__;
    self.slice = __slice_string__;
    self.reorder = __reorder_string__;
    self.replace = __replace_string__;

    return self;
}

string StringFromInt(int value)
{
    char * newval = malloc(11);
    sprintf(newval, "%d", value);
    return StringFromCharPointer(newval);
}

string StringFromLong(long value)
{
    char * newval = malloc(21);
    sprintf(newval, "%ld", value);
    return StringFromCharPointer(newval);
}

string StringFromChar(char value)
{
    return StringFromCharPointer(&value);
}

string StringFromInteger(integer value)
{
    return StringFromLong(value.value);
}

#define String(value) _Generic((value),\
    int     : StringFromInt,\
    long    : StringFromLong,\
    char    : StringFromChar,\
    char *  : StringFromCharPointer,\
    string  : StringFromString,\
    integer : StringFromInteger,\
    default : StringFromCharPointer)(value)

#define __string_Constructor__(value) String(value)

string __add_string__(string self, string other)
{
    char * x = (char *)malloc(self.length.value + other.length.value + 1);
    strcpy(x, self.value);
    strcat(x, other.value);

    string out = String(x);
    return out;
}

string __lstrip_string__(string s)
{
    if (!s.value)
        return s;

    while (s.value[0] == ' ')
        s.value++;

    return s;
}

string __rstrip_string__(string st)
{
    char * s = (char *)malloc(st.length.value + 1);
    strcpy(s, st.value);

    if (!s)
        return st;

    size_t size;
    char * end;

    size = st.length.value;

    if (!size)
        return st;

    end = s + size - 1;
    while (end >= s && isspace(* end))
        end--;
    *(end + 1) = '\0';

    return String(s);
}

string __strip_string__(string st)
{
    char * s = (char *)malloc(st.length.value + 1);
    strcpy(s, st.value);

    if (!s)
        return st;

    size_t size;
    char * end;

    size = st.length.value;

    if (!size)
        return st;

    end = s + size - 1;
    while (end >= s && isspace(* end))
        end--;
    *(end + 1) = '\0';

    return __lstrip_string__(String(s));
}

string __upper_string__(string st)
{
    char * x = (char *)malloc(st.length.value * sizeof(char));

    for (int i = 0; i < st.length.value; i++)
    {
        char letter[2] = {0};
        letter[0] = toupper(st.value[i]);
        strcat(x, letter);
    }

    return String(x);
}

string __lower_string__(string st)
{
    char * x = (char *)malloc(st.length.value * sizeof(char));

    for (int i = 0; i < st.length.value; i++)
    {
        char letter[2] = {0};
        letter[0] = tolower(st.value[i]);
        strcat(x, letter);
    }

    return String(x);
}

string __slice_string__(string st, int start, int stop)
{
    char * x = (char *)malloc(st.length.value + 1 * sizeof(char));
    strcpy(x, st.value);

    if (!st.value || !st.length.value)
        return st;

    char * end;

    end = x + st.length.value - 1;
    end -= stop;
    *(end + 1) = '\0';

    x += start;

    return String(x);
}

string __reorder_string__(string st, int step)
{
    char * x = (char *)malloc(st.length.value * sizeof(char));

    if (step < 1) step = 1;
    if (step > st.length.value) step = st.length.value;
    for (int i = 0; i < st.length.value; i += step)
    {
        char * letter = malloc(2 * sizeof(char));
        letter[0] = st.value[i];
        strcat(x, letter);
    }

    return String(x);
}

string __replace_string__(string st, string strep, string repwith) {
    char * orig = st.value;
    char * rep = strep.value;
    char * with = repwith.value;

    char * result; // the return string
    char * ins;    // the next insert point
    char * tmp;    // varies
    int len_rep;  // length of rep (the string to remove)
    int len_with; // length of with (the string to replace rep with)
    int len_front; // distance between rep and end of last rep
    int count;    // number of replacements

    // sanity checks and initialization
    if (!orig || !rep)
        return String("");
    len_rep = strlen(rep);
    if (len_rep == 0)
        return String(""); // empty rep causes infinite loop during count
    if (!with)
        with = "";
    len_with = strlen(with);

    // count the number of replacements needed
    ins = orig;
    for (count = 0; tmp = strstr(ins, rep); ++count) {
        ins = tmp + len_rep;
    }

    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result)
        return String("");

    // first time through the loop, all the variable are set correctly
    // from here on,
    //    tmp points to the end of the result string
    //    ins points to the next occurrence of rep in orig
    //    orig points to the remainder of orig after "end of rep"
    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep; // move to next "end of rep"
    }

    strcpy(tmp, orig);
    string out;
    out = String(result);
    return out;
}

//
/// bool
//

bool BoolFromInt(int value)
{
    bool self;

    if (value)
        self.value = Integer(1);
    else self.value = Integer(0);

    self.__add__ = __add_bool__;
    self.__sub__ = __sub_bool__;
    self.__mul__ = __mul_bool__;
    self.__div__ = __div_bool__;

    return self;
}

bool BoolFromBool(bool value)
{
    return value;
}

bool BoolFromString(string value)
{
    if (strlen(value.value))
        return BoolFromInt(1);
    return BoolFromInt(0);
}

bool BoolFromInteger(integer value)
{
    return BoolFromInt(value.value);
}

#define Bool(value) _Generic((value),\
    int     : BoolFromInt,\
    bool    : BoolFromBool,\
    string  : BoolFromString,\
    integer : BoolFromInteger,\
    default : BoolFromInt)(value)

#define __bool_Constructor__(value) Bool(value)

integer __add_bool__(bool self, bool other)
{
    if (self.value.value)
        return Integer(1 + other.value.value);
    return other.value;
}

integer __sub_bool__(bool self, bool other)
{
    if (self.value.value)
        return Integer(1 - other.value.value);
    return Integer(-other.value.value);
}

integer __mul_bool__(bool self, bool other)
{
    if (self.value.value)
        return other.value;
    return Integer(0);
}

integer __div_bool__(bool self, bool other)
{
    if (!other.value.value)
        return Integer(0);
    return Integer(self.value.value / other.value.value);
}

//
/// All the rest
//

void sarrappend(string * lst, string arg1, ...)
{
    printf("<%s>\n", lst[0].value);

    va_list ap;

    int len = arrsize(lst);

    lst = (string *)realloc(lst, (len + 2) * sizeof(string));
    lst[len + 1] = String("\0");
    lst[len] = arg1;
    len++;

    va_start(ap, arg1);

    string last = va_arg(ap, string);

    while (strcmp(last.value, "\0") != 0)
    {
        printf("[%s]\n", last.value);
        lst = (string *)realloc(lst, (len + 2) * sizeof(string));
        lst[len + 1] = String("\0");
        lst[len] = last;
        len++;
        last = va_arg(ap, string);
    }

    va_end(ap);

    printf("<%s>\n", lst[0].value);
}

string * sarradd(string * lst, string arg1, ...)
{
    va_list ap;

    int len = arrsize(lst);
    string * newlst = (string *)malloc(len + 1024 * sizeof(string));

    for (int i = 0; i < len; i++)
        newlst[i] = lst[i];

    newlst[len + 1] = String("\0");
    newlst[len] = arg1;

    va_start(ap, arg1);

    string last = va_arg(ap, string);

    while (last.value != "\0")
    {
        newlst[len + 1] = String("\0");
        newlst[len] = last;
        len++;
        last = va_arg(ap, string);
    }

    va_end(ap);

    return newlst;
}

char * cptrindex(char ** value, int index)
{
    return value[index];
}

int carrsize(char * arr[])
{
    int i = 0;
    long j = (long) arr[0];
    while (j != '\0')
    {
        i++;
        j = (long) arr[i];
    }
    return i;
}

int iarrsize(int * arr)
{
    return sizeof(arr) / sizeof(int);
}

int sarrsize(string * arr)
{
    int i = 0;
    string j = arr[0];
    while (j.value != "\0")
    {
        i++;
        j = arr[i];
    }
    return i;
}

int startswith(const char * a, const char * b)
{
    if (strncmp(a, b, strlen(b)) == 0) return 1;
    return 0;
}

int endswith(const char * str, const char * suffix)
{
    if (!str || !suffix)
        return 0;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix >  lenstr)
        return 0;
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

char * readfile(char * fname)
{
    char * buffer = 0;
    long length;

    FILE * f = fopen(fname, "rb");

    if (f)
    {
        fseek(f, 0, SEEK_END);
        length = ftell(f);
        fseek(f, 0, SEEK_SET);
        buffer = malloc(length);
        if (buffer)
            fread(buffer, 1, length, f);
        fclose(f);
    }

    return buffer;
}

char * nstrtok(char * string, char const * delimiter)
{
    static char * source = NULL;
    char * p, * ret = 0;

    if (string != NULL) source = string;
    if (source == NULL) return NULL;

    if ((p = strpbrk(source, delimiter)) != NULL)
    {
        * p = 0;
        ret = source;
        source = ++p;
    }

    return ret;
}

char * cpstrip(char * s)
{
    if (!s)
        return s;

    size_t size;
    char * end;

    size = strlen(s);

    if (!size)
        return s;

    end = s + size - 1;
    while (end >= s && isspace(* end))
        end--;
    *(end + 1) = '\0';

    while (* s && isspace( * s))
        s++;

    return s;
}

void tokenise(char * arr[], char * buffer, char * tokAt)
{
    int i = 0;
    arr[0] = strtok(buffer, tokAt);
    while (arr[i] != NULL)
    {
        i++;
        arr[i] = cpstrip(strtok(NULL, tokAt));
    }
}

void ntokenise(char * arr[], char * buffer, char * tokAt)
{
    int i = 0;
    arr[0] = nstrtok(buffer, tokAt);
    while (arr[i] != NULL)
    {
        i++;
        arr[i] = cpstrip(nstrtok(NULL, tokAt));
    }
}

int charCount(char * chst, char ch)
{
    string st = String(chst);

    int count = 0;

    for (int i = 0; i < st.length.value; i++)
    {
        if (st.value[i] == ch)
            count++;
    }

    return count;
}

char * getrealpath(char * path)
{
    char * rp = realpath(path, NULL);
    if (!rp) return NULL;
    return __replace_string__(String(rp), String('\\'), String('/')).value;
}

#define ptrindex(value, index) _Generic((value, index), char ** : cptrindex, default : cptrindex)(value, index)

string readchar(){return String(getchar());}
int readint(){int out;scanf("%d",&out);return out;}
float readfloat(){float out;scanf("%f",&out);return out;}

// print

int iprint(int value)
{
    printf("%d", value);
    return value;
}

long lprint(long value)
{
    printf("%ld", value);
    return value;
}

char cprint(char value)
{
    printf("%c", value);
    return value;
}

char * cpprint(char * value)
{
    printf("%s", value);
    return value;
}

string sprint(string value)
{
    printf("%s", value.value);
    return value;
}

string * saprint(string value[])
{
    printf("{");
    for (int i = 0; i < arrsize(value); i++)
    {
        if (i > 0)
            printf(", ");
        printf("%s", value[i].value);
    }
    printf("}");
    return value;
}

integer intprint(integer value)
{
    printf("%ld", value.value);
    return value;
}

bool bprint(bool value)
{
    if (value.value.value)
        printf("true");
    else printf("false");
}

// println

int iprintln(int value)
{
    printf("%d\n", value);
    return value;
}

long lprintln(long value)
{
    printf("%ld\n", value);
    return value;
}

char cprintln(char value)
{
    printf("%c\n", value);
    return value;
}

char * cpprintln(char * value)
{
    printf("%s\n", value);
    return value;
}

string sprintln(string value)
{
    printf("%s\n", value.value);
    return value;
}

string * saprintln(string value[])
{
    printf("{");
    for (int i = 0; i < arrsize(value); i++)
    {
        if (i > 0)
            printf(", ");
        printf("%s", value[i].value);
    }
    printf("}\n");
    return value;
}

integer intprintln(integer value)
{
    printf("%ld\n", value.value);
    return value;
}

bool bprintln(bool value)
{
    if (value.value.value)
        printf("true\n");
    else printf("false\n");
}

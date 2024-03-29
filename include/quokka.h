// VERSION STUFF
#define VERSION "0.6.6"

// Defines
#define LN10 2.3025850929940456840179914546844
#define QUOKKA_PATH_MAX 64 // Maximum number of paths in Quokka Path
#ifndef max
    #define max(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
    #define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

// Global file stuff
char * current_file;
char * cli_current_line;
int in_cli_mode = 0;
char ** quokka_path;
int quokka_path_len = 0;

//
/// OS-related definitions
//

#ifdef _WIN32
    #include <Windows.h>
    #include <direct.h>
    // #include <conio.h>
    #define chdir(value) _chdir(value)
    #define realpath(first, second) _fullpath(NULL, first, _MAX_PATH)

    char * strndup(const char * s1, size_t n)
    {
        char * copy = malloc(n + 1);
        memcpy(copy, s1, n);
        copy[n] = 0;
        return copy;
    }
#else
    #include <unistd.h>
    // #include "conio.h"
#endif

//
/// Includes deliberately after OS-related definitions
//

#include "qdef.h"

//
/// Memory
//

// Object ** mem;
// int memsize;
// int memalloc;
// int memalloc_size = 20; // Reallocates new memory per 20 Objects

//
/// Function declarations
//

char * strSlice(char * st, int start, int stop);
char * strnSlice(char * st, int start, int stop, int len);
char * strReplace(char * orig, char * rep, char * with);
void strInsertStart(char ** st, char * newstr);
void mstrcat(char ** charptr, char * newstr);
void mstrcattrip(char ** charptr, char * newstr, char * endstr);

// Compiler stuff
void error(char * text, int line);

// Interpreter stuff
void freeVars();
void freeMemory();
void freeStack();
void freeRetStack();
void emptyTrash();
void cleanupAll();

// arrsize
int arrsize(char ** arr);
int iarrsize(int * arr);

// println
int iprintln(int value);
long lprintln(long value);
char cprintln(char value);
char * cpprintln(char * value);

// print
int iprint(int value);
long lprint(long value);
char cprint(char value);
char * cpprint(char * value);

// Definitions
#define print(value) _Generic((value),\
    int      : iprint,\
    char     : cprint,\
    char *   : cpprint,\
    default  : cpprint)(value)

#define println(value) _Generic((value),\
    int      : iprintln,\
    char     : cprintln,\
    char *   : cpprintln,\
    default  : cpprintln)(value)

// Used to typecast Object * pointers into Quokka functions
typedef Object * (*standard_func_def)(int argc, Object ** argv);

//
/// All the rest
//

unsigned long intObjAddress(Object * ptr)
{
    return (uintptr_t)ptr;
}

char * strObjAddress(Object * ptr)
{
    // Can contain up to a 19 digit pointer address (in hexadecimal)
    char * res = malloc(20);

    snprintf(res, 20, "0x%" PRIXPTR, intObjAddress(ptr));

    return res;
}

char * neatObjAddress(Object * ptr)
{
    char * obj_addr = strObjAddress(ptr);

    char * out = malloc(2 + strlen(ptr->name) + 12 + strlen(obj_addr) + 1 + 1);
    strcpy(out, "<'");
    strcat(out, ptr->name);
    strcat(out, "' Object at ");
    strcat(out, obj_addr);
    strcat(out, ">");

    free(obj_addr);

    return out;
}

int arrsize(char ** arr)
{
    int i;
    for (i = 0; arr[i] != NULL; i++);

    return i;
}

int strarrsize(char *** arr)
{
    int i;
    for (i = 0; arr[i] != NULL; i++);

    return i;
}

int iarrsize(int * arr)
{
    return sizeof(arr) / sizeof(int);
}

char * getrealpath(char * path)
{
    char * rp = realpath(path, NULL);

    if (!rp)
    {
        free(rp);
        return 0;
    }

    char * res = strReplace(rp, "\\", "/");
    free(rp);

    return res;
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

char * string_reverse(char * str)
{
    char * p1, * p2;

    if (!str || !str[0])
        return str;

    for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; p1++, p2--)
    {
        *p1 ^= *p2;
        *p2 ^= *p1;
        *p1 ^= *p2;
    }

    return str;
}

char * readfile(char * fname)
{
    char * buffer;
    long length;

    FILE * f = fopen(fname, "rb");

    if (!f) return 0;

    fseek(f, 0, SEEK_END);
    length = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (!length)
    {
        fclose(f);
        return 0;
    }

    buffer = malloc(length);
    strcpy(buffer, "");

    if (buffer)
        fread(buffer, 1, length, f);

    fclose(f);

    buffer = realloc(buffer, length + 1);
    buffer[length] = '\0';

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
        *p = 0;
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
    while (end >= s && isspace(*end))
        end--;
    *(end + 1) = '\0';

    while (*s && isspace(*s))
        s++;

    return s;
}

int stringHasChar(char * s, char c)
{
    for (int i = 0; i < strlen(s); ++i)
    {
        if (s[i] == c)
            return 1;
    }
    return 0;
}

int charCount(char * st, char ch)
{
    int count = 0;

    for (int i = 0; st[i]; i++)
        if (st[i] == ch)
            count++;

    return count;
}

void tokenise(char * arr[], char * buffer, char * tokAt)
{
    int i = 0;
    arr[0] = cpstrip(strtok(buffer, tokAt));
    while (arr[i] != NULL)
    {
        i++;
        arr[i] = cpstrip(strtok(NULL, tokAt));
    }
}

void ntokenise(char * arr[], char * buffer, char * tokAt)
{
    int i = 0;
    arr[0] = cpstrip(nstrtok(buffer, tokAt));
    while (arr[i] != NULL)
    {
        i++;
        arr[i] = cpstrip(nstrtok(NULL, tokAt));
    }
}

char * makeLiteralString(char * str)
{
    if ((startswith(str, "'") && endswith(str, "'")) ||
        (startswith(str, "\"") && endswith(str, "\"")))
        str = strSlice(str, 1, 1);

    int current = 0;
    int len = strlen(str);
    int escaped = 0;

    char * newstr = malloc(len + 1);
    memset(newstr, 0, len + 1);
    strcpy(newstr, "");

    for (int i = 0; i < len; i++)
    {
        if (str[i] == '\\')
            escaped = 1;
        else if (escaped)
        {
            if      (str[i] == 'a')  strcat(newstr, "\a");
            else if (str[i] == 'b')  strcat(newstr, "\b");
            else if (str[i] == 'e')  strcat(newstr, "\e");
            else if (str[i] == 'f')  strcat(newstr, "\f");
            else if (str[i] == 'n')  strcat(newstr, "\n");
            else if (str[i] == 'r')  strcat(newstr, "\r");
            else if (str[i] == 't')  strcat(newstr, "\t");
            else if (str[i] == 'v')  strcat(newstr, "\v");
            else if (str[i] == '\\') strcat(newstr, "\\");
            else if (str[i] == '\'') strcat(newstr, "'");
            else if (str[i] == '"')  strcat(newstr, "\"");
            else if (str[i] == '?')  strcat(newstr, "?");
            current++;
            escaped = 0;
        }
        else
        {
            newstr[current] = str[i];
            current++;
            escaped = 0;
        }
    }

    free(str);

    return newstr;
}

char * makeRawString(char * str)
{
    int dquotes;

    char * newstr = malloc(2);

    if (stringHasChar(str, '\''))
    {
        strcpy(newstr, "\"");
        dquotes = 1;
    }
    else
    {
        strcpy(newstr, "'");
        dquotes = 0;
    }

    int len = strlen(str);
    int escaped = 0;

    for (int i = 0; i < len; i++)
    {
        if      (str[i] == '\a') mstrcat(&newstr, "\\a");
        else if (str[i] == '\b') mstrcat(&newstr, "\\b");
        else if (str[i] == '\e') mstrcat(&newstr, "\\e");
        else if (str[i] == '\f') mstrcat(&newstr, "\\f");
        else if (str[i] == '\n') mstrcat(&newstr, "\\n");
        else if (str[i] == '\r') mstrcat(&newstr, "\\r");
        else if (str[i] == '\t') mstrcat(&newstr, "\\t");
        else if (str[i] == '\v') mstrcat(&newstr, "\\v");
        else if (str[i] == '\\') mstrcat(&newstr, "\\\\");
        else if (str[i] == '\'' && !dquotes) mstrcat(&newstr, "\\'");
        else if (str[i] == '"' && dquotes)   mstrcat(&newstr, "\\\"");
        else
        {
            newstr = realloc(newstr, strlen(newstr) + 1 + 1);
            strncat(newstr, &str[i], 1);
        }
    }

    if (dquotes)
        mstrcat(&newstr, "\"");
    else mstrcat(&newstr, "'");

    return newstr;
}

// Convert literals into their escaped forms within a string
char * convertLiterals(char * str)
{
    char * newstr = malloc(2);
    strcpy(newstr, "");

    int len = strlen(str);
    int escaped = 0;

    for (int i = 0; i < len; i++)
    {
        if      (str[i] == '\a') mstrcat(&newstr, "\\a");
        else if (str[i] == '\b') mstrcat(&newstr, "\\b");
        else if (str[i] == '\e') mstrcat(&newstr, "\\e");
        else if (str[i] == '\f') mstrcat(&newstr, "\\f");
        else if (str[i] == '\n') mstrcat(&newstr, "\\n");
        else if (str[i] == '\r') mstrcat(&newstr, "\\r");
        else if (str[i] == '\t') mstrcat(&newstr, "\\t");
        else if (str[i] == '\v') mstrcat(&newstr, "\\v");
        else
        {
            newstr = realloc(newstr, strlen(newstr) + 1 + 1);
            strncat(newstr, &str[i], 1);
        }
    }

    return newstr;
}

double log10LN(double x)
{
    // printf("{%f}\n", x);

    double old_sum = 0.0;
    double xmlxpl = (x - 1) / (x + 1);
    double xmlxpl_2 = xmlxpl * xmlxpl;
    double denom = 1.0;
    double frac = xmlxpl;
    double term = frac;
    double sum = term;

    while (sum != old_sum)
    {
        old_sum = sum;
        denom += 2.0;
        frac *= xmlxpl_2;
        sum += frac / denom;
        // printf("[%f] [%f]\n", old_sum, sum);
    }

    return 2.0 * sum;
}

double log10(double x)
{
    return log10LN(x) / LN10;
}

// double log(int b, double n)
// {
//     double val = 0;

//     int i = 0;
//     int accurate = 10;
//     int reps = 0;

//     while (n != 1 && accurate >= 0)
//     {
//         for (i = 0; n >= b; i++)
//             n /= b;

//         n = pow(n, 10);
//         val = 10 * (val + i);
//         accurate--;
//         reps++;
//     }

//     return (double)val / pow(10, reps);
// }

int countDigitsInBase(int n, int base)
{
    // Calculating log using the base changing property, then taking it's
    // floor, and adding 1 at the end
    double res = log10(10000);
    println(n);
    printf("{%f}\n", res);

    return (int)(log10(n) / log10(base) + 1);
}

char * intToBinary(int n)
{
    println("Checkpoint 0");

    int sign = intsign(n);
    n = abs(n);
    println(n);

    char * res = malloc(countDigitsInBase(n, 2) + (sign < 0) + 1 + 1);

    println("Checkpoint 1");

    int i = 0;
    while (n > 0)
    {
        res[i++] = (!!(n % 2)) + '0';
        n /= 2;
    }

    // Prefix each binary number with 'b' to show that it's in binary
    // (Everything here is happening in reverse because the result string will
    // be reversed)
    res[i++] = 'b';

    if (sign < 0)
        res[i++] = '-';

    // Null byte
    res[i] = 0;

    println("Checkpoint -1");

    // Reverse the result string
    res = string_reverse(res);

    println("Checkpoint not 1");

    return res;
}

// Only needs to support the following bases:
//   2, 8, 10, 16
char * intToStrBase(int value, int base)
{
    print("P ");
    println(value);

    if (base == 2)
        return intToBinary(value);

    char * format = "";

    if (base == 8)
        format = "0o%o";
    else if (base == 10)
        format = "%d";
    else if (base == 16)
        format = "0x%" PRIXPTR;

    int length = snprintf(NULL, 0, format, value);

    if (!length)
    {
        char * empty = malloc(1);
        empty[0] = 0;

        return empty;
    }

    char * newval = malloc(length + 1);
    strcpy(newval, "");

    sprintf(newval, format, value);

    return newval;
}

char * intToStr(int value)
{
    int length = snprintf(NULL, 0, "%d", value);

    if (!length)
    {
        char * empty = malloc(1);
        empty[0] = 0;

        return empty;
    }

    char * newval = malloc(length + 1);
    strcpy(newval, "");

    sprintf(newval, "%d", value);

    return newval;
}

char * LLToStr(long long value)
{
    int length = snprintf(NULL, 0, "%lld", value);

    if (!length)
    {
        char * empty = malloc(1);
        empty[0] = '\0';

        return empty;
    }

    char * newval = malloc(length + 1);
    strcpy(newval, "");

    sprintf(newval, "%lld", value);

    return newval;
}

int ipowMath(int base, int exp)
{
    int result = 1;
    for (;;)
    {
        if (exp & 1)
            result *= base;
        exp >>= 1;
        if (!exp)
            break;
        base *= base;
    }

    return result;
}

long long lpowMath(long long base, int exp)
{
    long long result = 1;
    for (;;)
    {
        if (exp & 1)
            result *= base;
        exp >>= 1;
        if (!exp)
            break;
        base *= base;
    }

    return result;
}

char * strSlice(char * st, int start, int stop)
{
    int len = strlen(st);

    char * x = malloc(len - start - stop + 1);
    strcpy(x, "");

    if (!st || !len)
    {
        free(x);
        return st;
    }

    for (int i = start; i < len - stop; i++)
        strncat(x, &st[i], 1);

    return x;
}

char * strnSlice(char * st, int start, int stop, int len)
{
    char * x = malloc(len - start - stop + 1);
    strcpy(x, "");

    if (!st || !len)
    {
        free(x);
        return st;
    }

    for (int i = start; i < len - stop; i++)
        strncat(x, &st[i], 1);

    return x;
}

char * strReplace(char * orig, char * rep, char * with)
{
    char * result; // the return string
    char * ins;    // the next insert point
    char * tmp;    // varies
    int len_rep;  // length of rep (the string to remove)
    int len_with; // length of with (the string to replace rep with)
    int len_front; // distance between rep and end of last rep
    int count;    // number of replacements

    // sanity checks and initialization
    if (!orig || !rep)
        return 0;
    len_rep = strlen(rep);
    if (len_rep == 0)
        return 0; // empty rep causes infinite loop during count
    if (!with)
        with = "";
    len_with = strlen(with);

    // count the number of replacements needed
    ins = orig;
    for (count = 0; (tmp = strstr(ins, rep)); ++count) {
        ins = tmp + len_rep;
    }

    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result)
    {
        free(tmp);
        return 0;
    }

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

    return result;
}

void strInsertStart(char ** st, char * newstr)
{
    size_t len = strlen(*st);
    *st = realloc(*st, len + strlen(newstr) + 1);

    char * temp = strdup(*st);

    memset(*st, 0, len);
    strcpy(*st, newstr);
    strcat(*st, temp);

    free(temp);
}

// Appends characters to the left of a given string
// until it's length matches a given number
// (The original string is free'd)
char * lpadfree(char * st, int n, char ch)
{
    char * res = malloc(n + 1);
    strcpy(res, "");

    int len = strlen(st);

    for (int i = 0; i < n - len; i++)
        strncat(res, &ch, 1);

    strcat(res, st);
    free(st);

    return res;
}

char * getinput()
{
    char * buffer = malloc(1);
    strcpy(buffer, "");
    int buflen = 0;

    char last = 0;

    while (last != '\n' && last != '\r')
    {
        last = getchar();

        buffer = realloc(buffer, buflen + 1 + 1);
        buffer[buflen] = last;

        buflen++;
    }

    buffer[buflen - 1] = '\0';

    return buffer;
}

void mstrcat(char ** charptr, char * newstr)
{
    int len = strlen(*charptr) + strlen(newstr);
    *charptr = realloc(*charptr, len + 1);
    strcat(*charptr, newstr);
    (*charptr)[len] = '\0';
}

void mstrcattrip(char ** charptr, char * newstr, char * endstr)
{
    int len = strlen(*charptr) + strlen(newstr) + strlen(endstr);
    *charptr = realloc(*charptr, len + 1);
    strcat(*charptr, newstr);
    strcat(*charptr, endstr);
    (*charptr)[len] = '\0';
}

void mstrcatline(char ** charptr, char * newstr, char * lmidstr, char * rmidstr, char * endstr)
{
    int len = strlen(*charptr) + strlen(newstr) + strlen(lmidstr) + strlen(rmidstr) + strlen(endstr);
    *charptr = realloc(*charptr, len + 1);
    strcat(*charptr, newstr);
    strcat(*charptr, lmidstr);
    strcat(*charptr, rmidstr);
    strcat(*charptr, endstr);
    (*charptr)[len] = '\0';
}

int intArrHas(int * arr, int ptr, int size)
{
    for (int i = 0; i < size; i++)
        if (arr[i] == ptr)
            return 1;

    return 0;
}

// print

int iprint(int value)
{
    printf("%d", value);
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

// println

int iprintln(int value)
{
    printf("%d\n", value);
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

// Definitions:
#define SEPARATOR " "
#define INSTRUCTION_END "\n"

// Bools:
int compilation_error = 0;

// Function declarations
void error(char * text, int line);
int isidentifier(char * word);
int isinteger(char * word);
int stringInList(char * arr[], char * key);
void arrlstrip(char * line[]);
int stringHasChar(char * s, char c);

char * quokka_compile_line(char * linetext, int num, int lineLen, int isInline);
char * quokka_compile_line_tokens(char ** line, int num, int lineLen, int isInline);
char * quokka_compile_tokens(char ** tokens, int isInline);
char * quokka_compile_raw(char * rawtext, int maxtokensize, int isInline);
char * quokka_compile_fname(char * filename);

char ** quokka_line_tok(char * line);

void error(char * text, int line)
{
    int MAXLINE = 128;

    char * fullfile = readfile(current_file);

    char * lineprevious = malloc(MAXLINE);
    char * linepreview = malloc(MAXLINE);
    char * linenext = malloc(MAXLINE);

    strcpy(lineprevious, "");
    strcpy(linepreview, "");
    strcpy(linenext, "");

    int c = 0;
    for (int i = 0; i < strlen(fullfile) - 1; i++)
    {
        if (fullfile[i] == '\n')
            c++;
        else if (c == line - 1 && strlen(lineprevious) < MAXLINE && fullfile[i] != 10 && fullfile[i] != 13)
            strcat(lineprevious, String(fullfile[i]).value);
        else if (c == line && strlen(linepreview) < MAXLINE && fullfile[i] != 10 && fullfile[i] != 13)
            strcat(linepreview, String(fullfile[i]).value);
        else if (c == line + 1 && strlen(linenext) < MAXLINE && fullfile[i] != 10 && fullfile[i] != 13)
            strcat(linenext, String(fullfile[i]).value);
        if (c > line + 1)
            break;
    }

    line++;

    if (!compilation_error)
        println("\nProgram execution terminated:\n");

    printf("At %s : Line %d\n\n", current_file, line);

    if (line - 1 > 0)
        printf("  %d | %s\n", line - 1, lineprevious);
    printf("> %d > %s\n", line, linepreview);
    if (line < charCount(fullfile, '\n') + 1)
        printf("  %d | %s\n", line + 1, linenext);
    print("\n");

    printf("Error: %s\n\n", text);

    free(fullfile);
    free(lineprevious);
    free(linepreview);
    free(linenext);

    compilation_error = 1;
}

int isidentifier(char * word)
{
    int size = strlen(word);
    int hasnotnum = 0;

    for (int i = 0; i < size; i++)
    {
        if (!isalpha(word[i]) &&
            word[i] != '_' &&
            !(hasnotnum && isdigit(word[i])))
            return 0;
        if (isalpha(word[i]) || word[i] == '_')
            hasnotnum = 1;
    }

    return 1;
}

int isinteger(char * word)
{
    int size = strlen(word);

    for (int i = 0; i < size; i++)
    {
        if (!isdigit(word[i]))
            return 0;
    }

    return 1;
}

int stringInList(char * arr[], char * key)
{
    for (int n = 0; n < arrsize(arr); ++n)
    {
        if (strcmp(arr[n], key) == 0)
            return 1;
    }
    return 0;
}

void arrlstrip(char * line[])
{
    int position, c, n;
    position = 0;
    n = arrsize(line);
    for (c = position - 1; c < n - 1; c++)
        line[c] = line[c + 1];
    line[n - 1] = "\0";
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

int stringCount(char ** lst, char * st)
{
    int len = arrsize(lst);
    int count = 0;

    for (int i = 0; i < len; i++)
    {
        if (!strcmp(lst[i], st))
            count++;
    }

    return count;
}

char * quokka_compile_line(char * linetext, int num, int lineLen, int isInline)
{
    char ** line = quokka_line_tok(linetext);
    return quokka_compile_line_tokens(line, num, lineLen, isInline);
}

char * quokka_compile_line_tokens(char ** line, int num, int lineLen, int isInline)
{
    char * bytecode = malloc(2048);

    int len;
    if (lineLen == -1)
        len = arrsize(line);
    else len = lineLen;

    if (!len)
        return "";

    if (len < 2)
        line[1] = "";

    if (verbose) println(line[0]);

    if (stringInList(line, "="))
    {
        if (isInline) error("variables must be defined at the start of a line", num);
        if (!isidentifier(line[0]))
            error("variable name to assign must be a valid identifier", num);
        if (strcmp(line[1], "="))
            error("equals sign operator is in the wrong position for variable definition", num);
        if (len < 3) error("variable declaration missing variable value", num);

        char * varname = line[0];

        arrlstrip(line);
        arrlstrip(line);
        len -= 2;
        char * temp = quokka_compile_line_tokens(line, num, len, 1);

        strcat(bytecode, strndup(temp, strlen(temp)));

        free(temp);

        strcat(bytecode, "STORE_NAME");
        strcat(bytecode, SEPARATOR);
        strcat(bytecode, varname);
        strcat(bytecode, INSTRUCTION_END);
    }
    else if (stringInList(line, "+") || stringInList(line, "-") || stringInList(line, "*") || stringInList(line, "/"))
    {
        char * operslist = malloc(1);
        strcpy(operslist, "");

        char * valuelist = malloc(1);
        strcpy(valuelist, "");

        char * latestvalue = malloc(1);
        strcpy(latestvalue, "");

        int lastwasop = 1;

        for (int i = 0; i < len; i++)
        {
            if (!strcmp(line[i], "+"))
            {
                char * temp = quokka_compile_line(latestvalue, num, -1, 1);
                valuelist = realloc(valuelist, strlen(valuelist) + strlen(temp) + 1);
                strcat(valuelist, strdup(temp));
                free(temp);

                if (lastwasop)
                {
                    error("invalid syntax at '+'", num);
                    exit(1);
                }

                operslist = realloc(operslist, strlen(operslist) + 10 + strlen(INSTRUCTION_END) + 1);

                char * tmp = strdup(operslist);

                memset(operslist, 0, strlen(operslist));
                strcpy(operslist, "BINARY_ADD");
                strcat(operslist, INSTRUCTION_END);
                strcat(operslist, tmp);

                free(tmp);

                lastwasop = 1;
            }
            else if (!strcmp(line[i], "-"))
            {
                char * temp = quokka_compile_line(latestvalue, num, -1, 1);
                valuelist = realloc(valuelist, strlen(valuelist) + strlen(temp) + 1);
                strcat(valuelist, strdup(temp));
                free(temp);

                if (lastwasop)
                {
                    error("invalid syntax at '-'", num);
                    exit(1);
                }

                operslist = realloc(operslist, strlen(operslist) + 10 + strlen(INSTRUCTION_END) + 1);

                char * tmp = strdup(operslist);

                memset(operslist, 0, strlen(operslist));
                strcpy(operslist, "BINARY_SUB");
                strcat(operslist, INSTRUCTION_END);
                strcat(operslist, tmp);

                free(tmp);

                lastwasop = 1;
            }
            else if (!strcmp(line[i], "*"))
            {
                char * temp = quokka_compile_line(latestvalue, num, -1, 1);
                valuelist = realloc(valuelist, strlen(valuelist) + strlen(temp) + 1);
                strcat(valuelist, strdup(temp));
                free(temp);

                if (lastwasop)
                {
                    error("invalid syntax at '*'", num);
                    exit(1);
                }

                operslist = realloc(operslist, strlen(operslist) + 10 + strlen(INSTRUCTION_END) + 1);

                char * tmp = strdup(operslist);

                memset(operslist, 0, strlen(operslist));
                strcpy(operslist, "BINARY_MUL");
                strcat(operslist, INSTRUCTION_END);
                strcat(operslist, tmp);

                free(tmp);

                lastwasop = 1;
            }
            else if (!strcmp(line[i], "/"))
            {
                char * temp = quokka_compile_line(latestvalue, num, -1, 1);
                valuelist = realloc(valuelist, strlen(valuelist) + strlen(temp) + 1);
                strcat(valuelist, strdup(temp));
                free(temp);

                if (lastwasop)
                {
                    error("invalid syntax at '/'", num);
                    exit(1);
                }

                operslist = realloc(operslist, strlen(operslist) + 10 + strlen(INSTRUCTION_END) + 1);

                char * tmp = strdup(operslist);

                memset(operslist, 0, strlen(operslist));
                strcpy(operslist, "BINARY_DIV");
                strcat(operslist, INSTRUCTION_END);
                strcat(operslist, tmp);

                free(tmp);

                lastwasop = 1;
            }
            else
            {
                if (lastwasop)
                {
                    latestvalue = realloc(latestvalue, strlen(line[i]) + 1 + 1);
                    memset(latestvalue, 0, strlen(latestvalue) + 1);
                    strcpy(latestvalue, line[i]);
                    strcat(latestvalue, " ");
                }
                else
                {
                    latestvalue = realloc(latestvalue, strlen(line[i]) + 1 + 1);
                    strcat(latestvalue, line[i]);
                    strcat(latestvalue, " ");
                }

                lastwasop = 0;
            }
        }

        char * temp = quokka_compile_line(latestvalue, num, -1, 1);
        valuelist = realloc(valuelist, strlen(valuelist) + strlen(temp) + 1);
        strcat(valuelist, strndup(temp, strlen(temp)));
        free(temp);

        strcat(bytecode, strdup(valuelist));
        strcat(bytecode, strdup(operslist));

        free(valuelist);
        free(operslist);
    }
    else if (isidentifier(line[0]) && startswith(line[1], "(") && endswith(line[1], ")"))
    {
        if (len > 2)
        {
            error("invalid syntax", num);
            exit(1);
        }

        strcat(bytecode, "LOAD_NAME");
        strcat(bytecode, SEPARATOR);
        strcat(bytecode, line[0]);
        strcat(bytecode, INSTRUCTION_END);

        char * temp = quokka_compile_line(__slice_string__(String(line[1]), 1, 1).value, num, -1, 1);

        strcat(bytecode, strndup(temp, strlen(temp)));

        strcat(bytecode, "CALL_FUNCTION");
        strcat(bytecode, SEPARATOR);
        strcat(bytecode, String(stringCount(line, ",") + 1).value);
        strcat(bytecode, INSTRUCTION_END);

        free(temp);
    }
    else if (isinteger(line[0]) && len == 1)
    {
        // Clear leading 0's on integers
        while (startswith(line[0], "0") && strlen(line[0]) > 1)
            line[0]++;

        strcat(bytecode, "LOAD_INT");
        strcat(bytecode, SEPARATOR);
        strcat(bytecode, line[0]);
        strcat(bytecode, INSTRUCTION_END);
    }
    else if ((
        (startswith(line[0], "'") && endswith(line[0], "'")) ||
        (startswith(line[0], "\"") && endswith(line[0], "\""))
    ) && len == 1)
    {
        strcat(bytecode, "LOAD_STRING");
        strcat(bytecode, SEPARATOR);
        strcat(bytecode, line[0]);
        strcat(bytecode, INSTRUCTION_END);
    }
    else if (isidentifier(line[0]) && len == 1)
    {
        strcat(bytecode, "LOAD_NAME");
        strcat(bytecode, SEPARATOR);
        strcat(bytecode, line[0]);
        strcat(bytecode, INSTRUCTION_END);
    }
    else
    {
        if (isInline)
        {
            error("invalid syntax", num);
            exit(1);
        }

        int lastwascomma = 0;

        for (int p = 0; p < len; p++)
        {
            if (!strcmp(line[p], ","))
            {
                lastwascomma = 1;
                continue;
            }

            if (!lastwascomma && p > 0)
            {
                error("invalid syntax, expression is missing commas for value separation", num);
                break;
            }

            if (!strlen(line[p]))
                break;

            char * temp = quokka_compile_line(line[p], num, 1, 1);

            strcat(bytecode, strndup(temp, strlen(temp)));

            free(temp);

            lastwascomma = 0;
        }
    }

    bytecode = realloc(bytecode, strlen(bytecode) + 1);
    return bytecode;
}

char * quokka_compile_tokens(char ** tokens, int isInline)
{
    // This will eventually need to be fixed and made dynamic
    char * compiled = malloc(8192);
    strcpy(compiled, "");

    int size = arrsize(tokens);
    for (int i = 0; i < size; i++)
    {
        if (tokens[i] == NULL)
            continue;

        char * ins = quokka_compile_line(tokens[i], i, -1, isInline);

        if (strlen(ins))
        {
            strcat(compiled, String(i + 1).value);
            strcat(compiled, INSTRUCTION_END);

            strcat(compiled, ins);
        }
    }

    free(tokens);

    return compiled;
}

char * quokka_compile_raw(char * rawtext, int maxtokensize, int isInline)
{
    if (maxtokensize == -1)
        maxtokensize = 2048;
    char ** tokens = malloc(maxtokensize + 1);
    ntokenise(tokens, rawtext, "\n");

    char * res = quokka_compile_tokens(tokens, isInline);

    return res;
}

char * quokka_compile_fname(char * filename)
{
    char * buffer = readfile(filename);

    if (!buffer)
        return 0;

    char * res = quokka_compile_raw(buffer, -1, 0);

    return res;
}

char ** quokka_line_tok(char * line)
{
    line = cpstrip(line);

    char * tokenstr = malloc(1);
    strcpy(tokenstr, "");

    // Bools
    int sq = 0; // Single-Quote = false
    int dq = 0; // Double-Quote = false
    int bt = 0; // Backtick = false

    // Ints
    int rb = 0; // Regular-bracket = 0
    int sb = 0; // Square-bracket = 0
    int cb = 0; // Curly-bracket = 0

    // Used to identify a `//` comment
    int comment = 0; // 1 if `/` found, else 0    

    // Chars
    char q;
    char p = '\0';
    char t = 0;

    // Strings
    char * separator = "\n";

    // Set initial char type
    if (isalnum(line[0]))
        p = 'A';
    else if (isdigit(line[0]))
        p = 'D';
    else if (ispunct(line[0]))
        p = 'S';
    else if (isspace(line[0]))
        p = 'W';

    int len = strlen(line);
    for (int i = 0; i < len; i++)
    {
        q = p;
        char c = line[i];
        if (isalpha(c))
            p = 'A'; // Alphabet
        else if (isdigit(c))
            p = 'D'; // Digit
        else if (ispunct(c))
            p = 'S'; // Symbol
        else if (isspace(c))
            p = 'W'; // Whitespace
        if (((q != p && p != 'W') || p == 'S') && !(
            sq || dq || bt || rb > 0 || sb > 0 || cb > 0
        ) && !(
            (t == '-' || t == '+') && p == 'D' // Join pluses/minuses with digits, -10, +5
        ) && !(
            t == '-' && c == '-' // Join minuses together --, ---, etc
        ) && !(
            t == '+' && c == '+' // Join pluses together ++, +++, etc
        ) && !(
            (t == '-' || t == '+' || t == '*' || t == '/') && c == '=' // Join together operators: -= += *= /=
        ) && !(
            q == 'A' && c == '.' // Join together names like `word.upper` (second part is below)
        ) && !(
            p == 'A' && t == '.' // Second part to the line above.
        ) && !(
            t == '_' && (p == 'A' || c == '.') // Join together names like `string_one` (second part is below)
        ) && !(
            (q == 'A' || t == '.') && c == '_' // Second part to the line above.
        ) && !(
            t == '_' && c == '_' // Join together double underscores `__`
        ) && !(
            q == 'A' && p == 'D' // Join alphabetical and numerical characters.
        ) && !(
            p == 'A' && q == 'D' // Second part to the line above.
        ) && !(
            t == '.' && c == '.' // Join together all `.` tokens.
        ))
        {
            tokenstr = realloc(tokenstr, strlen(tokenstr) + strlen(separator) + 1);
            strncat(tokenstr, separator, strlen(separator));
        }
        if (c == '\'' && !(
            dq || rb > 0 || sb > 0 || cb > 0))
            sq = !sq;
        else if (c == '"' && !(
            sq || bt || rb > 0 || sb > 0 || cb > 0))
            dq = !dq;
        else if (c == '`' && !(
            sq || dq || rb > 0 || sb > 0 || cb > 0))
            bt = !bt;
        else if (c == '(' && !(
            sq || dq || bt || sb > 0 || cb > 0))
            rb++;
        else if (c == ')' && !(
            sq || dq || bt || sb > 0 || cb > 0))
            rb--;
        else if (c == '/' && !(
            sq || dq || bt))
        {
            comment++;
            if (comment >= 2)
            {
                tokenstr[strlen(tokenstr) - 2] = '\0';
                break;
            }
        }
        if (comment <= 1)
        {
            if (comment && c != '/')
                comment = 0;
            tokenstr = realloc(tokenstr, strlen(tokenstr) + 2);
            strncat(tokenstr, &c, 1);
        }
        t = c;
    }

    char ** output = (char **)malloc(1024 * sizeof(char *));
    output[0] = "\0";
    tokenise(output, tokenstr, separator);
    for (int p = 0; p < arrsize(output); p++)
        output[p] = cpstrip(output[p]);

    return output;
}

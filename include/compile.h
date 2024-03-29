// Definitions
#define SEPARATOR " "
#define INSTRUCTION_END "\n"

#define SEPARATOR_LEN 1
#define INSTRUCTION_END_LEN 1

// Trash
void ** trash;
int trashsize;
int trash_alloc;
int trash_alloc_size = 10;

// Bytecode Constants
char * bytecode_constants;
int bytecode_constant_count;

// Ints
int current_line;
int scope;
int file_token_index;
int file_line_count;

// String arrays
char ** file_tokens;

// Scope stack stuff
char ** scpstk;
int * scps;
int * scplines;
int scpstk_size;

// Function declarations
int isidentifier(char * word);
int isinteger(char * word);
int islong(char * word);
int stringInList(char ** arr, char * key, int len);
void arrlstrip(char ** line);
int stringCount(char ** lst, char * st);
int stringCountUntil(char ** lst, char * st, int len);

char * quokka_compile_line(char * linetext, int num, int lineLen, int isInline);
char * quokka_compile_line_tokens(char ** line, int num, int lineLen, int isInline);
char * quokka_compile_tokens(char ** tokens, int isInline);
char * quokka_compile_raw(char * rawtext, int isInline);
char * quokka_compile_fname(char * filename, int isInitFile);

char ** quokka_file_tok(char * text);
char ** quokka_tok(char * line, char ** waste);
char *** quokka_bc_file_tok(char * text);
char ** quokka_bc_tok(char * line);

void error(char * text, int line)
{
    int MAXLINE = 128;

    char * fullfile;

    if (in_cli_mode)
    {
        fullfile = cli_current_line;
        resetStack();
    }
    else
        fullfile = readfile(current_file);

    char * lineprev = malloc(MAXLINE);
    char * linemid = malloc(MAXLINE);
    char * linenext = malloc(MAXLINE);

    strcpy(lineprev, "");
    strcpy(linemid, "");
    strcpy(linenext, "");

    int c = 0;
    for (int i = 0; i < strlen(fullfile); i++)
    {
        if (fullfile[i] == '\n')
            c++;
        else if (c == line - 1 && strlen(lineprev) < MAXLINE && fullfile[i] != 10 && fullfile[i] != 13)
            strncat(lineprev, &fullfile[i], 1);
        else if (c == line && strlen(linemid) < MAXLINE && fullfile[i] != 10 && fullfile[i] != 13)
            strncat(linemid, &fullfile[i], 1);
        else if (c == line + 1 && strlen(linenext) < MAXLINE && fullfile[i] != 10 && fullfile[i] != 13)
            strncat(linenext, &fullfile[i], 1);
        if (c > line + 1)
            break;
    }

    line++;

    char * num_lineprev = intToStr(line - 1);
    char * num_linemid = intToStr(line);
    char * num_linenext = intToStr(line + 1);
    int pad = strlen(num_linenext);

    if (strlen(num_linemid) < pad)
        num_linemid = lpadfree(num_linemid, pad, ' ');

    if (strlen(num_lineprev) < pad)
        num_lineprev = lpadfree(num_lineprev, pad, ' ');

    println("\nProgram execution terminated:\n");

    printf("At %s : Line %d\n\n", current_file, line);

    if (line - 1 > 0)
        printf("  %s | %s\n", num_lineprev, lineprev);
    printf("> %s | %s\n", num_linemid, linemid);
    if (line < charCount(fullfile, '\n') + 1)
        printf("  %s | %s\n", num_linenext, linenext);

    printf("\nError: %s\n\n", text);

    if (!in_cli_mode)
        free(fullfile);

    free(lineprev);
    free(linemid);
    free(linenext);

    free(num_lineprev);
    free(num_linemid);
    free(num_linenext);

    // Exit and free the stack (if not in CLI mode)
    if (!in_cli_mode)
    {
        cleanupAll();
        exit(1);
    }
}

// Trash
void emptyTrash()
{
    for (int i = 0; i < trashsize; i++)
        free(trash[i]);

    free(trash);
    trashsize = 0;
}

void resetTrash()
{
    trash_alloc = trash_alloc_size;
    trash = malloc(trash_alloc * sizeof(void *));
    trashsize = 0;
}

// DO NOT pass an un-malloc'd pointer to this function
void pushTrash(void * ptr)
{
    if (++trashsize >= trash_alloc)
    {
        trash_alloc += trash_alloc_size;
        trash = realloc(trash, trash_alloc * sizeof(void *));
    }

    trash[trashsize - 1] = ptr;
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

// Returns the base of the integer if the given string is in
// fact an integer, otherwise returns 0.
int isinteger(char * word)
{
    int base;
    int size = strlen(word);

    if (!size)
        return 0;

    if (startswith(word, "b") || startswith(word, "B"))
        base = 2;
    else if (startswith(word, "0x") || startswith(word, "0X"))
        base = 16;
    else if (isdigit(*word))
        base = 10;
    else
        return 0;

    int i = 0;

    // Start from a different position if the number begins with any characters
    // used to identify it (b, 0x)
    if (base == 2)
        i = 1;
    else if (base == 16)
        i = 2;

    for (; i < size; i++)
    {
        if (base == 2)
            if (word[i] != '0' && word[i] != '1')
                return 0;
        else if (base == 10)
            if (!isdigit(word[i]))
                return 0;
        else if (base == 16)
        {
            char c = tolower(word[i]);
            if (!isdigit(word[i])
                // Outside the bounds of 'a' and 'f'
                || (c < 'a' && c > 'f'))
                return 0;
        }
    }

    return base;
}

int islong(char * word)
{
    int size = strlen(word);

    if (size <= 1)
        return 0;

    if (word[size - 1] != 'l' && word[size - 1] != 'L')
        return 0;

    for (int i = 0; i < size - 1; i++)
        if (!isdigit(word[i]))
            return 0;

    return 1;
}

int stringInList(char ** arr, char * key, int len)
{
    // If len is -1, make len INT_MAX to prevent
    // the loop from stopping at a fixed length
    if (len == -1)
        len = INT_MAX;

    for (int n = 0; arr[n] != NULL && n < len; n++)
    {
        if (!strcmp(arr[n], key))
            return 1;
    }

    return 0;
}

void arrlstrip(char ** line)
{
    int c;
    for (c = 0; line[c + 1] != NULL; c++)
        line[c] = line[c + 1];

    line[c] = "";
}

void arrdelindex(char ** line, int index)
{
    int c;
    for (c = index; line[c + 1] != NULL; c++)
        line[c] = line[c + 1];

    line[c] = "";
}

int stringCount(char ** lst, char * st)
{
    int count = 0;

    for (int i = 0; lst[i] != NULL; i++)
    {
        if (!strcmp(lst[i], st))
            count++;
    }

    return count;
}

int stringCountUntil(char ** lst, char * st, int len)
{
    int count = 0;

    for (int i = 0; i < len && lst[i] != NULL; i++)
    {
        if (!strcmp(lst[i], st))
            count++;
    }

    return count;
}

void scpstkPush(char * kw, int scp, int linenum)
{
    scpstk = realloc(scpstk, (scpstk_size + 1) * sizeof(char *));
    scpstk[scpstk_size] = kw;

    scps = realloc(scps, (scpstk_size + 1) * sizeof(int));
    scps[scpstk_size] = scp;

    scplines = realloc(scplines, (scpstk_size + 1) * sizeof(int));
    scplines[scpstk_size] = linenum;

    scpstk_size++;
}

void scpstkPop()
{
    scpstk = realloc(scpstk, (scpstk_size + 1) * sizeof(char *));
    scps = realloc(scps, (scpstk_size + 1) * sizeof(int));
    scplines = realloc(scplines, (scpstk_size + 1) * sizeof(int));

    scpstk_size--;
}

// Expand a square bracket bytecode expression into usable bytecode
char * expandSBrackExp(char * orig)
{
    int len = strlen(orig);
    char * expanded = malloc(len - 2 + 1); // `- 2` for `[]`, and `+ 1` for null byte

    int sb = 0; // Square-bracket = 0

    for (int i = 0; i < len - 2; i++)
    {
        expanded[i] = orig[i + 1];

        if      (expanded[i] == '[')
            sb++;
        else if (expanded[i] == ']')
            sb--;
        else if (expanded[i] == '\t' && !sb)
            expanded[i] = '\n';
    }

    expanded[len - 2] = 0;
    return expanded;
}

int findNextIfChain(char * kwtype, int cur_line, int cur_tok_index, int scp)
{
    // Index will be -1 if 'end' not found
    int ind = 0;
    int tempscope = scp - 1; // -1 because this keyword itself will increase the scope
    int blanks = 0; // Num of consecutive blank lines

    // 0 = 'if'
    // 1 = 'elif'
    // 2 = 'else'
    int kw;

    if (!strcmp(kwtype, "if"))
        kw = 0;
    else if (!strcmp(kwtype, "elif"))
    {
        tempscope++; // elif will not increase scope
        kw = 1;
    }
    else if (!strcmp(kwtype, "else"))
    {
        tempscope++; // else will not increase scope
        kw = 2;
    }

    int real_line = cur_line;

    for (int i = cur_tok_index; i < file_line_count; i++)
    {
        char * waste;
        char ** templine = quokka_tok(file_tokens[i], &waste);

        if (file_tokens[i][0] == '\n' || !file_tokens[i][0])
            real_line++;

        if (templine[0] == NULL)
        {
            free(waste);
            free(templine);
            continue;
        }

        // Error checking
        if (tempscope == scp && i > cur_tok_index)
        {
            if (kw == 1 && !strcmp(templine[0], "if"))
            {
                error("if statement after elif statement in the same if chain",
                    current_line);
            }
            else if (kw == 2 && !strcmp(templine[0], "if"))
            {
                error("if statement after else statement in the same if chain",
                    current_line);
            }
            else if (kw == 2 && !strcmp(templine[0], "elif"))
            {
                error("elif statement after else statement in the same if chain",
                    current_line);
            }
            else if (kw == 2 && !strcmp(templine[0], "else"))
            {
                error("else statement on the end of another else statement",
                    current_line);
            }
        }

        // Regular scope checking and all the rest
        if (!strcmp(templine[0], "if") ||
            !strcmp(templine[0], "while") ||
            !strcmp(templine[0], "until") ||
            !strcmp(templine[0], "fun") ||
            !strcmp(templine[0], "for"))
        {
            tempscope++;
        }
        else
        {
            if (!strcmp(templine[0], "end") ||
               (!strcmp(templine[0], "elif") && (kw == 0 || kw == 1)) ||
               (!strcmp(templine[0], "else") && (kw == 0 || kw == 1)))
            {
                if (tempscope == scp && i > cur_tok_index)
                {
                    ind = real_line;

                    free(waste);
                    free(templine);
                    break;
                }
            }
            if (!strcmp(templine[0], "end"))
                tempscope--;
        }

        free(waste);
        free(templine);

        // Update line number
        real_line += charCount(file_tokens[i], '\n');
        if (file_tokens[i][0] == '\n')
            real_line--;
    }

    return ind - 1;
}

int findNextEnd(char * kwtype, int cur_line, int cur_tok_index, int scp)
{
    // Index will be -1 if 'end' not found
    int ind = 0;
    int tempscope = scp - 1;
    int blanks = 0; // Num of consecutive blank lines

    if (!strcmp(kwtype, "elif") || !strcmp(kwtype, "else"))
        tempscope++; // elif and else will not increase scope

    int real_line = cur_line;

    for (int i = cur_tok_index; file_tokens[i] != NULL; i++)
    {
        char * waste;
        char ** templine = quokka_tok(file_tokens[i], &waste);

        if (file_tokens[i][0] == '\n' || !file_tokens[i][0] || i == cur_tok_index)
            real_line++;

        if (templine[0] == NULL)
        {
            free(waste);
            free(templine);
            continue;
        }

        if (!strcmp(templine[0], "if") ||
            !strcmp(templine[0], "while") ||
            !strcmp(templine[0], "until") ||
            !strcmp(templine[0], "fun") ||
            !strcmp(templine[0], "for"))
        {
            tempscope++;
        }
        else if (!strcmp(templine[0], "end"))
        {
            if (tempscope == scp && i > cur_tok_index)
            {
                ind = real_line;

                free(waste);
                free(templine);
                break;
            }
            else tempscope--;
        }

        free(waste);
        free(templine);

        // Update line number
        real_line += charCount(file_tokens[i], '\n');
        if (file_tokens[i][0] == '\n')
            real_line--;
    }

    return ind - 1;
}

int compile_comma_list(char *** outptr, char ** comma_list)
{
    outptr[0] = comma_list;

    int templen = arrsize(outptr[0]);
    int lastwascomma = 0;
    for (int i = 0; i < templen; i++)
    {
        if (!strcmp(outptr[0][i], ","))
        {
            if (lastwascomma)
            {
                arrdelindex(outptr[0], i);
                templen--;
            }
            lastwascomma = 1;
        }
        else lastwascomma = 0;
    }
    if (templen)
        while (!strcmp(outptr[0][templen - 1], ","))
            templen--;

    return templen;
}

int compile_comma_list_string(char *** outptr, char * comma_string)
{
    char * waste;
    char ** comma_list = quokka_tok(comma_string, &waste);

    int templen = compile_comma_list(outptr, comma_list);

    pushTrash(waste);

    return templen;
}

char * compile_inplace_assignment(
    char * inplace_instruction,
    char * symbol_token,
    char ** line,
    int len,
    int num)
{
    char * res = malloc(1);
    strcpy(res, "");

    // variable[index] = value
    if (startswith(line[1], "[") && endswith(line[1], "]"))
    {
        if (strcmp(line[2], symbol_token))
            error("invalid syntax", num - 1);

        if (len < 4)
            error("variable definition missing variable value", num - 1);

        char * sliced = strSlice(line[1], 1, 1);
        char * ind = quokka_compile_line(sliced, num, -1, 1);

        mstrcatline(&res,
            "LOAD_NAME",
            SEPARATOR,
            line[0],
            INSTRUCTION_END);

        mstrcat(&res, ind);

        free(sliced);
        free(ind);

        arrlstrip(line);
        arrlstrip(line);
        arrlstrip(line);
        len -= 3;

        char * temp = quokka_compile_line_tokens(line, num, len, 1);
        mstrcat(&res, temp);
        free(temp);

        mstrcattrip(&res,
            "SET_INDEX",
            INSTRUCTION_END);
    }
    // variable = value
    else
    {
        if (strcmp(line[1], symbol_token))
            error("invalid syntax", num - 1);

        if (len < 3)
            error("variable definition missing variable value", num - 1);

        char * varname = strdup(line[0]);

        arrlstrip(line);
        arrlstrip(line);
        len -= 2;
        char * temp = quokka_compile_line_tokens(line, num, len, 1);

        mstrcat(&res, temp);

        free(temp);

        mstrcatline(&res,
            inplace_instruction,
            SEPARATOR,
            varname,
            INSTRUCTION_END);

        free(varname);
    }

    return res;
}

// Returns the index of the newly added bytecode constant
int addBytecodeConstant(char * const_type, char * literal_value)
{
    int len_const_type = strlen(const_type);
    int len_literal_value = strlen(literal_value);

    bytecode_constants = realloc(bytecode_constants,
        strlen(bytecode_constants) + len_const_type + SEPARATOR_LEN
        + len_literal_value + INSTRUCTION_END_LEN + 1);

    strcat(bytecode_constants, const_type);
    strcat(bytecode_constants, SEPARATOR);
    strcat(bytecode_constants, literal_value);
    strcat(bytecode_constants, INSTRUCTION_END);

    return bytecode_constant_count++;
}

void set_bytecode_constants()
{
    /*

    NEVER REORDER THIS SECTION, CONSTANTS MUST
    STAY IN THE SAME ORDER.

    Only append to the end of this list.

    */
    bytecode_constants = malloc(
        9 + INSTRUCTION_END_LEN +
        11 + SEPARATOR_LEN + 2 + INSTRUCTION_END_LEN + 1);
    bytecode_constants[0] = 0;

    // Integers are no longer included in bytecode constants because
    // integers 0 through to 256 are created during program execution
    strcat(bytecode_constants, "LOAD_NULL" INSTRUCTION_END);
    // strcat(bytecode_constants, INSTRUCTION_END);

    strcat(bytecode_constants, "LOAD_STRING" SEPARATOR "''" INSTRUCTION_END);
    // strcat(bytecode_constants, SEPARATOR);
    // strcat(bytecode_constants, "''");
    // strcat(bytecode_constants, INSTRUCTION_END);

    // Remember to update this number to match the
    // constant count that the program begins with
    bytecode_constant_count = 2;
}

void compile_init()
{
    current_line = 0;

    free(scpstk);
    free(scps);
    free(scplines);

    scpstk = NULL;
    scps = NULL;
    scplines = NULL;
    scpstk_size = 0;

    scope = 0;
}

char * quokka_compile_line(char * linetext, int num, int lineLen, int isInline)
{
    char * waste;
    char ** line = quokka_tok(linetext, &waste);

    char * ret = quokka_compile_line_tokens(line, num, lineLen, isInline);

    free(waste);
    free(line);

    return ret;
}

char * quokka_compile_line_tokens(char ** line, int num, int lineLen, int isInline)
{
    char * bytecode = malloc(1);
    bytecode[0] = '\0';
    //strcpy(bytecode, "");

    int len;
    if (lineLen == -1)
        len = arrsize(line);
    else len = lineLen;

    if (!len)
        return bytecode;
    if (line[0] == NULL)
        return bytecode;
    if (!strlen(line[0]))
        return bytecode;

    if (len < 2 && line[1] == NULL)
        line[1] = "";
    if (len < 3 && line[2] == NULL)
        line[2] = "";

    if (verbose) println(line[0]);

    // The current line number as a char pointer
    char * str_line_num = intToStr(current_line);

    if (stringInList(line, "=", len))
    {
        // Set new line
        mstrcattrip(&bytecode, str_line_num, INSTRUCTION_END);

        if (isInline)
            error("variables must be defined at the start of a line", num - 1);

        if (!isidentifier(line[0]))
            error("variable name to assign must be a valid identifier", num - 1);

        // variable[index] = value
        if (startswith(line[1], "[") && endswith(line[1], "]"))
        {
            if (strcmp(line[2], "="))
                error("invalid syntax", num - 1);

            if (len < 4)
                error("variable definition missing variable value", num - 1);

            char * sliced = strSlice(line[1], 1, 1);
            char * ind = quokka_compile_line(sliced, num, -1, 1);

            mstrcatline(&bytecode,
                "LOAD_NAME",
                SEPARATOR,
                line[0],
                INSTRUCTION_END);

            mstrcat(&bytecode, ind);

            free(sliced);
            free(ind);

            arrlstrip(line);
            arrlstrip(line);
            arrlstrip(line);
            len -= 3;

            char * temp = quokka_compile_line_tokens(line, num, len, 1);
            mstrcat(&bytecode, temp);
            free(temp);

            mstrcattrip(&bytecode,
                "SET_INDEX",
                INSTRUCTION_END);
        }
        // variable = value
        else
        {
            if (strcmp(line[1], "="))
                error("invalid syntax", num - 1);

            if (len < 3)
                error("variable definition missing variable value", num - 1);

            char * varname = strndup(line[0], strlen(line[0]));

            arrlstrip(line);
            arrlstrip(line);
            len -= 2;
            char * temp = quokka_compile_line_tokens(line, num, len, 1);

            mstrcat(&bytecode, temp);

            free(temp);

            mstrcat(&bytecode, "STORE_NAME");
            mstrcat(&bytecode, SEPARATOR);
            mstrcat(&bytecode, varname);
            mstrcat(&bytecode, INSTRUCTION_END);

            free(varname);
        }
    }
    else if (stringInList(line, "+=", len))
    {
        // Set new line
        mstrcattrip(&bytecode, str_line_num, INSTRUCTION_END);

        if (isInline)
            error("variables must be defined at the start of a line", num - 1);

        if (!isidentifier(line[0]))
            error("variable name to assign must be a valid identifier", num - 1);

        char * temp = compile_inplace_assignment("INPLACE_ADD", "+=", line, len, num);
        mstrcat(&bytecode, temp);
        free(temp);
    }
    else if (stringInList(line, "-=", len))
    {
        // Set new line
        mstrcattrip(&bytecode, str_line_num, INSTRUCTION_END);

        if (isInline)
            error("variables must be defined at the start of a line", num - 1);

        if (!isidentifier(line[0]))
            error("variable name to assign must be a valid identifier", num - 1);

        char * temp = compile_inplace_assignment("INPLACE_SUB", "-=", line, len, num);
        mstrcat(&bytecode, temp);
        free(temp);
    }
    else if (stringInList(line, "*=", len))
    {
        // Set new line
        mstrcattrip(&bytecode, str_line_num, INSTRUCTION_END);

        if (isInline)
            error("variables must be defined at the start of a line", num - 1);

        if (!isidentifier(line[0]))
            error("variable name to assign must be a valid identifier", num - 1);

        char * temp = compile_inplace_assignment("INPLACE_MUL", "*=", line, len, num);
        mstrcat(&bytecode, temp);
        free(temp);
    }
    else if (stringInList(line, "/=", len))
    {
        // Set new line
        mstrcattrip(&bytecode, str_line_num, INSTRUCTION_END);

        if (isInline)
            error("variables must be defined at the start of a line", num - 1);

        if (!isidentifier(line[0]))
            error("variable name to assign must be a valid identifier", num - 1);

        char * temp = compile_inplace_assignment("INPLACE_MUL", "/=", line, len, num);
        mstrcat(&bytecode, temp);
        free(temp);
    }
    else if (!strcmp(line[0], "while"))
    {
        if (isInline)
            error("while loop can only be at the start of a line", num - 1);
        if (len <= 1)
            error("while loop requires a condition", num - 1);

        // Set new line
        mstrcattrip(&bytecode, str_line_num, INSTRUCTION_END);

        arrlstrip(line);
        len--;

        char * temp = quokka_compile_line_tokens(line, num, len, 1);

        mstrcat(&bytecode, temp);

        free(temp);

        int next = findNextEnd("while", num, file_token_index, scope);
        if (next < 0)
            error("while loop missing 'end' keyword", num - 1);

        mstrcat(&bytecode, "JUMP_IF_FALSE");
        mstrcat(&bytecode, SEPARATOR);

        char * intstr = intToStr(next);
        mstrcat(&bytecode, intstr);
        free(intstr);

        mstrcat(&bytecode, INSTRUCTION_END);

        scpstkPush("while", scope, current_line);

        scope++;
    }
    else if (!strcmp(line[0], "until"))
    {
        if (isInline)
            error("until loop can only be at the start of a line", num - 1);
        if (len <= 1)
            error("until loop requires a condition", num - 1);

        // Set new line
        mstrcattrip(&bytecode, str_line_num, INSTRUCTION_END);

        arrlstrip(line);
        len--;

        char * temp = quokka_compile_line_tokens(line, num, len, 1);

        mstrcat(&bytecode, temp);

        free(temp);

        int next = findNextEnd("until", num, file_token_index, scope);
        if (next < 0)
            error("until loop missing 'end' keyword", num - 1);

        mstrcat(&bytecode, "JUMP_IF_TRUE");
        mstrcat(&bytecode, SEPARATOR);

        char * intstr = intToStr(next);
        mstrcat(&bytecode, intstr);
        free(intstr);

        mstrcat(&bytecode, INSTRUCTION_END);

        scpstkPush("until", scope, current_line);

        scope++;
    }
    else if (!strcmp(line[0], "if"))
    {
        if (isInline)
            error("if statement can only be at the start of a line", num - 1);
        if (len <= 1)
            error("if statement requires a condition", num - 1);

        // Set new line
        mstrcattrip(&bytecode, str_line_num, INSTRUCTION_END);

        arrlstrip(line);
        len--;

        char * temp = quokka_compile_line_tokens(line, num, len, 1);

        mstrcat(&bytecode, temp);

        free(temp);

        int next = findNextIfChain("if", num, file_token_index, scope);
        if (next < 0)
            error("if statement missing 'end' keyword", num - 1);

        mstrcat(&bytecode, "JUMP_IF_FALSE");
        mstrcat(&bytecode, SEPARATOR);

        char * intstr = intToStr(next);
        mstrcat(&bytecode, intstr);
        free(intstr);

        mstrcat(&bytecode, INSTRUCTION_END);

        scope++;
    }
    else if (!strcmp(line[0], "elif"))
    {
        if (isInline)
            error("elif statement can only be at the start of a line", num - 1);
        if (len <= 1)
            error("elif statement requires a condition", num - 1);

        int nextend = findNextEnd("elif", num, file_token_index, scope);
        if (nextend < 0)
            error("elif statement missing 'end' keyword", num - 1);

        mstrcat(&bytecode, "JUMP_TO");
        mstrcat(&bytecode, SEPARATOR);

        char * intstr = intToStr(nextend);
        mstrcat(&bytecode, intstr);
        free(intstr);

        mstrcat(&bytecode, INSTRUCTION_END);

        // Set new line
        mstrcattrip(&bytecode, str_line_num, INSTRUCTION_END);

        arrlstrip(line);
        len--;

        char * temp = quokka_compile_line_tokens(line, num, len, 1);

        mstrcat(&bytecode, temp);

        free(temp);

        int next = findNextIfChain("elif", num, file_token_index, scope);
        if (next < 0)
            error("elif statement missing 'end' keyword", num - 1);

        mstrcat(&bytecode, "JUMP_IF_FALSE");
        mstrcat(&bytecode, SEPARATOR);

        intstr = intToStr(next);
        mstrcat(&bytecode, intstr);
        free(intstr);

        mstrcat(&bytecode, INSTRUCTION_END);
    }
    else if (!strcmp(line[0], "else"))
    {
        if (isInline)
            error("else statement can only be at the start of a line", num - 1);
        if (len > 1)
            error("else statement does not take arguments", num - 1);

        int nextend = findNextEnd("else", num, file_token_index, scope);
        if (nextend == -1)
            error("else statement missing 'end' keyword", num - 1);

        mstrcat(&bytecode, "JUMP_TO");
        mstrcat(&bytecode, SEPARATOR);

        char * intstr = intToStr(nextend);
        mstrcat(&bytecode, intstr);
        free(intstr);

        mstrcat(&bytecode, INSTRUCTION_END);

        // Set new line
        mstrcattrip(&bytecode, str_line_num, INSTRUCTION_END);
    }
    else if (!strcmp(line[0], "end"))
    {
        if (isInline)
            error("'end' can only be at the start of a line", num - 1);

        if (len > 1)
            error("'end' does not take arguments", num - 1);

        if (scpstk_size)
        {
            char * kw = scpstk[scpstk_size - 1];
            int scp = scps[scpstk_size - 1];
            int scpline = scplines[scpstk_size - 1];

            if (scope - 1 == scp)
            {
                if (!strcmp(kw, "while") || !strcmp(kw, "until"))
                {
                    mstrcat(&bytecode, "JUMP_BACK");
                    mstrcat(&bytecode, SEPARATOR);

                    char * intstr = intToStr(scpline);
                    mstrcat(&bytecode, intstr);
                    free(intstr);

                    mstrcat(&bytecode, INSTRUCTION_END);

                    scpstkPop();
                }
            }
        }

        // Set new line
        mstrcattrip(&bytecode, str_line_num, INSTRUCTION_END);

        scope--;
    }
    else if (!strcmp(line[0], "fun"))
    {
        if (isInline)
            error("function definition can only be at the start of a line", num - 1);
        if (len <= 1)
            error("function definition requires at least a function name", num - 1);
        if (len > 4)
            error("function definition received too many arguments", num - 1);

        // Set new line
        mstrcattrip(&bytecode, str_line_num, INSTRUCTION_END);

        int next = findNextEnd("fun", num, file_token_index, scope);
        if (next < 0)
            error("function definition missing 'end' keyword", num - 1);

        char * funcname = line[1];
        int argmin = 0;
        int argmax = 0;

        if (len > 3)
        {
            if (!strcmp(line[2], "*"))
                argmin = 0;
            else
                argmin = strtol(line[2], NULL, 10);

            if (!strcmp(line[3], "*"))
                argmax = -1;
            else
                argmax = strtol(line[3], NULL, 10);
        }
        else if (len > 2)
        {
            if (!strcmp(line[2], "*"))
            {
                argmin = 0;
                argmax = -1;
            }
            else
                argmin = argmax = strtol(line[2], NULL, 10);
        }

        char * funcbytecode = malloc(1);
        strcpy(funcbytecode, "");

        for (file_token_index++; current_line < next; file_token_index++)
        {
            char * t = file_tokens[file_token_index];

            if (t[0] == '\n' || !t[0])
                current_line++;

            char * comp_bc = quokka_compile_line(t, current_line, -1, 0);
            char * bc = strReplace(comp_bc, INSTRUCTION_END, "\t");

            mstrcat(&funcbytecode, bc);

            free(comp_bc);
            free(bc);

            // Update line number
            current_line += charCount(t, '\n');
            if (t[0] == '\n')
                current_line--;
        }

        current_line++;
        num = current_line;

        if (argmin < 0 && argmin != -1)
            error("minimum argument count for function definition can not be below 0", num - 1);
        if (argmax < argmin && argmax != -1)
            error("maximum argument count for function definition can not be below minimum argument count",
                num - 1);

        // Start defining the function
        mstrcatline(&bytecode,
            "DEFINE_FUNCTION",
            SEPARATOR,
            funcname,
            SEPARATOR);

        // Append argmin
        if (argmin == -1)
            mstrcat(&bytecode, "*");
        else
        {
            char * intstr = intToStr(argmin);
            mstrcat(&bytecode, intstr);
            free(intstr);
        }

        // Separator
        mstrcat(&bytecode, SEPARATOR);

        // Append argmax
        if (argmax == -1)
            mstrcat(&bytecode, "*");
        else
        {
            char * intstr = intToStr(argmax);
            mstrcat(&bytecode, intstr);
            free(intstr);
        }

        // Separator
        mstrcat(&bytecode, SEPARATOR);

        // Append function bytecode
        mstrcatline(&bytecode,
            "[",
            funcbytecode,
            "]",
            INSTRUCTION_END);

        free(funcbytecode);

        scope++;
    }
    else if (!strcmp(line[0], "lam"))
    {
        // Set new line
        if (!isInline)
            mstrcattrip(&bytecode, str_line_num, INSTRUCTION_END);

        if (len < 3)
            error("invalid syntax", num - 1);

        int argmin, argmax;

        if (!strcmp(line[1], ":"))
            argmin = argmax = 0;
        else if (!strcmp(line[2], ":"))
        {
            if (!strcmp(line[1], "*"))
            {
                argmin = 0;
                argmax = -1;
            }
            else
                argmin = argmax = strtol(line[1], NULL, 10);

            arrlstrip(line);
            len--;
        }
        else if (!strcmp(line[3], ":"))
        {
            if (!strcmp(line[1], "*"))
                argmin = 0;
            else
                argmin = strtol(line[1], NULL, 10);

            if (!strcmp(line[2], "*"))
                argmax = -1;
            else
                argmax = strtol(line[2], NULL, 10);

            arrlstrip(line);
            arrlstrip(line);
            len -= 2;
        }

        if (argmin < 0 && argmin != -1)
            error("minimum argument count for lambda definition can not be below 0", num - 1);
        if (argmax < argmin && argmax != -1)
            error("maximum argument count for lambda definition can not be below minimum argument count",
                num - 1);

        arrlstrip(line);
        arrlstrip(line);
        len -= 2;

        char * raw_bc = quokka_compile_line_tokens(line, num, len, 1);
        char * lam_bytecode = strReplace(raw_bc, INSTRUCTION_END, "\t");

        free(raw_bc);

        mstrcat(&lam_bytecode, "RETURN\t");

        // Start defining the lambda function
        mstrcattrip(&bytecode,
            "MAKE_LAMBDA",
            SEPARATOR);

        // Append argmin
        if (argmin == -1)
            mstrcat(&bytecode, "*");
        else
        {
            char * intstr = intToStr(argmin);
            mstrcat(&bytecode, intstr);
            free(intstr);
        }

        // Separator
        mstrcat(&bytecode, SEPARATOR);

        // Append argmax
        if (argmax == -1)
            mstrcat(&bytecode, "*");
        else
        {
            char * intstr = intToStr(argmax);
            mstrcat(&bytecode, intstr);
            free(intstr);
        }

        // Separator
        mstrcat(&bytecode, SEPARATOR);

        // Append lambda bytecode
        mstrcatline(&bytecode,
            "[",
            lam_bytecode,
            "]",
            INSTRUCTION_END);

        free(lam_bytecode);
    }
    else if (!strcmp(line[0], "del"))
    {
        if (isInline)
            error("del statement must be at the start of a line", num - 1);

        mstrcattrip(&bytecode, str_line_num, INSTRUCTION_END);

        for (int i = 1; i < len; i++)
        {
            if (!strcmp(line[i], ","))
                continue;

            if (!isidentifier(line[i]))
                error("invalid syntax", num - 1);

            mstrcatline(&bytecode,
                "DEL_VAR",
                SEPARATOR,
                line[i],
                INSTRUCTION_END);
        }
    }
    else if (!strcmp(line[0], "global"))
    {
        if (isInline)
            error("global statement must be at the start of a line", num - 1);

        // Set new line
        mstrcattrip(&bytecode, str_line_num, INSTRUCTION_END);

        for (int i = 1; i < len; i++)
        {
            if (!strcmp(line[i], ","))
                continue;

            if (!isidentifier(line[i]))
                error("invalid syntax", num - 1);

            mstrcatline(&bytecode,
                "MAKE_GLOBAL",
                SEPARATOR,
                line[i],
                INSTRUCTION_END);
        }
    }
    else if (!strcmp(line[0], "ret"))
    {
        if (isInline)
            error("ret statement must be at the start of a line", num - 1);

        // Set new line
        mstrcattrip(&bytecode, str_line_num, INSTRUCTION_END);

        arrlstrip(line);
        len--;

        char * temp = quokka_compile_line_tokens(line, num, len, 1);

        mstrcat(&bytecode, temp);

        free(temp);

        mstrcat(&bytecode, "RETURN");
        mstrcat(&bytecode, INSTRUCTION_END);
    }
    else if (!strcmp(line[0], "import"))
    {
        if (isInline)
            error("import keyword must be at the start of a line", num - 1);

        // Set new line
        mstrcattrip(&bytecode, str_line_num, INSTRUCTION_END);

        // Unpack values?
        int unpack = 0;

        if (!strcmp(line[1], "*"))
            unpack = 1;

        for (int i = 1 + unpack; i < len; i++)
        {
            if (!strcmp(line[i], ","))
                continue;

            char * temp = quokka_compile_line(line[i], num, 1, 1);

            mstrcat(&bytecode, temp);

            free(temp);

            if (unpack)
                mstrcattrip(&bytecode, "IMPORT_UNPACK", INSTRUCTION_END);
            else
                mstrcattrip(&bytecode, "IMPORT", INSTRUCTION_END);
        }
    }
    else if (stringInList(line, ",", len))
    {
        char * latestvalue = malloc(1);
        strcpy(latestvalue, "");

        int lastwascomma = 0;

        for (int p = 0; p < len; p++)
        {
            if (!strlen(line[p]))
                break;

            if (!strcmp(line[p], ","))
            {
                char * temp = quokka_compile_line(latestvalue, num, -1, 1);
                mstrcat(&bytecode, temp);
                free(temp);

                latestvalue = realloc(latestvalue, 1);
                strcpy(latestvalue, "");

                lastwascomma = 1;
            }
            else
            {
                latestvalue = realloc(latestvalue, strlen(latestvalue) + strlen(line[p]) + 1 + 1);
                strcat(latestvalue, line[p]);
                strcat(latestvalue, " ");

                lastwascomma = 0;
            }
        }

        if (strlen(latestvalue))
        {
            char * temp = quokka_compile_line(latestvalue, num, -1, 1);
            mstrcat(&bytecode, temp);
            free(temp);
        }

        free(latestvalue);
    }
    else if (stringInList(line, "?", len))
    {
        // Set new line
        if (!isInline)
            mstrcattrip(&bytecode, str_line_num, INSTRUCTION_END);

        // These statements can look like this:
        // condition ? true : false
        //
        // Or this:
        // condition ? true
        //
        // In the second case, `: null` is silently added to the end,
        // therefore returning null if condition is false.

        // condition ? true : false
        // <LOAD condition>
        // TERNARY_IF [<LOAD true>\t] [<LOAD false>\t]

        // // Count length needed to hold the condition before allocating
        // int condition_len = 0;
        // int first_marker = 0; // Equal to index of first "?"
        // for (; strcmp(line[first_marker], "?") && first_marker < len; first_marker++)
        //     condition_len += strlen(line[first_marker]) + 1;

        // // Allocate condition code string
        // char * condition_code = malloc(condition_len + 1);
        // strcpy(condition_code, "");

        // for (int i = 0; i < first_marker; i++)
        // {
        //     strcat(condition_code, line[i]);
        //     strcat(condition_code, " ");
        // }

        // println(condition_code);

        // Find "?"
        int first_marker;
        for (first_marker = 0; strcmp(line[first_marker], "?") && first_marker < len; first_marker++);

        // Error checking
        if (first_marker == len)
            error("ternary if statement missing `?`, or `?` is out of bounds", num - 1);

        // Add condition_code to bytecode
        char * condition_code = quokka_compile_line_tokens(line, num, first_marker, 1);
        mstrcat(&bytecode, condition_code);
        free(condition_code);

        // Start creating TERNARY_IF statement
        mstrcattrip(&bytecode, "TERNARY_IF", SEPARATOR);

        // Strip the line
        for (int i = 0; i < first_marker + 1; i++)
            arrlstrip(line);
        len -= first_marker + 1;

        // Find ":" if it exists
        int secnd_marker;
        for (secnd_marker = 0; strcmp(line[secnd_marker], ":") && secnd_marker < len; secnd_marker++);

        // Add true code to bytecode
        char * true_code = quokka_compile_line_tokens(line, num, secnd_marker, 1);
        char * true_rep = strReplace(true_code, "\n", "\t");

        mstrcatline(&bytecode,
            "[",
            true_rep,
            "]",
            SEPARATOR);

        free(true_rep);
        free(true_code);

        // Strip the line
        for (int i = 0; i < secnd_marker + 1; i++)
            arrlstrip(line);
        len -= secnd_marker + 1;

        // If there's anything left, add it to the statement
        if (len)
        {
            // Add false code to bytecode
            char * false_code = quokka_compile_line_tokens(line, num, len, 1);
            char * false_rep = strReplace(false_code, "\n", "\t");

            mstrcatline(&bytecode,
                "[",
                false_rep,
                "]",
                INSTRUCTION_END);

            free(false_rep);
            free(false_code);
        }
        else
        {
            // Load null by default if no value for false is given
            mstrcat(&bytecode, "[]" INSTRUCTION_END);
        }
    }
    else if (stringInList(line, "or", len) ||
             stringInList(line, "xor", len) ||
             stringInList(line, "nor", len) ||
             stringInList(line, "and", len))
    {
        // Set new line
        if (!isInline)
            mstrcattrip(&bytecode, str_line_num, INSTRUCTION_END);

        char * operslist = malloc(1);
        strcpy(operslist, "");

        char * valuelist = malloc(1);
        strcpy(valuelist, "");

        char * latestvalue = malloc(1);
        strcpy(latestvalue, "");

        int lastwasop = 1;

        for (int i = 0; i < len; i++)
        {
            if (!strcmp(line[i], "or"))
            {
                char * temp = quokka_compile_line(latestvalue, num, -1, 1);
                strInsertStart(&valuelist, temp);
                free(temp);

                if (lastwasop)
                    error("invalid syntax at 'or'", num - 1);

                mstrcat(&operslist, "BOOLEAN_OR" INSTRUCTION_END);

                lastwasop = 1;
            }
            else if (!strcmp(line[i], "xor"))
            {
                char * temp = quokka_compile_line(latestvalue, num, -1, 1);
                strInsertStart(&valuelist, temp);
                free(temp);

                if (lastwasop)
                    error("invalid syntax at 'xor'", num - 1);

                mstrcat(&operslist, "BOOLEAN_XOR" INSTRUCTION_END);

                lastwasop = 1;
            }
            else if (!strcmp(line[i], "nor"))
            {
                char * temp = quokka_compile_line(latestvalue, num, -1, 1);
                strInsertStart(&valuelist, temp);
                free(temp);

                if (lastwasop)
                    error("invalid syntax at 'nor'", num - 1);

                mstrcat(&operslist, "BOOLEAN_OR" INSTRUCTION_END);
                mstrcat(&operslist, "BOOLEAN_NOT" INSTRUCTION_END);

                lastwasop = 1;
            }
            else if (!strcmp(line[i], "and"))
            {
                char * temp = quokka_compile_line(latestvalue, num, -1, 1);
                strInsertStart(&valuelist, temp);
                free(temp);

                if (lastwasop)
                    error("invalid syntax at 'and'", num - 1);

                mstrcat(&operslist, "BOOLEAN_AND" INSTRUCTION_END);

                lastwasop = 1;
            }
            else
            {
                if (lastwasop)
                {
                    memset(latestvalue, 0, strlen(latestvalue) + 1);
                    latestvalue = realloc(latestvalue, strlen(line[i]) + 1 + 1);
                    strcpy(latestvalue, line[i]);
                    strcat(latestvalue, " ");
                }
                else
                {
                    latestvalue = realloc(latestvalue, strlen(latestvalue) + strlen(line[i]) + 1 + 1);
                    strcat(latestvalue, line[i]);
                    strcat(latestvalue, " ");
                }

                lastwasop = 0;
            }
        }

        char * temp = quokka_compile_line(latestvalue, num, -1, 1);
        strInsertStart(&valuelist, temp);
        free(temp);

        free(latestvalue);

        mstrcat(&bytecode, valuelist);
        mstrcat(&bytecode, operslist);

        free(valuelist);
        free(operslist);
    }
    else if (stringInList(line, "<", len) ||
             stringInList(line, ">", len) ||
             stringInList(line, "<=", len) ||
             stringInList(line, ">=", len) ||
             stringInList(line, "==", len) ||
             stringInList(line, "is", len) ||
             stringInList(line, "!=", len))
    {
        // Set new line
        if (!isInline)
            mstrcattrip(&bytecode, str_line_num, INSTRUCTION_END);

        char * operslist = malloc(1);
        strcpy(operslist, "");

        char * valuelist = malloc(1);
        strcpy(valuelist, "");

        char * latestvalue = malloc(1);
        strcpy(latestvalue, "");

        int lastwasop = 1;

        for (int i = 0; i < len; i++)
        {
            if (!strcmp(line[i], "<"))
            {
                char * temp = quokka_compile_line(latestvalue, num, -1, 1);
                strInsertStart(&valuelist, temp);
                free(temp);

                if (lastwasop)
                    error("invalid syntax at '<'", num - 1);

                mstrcat(&operslist, "CMP_LT" INSTRUCTION_END);

                lastwasop = 1;
            }
            else if (!strcmp(line[i], ">"))
            {
                char * temp = quokka_compile_line(latestvalue, num, -1, 1);
                strInsertStart(&valuelist, temp);
                free(temp);

                if (lastwasop)
                    error("invalid syntax at '>'", num - 1);

                mstrcat(&operslist, "CMP_GT" INSTRUCTION_END);

                lastwasop = 1;
            }
            else if (!strcmp(line[i], "<="))
            {
                char * temp = quokka_compile_line(latestvalue, num, -1, 1);
                strInsertStart(&valuelist, temp);
                free(temp);

                if (lastwasop)
                    error("invalid syntax at '<='", num - 1);

                mstrcat(&operslist, "CMP_LE" INSTRUCTION_END);

                lastwasop = 1;
            }
            else if (!strcmp(line[i], ">="))
            {
                char * temp = quokka_compile_line(latestvalue, num, -1, 1);
                strInsertStart(&valuelist, temp);
                free(temp);

                if (lastwasop)
                    error("invalid syntax at '>='", num - 1);

                mstrcat(&operslist, "CMP_GE" INSTRUCTION_END);

                lastwasop = 1;
            }
            else if (!strcmp(line[i], "=="))
            {
                char * temp = quokka_compile_line(latestvalue, num, -1, 1);
                strInsertStart(&valuelist, temp);
                free(temp);

                if (lastwasop)
                    error("invalid syntax at '=='", num - 1);

                mstrcat(&operslist, "CMP_EQ" INSTRUCTION_END);

                lastwasop = 1;
            }
            else if (!strcmp(line[i], "is"))
            {
                char * temp = quokka_compile_line(latestvalue, num, -1, 1);
                strInsertStart(&valuelist, temp);
                free(temp);

                if (lastwasop)
                    error("invalid syntax at 'is'", num - 1);

                mstrcat(&operslist, "CMP_IDENTICAL" INSTRUCTION_END);

                lastwasop = 1;
            }
            else if (!strcmp(line[i], "!="))
            {
                char * temp = quokka_compile_line(latestvalue, num, -1, 1);
                strInsertStart(&valuelist, temp);
                free(temp);

                if (lastwasop)
                    error("invalid syntax at '!='", num - 1);

                mstrcat(&operslist, "CMP_NEQ" INSTRUCTION_END);

                lastwasop = 1;
            }
            else
            {
                if (lastwasop)
                {
                    memset(latestvalue, 0, strlen(latestvalue) + 1);
                    latestvalue = realloc(latestvalue, strlen(line[i]) + 1 + 1);
                    strcpy(latestvalue, line[i]);
                    strcat(latestvalue, " ");
                }
                else
                {
                    latestvalue = realloc(latestvalue, strlen(latestvalue) + strlen(line[i]) + 1 + 1);
                    strcat(latestvalue, line[i]);
                    strcat(latestvalue, " ");
                }

                lastwasop = 0;
            }
        }

        char * temp = quokka_compile_line(latestvalue, num, -1, 1);
        strInsertStart(&valuelist, temp);
        free(temp);

        free(latestvalue);

        mstrcat(&bytecode, valuelist);
        mstrcat(&bytecode, operslist);

        free(valuelist);
        free(operslist);
    }
    else if (!strcmp(line[0], "&"))
    {
        // Set new line
        if (!isInline)
            mstrcattrip(&bytecode, str_line_num, INSTRUCTION_END);

        arrlstrip(line);
        len--;

        char * temp = quokka_compile_line_tokens(line, num, len, 1);
        mstrcat(&bytecode, temp);
        free(temp);

        mstrcat(&bytecode, "GET_ADDRESS" INSTRUCTION_END);
    }
    else if (isidentifier(line[0]) && !strcmp(line[1], "+") && !strcmp(line[2], "+"))
    {
        if (len > 3)
            error("invalid syntax", num - 1);

        // Set new line
        if (!isInline)
            mstrcattrip(&bytecode, str_line_num, INSTRUCTION_END);

        // Load original value
        mstrcatline(&bytecode,
            "LOAD_NAME",
            SEPARATOR,
            line[0],
            INSTRUCTION_END);

        // Increment (original value is also loaded)
        mstrcatline(&bytecode,
            "INCREMENT",
            SEPARATOR,
            line[0],
            INSTRUCTION_END);
    }
    else if (!strcmp(line[0], "+") && !strcmp(line[1], "+") && isidentifier(line[2]))
    {
        if (len > 3)
            error("invalid syntax", num - 1);

        // Set new line
        if (!isInline)
            mstrcattrip(&bytecode, str_line_num, INSTRUCTION_END);

        // Increment (new value will be loaded)
        mstrcatline(&bytecode,
            "INCREMENT",
            SEPARATOR,
            line[2],
            INSTRUCTION_END);

        // Load new value
        mstrcatline(&bytecode,
            "LOAD_NAME",
            SEPARATOR,
            line[2],
            INSTRUCTION_END);
    }
    else if (isidentifier(line[0]) && !strcmp(line[1], "-") && !strcmp(line[2], "-"))
    {
        if (len > 3)
            error("invalid syntax", num - 1);

        // Set new line
        if (!isInline)
            mstrcattrip(&bytecode, str_line_num, INSTRUCTION_END);

        // Load var
        mstrcatline(&bytecode,
            "LOAD_NAME",
            SEPARATOR,
            line[0],
            INSTRUCTION_END);

        // Decrement and return original value
        mstrcatline(&bytecode,
            "DECREMENT",
            SEPARATOR,
            line[0],
            INSTRUCTION_END);
    }
    else if (!strcmp(line[0], "-") && !strcmp(line[1], "-") && isidentifier(line[2]))
    {
        if (len > 3)
            error("invalid syntax", num - 1);

        // Set new line
        if (!isInline)
            mstrcattrip(&bytecode, str_line_num, INSTRUCTION_END);

        // Decrement (new value will be loaded)
        mstrcatline(&bytecode,
            "DECREMENT",
            SEPARATOR,
            line[2],
            INSTRUCTION_END);

        // Load new value
        mstrcatline(&bytecode,
            "LOAD_NAME",
            SEPARATOR,
            line[2],
            INSTRUCTION_END);
    }
    else if (stringInList(line, "+", len) ||
             stringInList(line, "-", len) ||
             stringInList(line, "*", len) ||
             stringInList(line, "/", len) ||
             stringInList(line, "**", len) ||
             stringInList(line, "%", len) ||
             stringInList(line, "^", len) ||
             stringInList(line, "<<", len) ||
             stringInList(line, ">>", len))
    {
        // Set new line
        if (!isInline)
            mstrcattrip(&bytecode, str_line_num, INSTRUCTION_END);

        char * operslist = malloc(1);
        strcpy(operslist, "");

        char * valuelist = malloc(1);
        strcpy(valuelist, "");

        char * latestvalue = malloc(1);
        strcpy(latestvalue, "");

        int lastwasop = 1;
        int lastwasunary = 0;

        int i;
        for (i = 0; i < len; i++)
        {
            if (!strcmp(line[i], "+"))
            {
                if (i + 1 >= len)
                    error("'+' missing second argument", num - 1);

                if (lastwasop)
                {
                    // Used for unaries beginning with '+', for example: '++--+'
                    char * tmp = malloc(1);
                    strcpy(tmp, "");

                    for (i = i; i < len; i++)
                    {
                        if (!strcmp(line[i], "+"))
                        {
                            tmp = realloc(tmp, strlen(tmp) + 9 + strlen(INSTRUCTION_END) + 1);
                            strcat(tmp, "UNARY_ADD");
                            strcat(tmp, INSTRUCTION_END);
                        }
                        else if (!strcmp(line[i], "-"))
                        {
                            tmp = realloc(tmp, strlen(tmp) + 9 + strlen(INSTRUCTION_END) + 1);
                            strcat(tmp, "UNARY_SUB");
                            strcat(tmp, INSTRUCTION_END);
                        }
                        else break;
                    }

                    char * temp = quokka_compile_line(line[i], num, -1, 1);
                    valuelist = realloc(valuelist, strlen(valuelist) + strlen(temp) + strlen(tmp) + 1);

                    strcat(valuelist, temp);
                    free(temp);

                    strcat(valuelist, tmp);
                    free(tmp);

                    latestvalue = realloc(latestvalue, 1);
                    memset(latestvalue, 0, 1);
                    strcpy(latestvalue, "");

                    lastwasop = 0;
                    lastwasunary = 1;
                }
                else
                {
                    char * temp = quokka_compile_line(latestvalue, num, -1, 1);
                    strInsertStart(&valuelist, temp);
                    free(temp);

                    latestvalue = realloc(latestvalue, 1);
                    memset(latestvalue, 0, 1);
                    strcpy(latestvalue, "");

                    mstrcat(&operslist, "BINARY_ADD" INSTRUCTION_END);

                    lastwasop = 1;
                    lastwasunary = 0;
                }
            }
            else if (!strcmp(line[i], "-"))
            {
                if (i + 1 >= len)
                    error("'-' missing second argument", num - 1);

                if (lastwasop)
                {
                    // Used for unaries beginning with '-', for example: '-+-+'
                    char * tmp = malloc(1);
                    strcpy(tmp, "");

                    for (i = i; i < len; i++)
                    {
                        if (!strcmp(line[i], "+"))
                        {
                            tmp = realloc(tmp, strlen(tmp) + 9 + strlen(INSTRUCTION_END) + 1);
                            strcat(tmp, "UNARY_ADD");
                            strcat(tmp, INSTRUCTION_END);
                        }
                        else if (!strcmp(line[i], "-"))
                        {
                            tmp = realloc(tmp, strlen(tmp) + 9 + strlen(INSTRUCTION_END) + 1);
                            strcat(tmp, "UNARY_SUB");
                            strcat(tmp, INSTRUCTION_END);
                        }
                        else break;
                    }

                    char * temp = quokka_compile_line(line[i], num, -1, 1);
                    valuelist = realloc(valuelist, strlen(valuelist) + strlen(temp) + strlen(tmp) + 1);

                    strcat(valuelist, temp);
                    free(temp);

                    strcat(valuelist, tmp);
                    free(tmp);

                    latestvalue = realloc(latestvalue, 1);
                    memset(latestvalue, 0, 1);
                    strcpy(latestvalue, "");

                    lastwasop = 0;
                    lastwasunary = 1;
                }
                else
                {
                    char * temp = quokka_compile_line(latestvalue, num, -1, 1);
                    strInsertStart(&valuelist, temp);
                    free(temp);

                    latestvalue = realloc(latestvalue, 1);
                    memset(latestvalue, 0, 1);
                    strcpy(latestvalue, "");

                    mstrcat(&operslist, "BINARY_SUB" INSTRUCTION_END);

                    lastwasop = 1;
                    lastwasunary = 0;
                }
            }
            else if (!strcmp(line[i], "*"))
            {
                char * temp = quokka_compile_line(latestvalue, num, -1, 1);
                strInsertStart(&valuelist, temp);
                free(temp);

                if (lastwasop)
                    error("'*' missing first argument", num - 1);

                if (i + 1 >= len)
                    error("'*' missing second argument", num - 1);

                latestvalue = realloc(latestvalue, 1);
                memset(latestvalue, 0, 1);

                mstrcat(&operslist, "BINARY_MUL" INSTRUCTION_END);

                lastwasop = 1;
                lastwasunary = 0;
            }
            else if (!strcmp(line[i], "/"))
            {
                char * temp = quokka_compile_line(latestvalue, num, -1, 1);
                strInsertStart(&valuelist, temp);
                free(temp);

                if (lastwasop)
                    error("'/' missing first argument", num - 1);

                if (i + 1 >= len)
                    error("'/' missing second argument", num - 1);

                latestvalue = realloc(latestvalue, 1);
                memset(latestvalue, 0, 1);

                mstrcat(&operslist, "BINARY_DIV" INSTRUCTION_END);

                lastwasop = 1;
                lastwasunary = 0;
            }
            else if (!strcmp(line[i], "**"))
            {
                char * temp = quokka_compile_line(latestvalue, num, -1, 1);
                strInsertStart(&valuelist, temp);
                free(temp);

                if (lastwasop)
                    error("'**' missing first argument", num - 1);

                if (i + 1 >= len)
                    error("'**' missing second argument", num - 1);

                latestvalue = realloc(latestvalue, 1);
                memset(latestvalue, 0, 1);

                mstrcat(&operslist, "BINARY_POW" INSTRUCTION_END);

                lastwasop = 1;
                lastwasunary = 0;
            }
            else if (!strcmp(line[i], "%"))
            {
                char * temp = quokka_compile_line(latestvalue, num, -1, 1);
                strInsertStart(&valuelist, temp);
                free(temp);

                if (lastwasop)
                    error("'%' missing first argument", num - 1);

                if (i + 1 >= len)
                    error("'%' missing second argument", num - 1);

                latestvalue = realloc(latestvalue, 1);
                memset(latestvalue, 0, 1);

                mstrcat(&operslist, "BINARY_MOD" INSTRUCTION_END);

                lastwasop = 1;
                lastwasunary = 0;
            }
            else if (!strcmp(line[i], "^"))
            {
                char * temp = quokka_compile_line(latestvalue, num, -1, 1);
                strInsertStart(&valuelist, temp);
                free(temp);

                if (lastwasop)
                    error("'^' missing first argument", num - 1);

                if (i + 1 >= len)
                    error("'^' missing second argument", num - 1);

                latestvalue = realloc(latestvalue, 1);
                memset(latestvalue, 0, 1);

                mstrcat(&operslist, "BINARY_XOR" INSTRUCTION_END);

                lastwasop = 1;
                lastwasunary = 0;
            }
            else if (!strcmp(line[i], "<<"))
            {
                char * temp = quokka_compile_line(latestvalue, num, -1, 1);
                strInsertStart(&valuelist, temp);
                free(temp);

                if (lastwasop)
                    error("'<<' missing first argument", num - 1);

                if (i + 1 >= len)
                    error("'<<' missing second argument", num - 1);

                latestvalue = realloc(latestvalue, 1);
                memset(latestvalue, 0, 1);

                mstrcat(&operslist, "BINARY_LSHIFT" INSTRUCTION_END);

                lastwasop = 1;
                lastwasunary = 0;
            }
            else if (!strcmp(line[i], ">>"))
            {
                char * temp = quokka_compile_line(latestvalue, num, -1, 1);
                strInsertStart(&valuelist, temp);
                free(temp);

                if (lastwasop)
                    error("'>>' missing first argument", num - 1);

                if (i + 1 >= len)
                    error("'>>' missing second argument", num - 1);

                latestvalue = realloc(latestvalue, 1);
                memset(latestvalue, 0, 1);

                mstrcat(&operslist, "BINARY_RSHIFT" INSTRUCTION_END);

                lastwasop = 1;
                lastwasunary = 0;
            }
            else
            {
                if (lastwasop)
                {
                    memset(latestvalue, 0, strlen(latestvalue) + 1);
                    latestvalue = realloc(latestvalue, strlen(line[i]) + 1 + 1);
                    strcpy(latestvalue, line[i]);
                    strcat(latestvalue, " ");
                }
                else
                {
                    latestvalue = realloc(latestvalue, strlen(latestvalue) + strlen(line[i]) + 1 + 1);
                    strcat(latestvalue, line[i]);
                    strcat(latestvalue, " ");
                }

                lastwasop = 0;
            }
        }

        if (!lastwasop && strlen(latestvalue) && lastwasunary)
            error("invalid syntax", num - 1);

        char * temp = quokka_compile_line(latestvalue, num, -1, 1);
        strInsertStart(&valuelist, temp);
        free(temp);

        free(latestvalue);

        mstrcat(&bytecode, valuelist);
        mstrcat(&bytecode, operslist);

        free(valuelist);
        free(operslist);
    }
    else if (!strcmp(line[0], "not") || !strcmp(line[0], "!")) // `not` or `!`
    {
        // Set new line
        if (!isInline)
            mstrcattrip(&bytecode, str_line_num, INSTRUCTION_END);

        // If `not` or `!`, then act as `0` or `false`
        if (len <= 1)
        {
            mstrcat(&bytecode, "LOAD_INT_CONST" SEPARATOR "0" INSTRUCTION_END);
        }
        // if `not value` or `!value`, then perform boolean NOT on `value`
        else
        {
            arrlstrip(line);
            len--;

            char * temp = quokka_compile_line_tokens(line, num, len, 1);

            mstrcat(&bytecode, temp);

            free(temp);

            // Perform boolean NOT
            mstrcat(&bytecode, "BOOLEAN_NOT" INSTRUCTION_END);
        }
    }
    else if (startswith(line[len - 1], "[") && endswith(line[len - 1], "]") && len > 1)
    {
        // Set new line
        if (!isInline)
            mstrcattrip(&bytecode, str_line_num, INSTRUCTION_END);

        char * indexarg = strndup(line[len - 1], strlen(line[len - 1]));

        char * temp = quokka_compile_line_tokens(line, num, len - 1, 1);

        mstrcat(&bytecode, temp);

        free(temp);

        // If index received an item, for example: [0]
        if (strlen(indexarg) > 2)
        {
            char * sliced = strSlice(indexarg, 1, 1);
            char * temp = quokka_compile_line(sliced, num, -1, 1);

            mstrcat(&bytecode, temp);

            free(sliced);
            free(temp);

            mstrcat(&bytecode, "GET_INDEX" SEPARATOR "1" INSTRUCTION_END);
        }
        // If nothing was given, for example: []
        else
            mstrcat(&bytecode, "GET_INDEX" SEPARATOR "*" INSTRUCTION_END);

        free(indexarg);
    }
    else if (stringInList(line, ".", len))
    {
        char * latestvalue = malloc(1);
        strcpy(latestvalue, "");

        int lastwasdot = 0;
        int lasthadcall = 0; // Did the expression before the previous `.` call something?
        int dotcount = 0; // Number of dots `.` in this expression

        for (int p = 0; p < len + 1; p++)
        {
            if (p == len)
                line[p] = ".";

            if (!strlen(line[p]))
                break;

            if (!strcmp(line[p], "."))
                dotcount++;

            if (!strcmp(line[p], ".") || p == len)
            {
                char * temp = quokka_compile_line(latestvalue, num, -1, 1);
                mstrcat(&bytecode, temp);
                free(temp);

                latestvalue = realloc(latestvalue, 1);
                strcpy(latestvalue, "");

                lastwasdot = 1;
            }
            else
            {
                if (lastwasdot)
                {
                    if ((lasthadcall || dotcount > 1) && p + 1 < len && startswith(line[p + 1], "(") && endswith(line[p + 1], ")"))
                    {
                        mstrcatline(&bytecode,
                            "GET_METHOD",
                            SEPARATOR,
                            line[p],
                            INSTRUCTION_END);
                    }
                    else
                    {
                        mstrcatline(&bytecode,
                            "GET_ATTR",
                            SEPARATOR,
                            line[p],
                            INSTRUCTION_END);
                    }

                    lastwasdot = 0;
                }
                else if ((strlen(latestvalue) || !lastwasdot) && startswith(line[p], "(") && endswith(line[p], ")"))
                {
                    // Split up the argument list into it's elements
                    char * sliced = strSlice(line[p], 1, 1);

                    char ** templine;
                    int templen = compile_comma_list_string(&templine, sliced);

                    free(sliced);

                    char * argcount;
                    if (templen)
                        argcount = intToStr(stringCountUntil(templine, ",", templen) + 1);
                    else
                    {
                        argcount = malloc(2);
                        strcpy(argcount, "0");
                    }

                    char * temp = quokka_compile_line(latestvalue, num, -1, 1);
                    mstrcat(&bytecode, temp);
                    free(temp);

                    temp = quokka_compile_line_tokens(templine, num, templen, 1);
                    mstrcat(&bytecode, temp);
                    free(temp);

                    latestvalue = realloc(latestvalue, 1);
                    strcpy(latestvalue, "");

                    if (lasthadcall || dotcount > 1)
                    {
                        mstrcatline(&bytecode,
                            "CALL_METHOD",
                            SEPARATOR,
                            argcount,
                            INSTRUCTION_END);
                    }
                    else
                    {
                        mstrcatline(&bytecode,
                            "CALL",
                            SEPARATOR,
                            argcount,
                            INSTRUCTION_END);
                    }

                    free(templine);
                    free(argcount);

                    // if (p < len - 1)
                    //     mstrcat(&bytecode, "REF_TOP" INSTRUCTION_END);

                    lastwasdot = 0;
                    lasthadcall = 1;
                }
                else
                {
                    latestvalue = realloc(latestvalue, strlen(latestvalue) + strlen(line[p]) + 1 + 1);
                    strcat(latestvalue, line[p]);
                    strcat(latestvalue, " ");

                    lastwasdot = 0;
                }
            }
        }

        if (strlen(latestvalue))
        {
            char * temp = quokka_compile_line(latestvalue, num, -1, 1);
            mstrcat(&bytecode, temp);
            free(temp);
        }

        free(latestvalue);
    }
    else if (startswith(line[len - 1], "(") && endswith(line[len - 1], ")") && len > 1)
    {
        // Set new line
        if (!isInline)
            mstrcattrip(&bytecode, str_line_num, INSTRUCTION_END);

        char * temp;

        temp = quokka_compile_line_tokens(line, num, len - 1, 1);
        mstrcat(&bytecode, temp);
        free(temp);

        // Split up the argument list into it's elements
        char * sliced = strSlice(line[len - 1], 1, 1);

        char ** templine;
        int templen = compile_comma_list_string(&templine, sliced);

        temp = quokka_compile_line_tokens(templine, num, templen, 1);

        mstrcat(&bytecode, temp);

        free(sliced);
        free(temp);

        int argc = stringCountUntil(templine, ",", templen);

        int has_args = 0;
        for (int i = 0; i < templen; i++)
            if (strlen(templine[i]))
                has_args = 1;

        char * argcount = intToStr(has_args ? argc + 1 : 0);

        mstrcatline(&bytecode,
            "CALL",
            SEPARATOR,
            argcount,
            INSTRUCTION_END);

        free(templine);
        free(argcount);
    }
    else if (startswith(line[0], "(") && endswith(line[0], ")") && len == 1)
    {
        // Set new line
        if (!isInline)
            mstrcattrip(&bytecode, str_line_num, INSTRUCTION_END);

        // Split up the argument list into it's elements
        char * sliced = strSlice(line[0], 1, 1);

        char ** templine;
        int templen = compile_comma_list_string(&templine, sliced);

        char * temp = quokka_compile_line_tokens(templine, num, templen, 1);

        mstrcat(&bytecode, temp);

        free(sliced);
        free(temp);
        free(templine);
    }
    else if (islong(line[0]) && len == 1)
    {
        // Set new line
        if (!isInline)
            mstrcattrip(&bytecode, str_line_num, INSTRUCTION_END);

        // Clear leading 0's on integers
        while (startswith(line[0], "0") && strlen(line[0]) > 2)
            line[0]++;

        int ind = addBytecodeConstant("LOAD_LONG", line[0]);

        char * intstr = intToStr(ind);
        mstrcatline(&bytecode,
            "LOAD_CONST",
            SEPARATOR,
            intstr,
            INSTRUCTION_END);
        free(intstr);
    }
    else if (isinteger(line[0]) && len == 1)
    {
        // Set new line
        if (!isInline)
            mstrcattrip(&bytecode, str_line_num, INSTRUCTION_END);

        int base = isinteger(line[0]);

        // // Clear leading 0's on integers
        // while (startswith(line[0], "0") && strlen(line[0]) > 1)
        //     line[0]++;

        // If number is more than 10 digits, AND the number's base is 10,
        // make a long instead of an integer
        if (strlen(line[0]) > 10 && base == 10)
        {
            int ind = addBytecodeConstant("LOAD_LONG", line[0]);

            char * intstr = intToStr(ind);
            mstrcatline(&bytecode,
                "LOAD_CONST",
                SEPARATOR,
                intstr,
                INSTRUCTION_END);
            free(intstr);
        }
        // Otherwise, make an int
        else
        {
            if (!strcmp(line[0], "0"))
                mstrcat(&bytecode, "LOAD_INT_CONST" SEPARATOR "0" INSTRUCTION_END);
            else if (!strcmp(line[0], "1"))
                mstrcat(&bytecode, "LOAD_INT_CONST" SEPARATOR "1" INSTRUCTION_END);
            else
            {
                int ind = 0;
                if (base == 2)
                    ind = addBytecodeConstant("LOAD_INT_BIN", line[0]);
                else if (base == 10)
                    ind = addBytecodeConstant("LOAD_INT", line[0]);
                else if (base == 16)
                    ind = addBytecodeConstant("LOAD_INT_HEX", line[0]);

                char * intstr = intToStr(ind);

                mstrcatline(&bytecode,
                    "LOAD_CONST",
                    SEPARATOR,
                    intstr,
                    INSTRUCTION_END);

                free(intstr);
            }
        }
    }
    else if (!strcmp(line[0], "null") && len == 1)
    {
        // Set new line
        if (!isInline)
            mstrcattrip(&bytecode, str_line_num, INSTRUCTION_END);

        // Load constant 0
        mstrcat(&bytecode, "LOAD_CONST" SEPARATOR "0" INSTRUCTION_END);
    }
    else if ((
        (startswith(line[0], "'") && endswith(line[0], "'")) ||
        (startswith(line[0], "\"") && endswith(line[0], "\""))
    ) && len == 1)
    {
        // Set new line
        if (!isInline)
            mstrcattrip(&bytecode, str_line_num, INSTRUCTION_END);

        // If length of string <= 2, it's empty, so load the empty string constant
        if (strnlen(line[0], 3) <= 2)
            mstrcat(&bytecode, "LOAD_CONST" SEPARATOR "1" INSTRUCTION_END);
        else
        {
            char * processed_string = convertLiterals(line[0]);
            int ind = addBytecodeConstant("LOAD_STRING", processed_string);
            free(processed_string);

            char * intstr = intToStr(ind);
            mstrcatline(&bytecode,
                "LOAD_CONST",
                SEPARATOR,
                intstr,
                INSTRUCTION_END);
            free(intstr);
        }
    }
    else if (isidentifier(line[0]) && len == 1)
    {
        // Set new line
        if (!isInline)
            mstrcattrip(&bytecode, str_line_num, INSTRUCTION_END);

        mstrcat(&bytecode, "LOAD_NAME");
        mstrcat(&bytecode, SEPARATOR);
        mstrcat(&bytecode, line[0]);
        mstrcat(&bytecode, INSTRUCTION_END);
    }
    else if (startswith(line[0], "[") && endswith(line[0], "]"))
    {
        // Set new line
        if (!isInline)
            mstrcattrip(&bytecode, str_line_num, INSTRUCTION_END);

        if (len > 1)
            error("invalid syntax", num - 1);

        // Split up the list into it's elements
        char * sliced = strSlice(line[0], 1, 1);

        char ** templine;
        int templen = compile_comma_list_string(&templine, sliced);

        char * temp = quokka_compile_line_tokens(templine, num, templen, 1);

        mstrcat(&bytecode, temp);

        free(sliced);
        free(temp);

        mstrcat(&bytecode, "MAKE_LIST" SEPARATOR);

        if (templen)
        {
            char * intstr = intToStr(stringCountUntil(templine, ",", templen) + 1);
            mstrcat(&bytecode, intstr);

            free(intstr);
        }
        else
            mstrcat(&bytecode, "0");

        mstrcat(&bytecode, INSTRUCTION_END);

        free(templine);
    }
    else
    {
        if (isInline)
        {
            error("invalid syntax", num - 1);
            return bytecode;
        }

        // Set new line
        mstrcattrip(&bytecode, str_line_num, INSTRUCTION_END);

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
                error("invalid syntax", num - 1);
                break;
            }

            if (!strlen(line[p]))
                break;

            char * temp = quokka_compile_line(line[p], num, 1, 1);

            mstrcat(&bytecode, temp);

            free(temp);

            lastwascomma = 0;
        }
    }

    free(str_line_num);
    return bytecode;
}

char * quokka_compile_tokens(char ** tokens, int isInline)
{
    char * compiled = malloc(1);
    compiled[0] = 0;

    file_line_count = arrsize(tokens);
    file_token_index = 0;
    current_line = 1;

    for (file_token_index = 0; tokens[file_token_index] != NULL; file_token_index++)
    {
        char * t = tokens[file_token_index];

        if (t[0] == '\n')
            current_line++;
        else if (!t[0])
        {
            current_line++;
            continue;
        }

        char * short_line = strReplace(t, "\n", "");
        char * ins = quokka_compile_line(short_line, current_line, -1, isInline);

        mstrcat(&compiled, ins);

        free(short_line);
        free(ins);

        // Update line number
        current_line += charCount(t, '\n');
        if (t[0] == '\n')
            current_line--;
    }

    char * result = malloc(strlen(bytecode_constants) + strlen(compiled) + 1);
    strcpy(result, bytecode_constants);
    strcat(result, compiled);

    free(compiled);

    return result;
}

char * quokka_compile_raw(char * rawtext, int isInline)
{
    file_tokens = quokka_file_tok(rawtext);

    return quokka_compile_tokens(file_tokens, isInline);
}

char * quokka_compile_fname(char * filename, int isInitFile)
{
    /* Start */
    char * buffer = readfile(filename);

    if (!buffer)
        return 0;

    // If this is the main file being compiled, set bytecode constants
    // as normal
    if (isInitFile)
        set_bytecode_constants();
    // You don't want the bytecode constant count to reset during an import
    else
    {
        // Save the bytecode constant count
        int old_bytecode_constant_count = bytecode_constant_count;

        // Reset the bytecode constants completely, but not the count
        // (Assuming bytecode_constants IS in fact already malloc'd)
        free(bytecode_constants);
        bytecode_constants = malloc(1);
        bytecode_constants[0] = 0;

        // Maintain the bytecode constant count during imports
        bytecode_constant_count = old_bytecode_constant_count;
    }

    compile_init();

    pushTrash(buffer);

    /* Main */
    char * res = quokka_compile_raw(buffer, 0);

    /* End */
    free(file_tokens);

    return res;
}

char ** quokka_file_tok(char * text)
{
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
    char * separator = "\b";

    // Set initial char type
    if (isalnum(text[0]))
        p = 'A';
    else if (isdigit(text[0])) // isalnum conflicts with this? Check this out at some point
                               // Previous line might need isalpha instead
                               // Otherwise, the 'D' char type might need to be removed
        p = 'D';
    else if (ispunct(text[0]))
        p = 'S';
    else if (text[0] == '\n')
        p = 'L'; // Line ending
    else if (isspace(text[0]))
        p = 'W';

    int len = strlen(text);
    for (int i = 0; i < len; i++)
    {
        q = p;
        char c = text[i];

        int semicolon = 0;

        if      (isalpha(c))
            p = 'A'; // Alphabet
        else if (isdigit(c))
            p = 'D'; // Digit
        else if (ispunct(c))
            p = 'S'; // Symbol
        else if (c == '\n')
            p = 'L'; // Line endings
        else if (isspace(c))
            p = 'W'; // Whitespace

        if ((p == 'L' || c == ';') && !(
            sq || dq || bt || rb > 0 || sb > 0 || cb > 0
        ))
        {
            tokenstr = realloc(tokenstr, strlen(tokenstr) + strlen(separator) + 1);
            strncat(tokenstr, separator, strlen(separator));

            if (c == ';')
                semicolon = 1;
        }

        if (comment >= 2);
        else if (c == '\'' && !(
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
        else if (c == '[' && !(
            sq || dq || bt || rb > 0 || cb > 0))
            sb++;
        else if (c == ']' && !(
            sq || dq || bt || rb > 0 || cb > 0))
            sb--;
        else if (c == '/' && !(
            sq || dq || bt))
        {
            comment++;
            if (comment >= 2)
            {
                tokenstr[strlen(tokenstr) - 1] = '\0';
            }
        }

        if (c == '\n')
        {
            comment = 0;
        }

        if (comment <= 1 || c == '\n')
        {
            if (comment && c != '/')
                comment = 0;

            if (!semicolon)
            {
                tokenstr = realloc(tokenstr, strlen(tokenstr) + 1 + 1);
                strncat(tokenstr, &c, 1);
            }
        }

        t = c;
    }

    mstrcat(&tokenstr, separator);

    char ** output = malloc(sizeof(char *));
    output[0] = "";

    int i = 0;
    output[0] = nstrtok(tokenstr, separator);

    while (output[i] != NULL)
    {
        // Protects against empty parts of a line or string
        if (!strlen(output[i]))
        {
            output = realloc(output, (i + 1) * sizeof(char *));
            output[i] = nstrtok(NULL, separator);
            continue;
        }

        i++;
        output = realloc(output, (i + 1) * sizeof(char *));
        output[i] = nstrtok(NULL, separator);
    }

    output = realloc(output, (i + 2) * sizeof(char *));
    output[i + 1] = NULL;

    pushTrash(tokenstr);

    return output;
}

char ** quokka_tok(char * line, char ** waste)
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
    char * separator = "\b";

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

        if      (isalpha(c))
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
            // Join together operators: -= += *= /= == >= <=
            (t == '-' || t == '+' || t == '*' || t == '/' || t == '=' || t == '>' || t == '<' || t == '!') && c == '='
        // ) && !(
        //     q == 'A' && c == '.' // Join together names like `word.upper` (second part is below)
        // ) && !(
        //     p == 'A' && t == '.' // Second part to the line above.
        ) && !(
            t == '_' && p == 'A' // Join together names like `string_one` (second part is below)
        ) && !(
            q == 'A' && c == '_' // Second part to the line above.
        ) && !(
            q == 'A' && p == 'D' // Join alphabetical and numerical characters.
        ) && !(
            p == 'A' && q == 'D' // Second part to the line above.
        ) && !(
            t == '_' && c == '_' // Join together all `_` tokens.
        ) && !(
            t == '.' && c == '.' // Join together all `.` tokens.
        ) && !(
            t == '*' && c == '*' // Join together all `*` tokens.
        ) && !(
            t == '<' && c == '<' // Join together all '<' tokens.
        ) && !(
            t == '>' && c == '>' // Join together all '>' tokens.
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
        else if (c == '[' && !(
            sq || dq || bt || rb > 0 || cb > 0))
            sb++;
        else if (c == ']' && !(
            sq || dq || bt || rb > 0 || cb > 0))
            sb--;
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
            tokenstr = realloc(tokenstr, strlen(tokenstr) + 1 + 1);
            strncat(tokenstr, &c, 1);
        }

        t = c;
    }

    mstrcat(&tokenstr, separator);

    char ** output = malloc(sizeof(char *));
    output[0] = "";

    int i = 0;
    output[0] = cpstrip(nstrtok(tokenstr, separator));

    while (output[i] != NULL)
    {
        // Protects against empty parts of a line or string
        if (!strlen(output[i]))
        {
            output = realloc(output, (i + 1) * sizeof(char *));
            output[i] = cpstrip(nstrtok(NULL, separator));
            continue;
        }

        i++;
        output = realloc(output, (i + 1) * sizeof(char *));
        output[i] = cpstrip(nstrtok(NULL, separator));
    }

    output = realloc(output, (i + 2) * sizeof(char *));
    output[i + 1] = NULL;

    *waste = tokenstr;

    return output;
}

char *** quokka_bc_file_tok(char * text)
{
    // ESTIMATIONS WILL ALWAYS BE ENOUGH TO HOLD THE OUTPUT
    int est = charCount(text, INSTRUCTION_END[0]) + 1; // Estimated token count
    // int len = strlen(text); // Bytecode line length
    // int est_tok = len / est; // Estimated token length

    char *** tokens = malloc((est + 1) * sizeof(char **));

    char * last;

    int t = 0; // Current token index
    last = strtok(text, INSTRUCTION_END);
    while (last)
    {
        tokens[t] = quokka_bc_tok(last);
        t++;
        last = strtok(NULL, INSTRUCTION_END);
    }

    tokens[t] = NULL;

    return tokens;
}

char ** quokka_bc_tok(char * line)
{
    // ESTIMATIONS WILL ALWAYS BE ENOUGH TO HOLD THE OUTPUT
    int est = charCount(line, SEPARATOR[0]) + 1; // Estimated token count
    int len = strlen(line); // Bytecode line length
    int est_tok = len + 1 / est; // Estimated token length

    char ** tokens = malloc((est + 1) * sizeof(char *));
    tokens[0] = malloc(est_tok + 1);
    tokens[0][0] = 0;
    int t = 0; // Current token index

    // Bools
    int sq = 0; // Single-Quote = false
    int dq = 0; // Double-Quote = false

    // Ints
    int sb = 0; // Square-bracket = 0

    for (int i = 0; i < len; i++)
    {
        if (line[i] == ' ' && sb <= 0 && !sq && !dq)
        {
            t++;
            tokens[t] = malloc(est_tok + 1);
            tokens[t][0] = 0;
        }
        else
            strncat(tokens[t], &line[i], 1);

        if (line[i] == '[')
            sb++;
        else if (line[i] == ']')
            sb--;
        else if (line[i] == '\'')
            sq = !sq;
        else if (line[i] == '"')
            dq = !dq;
    }

    tokens[t + 1] = NULL;

    return tokens;
}

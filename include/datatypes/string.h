Object * __add___string(int argc, Object ** argv)
{
    if (!strcmp(argv[1]->name, "string"))
    {
        char * first = objectGetAttr(argv[0], "value");
        char * secnd = objectGetAttr(argv[1], "value");

        char * third = malloc(strlen(first) + strlen(secnd) + 1);
        strcpy(third, first);
        strcat(third, secnd);

        return makeString(third, 1);
    }
    else if (!strcmp(argv[1]->name, "null"))
    {
        Object ** arglist = makeArglist(argv[1]);

        char * first = objectGetAttr(argv[0], "value");
        char * secnd = objectGetAttr(q_function_string(1, arglist), "value");

        char * third = malloc(strlen(first) + strlen(secnd) + 1);
        strcpy(third, first);
        strcat(third, secnd);

        free(arglist);

        return makeString(third, 1);
    }

    char * err = malloc(20 + strlen(argv[1]->name) + 30 + 1);
    strcpy(err, "types 'string' and '");
    strcat(err, argv[1]->name);
    strcat(err, "' are invalid operands for '+'");
    error(err, line_num);

    return makeNull();
}

Object * __mul___string(int argc, Object ** argv)
{
    if (!strcmp(argv[1]->name, "int"))
    {
        char * first = objectGetAttr(argv[0], "value");
        int * secnd = objectGetAttr(argv[1], "value");

        char * third = malloc(strlen(first) + 1);
        strcpy(third, "");

        for (int i = 0; i < secnd[0]; i++)
        {
            mstrcat(&third, first);
        }

        return makeString(third, 1);
    }

    char * err = malloc(20 + strlen(argv[1]->name) + 30 + 1);
    strcpy(err, "types 'string' and '");
    strcat(err, argv[1]->name);
    strcat(err, "' are invalid operands for '*'");
    error(err, line_num);

    return makeNull();
}

Object * __eq___string(int argc, Object ** argv)
{
    if (strcmp(argv[1]->name, "string"))
    {
        char * err = malloc(20 + strlen(argv[1]->name) + 31 + 1);
        strcpy(err, "types 'string' and '");
        strcat(err, argv[1]->name);
        strcat(err, "' can not be compared with '=='");
        error(err, line_num);
    }

    char * first = objectGetAttr(argv[0], "value");
    char * secnd = objectGetAttr(argv[1], "value");

    if (!strcmp(first, secnd))
        return makeInt(&truePtr, 0);
    return makeInt(&falsePtr, 0);
}

Object * __index___string(int argc, Object ** argv)
{
    if (strcmp(argv[1]->name, "int"))
    {
        char * err = malloc(42 + strlen(argv[1]->name) + 1 + 1);
        strcpy(err, "string index argument must be 'int', not '");
        strcat(err, argv[1]->name);
        strcat(err, "'");
        error(err, line_num);
    }

    int ind = ((int *)objectGetAttr(argv[1], "value"))[0];
    int length = strlen((char *)objectGetAttr(argv[0], "value"));

    // If index is -1, retrieve length - 1
    if (ind < 0)
        ind = length + ind;

    if (ind >= length)
    {
        char * chst = malloc(1);
        chst[0] = '\0';

        return makeString(chst, 1);
    }

    if (ind < 0)
    {
        char * chst = malloc(1);
        chst[0] = '\0';

        return makeString(chst, 1);
    }

    char ch = ((char *)objectGetAttr(argv[0], "value"))[ind];

    char * chst = malloc(2);
    chst[0] = ch;
    chst[1] = '\0';

    return makeString(chst, 1);
}

Object * __sizeof___string(int argc, Object ** argv)
{
    char * thisvalue = ((char *)objectGetAttr(argv[0], "value"));

    int * size = makeIntPtr(sizeof(argv[0]) + strlen(thisvalue));

    return makeInt(size, 1);
}

Object * __copy___string(int argc, Object ** argv)
{
    char * thisvalue = (char *)objectGetAttr(argv[0], "value");

    return makeString(strdup(thisvalue), 1);
}

Object * __len___string(int argc, Object ** argv)
{
    long long len = strlen((char *)objectGetAttr(argv[0], "value"));

    int * size = makeIntPtr(len);

    return makeInt(size, 1);
}

Object * __disp___string(int argc, Object ** argv)
{
    char * selftext = (char *)objectGetAttr(argv[0], "value");
    char * rawst = makeRawString(selftext);

    return makeString(rawst, 1);
}

Object * __bool___string(int argc, Object ** argv)
{
    char * thisvalue = ((char *)objectGetAttr(argv[0], "value"));

    if (strlen(thisvalue))
        return makeInt(&truePtr, 0);
    return makeInt(&falsePtr, 0);
}

Object * __int___string(int argc, Object ** argv)
{
    char * thisvalue = ((char *)objectGetAttr(argv[0], "value"));

    int * toint = makeIntPtr(strtol(thisvalue, NULL, 10));

    return makeInt(toint, 1);
}

Object * __long___string(int argc, Object ** argv)
{
    char * thisvalue = ((char *)objectGetAttr(argv[0], "value"));

    long long * tolong = makeLLPtr(strtoll(thisvalue, NULL, 10));

    return makeLong(tolong, 1);
}

Object * __string___string(int argc, Object ** argv)
{
    // Return a new string with the same contents as this one

    // (the rest of the stuff doesn't matter, since the string()
    // function call is expected to return a new string anyway,
    // so we won't need to go through the trouble of deep
    // copying this string)

    char * thisvalue = ((char *)objectGetAttr(argv[0], "value"));

    return makeString(strdup(thisvalue), 1);
}

Object * __free___string(int argc, Object ** argv)
{
    char * thisvalue = objectGetAttr(argv[0], "value");
    free(thisvalue);

    return makeNull();
}

Object * makeString(char * value, int is_malloc_ptr)
{
    Object * self = objectPointer();

    self->name = "string";

    // 25 to 26 Attributes
    if (is_malloc_ptr)
    {
        self->names = malloc(26 * sizeof(char *));
        self->values = malloc(26 * sizeof(void *));
    }
    else
    {
        self->names = malloc(25 * sizeof(char *));
        self->values = malloc(25 * sizeof(void *));
    }
    self->value_count = 0;

    self = objectAddAttr(self, "value", value);

    // Two argument methods

    // __add__
    self = objectAddAttr(self, "__add__", &__add___string);
    self = objectAddAttr(self, "__add__argc", &twoArgc);

    // __mul__
    self = objectAddAttr(self, "__mul__", &__mul___string);
    self = objectAddAttr(self, "__mul__argc", &twoArgc);

    // __eq__
    self = objectAddAttr(self, "__eq__", &__eq___string);
    self = objectAddAttr(self, "__eq__argc", &twoArgc);

    // __index__
    self = objectAddAttr(self, "__index__", &__index___string);
    self = objectAddAttr(self, "__index__argc", &twoArgc);

    // One argument methods

    // __sizeof__
    self = objectAddAttr(self, "__sizeof__", &__sizeof___string);
    self = objectAddAttr(self, "__sizeof__argc", &oneArgc);

    // __copy__
    self = objectAddAttr(self, "__copy__", &__copy___string);
    self = objectAddAttr(self, "__copy__argc", &oneArgc);

    // __len__
    self = objectAddAttr(self, "__len__", &__len___string);
    self = objectAddAttr(self, "__len__argc", &oneArgc);

    // __disp__
    self = objectAddAttr(self, "__disp__", &__disp___string);
    self = objectAddAttr(self, "__disp__argc", &oneArgc);

    // __bool__
    self = objectAddAttr(self, "__bool__", &__bool___string);
    self = objectAddAttr(self, "__bool__argc", &oneArgc);

    // __int__
    self = objectAddAttr(self, "__int__", &__int___string);
    self = objectAddAttr(self, "__int__argc", &oneArgc);

    // __long__
    self = objectAddAttr(self, "__long__", &__long___string);
    self = objectAddAttr(self, "__long__argc", &oneArgc);

    // __string__
    self = objectAddAttr(self, "__string__", &__string___string);
    self = objectAddAttr(self, "__string__argc", &oneArgc);

    if (is_malloc_ptr)
    {
        // __free__
        self = objectAddAttr(self, "__free__", &__free___string);
    }

    return self;
}

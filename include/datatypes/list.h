Object __add___list(int argc, Object * argv)
{
    if (!strcmp(argv[1].name, "list"))
    {
        Object * first = objectGetAttr(argv[0], "value");
        Object * secnd = objectGetAttr(argv[1], "value");

        int firstlen = ((int *)objectGetAttr(argv[0], "length"))[0];
        int secndlen = ((int *)objectGetAttr(argv[1], "length"))[0];

        Object * third = malloc((firstlen + secndlen) * sizeof(Object));

        // Add first list to final list
        for (int i = 0; i < firstlen; i++)
            third[i] = first[i];

        // Add second list to final list
        for (int i = 0; i < secndlen; i++)
            third[firstlen + i] = secnd[i];

        Object ret = makeList(firstlen + secndlen, third, 0);

        free(third);

        return ret;
    }

    char * err = malloc(18 + strlen(argv[1].name) + 30 + 1);
    strcpy(err, "types 'list' and '");
    strcat(err, argv[1].name);
    strcat(err, "' are invalid operands for '+'");
    error(err, line_num);

    return makeNull();
}

Object __index___list(int argc, Object * argv)
{
    if (strcmp(argv[1].name, "int"))
    {
        char * err = malloc(40 + strlen(argv[1].name) + 1 + 1);
        strcpy(err, "list index argument must be 'int', not '");
        strcat(err, argv[1].name);
        strcat(err, "'");
        error(err, line_num);
    }

    int ind = ((int *)objectGetAttr(argv[1], "value"))[0];
    int length = ((int *)objectGetAttr(argv[0], "length"))[0];

    // If index is -1, retrieve length - 1
    if (ind < 0)
        ind = length + ind;

    // Check bounds
    if (ind >= length)
        return makeNull();
    if (ind < 0)
        return makeNull();

    Object obj = ((Object *)objectGetAttr(argv[0], "value"))[ind];

    // Push a COPY of this Object to memory

    return obj;//objectCopy(obj);
}

Object __sizeof___list(int argc, Object * argv)
{
    int * thisvalue = objectGetAttr(argv[0], "value");
    int * length = objectGetAttr(argv[0], "length");

    int * size = makeIntPtr(sizeof(argv[0]) + (sizeof(thisvalue[0]) * length[0]));
    pushTrash(size);

    return makeInt(size);
}

Object __copy___list(int argc, Object * argv)
{
    return makeList(
        ((int *)objectGetAttr(argv[0], "length"))[0],
        objectGetAttr(argv[0], "value"),
        0); // 0 so it doesn't flip
}

Object __len___list(int argc, Object * argv)
{
    return makeInt(objectGetAttr(argv[0], "length"));
}

Object __bool___list(int argc, Object * argv)
{
    int length = ((int *)objectGetAttr(argv[0], "length"))[0];

    if (length)
        return makeInt(&truePtr);
    return makeInt(&falsePtr);
}

Object __disp___list(int argc, Object * argv)
{
    char * out = malloc(2);
    strcpy(out, "[");

    Object * lst = objectGetAttr(argv[0], "value");
    int lstlen = ((int *)objectGetAttr(argv[0], "length"))[0];

    for (int p = 0; p < lstlen; p++)
    {
        Object * arglist = makeArglist(lst[p]);

        Object disp = q_function_display(1, arglist);

        free(arglist);

        mstrcat(&out, objectGetAttr(disp, "value"));
        if (p + 1 < lstlen)
            mstrcat(&out, ", ");

        freeObject(disp);
    }

    mstrcat(&out, "]");

    pushTrash(out);

    return makeString(out);
}

Object makeList(int length, Object * value, int flipped)
{
    Object self;

    Object * lst = malloc(length * sizeof(Object));

    // Contents of this list will be freed after program execution
    pushTrash(lst);

    // Reverse items before creating list
    for (int i = 0; i < length; i++)
    {
        if (flipped)
            lst[length - i - 1] = value[i];
        else
            lst[i] = value[i];
    }

    int * len_ptr = makeIntPtr(length);
    pushTrash(len_ptr);

    self = makeObject("list", lst);
    self = addObjectValue(self, "length", len_ptr);

    // Two argument methods

    // __add__
    self = addObjectValue(self, "__add__", &__add___list);
    self = addObjectValue(self, "__add__argc", &twoArgc);

    // __index__
    self = addObjectValue(self, "__index__", &__index___list);
    self = addObjectValue(self, "__index__argc", &twoArgc);

    // One argument methods

    // __sizeof__
    self = addObjectValue(self, "__sizeof__", &__sizeof___list);
    self = addObjectValue(self, "__sizeof__argc", &oneArgc);

    // __copy__
    self = addObjectValue(self, "__copy__", &__copy___list);
    self = addObjectValue(self, "__copy__argc", &oneArgc);

    // __len__
    self = addObjectValue(self, "__len__", &__len___list);
    self = addObjectValue(self, "__len__argc", &oneArgc);

    // __bool__
    self = addObjectValue(self, "__bool__", &__bool___list);
    self = addObjectValue(self, "__bool__argc", &oneArgc);

    // __disp__
    self = addObjectValue(self, "__disp__", &__disp___list);
    self = addObjectValue(self, "__disp__argc", &oneArgc);

    // __string__
    self = addObjectValue(self, "__string__", &__disp___list);
    self = addObjectValue(self, "__string__argc", &oneArgc);

    return self;
}

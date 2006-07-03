BEGIN {
    print "static PY_CONSTANT_REC py_constants[] = {";
}

{
    if (NF >= 2)
        constant = $2;
    else
        constant = $1;

    printf("    {\"%s\", %25s},\n", $1, constant);
}

END {
    print "    {NULL}";
    print "};"
}


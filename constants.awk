BEGIN {
    print "static PY_CONSTANT_REC py_constants[] = {";
}

{
    printf("    {\"%s\", %25s},\n", $1,$1);
}

END {
    print "    {NULL}";
    print "};"
}


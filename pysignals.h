#ifndef _PYSIGNALS_H_
#define _PYSIGNALS_H_
#include <Python.h>

typedef struct
{
    char *name;
    PyObject *handler;
} PY_COMMAND_REC;

void py_command_bind(const char *category, PY_COMMAND_REC *crec);
void py_command_unbind(PY_COMMAND_REC *crec);

#endif

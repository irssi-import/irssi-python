#ifndef _PY_MODULE_H_
#define _PY_MODULE_H_

#include <Python.h>

/* This is global so that type objects and such can be easily attached */
extern PyObject *py_module;
PyObject *PyInit_IrssiModule(void);
int pymodule_init(void);
void pymodule_deinit(void);

#endif

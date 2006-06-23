#ifndef _PYSIGNALS_H_
#define _PYSIGNALS_H_
#include <Python.h>

/* forward */
struct _PY_SIGNAL_SPEC_REC;

typedef struct _PY_SIGNAL_REC
{
    struct _PY_SIGNAL_SPEC_REC *signal;
    char *command; /* NULL if this is signal */
    PyObject *handler;
} PY_SIGNAL_REC;

PY_SIGNAL_REC *pysignals_command_bind(const char *cmd, PyObject *func, 
        const char *category, int priority);
PY_SIGNAL_REC *pysignals_signal_add(const char *signal, PyObject *func, 
        int priority);
void pysignals_command_unbind(PY_SIGNAL_REC *rec);
void pysignals_signal_remove(PY_SIGNAL_REC *rec);
void pysignals_remove_generic(PY_SIGNAL_REC *rec);
int pysignals_register(const char *name, const char *arglist);
int pysignals_unregister(const char *name);
void pysignals_init(void);
void pysignals_deinit(void);

#endif

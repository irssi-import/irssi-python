#ifndef _PYSOURCE_H_
#define _PYSOURCE_H_

#include <Python.h>

/* condition is G_INPUT_READ or G_INPUT_WRITE */
int pysource_input_add_list(GSList **list, int fd, int cond, PyObject *func, PyObject *data, int once);
int pysource_timeout_add_list(GSList **list, int msecs, PyObject *func, PyObject *data, int once);

int pysource_remove_tag(GSList **list, int handle);
void pysource_remove_list(GSList *list);

#endif

#include <Python.h>
#include "pyirssi.h"
#include "pysource.h"

typedef struct _PY_SOURCE_REC
{
    int once;
    int tag;
    int fd;
    GSList **container; /* "container" points to a list owned by a Script object */
    PyObject *handler;
    PyObject *data;
} PY_SOURCE_REC;

static PY_SOURCE_REC *py_source_new(GSList **list, int once, PyObject *handler, PyObject *data)
{
    PY_SOURCE_REC *rec;

    rec = g_new0(PY_SOURCE_REC, 1);
    rec->once = once;
    rec->fd = -1;
    rec->handler = handler;
    rec->data = data;
    rec->container = list;

    Py_INCREF(handler);
    Py_XINCREF(data); 

    return rec;
}

static void py_source_destroy(PY_SOURCE_REC *rec)
{
    g_source_remove(rec->tag);
    Py_DECREF(rec->handler);
    Py_XDECREF(rec->data);
    g_free(rec);
}

static int py_source_proxy(PY_SOURCE_REC *rec)
{
    char args[3] = {0,0,0};
    int fd;
    PyObject *ret;
    PyObject *handler, *data;

    /* Copy data out of the rec (there's not much). The rec may be deleted in 
       the if block below or by the Python code executed. Protect handler & data 
       with INCREF.
    */

    fd = rec->fd;
    handler = rec->handler;
    data = rec->data;
    Py_INCREF(handler);
    Py_XINCREF(data);
    
    if (rec->once)
    {
        *rec->container = g_slist_remove(*rec->container, rec);
        py_source_destroy(rec);
    }

    /* call python function with fd and/or data if available. 
       IO handler will be called with either "iO" or "i". 
       Timeout with "O" or "".
    */

    if (fd >= 0)
    {
        /* IO handler */
        args[0] = 'i';
        if (data)
            args[1] = 'O';

        ret = PyObject_CallFunction(handler, args, fd, data);
    }
    else
    {
        /* Timeout handler */
        if (data)
            args[0] = 'O';

        ret = PyObject_CallFunction(handler, args, data);
    }

    if (!ret)
        PyErr_Print();
    else
        Py_DECREF(ret);

    Py_DECREF(handler);
    Py_XDECREF(data);
    
    return 1;
}

int pysource_timeout_add_list(GSList **list, int msecs, PyObject *func, PyObject *data, int once)
{
    PY_SOURCE_REC *rec;

    g_return_val_if_fail(func != NULL, -1);

    rec = py_source_new(list, once, func, data);
    rec->tag = g_timeout_add(msecs, (GSourceFunc)py_source_proxy, rec);

    *list = g_slist_append(*list, rec);

    return rec->tag;
}

int pysource_input_add_list(GSList **list, int fd, int cond, PyObject *func, PyObject *data, int once)
{
    PY_SOURCE_REC *rec;
    GIOChannel *channel;

    g_return_val_if_fail(func != NULL, 1);
    rec = py_source_new(list, once, func, data);
    rec->fd = fd;  
    channel = g_io_channel_unix_new(fd);
    rec->tag = g_input_add(channel, cond, (GInputFunction)py_source_proxy, rec);
    g_io_channel_unref(channel);
   
    *list = g_slist_append(*list, rec);
    
    return rec->tag;
}

int pysource_remove_tag(GSList **list, int handle)
{
    GSList *node;

    for (node = *list; node != NULL; node = node->next)
    {
        PY_SOURCE_REC *rec = node->data;

        if (rec->tag == handle)
        {
            py_source_destroy(rec);
            *list = g_slist_delete_link(*list, node);

            return 1;
        }
    }

    return 0;
}

void pysource_remove_list(GSList *list)
{
    GSList *node;

    for (node = list; node != NULL; node = node->next)
        py_source_destroy(node->data);
}


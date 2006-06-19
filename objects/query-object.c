#include <Python.h>
#include "pyirssi.h"
#include "pymodule.h"
#include "base-objects.h"
#include "window-item-object.h"
#include "query-object.h"
#include "server-object.h"
#include "pycore.h"

/* member IDs */
enum
{
    M_QUERY_ADDRESS,
    M_QUERY_SERVER_TAG,
    M_QUERY_UNWANTED,
};

/* monitor "query destroyed" signal */
static void query_cleanup(QUERY_REC *query)
{
    PyQuery *pyquery = signal_get_user_data();

    if (query == pyquery->data)
    {
        pyquery->data = NULL;
        pyquery->cleanup_installed = 0;
        signal_remove_data("query destroyed", query_cleanup, pyquery);
    }
}

static void PyQuery_dealloc(PyQuery *self)
{
    if (self->cleanup_installed)
        signal_remove_data("query destroyed", query_cleanup, self);

    self->ob_type->tp_free((PyObject*)self);
}


static PyObject *PyQuery_get(PyQuery *self, void *closure)
{
    int member = GPOINTER_TO_INT(closure);

    RET_NULL_IF_INVALID(self->data);

    switch (member)
    {
        case M_QUERY_ADDRESS:
            RET_AS_STRING_OR_NONE(self->data->address);
        case M_QUERY_SERVER_TAG:
            RET_AS_STRING_OR_NONE(self->data->server_tag);
        case M_QUERY_UNWANTED:
            return PyBool_FromLong(self->data->unwanted);
    }

    /* This shouldn't be reached... but... */
    return PyErr_Format(PyExc_RuntimeError, "invalid member id, %d", member);
}

/* specialized getters/setters */
static PyGetSetDef PyQuery_getseters[] = {
    {"address", (getter)PyQuery_get, NULL, 
        "Host address of the queries nick", 
        GINT_TO_POINTER(M_QUERY_ADDRESS)},

    {"server_tag", (getter)PyQuery_get, NULL, 
        "Server tag used for this nick (doesn't get erased if server gets disconnected)", 
        GINT_TO_POINTER(M_QUERY_SERVER_TAG)},

    {"unwanted", (getter)PyQuery_get, NULL, 
        "1 if the other side closed or some error occured (DCC chats)", 
        GINT_TO_POINTER(M_QUERY_UNWANTED)},

    {NULL}
};

PyDoc_STRVAR(change_server_doc,
    "Change the active server for the query."
);
static PyObject *PyQuery_change_server(PyQuery *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"server", NULL};
    PyObject *server;

    RET_NULL_IF_INVALID(self->data);
    
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &server))
        return NULL;

    if (!pyserver_check(server))
        return PyErr_Format(PyExc_TypeError, "argument must be server object");
   
    query_change_server(self->data, ((PyServer*)server)->data);
    
    Py_RETURN_NONE;
}

/* Methods for object */
static PyMethodDef PyQuery_methods[] = {
    {"change_server", (PyCFunction)PyQuery_change_server, METH_VARARGS | METH_KEYWORDS, 
        change_server_doc},

    {NULL}  /* Sentinel */
};

PyTypeObject PyQueryType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "Query",            /*tp_name*/
    sizeof(PyQuery),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)PyQuery_dealloc,    /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "PyQuery objects",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    PyQuery_methods,             /* tp_methods */
    0,                      /* tp_members */
    PyQuery_getseters,        /* tp_getset */
    &PyWindowItemType,          /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,      /* tp_init */
    0,                         /* tp_alloc */
    0,                 /* tp_new */
};


/* query factory function */
PyObject *pyquery_new(void *query)
{
    static const char *BASE_NAME = "QUERY";
    PyObject *pyquery;

    pyquery = pywindow_item_sub_new(query, BASE_NAME, &PyQueryType);
    if (pyquery)
    {
        PyQuery *pyq = (PyQuery *)pyquery;
        signal_add_last_data("query destroyed", query_cleanup, pyq);
        pyq->cleanup_installed = 1;
    }

    return pyquery;
}

int query_object_init(void) 
{
    g_return_val_if_fail(py_module != NULL, 0);

    if (PyType_Ready(&PyQueryType) < 0)
        return 0;
    
    Py_INCREF(&PyQueryType);
    PyModule_AddObject(py_module, "Query", (PyObject *)&PyQueryType);

    return 1;
}

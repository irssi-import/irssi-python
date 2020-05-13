/* 
    irssi-python

    Copyright (C) 2006 Christopher Davis

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <Python.h>
#include "pyirssi.h"
#include "pymodule.h"
#include "base-objects.h"
#include "window-item-object.h"
#include "query-object.h"
#include "server-object.h"
#include "pycore.h"

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

    Py_XDECREF(self->server);

    Py_TYPE(self)->tp_free((PyObject *)self);
}

/* Getters */
PyDoc_STRVAR(PyQuery_address_doc,
    "Host address of the queries nick"
);
static PyObject *PyQuery_address_get(PyQuery *self, void *closure)
{
    RET_NULL_IF_INVALID(self->data);
    RET_AS_STRING_OR_NONE(self->data->address);
}

PyDoc_STRVAR(PyQuery_server_tag_doc,
    "Server tag used for this nick (doesn't get erased if server gets disconnected)"
);
static PyObject *PyQuery_server_tag_get(PyQuery *self, void *closure)
{
    RET_NULL_IF_INVALID(self->data);
    RET_AS_STRING_OR_NONE(self->data->server_tag);
}

PyDoc_STRVAR(PyQuery_unwanted_doc,
    "1 if the other side closed or some error occured (DCC chats)"
);
static PyObject *PyQuery_unwanted_get(PyQuery *self, void *closure)
{
    RET_NULL_IF_INVALID(self->data);
    return PyBool_FromLong(self->data->unwanted);
}

/* specialized getters/setters */
static PyGetSetDef PyQuery_getseters[] = {
    {"address", (getter)PyQuery_address_get, NULL,
        PyQuery_address_doc, NULL},
    {"server_tag", (getter)PyQuery_server_tag_get, NULL,
        PyQuery_server_tag_doc, NULL},
    {"unwanted", (getter)PyQuery_unwanted_get, NULL,
        PyQuery_unwanted_doc, NULL},
    {NULL}
};

PyDoc_STRVAR(change_server_doc,
    "change_server(server) -> None\n"
    "\n"
    "Change the active server for the query.\n"
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
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name      = "irssi.Query",                            /*tp_name*/
    .tp_basicsize = sizeof(PyQuery),                          /*tp_basicsize*/
    .tp_dealloc   = (destructor)PyQuery_dealloc,              /*tp_dealloc*/
    .tp_flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    .tp_doc       = "PyQuery objects",                        /* tp_doc */
    .tp_methods   = PyQuery_methods,                          /* tp_methods */
    .tp_getset    = PyQuery_getseters,                        /* tp_getset */
    .tp_base      = &PyWindowItemType,                        /* tp_base */
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

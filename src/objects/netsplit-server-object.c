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
#include "pyirssi_irc.h"
#include "pymodule.h"
#include "netsplit-object.h"
#include "factory.h"
#include "pycore.h"

#define NETSPLIT_SERVER(ns) ((NETSPLIT_SERVER_REC*)ns)

/* monitor "netsplit remove" signal */
static void netsplit_server_cleanup(NETSPLIT_SERVER_REC *netsplit)
{
    PyNetsplitServer *pynetsplit = signal_get_user_data();

    if (netsplit == pynetsplit->data)
    {
        pynetsplit->data = NULL;
        pynetsplit->cleanup_installed = 0;
        signal_remove_data("netsplit remove", netsplit_server_cleanup, pynetsplit);
    }
}

static void PyNetsplitServer_dealloc(PyNetsplitServer *self)
{
    if (self->cleanup_installed)
        signal_remove_data("netsplit remove", netsplit_server_cleanup, self);

    Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *PyNetsplitServer_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyNetsplitServer *self;

    self = (PyNetsplitServer *)type->tp_alloc(type, 0);
    if (!self)
        return NULL;

    return (PyObject *)self;
}

/* Getters */
PyDoc_STRVAR(PyNetsplitServer_server_doc,
    "The server nick was in"
);
static PyObject *PyNetsplitServer_server_get(PyNetsplitServer *self, void *closure)
{
    RET_NULL_IF_INVALID(self->data);
    RET_AS_STRING_OR_NONE(NETSPLIT_SERVER(self->data)->server);
}

PyDoc_STRVAR(PyNetsplitServer_destserver_doc,
    "The other server where split occured."
);
static PyObject *PyNetsplitServer_destserver_get(PyNetsplitServer *self, void *closure)
{
    RET_NULL_IF_INVALID(self->data);
    RET_AS_STRING_OR_NONE(NETSPLIT_SERVER(self->data)->destserver);
}

PyDoc_STRVAR(PyNetsplitServer_count_doc,
    "Number of splits in server"
);
static PyObject *PyNetsplitServer_count_get(PyNetsplitServer *self, void *closure)
{
    RET_NULL_IF_INVALID(self->data);
    return PyLong_FromLong(NETSPLIT_SERVER(self->data)->count);
}

/* specialized getters/setters */
static PyGetSetDef PyNetsplitServer_getseters[] = {
    {"server", (getter)PyNetsplitServer_server_get, NULL,
        PyNetsplitServer_server_doc, NULL},
    {"destserver", (getter)PyNetsplitServer_destserver_get, NULL,
        PyNetsplitServer_destserver_doc, NULL},
    {"count", (getter)PyNetsplitServer_count_get, NULL,
        PyNetsplitServer_count_doc, NULL},
    {NULL}
};

/* Methods */
/* Methods for object */
static PyMethodDef PyNetsplitServer_methods[] = {
    {NULL}  /* Sentinel */
};

PyTypeObject PyNetsplitServerType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name      = "irssi.NetsplitServer",                   /*tp_name*/
    .tp_basicsize = sizeof(PyNetsplitServer),                 /*tp_basicsize*/
    .tp_dealloc   = (destructor)PyNetsplitServer_dealloc,     /*tp_dealloc*/
    .tp_flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    .tp_doc       = "PyNetsplitServer objects",               /* tp_doc */
    .tp_methods   = PyNetsplitServer_methods,                 /* tp_methods */
    .tp_getset    = PyNetsplitServer_getseters,               /* tp_getset */
    .tp_new       = PyNetsplitServer_new,                     /* tp_new */
};

/* window item wrapper factory function */
PyObject *pynetsplit_server_new(void *nss)
{
    PyNetsplitServer *pynss;

    pynss = py_inst(PyNetsplitServer, PyNetsplitServerType);
    if (!pynss)
        return NULL;

    pynss->data = nss;
    pynss->cleanup_installed = 1;
    signal_add_last_data("netsplit server remove", netsplit_server_cleanup, pynss);

    return (PyObject *)pynss;
}

int netsplit_server_object_init(void) 
{
    g_return_val_if_fail(py_module != NULL, 0);

    if (PyType_Ready(&PyNetsplitServerType) < 0)
        return 0;
    
    Py_INCREF(&PyNetsplitServerType);
    PyModule_AddObject(py_module, "NetsplitServer", (PyObject *)&PyNetsplitServerType);

    return 1;
}

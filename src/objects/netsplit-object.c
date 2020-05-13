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

#define NETSPLIT(ns) ((NETSPLIT_REC*)ns)

/* monitor "netsplit remove" signal */
static void netsplit_cleanup(NETSPLIT_REC *netsplit)
{
    PyNetsplit *pynetsplit = signal_get_user_data();

    if (netsplit == pynetsplit->data)
    {
        pynetsplit->data = NULL;
        pynetsplit->cleanup_installed = 0;
        signal_remove_data("netsplit remove", netsplit_cleanup, pynetsplit);
    }
}

static void PyNetsplit_dealloc(PyNetsplit *self)
{
    if (self->cleanup_installed)
        signal_remove_data("netsplit remove", netsplit_cleanup, self);

    Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *PyNetsplit_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyNetsplit *self;

    self = (PyNetsplit *)type->tp_alloc(type, 0);
    if (!self)
        return NULL;

    return (PyObject *)self;
}

/* Getters */
PyDoc_STRVAR(PyNetsplit_nick_doc,
    "Nick"
);
static PyObject *PyNetsplit_nick_get(PyNetsplit *self, void *closure)
{
    RET_NULL_IF_INVALID(self->data);
    RET_AS_STRING_OR_NONE(NETSPLIT(self->data)->nick);
}

PyDoc_STRVAR(PyNetsplit_address_doc,
    "Nick's host"
);
static PyObject *PyNetsplit_address_get(PyNetsplit *self, void *closure)
{
    RET_NULL_IF_INVALID(self->data);
    RET_AS_STRING_OR_NONE(NETSPLIT(self->data)->address);
}

PyDoc_STRVAR(PyNetsplit_destroy_doc,
    "Timestamp when this record should be destroyed"
);
static PyObject *PyNetsplit_destroy_get(PyNetsplit *self, void *closure)
{
    RET_NULL_IF_INVALID(self->data);
    return PyLong_FromUnsignedLong(NETSPLIT(self->data)->destroy);
}

PyDoc_STRVAR(PyNetsplit_server_doc,
    "Netsplitserver object"
);
static PyObject *PyNetsplit_server_get(PyNetsplit *self, void *closure)
{
    RET_NULL_IF_INVALID(self->data);
    RET_AS_OBJ_OR_NONE(self->server);
}

/* specialized getters/setters */
static PyGetSetDef PyNetsplit_getseters[] = {
    {"nick", (getter)PyNetsplit_nick_get, NULL,
        PyNetsplit_nick_doc, NULL},
    {"address", (getter)PyNetsplit_address_get, NULL,
        PyNetsplit_address_doc, NULL},
    {"destroy", (getter)PyNetsplit_destroy_get, NULL,
        PyNetsplit_destroy_doc, NULL},
    {"server", (getter)PyNetsplit_server_get, NULL,
        PyNetsplit_server_doc, NULL},
    {NULL}
};

/* Methods */
PyDoc_STRVAR(PyNetsplit_channels_doc,
    "channels() -> list of NetsplitChannel objects\n"
    "\n"
    "Return list of NetsplitChannel objects\n"
);
static PyObject *PyNetsplit_channels(PyNetsplit *self, PyObject *args)
{
    RET_NULL_IF_INVALID(self->data);
    return py_irssi_objlist_new(NETSPLIT(self->data)->channels, 1, 
            (InitFunc)pynetsplit_channel_new);
}

/* Methods for object */
static PyMethodDef PyNetsplit_methods[] = {
    {"channels", (PyCFunction)PyNetsplit_channels, METH_NOARGS,
        PyNetsplit_channels_doc},
    {NULL}  /* Sentinel */
};

PyTypeObject PyNetsplitType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name      = "irssi.Netsplit",                         /*tp_name*/
    .tp_basicsize = sizeof(PyNetsplit),                       /*tp_basicsize*/
    .tp_dealloc   = (destructor)PyNetsplit_dealloc,           /*tp_dealloc*/
    .tp_flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    .tp_doc       = "PyNetsplit objects",                     /* tp_doc */
    .tp_methods   = PyNetsplit_methods,                       /* tp_methods */
    .tp_getset    = PyNetsplit_getseters,                     /* tp_getset */
    .tp_new       = PyNetsplit_new,                           /* tp_new */
};

/* window item wrapper factory function */
PyObject *pynetsplit_new(void *netsplit)
{
    PyNetsplit *pynetsplit;

    //FIXME: add netsplit server 
    
    pynetsplit = py_inst(PyNetsplit, PyNetsplitType);
    if (!pynetsplit)
        return NULL;

    pynetsplit->data = netsplit;
    pynetsplit->cleanup_installed = 1;
    signal_add_last_data("netsplit remove", netsplit_cleanup, pynetsplit);

    return (PyObject *)pynetsplit;
}

int netsplit_object_init(void) 
{
    g_return_val_if_fail(py_module != NULL, 0);

    if (PyType_Ready(&PyNetsplitType) < 0)
        return 0;
    
    Py_INCREF(&PyNetsplitType);
    PyModule_AddObject(py_module, "Netsplit", (PyObject *)&PyNetsplitType);

    return 1;
}

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
#include "pycore.h"
#include "factory.h"
#include "reconnect-object.h"

/*XXX: no Reconnect cleanup/destroy sig. Maybe value copy the two members? */

static void PyReconnect_dealloc(PyReconnect *self)
{
    Py_XDECREF(self->connect);
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *PyReconnect_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyReconnect *self;

    self = (PyReconnect *)type->tp_alloc(type, 0);
    if (!self)
        return NULL;

    return (PyObject *)self;
}

/* Getters */
PyDoc_STRVAR(PyReconnect_tag_doc,
    "Unique numeric tag"
);
static PyObject *PyReconnect_tag_get(PyReconnect *self, void *closure)
{
    RECONNECT_REC *data = self->data;
    RET_NULL_IF_INVALID(self->data);

    return PyLong_FromLong(data->tag);
}

PyDoc_STRVAR(PyReconnect_next_connect_doc,
    "Unix time stamp when the next connection occurs"
);
static PyObject *PyReconnect_next_connect_get(PyReconnect *self, void *closure)
{
    RECONNECT_REC *data = self->data;
    RET_NULL_IF_INVALID(self->data);

    return PyLong_FromUnsignedLong(data->next_connect);
}

PyDoc_STRVAR(PyReconnect_connect_doc,
    "Connection object"
);
static PyObject *PyReconnect_connect_get(PyReconnect *self, void *closure)
{
    RET_NULL_IF_INVALID(self->data);
    RET_AS_OBJ_OR_NONE(self->connect);
}

/* specialized getters/setters */
static PyGetSetDef PyReconnect_getseters[] = {
    {"tag", (getter)PyReconnect_tag_get, NULL,
        PyReconnect_tag_doc, NULL},
    {"next_connect", (getter)PyReconnect_next_connect_get, NULL,
        PyReconnect_next_connect_doc, NULL},
    {"connect", (getter)PyReconnect_connect_get, NULL,
        PyReconnect_connect_doc, NULL},
    {NULL}
};

/* Methods for object */
static PyMethodDef PyReconnect_methods[] = {
    {NULL}  /* Sentinel */
};

PyTypeObject PyReconnectType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name      = "irssi.Reconnect",                        /*tp_name*/
    .tp_basicsize = sizeof(PyReconnect),                      /*tp_basicsize*/
    .tp_dealloc   = (destructor)PyReconnect_dealloc,          /*tp_dealloc*/
    .tp_flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    .tp_doc       = "PyReconnect objects",                    /* tp_doc */
    .tp_methods   = PyReconnect_methods,                      /* tp_methods */
    .tp_getset    = PyReconnect_getseters,                    /* tp_getset */
    .tp_new       = PyReconnect_new,                          /* tp_new */
};

/* window item wrapper factory function */
PyObject *pyreconnect_new(void *recon)
{
    RECONNECT_REC *rec = recon;
    PyObject *connect;
    PyReconnect *pyrecon;

    /* XXX: get a managed connect because there's no signals to manage reconnect */
    connect = py_irssi_chat_new(rec->conn, 1);
    if (!connect)
        return NULL;

    pyrecon = py_inst(PyReconnect, PyReconnectType);
    if (!pyrecon)
        return NULL;

    pyrecon->data = recon;
    pyrecon->connect = connect;

    return (PyObject *)pyrecon;
}

int reconnect_object_init(void) 
{
    g_return_val_if_fail(py_module != NULL, 0);

    if (PyType_Ready(&PyReconnectType) < 0)
        return 0;
    
    Py_INCREF(&PyReconnectType);
    PyModule_AddObject(py_module, "Reconnect", (PyObject *)&PyReconnectType);

    return 1;
}

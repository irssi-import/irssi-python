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
#include "netsplit-channel-object.h"
#include "factory.h"
#include "pycore.h"

/* value copied -- no special cleanup */

static void PyNetsplitChannel_dealloc(PyNetsplitChannel *self)
{
    Py_XDECREF(self->name);

    Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *PyNetsplitChannel_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyNetsplitChannel *self;

    self = (PyNetsplitChannel *)type->tp_alloc(type, 0);
    if (!self)
        return NULL;

    return (PyObject *)self;
}

/* Getters */
PyDoc_STRVAR(PyNetsplitChannel_name_doc,
    "Channel name"
);
static PyObject *PyNetsplitChannel_name_get(PyNetsplitChannel *self, void *closure)
{
    RET_AS_OBJ_OR_NONE(self->name);
}

PyDoc_STRVAR(PyNetsplitChannel_op_doc,
    "is op"
);
static PyObject *PyNetsplitChannel_op_get(PyNetsplitChannel *self, void *closure)
{
    return PyBool_FromLong(self->op);
}

PyDoc_STRVAR(PyNetsplitChannel_halfop_doc,
    "is halfop"
);
static PyObject *PyNetsplitChannel_halfop_get(PyNetsplitChannel *self, void *closure)
{
    return PyBool_FromLong(self->halfop);
}

PyDoc_STRVAR(PyNetsplitChannel_voice_doc,
    "is voice"
);
static PyObject *PyNetsplitChannel_voice_get(PyNetsplitChannel *self, void *closure)
{
    return PyBool_FromLong(self->voice);
}

PyDoc_STRVAR(PyNetsplitChannel_other_doc,
    "?"
);
static PyObject *PyNetsplitChannel_other_get(PyNetsplitChannel *self, void *closure)
{
    return PyLong_FromLong(self->other);
}

/* specialized getters/setters */
static PyGetSetDef PyNetsplitChannel_getseters[] = {
    {"name", (getter)PyNetsplitChannel_name_get, NULL,
        PyNetsplitChannel_name_doc, NULL},
    {"op", (getter)PyNetsplitChannel_op_get, NULL,
        PyNetsplitChannel_op_doc, NULL},
    {"halfop", (getter)PyNetsplitChannel_halfop_get, NULL,
        PyNetsplitChannel_halfop_doc, NULL},
    {"voice", (getter)PyNetsplitChannel_voice_get, NULL,
        PyNetsplitChannel_voice_doc, NULL},
    {"other", (getter)PyNetsplitChannel_other_get, NULL,
        PyNetsplitChannel_other_doc, NULL},
    {NULL}
};

/* Methods */
/* Methods for object */
static PyMethodDef PyNetsplitChannel_methods[] = {
    {NULL}  /* Sentinel */
};

PyTypeObject PyNetsplitChannelType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name      = "irssi.NetsplitChannel",                  /*tp_name*/
    .tp_basicsize = sizeof(PyNetsplitChannel),                /*tp_basicsize*/
    .tp_dealloc   = (destructor)PyNetsplitChannel_dealloc,    /*tp_dealloc*/
    .tp_flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    .tp_doc       = "PyNetsplitChannel objects",              /* tp_doc */
    .tp_methods   = PyNetsplitChannel_methods,                /* tp_methods */
    .tp_getset    = PyNetsplitChannel_getseters,              /* tp_getset */
    .tp_new       = PyNetsplitChannel_new,                    /* tp_new */
};

/* window item wrapper factory function */
PyObject *pynetsplit_channel_new(void *netsplit)
{
    NETSPLIT_CHAN_REC *rec = netsplit;
    PyNetsplitChannel *pynetsplit;
    PyObject *name;

    name = PyBytes_FromString(rec->name);
    if (!name)
        return NULL;

    pynetsplit = py_inst(PyNetsplitChannel, PyNetsplitChannelType);
    if (!pynetsplit)
    {
        Py_DECREF(name);
        return NULL;
    }

    pynetsplit->name = name;
    pynetsplit->op = rec->op;
    pynetsplit->halfop = rec->halfop;
    pynetsplit->other = 0;
    
    return (PyObject *)pynetsplit;
}

int netsplit_channel_object_init(void) 
{
    g_return_val_if_fail(py_module != NULL, 0);

    if (PyType_Ready(&PyNetsplitChannelType) < 0)
        return 0;
    
    Py_INCREF(&PyNetsplitChannelType);
    PyModule_AddObject(py_module, "NetsplitChannel", (PyObject *)&PyNetsplitChannelType);

    return 1;
}

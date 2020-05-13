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
#include "logitem-object.h"
#include "pycore.h"

/* no special cleanup -- value copy is made */

static void PyLogitem_dealloc(PyLogitem *self)
{
    Py_XDECREF(self->type);
    Py_XDECREF(self->name);
    Py_XDECREF(self->servertag);

    Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *PyLogitem_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyLogitem *self;

    self = (PyLogitem *)type->tp_alloc(type, 0);
    if (!self)
        return NULL;

    return (PyObject *)self;
}

/* Getters */
PyDoc_STRVAR(PyLogitem_type_doc,
    "0=target, 1=window refnum"
);
static PyObject *PyLogitem_type_get(PyLogitem *self, void *closure)
{
    RET_AS_OBJ_OR_NONE(self->type);
}

PyDoc_STRVAR(PyLogitem_name_doc,
    "Name"
);
static PyObject *PyLogitem_name_get(PyLogitem *self, void *closure)
{
    RET_AS_OBJ_OR_NONE(self->name);
}

PyDoc_STRVAR(PyLogitem_servertag_doc,
    "Server tag"
);
static PyObject *PyLogitem_servertag_get(PyLogitem *self, void *closure)
{
    RET_AS_OBJ_OR_NONE(self->servertag);
}

/* specialized getters/setters */
static PyGetSetDef PyLogitem_getseters[] = {
    {"type", (getter)PyLogitem_type_get, NULL,
        PyLogitem_type_doc, NULL},
    {"name", (getter)PyLogitem_name_get, NULL,
        PyLogitem_name_doc, NULL},
    {"servertag", (getter)PyLogitem_servertag_get, NULL,
        PyLogitem_servertag_doc, NULL},
    {NULL}
};

/* Methods for object */
static PyMethodDef PyLogitem_methods[] = {
    {NULL}  /* Sentinel */
};

PyTypeObject PyLogitemType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name      = "irssi.Logitem",                          /*tp_name*/
    .tp_basicsize = sizeof(PyLogitem),                        /*tp_basicsize*/
    .tp_dealloc   = (destructor)PyLogitem_dealloc,            /*tp_dealloc*/
    .tp_flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    .tp_doc       = "PyLogitem objects",                      /* tp_doc */
    .tp_methods   = PyLogitem_methods,                        /* tp_methods */
    .tp_getset    = PyLogitem_getseters,                      /* tp_getset */
    .tp_new       = PyLogitem_new,                            /* tp_new */
};

/* log item factory function */
PyObject *pylogitem_new(void *log)
{
    LOG_ITEM_REC *li = log;
    PyLogitem *pylog = NULL;

    pylog = py_inst(PyLogitem, PyLogitemType);
    if (!pylog)
        return NULL;

    pylog->type = PyLong_FromLong(li->type);
    if (!pylog->type)
        goto error;

    pylog->name = PyBytes_FromString(li->name);
    if (!pylog->name)
        goto error;

    if (li->servertag)
    {
        pylog->servertag = PyBytes_FromString(li->servertag);
        if (!pylog->servertag)
            goto error;
    }

    return (PyObject *)pylog;

error:
    Py_XDECREF(pylog);
    return NULL;
}

int logitem_object_init(void) 
{
    g_return_val_if_fail(py_module != NULL, 0);

    if (PyType_Ready(&PyLogitemType) < 0)
        return 0;
    
    Py_INCREF(&PyLogitemType);
    PyModule_AddObject(py_module, "Logitem", (PyObject *)&PyLogitemType);

    return 1;
}

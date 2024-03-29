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
#include "dcc-get-object.h"
#include "factory.h"
#include "pycore.h"

#define DCC_GET_CAST(rec) ((GET_DCC_REC *)rec)

/* inherit destroy and cleanup from Dcc type */

/* Getters */
PyDoc_STRVAR(PyDccGet_size_doc,
    "File size"
);
static PyObject *PyDccGet_size_get(PyDccGet *self, void *closure)
{
    RET_NULL_IF_INVALID(self->data);
    return PyLong_FromUnsignedLong(DCC_GET_CAST(self->data)->size);
}

PyDoc_STRVAR(PyDccGet_skipped_doc,
    "Bytes skipped from start (resuming file)"
);
static PyObject *PyDccGet_skipped_get(PyDccGet *self, void *closure)
{
    RET_NULL_IF_INVALID(self->data);
    return PyLong_FromUnsignedLong(DCC_GET_CAST(self->data)->skipped);
}

PyDoc_STRVAR(PyDccGet_get_type_doc,
    "What to do if file exists? 0=default, 1=rename, 2=overwrite, 3=resume"
);
static PyObject *PyDccGet_get_type_get(PyDccGet *self, void *closure)
{
    RET_NULL_IF_INVALID(self->data);
    return PyLong_FromLong(DCC_GET_CAST(self->data)->get_type);
}

PyDoc_STRVAR(PyDccGet_file_doc,
    "The real file name which we use."
);
static PyObject *PyDccGet_file_get(PyDccGet *self, void *closure)
{
    RET_NULL_IF_INVALID(self->data);
    RET_AS_STRING_OR_NONE(DCC_GET_CAST(self->data)->file);
}

PyDoc_STRVAR(PyDccGet_file_quoted_doc,
    "true if file name was received quoted (\"file name\")"
);
static PyObject *PyDccGet_file_quoted_get(PyDccGet *self, void *closure)
{
    RET_NULL_IF_INVALID(self->data);
    return PyBool_FromLong(DCC_GET_CAST(self->data)->file_quoted);
}

/* specialized getters/setters */
static PyGetSetDef PyDccGet_getseters[] = {
    {"size", (getter)PyDccGet_size_get, NULL,
        PyDccGet_size_doc, NULL},
    {"skipped", (getter)PyDccGet_skipped_get, NULL,
        PyDccGet_skipped_doc, NULL},
    {"get_type", (getter)PyDccGet_get_type_get, NULL,
        PyDccGet_get_type_doc, NULL},
    {"file", (getter)PyDccGet_file_get, NULL,
        PyDccGet_file_doc, NULL},
    {"file_quoted", (getter)PyDccGet_file_quoted_get, NULL,
        PyDccGet_file_quoted_doc, NULL},
    {NULL}
};

/* Methods */
/* Methods for object */
static PyMethodDef PyDccGet_methods[] = {
    {NULL}  /* Sentinel */
};

PyTypeObject PyDccGetType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name      = "irssi.DccGet",                           /*tp_name*/
    .tp_basicsize = sizeof(PyDccGet),                         /*tp_basicsize*/
    .tp_flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    .tp_doc       = "PyDccGet objects",                       /* tp_doc */
    .tp_methods   = PyDccGet_methods,                         /* tp_methods */
    .tp_getset    = PyDccGet_getseters,                       /* tp_getset */
    .tp_base      = &PyDccType,                               /* tp_base */
};

PyObject *pydcc_get_new(void *dcc)
{
    static const char *name = "DCC GET";
    return pydcc_sub_new(dcc, name, &PyDccGetType);
}

int dcc_get_object_init(void) 
{
    g_return_val_if_fail(py_module != NULL, 0);

    if (PyType_Ready(&PyDccGetType) < 0)
        return 0;
    
    Py_INCREF(&PyDccGetType);
    PyModule_AddObject(py_module, "DccGet", (PyObject *)&PyDccGetType);

    return 1;
}

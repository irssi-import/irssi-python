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
#include "dcc-send-object.h"
#include "factory.h"
#include "pycore.h"

#define DCC_SEND_CAST(rec) ((SEND_DCC_REC *)rec)

/* inherit destroy and cleanup from Dcc type */

/* Getters */
PyDoc_STRVAR(PyDccSend_size_doc,
    "File size"
);
static PyObject *PyDccSend_size_get(PyDccSend *self, void *closure)
{
    RET_NULL_IF_INVALID(self->data);
    return PyLong_FromUnsignedLong(DCC_SEND_CAST(self->data)->size);
}

PyDoc_STRVAR(PyDccSend_skipped_doc,
    "Bytes skipped from start (resuming file)"
);
static PyObject *PyDccSend_skipped_get(PyDccSend *self, void *closure)
{
    RET_NULL_IF_INVALID(self->data);
    return PyLong_FromUnsignedLong(DCC_SEND_CAST(self->data)->skipped);
}

PyDoc_STRVAR(PyDccSend_file_quoted_doc,
    "True if file name was received quoted (\"file name\")"
);
static PyObject *PyDccSend_file_quoted_get(PyDccSend *self, void *closure)
{
    RET_NULL_IF_INVALID(self->data);
    return PyBool_FromLong(DCC_SEND_CAST(self->data)->file_quoted);
}

PyDoc_STRVAR(PyDccSend_waitforend_doc,
    "File is sent, just wait for the replies from the other side"
);
static PyObject *PyDccSend_waitforend_get(PyDccSend *self, void *closure)
{
    RET_NULL_IF_INVALID(self->data);
    return PyBool_FromLong(DCC_SEND_CAST(self->data)->waitforend);
}

PyDoc_STRVAR(PyDccSend_gotalldata_doc,
    "Got all acks from the other end"
);
static PyObject *PyDccSend_gotalldata_get(PyDccSend *self, void *closure)
{
    RET_NULL_IF_INVALID(self->data);
    return PyBool_FromLong(DCC_SEND_CAST(self->data)->gotalldata);
}

/* specialized getters/setters */
static PyGetSetDef PyDccSend_getseters[] = {
    {"size", (getter)PyDccSend_size_get, NULL,
        PyDccSend_size_doc, NULL},
    {"skipped", (getter)PyDccSend_skipped_get, NULL,
        PyDccSend_skipped_doc, NULL},
    {"file_quoted", (getter)PyDccSend_file_quoted_get, NULL,
        PyDccSend_file_quoted_doc, NULL},
    {"waitforend", (getter)PyDccSend_waitforend_get, NULL,
        PyDccSend_waitforend_doc, NULL},
    {"gotalldata", (getter)PyDccSend_gotalldata_get, NULL,
        PyDccSend_gotalldata_doc, NULL},
    {NULL}
};

/* Methods */
/* Methods for object */
static PyMethodDef PyDccSend_methods[] = {
    {NULL}  /* Sentinel */
};

PyTypeObject PyDccSendType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name      = "irssi.DccSend",                          /*tp_name*/
    .tp_basicsize = sizeof(PyDccSend),                        /*tp_basicsize*/
    .tp_flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    .tp_doc       = "PyDccSend objects",                      /* tp_doc */
    .tp_methods   = PyDccSend_methods,                        /* tp_methods */
    .tp_getset    = PyDccSend_getseters,                      /* tp_getset */
    .tp_base      = &PyDccType,                               /* tp_base */
};

PyObject *pydcc_send_new(void *dcc)
{
    static const char *name = "DCC SEND";
    return pydcc_sub_new(dcc, name, &PyDccSendType);
}

int dcc_send_object_init(void) 
{
    g_return_val_if_fail(py_module != NULL, 0);

    if (PyType_Ready(&PyDccSendType) < 0)
        return 0;
    
    Py_INCREF(&PyDccSendType);
    PyModule_AddObject(py_module, "DccSend", (PyObject *)&PyDccSendType);

    return 1;
}

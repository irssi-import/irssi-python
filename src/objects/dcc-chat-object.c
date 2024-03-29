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
#include "dcc-chat-object.h"
#include "factory.h"
#include "pycore.h"

/* inherit destroy and cleanup from DccChat type */

/* Getters */
PyDoc_STRVAR(PyDccChat_id_doc,
    "Unique identifier - usually same as nick"
);
static PyObject *PyDccChat_id_get(PyDccChat *self, void *closure)
{
    RET_NULL_IF_INVALID(self->data);
    RET_AS_STRING_OR_NONE(self->data->id);
}

PyDoc_STRVAR(PyDccChat_mirc_ctcp_doc,
    "Send CTCPs without the CTCP_MESSAGE prefix"
);
static PyObject *PyDccChat_mirc_ctcp_get(PyDccChat *self, void *closure)
{
    RET_NULL_IF_INVALID(self->data);
    return PyBool_FromLong(self->data->mirc_ctcp);
}

PyDoc_STRVAR(PyDccChat_connection_lost_doc,
    "Other side closed connection"
);
static PyObject *PyDccChat_connection_lost_get(PyDccChat *self, void *closure)
{
    RET_NULL_IF_INVALID(self->data);
    return PyBool_FromLong(self->data->connection_lost);
}

/* specialized getters/setters */
static PyGetSetDef PyDccChat_getseters[] = {
    {"id", (getter)PyDccChat_id_get, NULL,
        PyDccChat_id_doc, NULL},
    {"mirc_ctcp", (getter)PyDccChat_mirc_ctcp_get, NULL,
        PyDccChat_mirc_ctcp_doc, NULL},
    {"connection_lost", (getter)PyDccChat_connection_lost_get, NULL,
        PyDccChat_connection_lost_doc, NULL},
    {NULL}
};

/* Methods */
PyDoc_STRVAR(PyDccChat_chat_send_doc,
    "chat_send(data) -> None\n"
    "\n"
    "Send data to a dcc chat session.\n"
);
static PyObject *PyDccChat_chat_send(PyDccChat *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"data", NULL};
    char *data = "";

    RET_NULL_IF_INVALID(self->data);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "y", kwlist, &data))
        return NULL;

    dcc_chat_send(self->data, data);
    
    Py_RETURN_NONE;
}

/* Methods for object */
static PyMethodDef PyDccChat_methods[] = {
    {"chat_send", (PyCFunction)PyDccChat_chat_send, METH_VARARGS | METH_KEYWORDS,
        PyDccChat_chat_send_doc},
    {NULL}  /* Sentinel */
};

PyTypeObject PyDccChatType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name      = "irssi.DccChat",                          /*tp_name*/
    .tp_basicsize = sizeof(PyDccChat),                        /*tp_basicsize*/
    .tp_flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    .tp_doc       = "PyDccChat objects",                      /* tp_doc */
    .tp_methods   = PyDccChat_methods,                        /* tp_methods */
    .tp_getset    = PyDccChat_getseters,                      /* tp_getset */
    .tp_base      = &PyDccType,                               /* tp_base */
};

PyObject *pydcc_chat_new(void *dcc)
{
    static const char *name = "DCC CHAT";
    return pydcc_sub_new(dcc, name, &PyDccChatType);
}

int dcc_chat_object_init(void) 
{
    g_return_val_if_fail(py_module != NULL, 0);

    if (PyType_Ready(&PyDccChatType) < 0)
        return 0;
    
    Py_INCREF(&PyDccChatType);
    PyModule_AddObject(py_module, "DccChat", (PyObject *)&PyDccChatType);

    return 1;
}

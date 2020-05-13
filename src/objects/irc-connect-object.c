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
#include "pymodule.h"
#include "base-objects.h"
#include "irc-connect-object.h"
#include "pyirssi_irc.h"
#include "pycore.h"
#include "pyutils.h"

/* cleanup and deallocation handled by Connect base */

/* Getters */
PyDoc_STRVAR(PyIrcConnect_alternate_nick_doc,
    "Alternate nick to use if default nick is taken"
);
static PyObject *PyIrcConnect_alternate_nick_get(PyIrcConnect *self, void *closure)
{
    RET_NULL_IF_INVALID(self->data);
    RET_AS_STRING_OR_NONE(self->data->alternate_nick);
}

/* Get/Set */
static PyGetSetDef PyIrcConnect_getseters[] = {
    {"alternate_nick", (getter)PyIrcConnect_alternate_nick_get, NULL,
        PyIrcConnect_alternate_nick_doc, NULL},
    {NULL}
};

PyTypeObject PyIrcConnectType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name      = "irssi.IrcConnect",                       /*tp_name*/
    .tp_basicsize = sizeof(PyIrcConnect),                     /*tp_basicsize*/
    .tp_flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    .tp_doc       = "PyIrcConnect objects",                   /* tp_doc */
    .tp_getset    = PyIrcConnect_getseters,                   /* tp_getset */
    .tp_base      = &PyConnectType,                           /* tp_base */
};

PyObject *pyirc_connect_new(void *connect, int managed)
{
    return pyconnect_sub_new(connect, &PyIrcConnectType, managed);
}

int irc_connect_object_init(void) 
{
    g_return_val_if_fail(py_module != NULL, 0);

    if (PyType_Ready(&PyIrcConnectType) < 0)
        return 0;
    
    Py_INCREF(&PyIrcConnectType);
    PyModule_AddObject(py_module, "IrcConnect", (PyObject *)&PyIrcConnectType);

    return 1;
}

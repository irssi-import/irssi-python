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
#include "chatnet-object.h"
#include "pyirssi.h"
#include "pycore.h"
#include "pyutils.h"

static void chatnet_cleanup(CHATNET_REC *cn)
{
    PyChatnet *pycn = signal_get_user_data();

    if (cn == pycn->data)
    {
        pycn->data = NULL;
        pycn->cleanup_installed = 0;
        signal_remove_data("chatnet destroyed", chatnet_cleanup, pycn);
    }
}

static void PyChatnet_dealloc(PyChatnet *self)
{
    if (self->cleanup_installed)
        signal_remove_data("chatnet destroyed", chatnet_cleanup, self);

    Py_TYPE(self)->tp_free((PyObject *)self);
}

/* Getters */
PyDoc_STRVAR(PyChatnet_name_doc,
    "name of chat network"
);
static PyObject *PyChatnet_name_get(PyChatnet *self, void *closure)
{
    RET_NULL_IF_INVALID(self->data);
    RET_AS_STRING_OR_NONE(self->data->name);
}

PyDoc_STRVAR(PyChatnet_nick_doc,
    "if not empty, nick preferred in this network"
);
static PyObject *PyChatnet_nick_get(PyChatnet *self, void *closure)
{
    RET_NULL_IF_INVALID(self->data);
    RET_AS_STRING_OR_NONE(self->data->nick);
}

PyDoc_STRVAR(PyChatnet_username_doc,
    "if not empty, username preferred in this network"
);
static PyObject *PyChatnet_username_get(PyChatnet *self, void *closure)
{
    RET_NULL_IF_INVALID(self->data);
    RET_AS_STRING_OR_NONE(self->data->username);
}

PyDoc_STRVAR(PyChatnet_realname_doc,
    "if not empty, realname preferred in this network"
);
static PyObject *PyChatnet_realname_get(PyChatnet *self, void *closure)
{
    RET_NULL_IF_INVALID(self->data);
    RET_AS_STRING_OR_NONE(self->data->realname);
}

PyDoc_STRVAR(PyChatnet_own_host_doc,
    "address to use when connecting to this network"
);
static PyObject *PyChatnet_own_host_get(PyChatnet *self, void *closure)
{
    RET_NULL_IF_INVALID(self->data);
    RET_AS_STRING_OR_NONE(self->data->own_host);
}

PyDoc_STRVAR(PyChatnet_autosendcmd_doc,
    "command to send after connecting to this network"
);
static PyObject *PyChatnet_autosendcmd_get(PyChatnet *self, void *closure)
{
    RET_NULL_IF_INVALID(self->data);
    RET_AS_STRING_OR_NONE(self->data->autosendcmd);
}

/* specialized getters/setters */
static PyGetSetDef PyChatnet_getseters[] = {
    {"name", (getter)PyChatnet_name_get, NULL,
        PyChatnet_name_doc, NULL},
    {"nick", (getter)PyChatnet_nick_get, NULL,
        PyChatnet_nick_doc, NULL},
    {"username", (getter)PyChatnet_username_get, NULL,
        PyChatnet_username_doc, NULL},
    {"realname", (getter)PyChatnet_realname_get, NULL,
        PyChatnet_realname_doc, NULL},
    {"own_host", (getter)PyChatnet_own_host_get, NULL,
        PyChatnet_own_host_doc, NULL},
    {"autosendcmd", (getter)PyChatnet_autosendcmd_get, NULL,
        PyChatnet_autosendcmd_doc, NULL},
    {NULL}
};

static PyMethodDef PyChatnet_methods[] = {
    {NULL}  /* Sentinel */
};

PyTypeObject PyChatnetType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name      = "irssi.Chatnet",                          /*tp_name*/
    .tp_basicsize = sizeof(PyChatnet),                        /*tp_basicsize*/
    .tp_dealloc   = (destructor)PyChatnet_dealloc,            /*tp_dealloc*/
    .tp_flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    .tp_doc       = "PyChatnet objects",                      /* tp_doc */
    .tp_methods   = PyChatnet_methods,                        /* tp_methods */
    .tp_getset    = PyChatnet_getseters,                      /* tp_getset */
    .tp_base      = &PyIrssiChatBaseType,                     /* tp_base */
};

/* chatnet factory function */
PyObject *pychatnet_sub_new(void *cn, PyTypeObject *subclass)
{
    static const char *name = "CHATNET";
    PyChatnet *pycn = NULL;

    pycn = py_instp(PyChatnet, subclass); 
    if (!pycn)
        return NULL;

    pycn->data = cn;
    pycn->base_name = name;
    signal_add_last_data("chatnet destroyed", chatnet_cleanup, pycn);
    pycn->cleanup_installed = 1;

    return (PyObject *)pycn;
}

PyObject *pychatnet_new(void *cn)
{
    return pychatnet_sub_new(cn, &PyChatnetType);
}

int chatnet_object_init(void) 
{
    g_return_val_if_fail(py_module != NULL, 0);

    if (PyType_Ready(&PyChatnetType) < 0)
        return 0;
    
    Py_INCREF(&PyChatnetType);
    PyModule_AddObject(py_module, "Chatnet", (PyObject *)&PyChatnetType);

    return 1;
}

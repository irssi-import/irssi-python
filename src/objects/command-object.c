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
#include "command-object.h"
#include "pycore.h"

#define COMMAND(cmd) ((COMMAND_REC *)cmd)

/* monitor "commandlist remove" signal */
static void command_cleanup(COMMAND_REC *command)
{
    PyCommand *pycommand = signal_get_user_data();

    if (command == pycommand->data)
    {
        pycommand->data = NULL;
        pycommand->cleanup_installed = 0;
        signal_remove_data("commandlist remove", command_cleanup, pycommand);
    }
}

static void PyCommand_dealloc(PyCommand *self)
{
    if (self->cleanup_installed)
        signal_remove_data("commandlist remove", command_cleanup, self);

    Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *PyCommand_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyCommand *self;

    self = (PyCommand *)type->tp_alloc(type, 0);
    if (!self)
        return NULL;

    return (PyObject *)self;
}

/* Getters */
PyDoc_STRVAR(PyCommand_cmd_doc,
    "Command name"
);
static PyObject *PyCommand_cmd_get(PyCommand *self, void *closure)
{
    RET_NULL_IF_INVALID(self->data);
    RET_AS_STRING_OR_NONE(COMMAND(self->data)->cmd);
}

PyDoc_STRVAR(PyCommand_category_doc,
    "Category"
);
static PyObject *PyCommand_category_get(PyCommand *self, void *closure)
{
    RET_NULL_IF_INVALID(self->data);
    RET_AS_STRING_OR_NONE(COMMAND(self->data)->category);
}

/* specialized getters/setters */
static PyGetSetDef PyCommand_getseters[] = {
    {"cmd", (getter)PyCommand_cmd_get, NULL,
        PyCommand_cmd_doc, NULL},
    {"category", (getter)PyCommand_category_get, NULL,
        PyCommand_category_doc, NULL},
    {NULL}
};

/* Methods */
/* Methods for object */
static PyMethodDef PyCommand_methods[] = {
    {NULL}  /* Sentinel */
};

PyTypeObject PyCommandType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name      = "irssi.Command",                          /*tp_name*/
    .tp_basicsize = sizeof(PyCommand),                        /*tp_basicsize*/
    .tp_dealloc   = (destructor)PyCommand_dealloc,            /*tp_dealloc*/
    .tp_flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    .tp_doc       = "PyCommand objects",                      /* tp_doc */
    .tp_methods   = PyCommand_methods,                        /* tp_methods */
    .tp_getset    = PyCommand_getseters,                      /* tp_getset */
    .tp_new       = PyCommand_new,                            /* tp_new */
};

/* command factory function */
PyObject *pycommand_new(void *command)
{
    PyCommand *pycommand;

    pycommand = py_inst(PyCommand, PyCommandType);
    if (!pycommand)
        return NULL;

    pycommand->data = command;
    pycommand->cleanup_installed = 1;
    signal_add_last_data("commandlist remove", command_cleanup, pycommand);

    return (PyObject *)pycommand;
}

int command_object_init(void) 
{
    g_return_val_if_fail(py_module != NULL, 0);

    if (PyType_Ready(&PyCommandType) < 0)
        return 0;
    
    Py_INCREF(&PyCommandType);
    PyModule_AddObject(py_module, "Command", (PyObject *)&PyCommandType);

    return 1;
}

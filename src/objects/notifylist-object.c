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
#include "notifylist-object.h"
#include "pycore.h"

#define NOTIFYLIST(nl) ((NOTIFYLIST_REC *)nl)

/* monitor "notifylist remove" signal */
static void notifylist_cleanup(NOTIFYLIST_REC *notifylist)
{
    PyNotifylist *pynotifylist = signal_get_user_data();

    if (notifylist == pynotifylist->data)
    {
        pynotifylist->data = NULL;
        pynotifylist->cleanup_installed = 0;
        signal_remove_data("notifylist remove", notifylist_cleanup, pynotifylist);
    }
}

static void PyNotifylist_dealloc(PyNotifylist *self)
{
    if (self->cleanup_installed)
        signal_remove_data("notifylist remove", notifylist_cleanup, self);

    Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *PyNotifylist_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyNotifylist *self;

    self = (PyNotifylist *)type->tp_alloc(type, 0);
    if (!self)
        return NULL;

    return (PyObject *)self;
}

/* Getters */
PyDoc_STRVAR(PyNotifylist_mask_doc,
    "Notify nick mask"
);
static PyObject *PyNotifylist_mask_get(PyNotifylist *self, void *closure)
{
    RET_NULL_IF_INVALID(self->data);
    RET_AS_STRING_OR_NONE(NOTIFYLIST(self->data)->mask);
}

PyDoc_STRVAR(PyNotifylist_away_check_doc,
    "Notify away status changes"
);
static PyObject *PyNotifylist_away_check_get(PyNotifylist *self, void *closure)
{
    RET_NULL_IF_INVALID(self->data);
    return PyBool_FromLong(NOTIFYLIST(self->data)->away_check);
}

/* specialized getters/setters */
static PyGetSetDef PyNotifylist_getseters[] = {
    {"mask", (getter)PyNotifylist_mask_get, NULL,
        PyNotifylist_mask_doc, NULL},
    {"away_check", (getter)PyNotifylist_away_check_get, NULL,
        PyNotifylist_away_check_doc, NULL},
    {NULL}
};

/* Methods */
PyDoc_STRVAR(PyNotifylist_ircnets_doc,
    "ircnets() -> list of str\n"
    "\n"
    "Return list of ircnets the notify is checked\n"
);
static PyObject *PyNotifylist_ircnets(PyNotifylist *self, PyObject *args)
{
    PyObject *list;
    char **nets;

    RET_NULL_IF_INVALID(self->data);

    nets = NOTIFYLIST(self->data)->ircnets;
    list = PyList_New(0);
    if (!list)
        return NULL;

    while (nets && *nets)
    {
        int ret;
        PyObject *str = PyBytes_FromString(*nets);

        if (!str)
        {
            Py_DECREF(list);
            return NULL;
        }

        ret = PyList_Append(list, str);
        Py_DECREF(str);
        if (ret != 0)
        {
            Py_DECREF(list);
            return NULL;
        }
        
        nets++;
    }

    return list;
}

PyDoc_STRVAR(PyNotifylist_ircnets_match_doc,
    "ircnets_match(ircnet) -> bool\n"
    "\n"
    "Return True if notify is checked in ircnet\n"
);
static PyObject *PyNotifylist_ircnets_match(PyNotifylist *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"ircnet", NULL};
    char *ircnet = "";

    RET_NULL_IF_INVALID(self->data);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "y", kwlist, &ircnet))
        return NULL;

    return PyBool_FromLong(notifylist_ircnets_match(self->data, ircnet));
}

/* Methods for object */
static PyMethodDef PyNotifylist_methods[] = {
    {"ircnets", (PyCFunction)PyNotifylist_ircnets, METH_NOARGS,
        PyNotifylist_ircnets_doc},
    {"ircnets_match", (PyCFunction)PyNotifylist_ircnets_match, METH_VARARGS | METH_KEYWORDS,
        PyNotifylist_ircnets_match_doc},
    {NULL}  /* Sentinel */
};

PyTypeObject PyNotifylistType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name      = "irssi.Notifylist",                       /*tp_name*/
    .tp_basicsize = sizeof(PyNotifylist),                     /*tp_basicsize*/
    .tp_dealloc   = (destructor)PyNotifylist_dealloc,         /*tp_dealloc*/
    .tp_flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    .tp_doc       = "PyNotifylist objects",                   /* tp_doc */
    .tp_methods   = PyNotifylist_methods,                     /* tp_methods */
    .tp_getset    = PyNotifylist_getseters,                   /* tp_getset */
    .tp_new       = PyNotifylist_new,                         /* tp_new */
};

/* window item wrapper factory function */
PyObject *pynotifylist_new(void *notifylist)
{
    PyNotifylist *pynotifylist;

    pynotifylist = py_inst(PyNotifylist, PyNotifylistType);
    if (!pynotifylist)
        return NULL;

    pynotifylist->data = notifylist;
    pynotifylist->cleanup_installed = 1;
    signal_add_last_data("notifylist remove", notifylist_cleanup, pynotifylist);

    return (PyObject *)pynotifylist;
}

int notifylist_object_init(void) 
{
    g_return_val_if_fail(py_module != NULL, 0);

    if (PyType_Ready(&PyNotifylistType) < 0)
        return 0;
    
    Py_INCREF(&PyNotifylistType);
    PyModule_AddObject(py_module, "Notifylist", (PyObject *)&PyNotifylistType);

    return 1;
}

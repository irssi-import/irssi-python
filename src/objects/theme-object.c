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
#include "theme-object.h"
#include "factory.h"
#include "pycore.h"

/* monitor "theme destroyed" signal */
static void theme_cleanup(THEME_REC *rec)
{
    PyTheme *pytheme = signal_get_user_data();
    if (pytheme->data == rec)
    {
        pytheme->data = NULL;
        pytheme->cleanup_installed = 0;
        signal_remove_data("theme destroyed", theme_cleanup, pytheme);
    }
}

static void PyTheme_dealloc(PyTheme *self)
{
    if (self->cleanup_installed)
        signal_remove_data("theme destroyed", theme_cleanup, self);

    Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *PyTheme_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyTheme *self;

    self = (PyTheme *)type->tp_alloc(type, 0);
    if (!self)
        return NULL;

    return (PyObject *)self;
}

/* Getters */
/* specialized getters/setters */
static PyGetSetDef PyTheme_getseters[] = {
    {NULL}
};

/* Methods */
PyDoc_STRVAR(PyTheme_format_expand_doc,
    "format_expand(format, flags=0) -> str or None\n"
);
static PyObject *PyTheme_format_expand(PyTheme *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"format", "flags", NULL};
    char *format = "";
    int flags = 0;
    char *ret;
    PyObject *pyret;

    RET_NULL_IF_INVALID(self->data);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "y|i", kwlist, &format,
                                     &flags))
        return NULL;

    if (flags == 0)
        ret = theme_format_expand(self->data, format);
    else {
	theme_rm_col reset;
	strcpy(reset.m, "n");
        ret = theme_format_expand_data(self->data, (const char **)&format,
		reset, reset, NULL, NULL, EXPAND_FLAG_ROOT | flags);
    }

    if (ret)
    {
        pyret = PyBytes_FromString(ret);
        g_free(ret);
        return pyret;
    }

    Py_RETURN_NONE;
}

PyDoc_STRVAR(PyTheme_get_format_doc,
    "get_format(module, tag) -> str\n"
);
static PyObject *PyTheme_get_format(PyTheme *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"module", "tag", NULL};
    char *module = "";
    char *tag = "";
    THEME_REC *theme = self->data;
    FORMAT_REC *formats;
    MODULE_THEME_REC *modtheme; 
    int i;

    RET_NULL_IF_INVALID(self->data);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "yy", kwlist, &module, &tag))
        return NULL;

    formats = g_hash_table_lookup(default_formats, module);
    if (!formats)
        return PyErr_Format(PyExc_KeyError, "unknown module, %s", module);

    for (i = 0; formats[i].def; i++)
    {
        if (formats[i].tag && !g_ascii_strcasecmp(formats[i].tag, tag))
        { 
            modtheme = g_hash_table_lookup(theme->modules, module);
            if (modtheme && modtheme->formats[i])
                return PyBytes_FromString(modtheme->formats[i]);
            else
                return PyBytes_FromString(formats[i].def);
        }
    }
   
    return PyErr_Format(PyExc_KeyError, "unknown format tag, %s", tag);    
}

/* Methods for object */
static PyMethodDef PyTheme_methods[] = {
    {"format_expand", (PyCFunction)PyTheme_format_expand, METH_VARARGS | METH_KEYWORDS,
        PyTheme_format_expand_doc},
    {"get_format", (PyCFunction)PyTheme_get_format, METH_VARARGS | METH_KEYWORDS,
        PyTheme_get_format_doc},
    {NULL}  /* Sentinel */
};

PyTypeObject PyThemeType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name      = "irssi.Theme",                            /*tp_name*/
    .tp_basicsize = sizeof(PyTheme),                          /*tp_basicsize*/
    .tp_dealloc   = (destructor)PyTheme_dealloc,              /*tp_dealloc*/
    .tp_flags     = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    .tp_methods   = PyTheme_methods,                          /* tp_methods */
    .tp_getset    = PyTheme_getseters,                        /* tp_getset */
    .tp_new       = PyTheme_new,                              /* tp_new */
};

/* Theme factory function */
PyObject *pytheme_new(void *td)
{
    PyTheme *pytheme;

    pytheme = py_inst(PyTheme, PyThemeType);
    if (!pytheme)
        return NULL;

    pytheme->data = td;
    signal_add_last_data("theme destroyed", theme_cleanup, pytheme);
    pytheme->cleanup_installed = 1;

    return (PyObject *)pytheme;
}

int theme_object_init(void) 
{
    g_return_val_if_fail(py_module != NULL, 0);

    if (PyType_Ready(&PyThemeType) < 0)
        return 0;
    
    Py_INCREF(&PyThemeType);
    PyModule_AddObject(py_module, "Theme", (PyObject *)&PyThemeType);

    return 1;
}

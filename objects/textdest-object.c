#include <Python.h>
#include "pyirssi_irc.h"
#include "pymodule.h"
#include "textdest-object.h"
#include "factory.h"
#include "pycore.h"

/* XXX: no cleanup signal for textdest */

static void PyTextDest_dealloc(PyTextDest *self)
{
    Py_XDECREF(self->window);
    Py_XDECREF(self->server);

    self->ob_type->tp_free((PyObject*)self);
}

static PyObject *PyTextDest_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyTextDest *self;

    self = (PyTextDest *)type->tp_alloc(type, 0);
    if (!self)
        return NULL;

    return (PyObject *)self;
}

PyDoc_STRVAR(PyTextDest_window_doc,
    "Window where the text will be written"
);
static PyObject *PyTextDest_window_get(PyTextDest *self, void *closure)
{
    RET_NULL_IF_INVALID(self->data);
    RET_AS_OBJ_OR_NONE(self->window);
}

PyDoc_STRVAR(PyTextDest_server_doc,
    "Target server"
);
static PyObject *PyTextDest_server_get(PyTextDest *self, void *closure)
{
    RET_NULL_IF_INVALID(self->data);
    RET_AS_OBJ_OR_NONE(self->server);
}

PyDoc_STRVAR(PyTextDest_target_doc,
    "Target channel/query/etc name"
);
static PyObject *PyTextDest_target_get(PyTextDest *self, void *closure)
{
    RET_NULL_IF_INVALID(self->data);
    RET_AS_STRING_OR_NONE(self->data->target);
}

PyDoc_STRVAR(PyTextDest_level_doc,
    "Text level"
);
static PyObject *PyTextDest_level_get(PyTextDest *self, void *closure)
{
    RET_NULL_IF_INVALID(self->data);
    return PyInt_FromLong(self->data->level);
}

PyDoc_STRVAR(PyTextDest_hilight_priority_doc,
    "Priority for the hilighted text"
);
static PyObject *PyTextDest_hilight_priority_get(PyTextDest *self, void *closure)
{
    RET_NULL_IF_INVALID(self->data);
    return PyInt_FromLong(self->data->hilight_priority);
}

PyDoc_STRVAR(PyTextDest_hilight_color_doc,
    "Color for the hilighted text"
);
static PyObject *PyTextDest_hilight_color_get(PyTextDest *self, void *closure)
{
    RET_NULL_IF_INVALID(self->data);
    RET_AS_STRING_OR_NONE(self->data->hilight_color);
}

/* specialized getters/setters */
static PyGetSetDef PyTextDest_getseters[] = {
    {"window", (getter)PyTextDest_window_get, NULL,
        PyTextDest_window_doc, NULL},
    {"server", (getter)PyTextDest_server_get, NULL,
        PyTextDest_server_doc, NULL},
    {"target", (getter)PyTextDest_target_get, NULL,
        PyTextDest_target_doc, NULL},
    {"level", (getter)PyTextDest_level_get, NULL,
        PyTextDest_level_doc, NULL},
    {"hilight_priority", (getter)PyTextDest_hilight_priority_get, NULL,
        PyTextDest_hilight_priority_doc, NULL},
    {"hilight_color", (getter)PyTextDest_hilight_color_get, NULL,
        PyTextDest_hilight_color_doc, NULL},
    {NULL}
};

/* Methods for object */
static PyMethodDef PyTextDest_methods[] = {
    {NULL}  /* Sentinel */
};

PyTypeObject PyTextDestType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "TextDest",            /*tp_name*/
    sizeof(PyTextDest),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)PyTextDest_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "PyTextDest objects",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    PyTextDest_methods,             /* tp_methods */
    0,                      /* tp_members */
    PyTextDest_getseters,        /* tp_getset */
    0,          /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,      /* tp_init */
    0,                         /* tp_alloc */
    PyTextDest_new,                 /* tp_new */
};


/* TextDest factory function */
PyObject *pytextdest_new(void *td)
{
    PyObject *window, *server;
    PyTextDest *pytdest;
    TEXT_DEST_REC *tdest = td;

    window = py_irssi_chat_new(tdest->window, 1);
    if (!window)
        return NULL;

    server = py_irssi_chat_new(tdest->server, 1);
    if (!server)
    {
        Py_DECREF(window);
        return NULL;
    }

    pytdest = py_inst(PyTextDest, PyTextDestType);
    if (!pytdest)
        return NULL;

    pytdest->data = td;
    pytdest->window = window;
    pytdest->server = server;

    return (PyObject *)pytdest;
}

int textdest_object_init(void) 
{
    g_return_val_if_fail(py_module != NULL, 0);

    if (PyType_Ready(&PyTextDestType) < 0)
        return 0;
    
    Py_INCREF(&PyTextDestType);
    PyModule_AddObject(py_module, "TextDest", (PyObject *)&PyTextDestType);

    return 1;
}

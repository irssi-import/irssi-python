#include <Python.h>
#include "structmember.h"
#include "pymodule.h"
#include "base-objects.h"
#include "pyirssi.h"

/* This is the base type for most, if not all, Irssi objects with a type
   id. The user can find the type name, type id, and check if the object is
   wrapping a valid Irssi record. */

/* member IDs */
enum
{
    M_BASE_TYPE,
    M_BASE_NAME,
    M_BASE_VALID,
};

static void PyIrssiBase_dealloc(PyIrssiBase *self)
{
    self->ob_type->tp_free((PyObject*)self);
}

static PyObject *
PyIrssiBase_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyIrssiBase *self;

    self = (PyIrssiBase *)type->tp_alloc(type, 0);
    if (!self)
        return NULL;

    return (PyObject *)self;
}

static PyObject *PyIrssiBase_get(PyIrssiBase *self, void *closure)
{
    int member = GPOINTER_TO_INT(closure);

    /* If the user passed the valid member, don't trigger an exception */
    if (member != M_BASE_VALID)
        RET_NULL_IF_INVALID(self->data);

    switch (member)
    {
        case M_BASE_TYPE:
            return PyInt_FromLong(self->data->type);
        case M_BASE_NAME:
            RET_AS_STRING_OR_NONE(self->base_name);
        case M_BASE_VALID:
            if (self->data != NULL)
                Py_RETURN_TRUE;
            else
                Py_RETURN_FALSE;
    }

    INVALID_MEMBER(member);
}

/* specialized getters/setters */
static PyGetSetDef PyIrssiBase_getseters[] = {
    {"type_id", (getter)PyIrssiBase_get, NULL, 
        "Irssi's type id for object", 
        GINT_TO_POINTER(M_BASE_TYPE)},

    {"type", (getter)PyIrssiBase_get, NULL, 
        "Irssi's name for object", 
        GINT_TO_POINTER(M_BASE_NAME)},

    {"valid", (getter)PyIrssiBase_get, NULL, 
        "True if the object is valid", 
        GINT_TO_POINTER(M_BASE_VALID)},
    {NULL}
};

/* Methods for object */
static PyMethodDef PyIrssiBase_methods[] = {
    /* {"somemeth", (PyCFunction)PyIrssiBase_name, METH_NOARGS, "docstr"}, */
    {NULL}  /* Sentinel */
};

PyTypeObject PyIrssiBaseType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "IrssiBase",            /*tp_name*/
    sizeof(PyIrssiBase),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)PyIrssiBase_dealloc, /*tp_dealloc*/
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
    "PyIrssiBase objects",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    PyIrssiBase_methods,             /* tp_methods */
    0,                      /* tp_members */
    PyIrssiBase_getseters,        /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,      /* tp_init */
    0,                         /* tp_alloc */
    PyIrssiBase_new,                 /* tp_new */
};


/* IrssiChatBase is a base type for any object with a chat type. The user
   can find the chat type string name with the chat_type member or
   the type id with the chat_type_id member. It inherits from IrssiBase
   so the type, valid, and type_id members are visible to the user, too */

/* member IDs */
enum
{
    M_CHAT_CHAT_TYPE,
    M_CHAT_CHAT_NAME,
};

static void
PyIrssiChatBase_dealloc(PyIrssiChatBase *self)
{
    self->ob_type->tp_free((PyObject*)self);
}

static PyObject *
PyIrssiChatBase_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyIrssiChatBase *self;

    self = (PyIrssiChatBase *)type->tp_alloc(type, 0);
    if (!self)
        return NULL;

    return (PyObject *)self;
}

static PyObject *PyIrssiChatBase_get(PyIrssiChatBase *self, void *closure)
{
    int member = GPOINTER_TO_INT(closure);

    RET_NULL_IF_INVALID(self->data);

    switch (member)
    {
        case M_CHAT_CHAT_TYPE:
            return PyInt_FromLong(self->data->chat_type);
        case M_CHAT_CHAT_NAME:
        {
            CHAT_PROTOCOL_REC *rec = chat_protocol_find_id(self->data->chat_type);
            if (rec)
                RET_AS_STRING_OR_NONE(rec->name);
            else
                Py_RETURN_NONE;
        }
    }

    INVALID_MEMBER(member);
}

//specialized getters/setters
static PyGetSetDef PyIrssiChatBase_getseters[] = {
    {"chat_type_id", (getter)PyIrssiChatBase_get, NULL, 
        "Chat Type id", 
        GINT_TO_POINTER(M_CHAT_CHAT_TYPE)},

    {"chat_type", (getter)PyIrssiChatBase_get, NULL, 
        "Chat Name", 
        GINT_TO_POINTER(M_CHAT_CHAT_NAME)},
    {NULL}
};

static PyMethodDef PyIrssiChatBase_methods[] = {
    {NULL}  /* Sentinel */
};

PyTypeObject PyIrssiChatBaseType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "IrssiChatBase",            /*tp_name*/
    sizeof(PyIrssiChatBase),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)PyIrssiChatBase_dealloc, /*tp_dealloc*/
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
    "PyIrssiChatBase objects",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    PyIrssiChatBase_methods,             /* tp_methods */
    0,                      /* tp_members */
    PyIrssiChatBase_getseters,        /* tp_getset */
    &PyIrssiBaseType,         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,      /* tp_init */
    0,                         /* tp_alloc */
    PyIrssiChatBase_new,                 /* tp_new */
};

int base_objects_init(void) 
{
    g_return_val_if_fail(py_module != NULL, 0);

    if (PyType_Ready(&PyIrssiBaseType) < 0)
        return 0;
    if (PyType_Ready(&PyIrssiChatBaseType) < 0)
        return 0;

    Py_INCREF(&PyIrssiBaseType);
    Py_INCREF(&PyIrssiChatBaseType);
    PyModule_AddObject(py_module, "IrssiBase", (PyObject *)&PyIrssiBaseType);
    PyModule_AddObject(py_module, "IrssiChatBase", (PyObject *)&PyIrssiChatBaseType);

    return 1;
}

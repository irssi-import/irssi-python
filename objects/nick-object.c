#include <Python.h>
#include "pymodule.h"
#include "base-objects.h"
#include "nick-object.h"
#include "pyirssi.h"
#include "pycore.h"
#include "pyutils.h"

/* member IDs */
enum
{
    M_NICK_NICK,
    M_NICK_HOST,
    M_NICK_REALNAME,
    M_NICK_HOPS,
    M_NICK_GONE,
    M_NICK_SERVEROP,
    M_NICK_OP,
    M_NICK_VOICE,
    M_NICK_HALFOP,
    M_NICK_LAST_CHECK,
    M_NICK_SEND_MASSJOIN,
};

static void nick_cleanup(CHANNEL_REC *chan, NICK_REC *nick)
{
    PyNick *pynick = signal_get_user_data();

    if (nick == pynick->data)
    {
        pynick->data = NULL;
        pynick->cleanup_installed = 0;
        signal_remove_data("nicklist remove", nick_cleanup, pynick);
    }
}

static void PyNick_dealloc(PyNick *self)
{
    if (self->cleanup_installed)
        signal_remove_data("nicklist remove", nick_cleanup, self);

    self->ob_type->tp_free((PyObject*)self);
}

static PyObject *PyNick_get(PyNick *self, void *closure)
{
    int member = GPOINTER_TO_INT(closure);

    RET_NULL_IF_INVALID(self->data);

    switch (member)
    {
        case M_NICK_NICK:
            RET_AS_STRING_OR_NONE(self->data->nick);
        case M_NICK_HOST:
            RET_AS_STRING_OR_NONE(self->data->host);
        case M_NICK_REALNAME:
            RET_AS_STRING_OR_NONE(self->data->realname);
        case M_NICK_HOPS:
            return PyInt_FromLong(self->data->hops);
        case M_NICK_GONE:
            return PyBool_FromLong(self->data->gone);
        case M_NICK_SERVEROP:
            return PyBool_FromLong(self->data->serverop);
        case M_NICK_OP:
            return PyBool_FromLong(self->data->op);
        case M_NICK_VOICE:
            return PyBool_FromLong(self->data->voice);
        case M_NICK_HALFOP:
            return PyBool_FromLong(self->data->halfop);
        case M_NICK_LAST_CHECK:
            return PyLong_FromUnsignedLong(self->data->last_check);
    }

    INVALID_MEMBER(member);
}

PyDoc_STRVAR(PyNick_send_massjoin_doc,
    "Waiting to be sent in a 'massjoin' signal, True or False"
);
static PyObject *PyNick_send_massjoin_get(PyNick *self, void *closure)
{
    RET_NULL_IF_INVALID(self->data);
    return PyBool_FromLong(self->data->send_massjoin);
}



/* specialized getters/setters */
static PyGetSetDef PyNick_getseters[] = {
    {"nick", (getter)PyNick_get, NULL, 
        "Plain nick", 
        GINT_TO_POINTER(M_NICK_NICK)},

    {"host", (getter)PyNick_get, NULL, 
        "Host address", 
        GINT_TO_POINTER(M_NICK_HOST)},

    {"realname", (getter)PyNick_get, NULL, 
        "Real name", 
        GINT_TO_POINTER(M_NICK_REALNAME)},

    {"hops", (getter)PyNick_get, NULL, 
        "Hop count to the server the nick is using", 
        GINT_TO_POINTER(M_NICK_HOPS)},

    {"gone", (getter)PyNick_get, NULL, 
        "User status", 
        GINT_TO_POINTER(M_NICK_GONE)},

    {"serverop", (getter)PyNick_get, NULL, 
        "User status", 
        GINT_TO_POINTER(M_NICK_SERVEROP)},

    {"op", (getter)PyNick_get, NULL, 
        "User status", 
        GINT_TO_POINTER(M_NICK_OP)},

    {"voice", (getter)PyNick_get, NULL, 
        "Channel status", 
        GINT_TO_POINTER(M_NICK_VOICE)},

    {"halfop", (getter)PyNick_get, NULL, 
        "Channel status", 
        GINT_TO_POINTER(M_NICK_HALFOP)},

    {"last_check", (getter)PyNick_get, NULL, 
        "timestamp when last checked gone/ircop status.", 
        GINT_TO_POINTER(M_NICK_LAST_CHECK)},
    {"send_massjoin", (getter)PyNick_send_massjoin_get, NULL,
        PyNick_send_massjoin_doc, NULL},
    {NULL}
};

static PyMethodDef PyNick_methods[] = {
    {NULL}  /* Sentinel */
};

PyTypeObject PyNickType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "Nick",            /*tp_name*/
    sizeof(PyNick),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)PyNick_dealloc, /*tp_dealloc*/
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
    "PyNick objects",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    PyNick_methods,             /* tp_methods */
    0,                      /* tp_members */
    PyNick_getseters,        /* tp_getset */
    &PyIrssiChatBaseType,          /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,      /* tp_init */
    0,                         /* tp_alloc */
    0,                 /* tp_new */
};


/* nick factory function */
PyObject *pynick_sub_new(void *nick, PyTypeObject *subclass)
{
    static const char *name = "NICK";
    PyNick *pynick = NULL;

    pynick = py_instp(PyNick, subclass); 
    if (!pynick)
        return NULL;

    pynick->data = nick;
    pynick->base_name = name;
    signal_add_last_data("nicklist remove", nick_cleanup, pynick);
    pynick->cleanup_installed = 1;

    return (PyObject *)pynick;
}

PyObject *pynick_new(void *nick)
{
    return pynick_sub_new(nick, &PyNickType);
}

int nick_object_init(void) 
{
    g_return_val_if_fail(py_module != NULL, 0);

    if (PyType_Ready(&PyNickType) < 0)
        return 0;
    
    Py_INCREF(&PyNickType);
    PyModule_AddObject(py_module, "Nick", (PyObject *)&PyNickType);

    return 1;
}

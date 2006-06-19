#include <Python.h>
#include "pyirssi.h"
#include "pymodule.h"
#include "factory.h"
#include "channel-object.h"
#include "pycore.h"

/* member IDs */
enum
{
    M_CHANNEL_TOPIC,
    M_CHANNEL_TOPIC_BY,
    M_CHANNEL_TOPIC_TIME,
    M_CHANNEL_NO_MODES,
    M_CHANNEL_MODE,
    M_CHANNEL_LIMIT,
    M_CHANNEL_KEY,
    M_CHANNEL_CHANOP,
    M_CHANNEL_NAMES_GOT,
    M_CHANNEL_WHOLIST,
    M_CHANNEL_SYNCED,
    M_CHANNEL_JOINED,
    M_CHANNEL_LEFT,
    M_CHANNEL_KICKED,
};

/* monitor "channel destroyed" signal */
static void chan_cleanup(CHANNEL_REC *chan)
{
    PyChannel *pychan = signal_get_user_data();

    if (chan == pychan->data)
    {
        pychan->data = NULL;
        pychan->cleanup_installed = 0;
        signal_remove_data("channel destroyed", chan_cleanup, pychan);
    }
}

static void PyChannel_dealloc(PyChannel *self)
{
    if (self->cleanup_installed)
        signal_remove_data("channel destroyed", chan_cleanup, self);

    self->ob_type->tp_free((PyObject*)self);
}


static PyObject *PyChannel_get(PyChannel *self, void *closure)
{
    int member = GPOINTER_TO_INT(closure);

    RET_NULL_IF_INVALID(self->data);

    switch (member)
    {
        case M_CHANNEL_TOPIC:
            RET_AS_STRING_OR_NONE(self->data->topic);
        case M_CHANNEL_TOPIC_BY:
            RET_AS_STRING_OR_NONE(self->data->topic_by);
        case M_CHANNEL_TOPIC_TIME:
            return PyLong_FromLong(self->data->topic_time);
        case M_CHANNEL_NO_MODES:
            return PyBool_FromLong(self->data->no_modes);
        case M_CHANNEL_MODE:
            RET_AS_STRING_OR_NONE(self->data->mode);
        case M_CHANNEL_LIMIT:
            return PyInt_FromLong(self->data->limit);
        case M_CHANNEL_KEY:
            RET_AS_STRING_OR_NONE(self->data->key);
        case M_CHANNEL_CHANOP:
            return PyBool_FromLong(self->data->chanop);
        case M_CHANNEL_NAMES_GOT:
            return PyBool_FromLong(self->data->names_got);
        case M_CHANNEL_WHOLIST:
            return PyBool_FromLong(self->data->wholist);
        case M_CHANNEL_SYNCED:
            return PyBool_FromLong(self->data->synced);
        case M_CHANNEL_JOINED:
            return PyBool_FromLong(self->data->joined);
        case M_CHANNEL_LEFT:
            return PyBool_FromLong(self->data->left);
        case M_CHANNEL_KICKED:
            return PyBool_FromLong(self->data->kicked);
    }

    /* This shouldn't be reached... but... */
    return PyErr_Format(PyExc_RuntimeError, "invalid member id, %d", member);
}

/* specialized getters/setters */
static PyGetSetDef PyChannel_getseters[] = {
    {"topic", (getter)PyChannel_get, NULL, 
        "Channel topic", 
        GINT_TO_POINTER(M_CHANNEL_TOPIC)},

    {"topic_by", (getter)PyChannel_get, NULL, 
        "Nick who set the topic", 
        GINT_TO_POINTER(M_CHANNEL_TOPIC_BY)},

    {"topic_time", (getter)PyChannel_get, NULL, 
        "Timestamp when the topic was set", 
        GINT_TO_POINTER(M_CHANNEL_TOPIC_TIME)},

    {"no_modes", (getter)PyChannel_get, NULL, 
        "Channel is modeless", 
        GINT_TO_POINTER(M_CHANNEL_NO_MODES)},

    {"mode", (getter)PyChannel_get, NULL, 
        "Channel mode", 
        GINT_TO_POINTER(M_CHANNEL_MODE)},

    {"limit", (getter)PyChannel_get, NULL, 
        "Max. users in channel (+l mode)", 
        GINT_TO_POINTER(M_CHANNEL_LIMIT)},

    {"key", (getter)PyChannel_get, NULL, 
        "Channel key (password)", 
        GINT_TO_POINTER(M_CHANNEL_KEY)},

    {"chanop", (getter)PyChannel_get, NULL, 
        "You are channel operator", 
        GINT_TO_POINTER(M_CHANNEL_CHANOP)},

    {"names_got", (getter)PyChannel_get, NULL, 
        "/NAMES list has been received", 
        GINT_TO_POINTER(M_CHANNEL_NAMES_GOT)},

    {"wholist", (getter)PyChannel_get, NULL, 
        "/WHO list has been received", 
        GINT_TO_POINTER(M_CHANNEL_WHOLIST)},

    {"synced", (getter)PyChannel_get, NULL, 
        "Channel is fully synchronized", 
        GINT_TO_POINTER(M_CHANNEL_SYNCED)},

    {"joined", (getter)PyChannel_get, NULL, 
        "JOIN event for this channel has been received", 
        GINT_TO_POINTER(M_CHANNEL_JOINED)},

    {"left", (getter)PyChannel_get, NULL, 
        "You just left the channel (for 'channel destroyed' event)", 
        GINT_TO_POINTER(M_CHANNEL_LEFT)},

    {"kicked", (getter)PyChannel_get, NULL, 
        "You was just kicked out of the channel (for 'channel destroyed' event)", 
        GINT_TO_POINTER(M_CHANNEL_KICKED)},

    {NULL}
};

PyDoc_STRVAR(PyChannel_nicks_doc,
    "Return a list of nicks in the channel."
);
static PyObject *PyChannel_nicks(PyChannel *self, PyObject *args)
{
    RET_NULL_IF_INVALID(self->data);

    return py_irssi_chatlist_new(nicklist_getnicks(self->data), 1);
}

PyDoc_STRVAR(PyChannel_nicks_find_mask_doc,
    "Find nick mask from nicklist, wildcards allowed."
);
static PyObject *PyChannel_nicks_find_mask(PyChannel *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"mask", NULL};
    char *mask = "";

    RET_NULL_IF_INVALID(self->data);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, 
           &mask))
        return NULL;

    return py_irssi_chat_new(nicklist_find_mask(self->data, mask), 1);
}

PyDoc_STRVAR(PyChannel_nick_find_doc,
    "Find nick from nicklist."
);
static PyObject *PyChannel_nick_find(PyChannel *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"nick", NULL};
    char *nick = "";

    RET_NULL_IF_INVALID(self->data);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, 
           &nick))
        return NULL;

    return py_irssi_chat_new(nicklist_find(self->data, nick), 1);
}

PyDoc_STRVAR(PyChannel_nick_remove_doc,
    "Remove nick from nicklist."
);
static PyObject *PyChannel_nick_remove(PyChannel *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"nick", NULL};
    PyObject *nick = NULL;

    RET_NULL_IF_INVALID(self->data);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, 
           &nick))
        return NULL;

    if (!pynick_check(nick))
        return PyErr_Format(PyExc_TypeError, "arg must be nick");

    nicklist_remove(self->data, ((PyNick*)nick)->data);
    
    Py_RETURN_NONE;
}

PyDoc_STRVAR(PyChannel_nick_insert_obj_doc,
    "Insert nick object into nicklist."
);
static PyObject *PyChannel_nick_insert_obj(PyChannel *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"nick", NULL};
    PyObject *nick = NULL;

    RET_NULL_IF_INVALID(self->data);

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, 
           &nick))
        return NULL;

    if (!pynick_check(nick))
        return PyErr_Format(PyExc_TypeError, "arg must be nick");

    nicklist_insert(self->data, ((PyNick*)nick)->data);

    Py_RETURN_NONE;
}

/* Methods for object */
static PyMethodDef PyChannel_methods[] = {
    {"nicks", (PyCFunction)PyChannel_nicks, METH_NOARGS,
        PyChannel_nicks_doc},
    {"nicks_find_mask", (PyCFunction)PyChannel_nicks_find_mask, METH_VARARGS | METH_KEYWORDS,
        PyChannel_nicks_find_mask_doc},
    {"nick_find", (PyCFunction)PyChannel_nick_find, METH_VARARGS | METH_KEYWORDS,
        PyChannel_nick_find_doc},
    {"nick_remove", (PyCFunction)PyChannel_nick_remove, METH_VARARGS | METH_KEYWORDS,
        PyChannel_nick_remove_doc},
    {"nick_insert_obj", (PyCFunction)PyChannel_nick_insert_obj, METH_VARARGS | METH_KEYWORDS,
        PyChannel_nick_insert_obj_doc},
    {NULL}  /* Sentinel */
};

PyTypeObject PyChannelType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "Channel",            /*tp_name*/
    sizeof(PyChannel),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)PyChannel_dealloc, /*tp_dealloc*/
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
    "PyChannel objects",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    PyChannel_methods,             /* tp_methods */
    0,                      /* tp_members */
    PyChannel_getseters,        /* tp_getset */
    &PyWindowItemType,          /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,      /* tp_init */
    0,                         /* tp_alloc */
    0,                 /* tp_new */
};


/* window item wrapper factory function */
PyObject *pychannel_sub_new(void *chan, const char *name, PyTypeObject *type) 
{
    PyObject *pychan;

    pychan = pywindow_item_sub_new(chan, name, type);
    if (pychan)
    {
        PyChannel *pych = (PyChannel *)pychan;
        signal_add_last_data("channel destroyed", chan_cleanup, pych);
        pych->cleanup_installed = 1;
    }

    return pychan;
}

PyObject *pychannel_new(void *chan)
{
    static const char *name = "CHANNEL";
    return pychannel_sub_new(chan, name, &PyChannelType);
}

int channel_object_init(void) 
{
    g_return_val_if_fail(py_module != NULL, 0);

    if (PyType_Ready(&PyChannelType) < 0)
        return 0;
    
    Py_INCREF(&PyChannelType);
    PyModule_AddObject(py_module, "Channel", (PyObject *)&PyChannelType);

    return 1;
}

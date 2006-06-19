#include <Python.h>
#include "pymodule.h"
#include "base-objects.h"
#include "server-object.h"
#include "irc-server-object.h"
#include "irc-connect-object.h"
#include "pyirssi_irc.h"
#include "pycore.h"
#include "pyutils.h"

/* member IDs */
enum
{
    M_IRC_SERVER_REAL_ADDRESS,
    M_IRC_SERVER_USERMODE,
    M_IRC_SERVER_USERHOST,
};

/* cleanup and dealloc inherited from base Server */

static PyObject *PyIrcServer_get(PyIrcServer *self, void *closure)
{
    int member = GPOINTER_TO_INT(closure);

    RET_NULL_IF_INVALID(self->data);

    switch (member)
    {
        case M_IRC_SERVER_REAL_ADDRESS:
            RET_AS_STRING_OR_NONE(self->data->real_address);
        case M_IRC_SERVER_USERMODE:
            RET_AS_STRING_OR_NONE(self->data->usermode);
        case M_IRC_SERVER_USERHOST:
            RET_AS_STRING_OR_NONE(self->data->userhost);
    }

    /* This shouldn't be reached... but... */
    return PyErr_Format(PyExc_RuntimeError, "invalid member id, %d", member);
}

static PyGetSetDef PyIrcServer_getseters[] = {
    {"real_address", (getter)PyIrcServer_get, NULL, 
        "Address the IRC server gives",
        GINT_TO_POINTER(M_IRC_SERVER_REAL_ADDRESS)},

    {"usermode", (getter)PyIrcServer_get, NULL, 
        "User mode in server",
        GINT_TO_POINTER(M_IRC_SERVER_USERMODE)},

    {"userhost", (getter)PyIrcServer_get, NULL, 
        "Your user host in server",
        GINT_TO_POINTER(M_IRC_SERVER_USERHOST)},

    {NULL}
};

PyDoc_STRVAR(get_channels_doc,
    "Return a string of all channels (and keys, if any have them) in server,\n"
    "like '#a,#b,#c,#d x,b_chan_key,x,x' or just '#e,#f,#g'\n"
);
static PyObject *PyIrcServer_get_channels(PyIrcServer *self, PyObject *args)
{
    char *list;
    PyObject *ret;

    RET_NULL_IF_INVALID(self->data);

    list = irc_server_get_channels(self->data);
    ret = PyString_FromString(list);
    g_free(list);

    return ret;
}

PyDoc_STRVAR(send_raw_doc,
    "Send raw message to server, it will be flood protected so you\n"
    "don't need to worry about it.\n"
);
static PyObject *PyIrcServer_send_raw(PyIrcServer *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"cmd", NULL};
    char *cmd;

    RET_NULL_IF_INVALID(self->data);
    
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, &cmd))
        return NULL;

    irc_send_cmd(self->data, cmd);
    
    Py_RETURN_NONE;
}

PyDoc_STRVAR(send_raw_now_doc,
    "Send raw message to server immediately without flood protection.\n"
);
static PyObject *PyIrcServer_send_raw_now(PyIrcServer *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"cmd", NULL};
    char *cmd;

    RET_NULL_IF_INVALID(self->data);
    
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, &cmd))
        return NULL;

    irc_send_cmd_now(self->data, cmd);
    
    Py_RETURN_NONE;
}

PyDoc_STRVAR(send_raw_split_doc,
    "Split the `cmd' into several commands so `nickarg' argument has only\n"
    "`max_nicks' number of nicks.\n"
    "\n"
    "Example:\n"
    "server.send_raw_split('KICK #channel nick1,nick2,nick3 :byebye', 3, 2)\n"
    "\n"
    "Irssi will send commands 'KICK #channel nick1,nick2 :byebye' and\n"
    "'KICK #channel nick3 :byebye' to server.\n"
);
static PyObject *PyIrcServer_send_raw_split(PyIrcServer *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"cmd", "nickarg", "max_nicks", NULL};
    char *cmd;
    int nickarg;
    int max_nicks;

    RET_NULL_IF_INVALID(self->data);
    
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "sii", kwlist, &cmd, &nickarg, &max_nicks))
        return NULL;

    irc_send_cmd_split(self->data, cmd, nickarg, max_nicks);
    
    Py_RETURN_NONE;
}

PyDoc_STRVAR(ctcp_send_reply_doc,
    "Send CTCP reply. This will be 'CTCP flood protected' so if there's too\n"
    "many CTCP requests in buffer, this reply might not get sent. The data\n"
    "is the full raw command to be sent to server, like\n"
    "'NOTICE nick :\001VERSION irssi\001'\n"
);
static PyObject *PyIrcServer_ctcp_send_reply(PyIrcServer *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"data", NULL};
    char *data;

    RET_NULL_IF_INVALID(self->data);
    
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, &data))
        return NULL;

    ctcp_send_reply(self->data, data);
    
    Py_RETURN_NONE;
}

PyDoc_STRVAR(isupport_doc,
    "Returns the value of the named item in the ISUPPORT (005) numeric to the\n"
    "script. If the item is not present returns undef, if the item has no value\n"
    "then '' is returned use defined server.isupport('name') if you need to\n"
    "check whether a property is present.\n"
    "See http://www.ietf.org/internet-drafts/draft-brocklesby-irc-isupport-01.txt\n"  
    "for more information on the ISUPPORT numeric.\n"
);
static PyObject *PyIrcServer_isupport(PyIrcServer *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"name", NULL};
    char *name;
    char *found;

    RET_NULL_IF_INVALID(self->data);
    
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, &name))
        return NULL;

    found = g_hash_table_lookup(self->data->isupport, name);

    RET_AS_STRING_OR_NONE(found);
}

/* Methods for object */
static PyMethodDef PyIrcServer_methods[] = {
    {"get_channels", (PyCFunction)PyIrcServer_get_channels, METH_NOARGS,
        get_channels_doc},
        
    {"send_raw", (PyCFunction)PyIrcServer_send_raw, METH_VARARGS | METH_KEYWORDS, 
        send_raw_doc},

    {"send_raw_now", (PyCFunction)PyIrcServer_send_raw_now, METH_VARARGS | METH_KEYWORDS, 
        send_raw_now_doc},
    
    {"send_raw_split", (PyCFunction)PyIrcServer_send_raw_split, METH_VARARGS | METH_KEYWORDS, 
        send_raw_split_doc},

    {"ctcp_send_reply", (PyCFunction)PyIrcServer_ctcp_send_reply, METH_VARARGS | METH_KEYWORDS, 
        ctcp_send_reply_doc},

    {"isupport", (PyCFunction)PyIrcServer_isupport, METH_VARARGS | METH_KEYWORDS, 
        isupport_doc},

    {NULL}  /* Sentinel */
};

PyTypeObject PyIrcServerType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "IrcServer",            /*tp_name*/
    sizeof(PyIrcServer),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    0,                         /*tp_dealloc*/
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
    "PyIrcServer objects",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    PyIrcServer_methods,             /* tp_methods */
    0,                      /* tp_members */
    PyIrcServer_getseters,        /* tp_getset */
    &PyServerType,          /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,      /* tp_init */
    0,                         /* tp_alloc */
    0,                 /* tp_new */
};

PyObject *pyirc_server_new(void *server)
{
    return pyserver_sub_new(server, &PyIrcServerType);
}

int irc_server_object_init(void) 
{
    g_return_val_if_fail(py_module != NULL, 0);

    if (PyType_Ready(&PyIrcServerType) < 0)
        return 0;
    
    Py_INCREF(&PyIrcServerType);
    PyModule_AddObject(py_module, "IrcServer", (PyObject *)&PyIrcServerType);

    return 1;
}

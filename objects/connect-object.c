#include <Python.h>
#include "pymodule.h"
#include "base-objects.h"
#include "connect-object.h"
#include "pyirssi.h"
#include "pycore.h"
#include "pyutils.h"

/* member IDs */
enum
{
    M_CONNECT_ADDRESS,
    M_CONNECT_PORT,
    M_CONNECT_CHATNET,
    M_CONNECT_PASSWORD,
    M_CONNECT_WANTED_NICK,
    M_CONNECT_USERNAME,
    M_CONNECT_REALNAME,
};

static void connect_cleanup(SERVER_CONNECT_REC *connect)
{
    PyConnect *pyconn = signal_get_user_data();

    /*
    if (server == pyconn->data)
    {
        pyserver->data = NULL;
        pyserver->cleanup_installed = 0;
        signal_remove_data("server disconnected", connect_cleanup, pyserver);
    }
    */
}

static void PyConnect_dealloc(PyConnect *self)
{
    if (self->cleanup_installed)
        signal_remove_data("server disconnected", connect_cleanup, self);

    self->ob_type->tp_free((PyObject*)self);
}

static PyObject *PyConnect_get(PyConnect *self, void *closure)
{
    int member = GPOINTER_TO_INT(closure);

    RET_NULL_IF_INVALID(self->data);

    switch (member)
    {
        case M_CONNECT_ADDRESS:
            RET_AS_STRING_OR_NONE(self->data->address);
        case M_CONNECT_PORT:
            return PyInt_FromLong(self->data->port);
        case M_CONNECT_CHATNET:
            RET_AS_STRING_OR_NONE(self->data->chatnet);
        case M_CONNECT_PASSWORD:
            RET_AS_STRING_OR_NONE(self->data->password);
        case M_CONNECT_WANTED_NICK:
            RET_AS_STRING_OR_NONE(self->data->nick);
        case M_CONNECT_USERNAME:
            RET_AS_STRING_OR_NONE(self->data->username);
        case M_CONNECT_REALNAME:
            RET_AS_STRING_OR_NONE(self->data->realname);
    }

    /* This shouldn't be reached... but... */
    return PyErr_Format(PyExc_RuntimeError, "invalid member id, %d", member);
}

static PyGetSetDef PyConnect_getseters[] = {
    {"address", (getter)PyConnect_get, NULL, 
        "Address where we connected (irc.blah.org)", 
        GINT_TO_POINTER(M_CONNECT_ADDRESS)},

    {"port", (getter)PyConnect_get, NULL, 
        "Port where we connected",
        GINT_TO_POINTER(M_CONNECT_PORT)},

    {"chatnet", (getter)PyConnect_get, NULL, 
        "Chat network",
        GINT_TO_POINTER(M_CONNECT_CHATNET)},

    {"password", (getter)PyConnect_get, NULL, 
        "Password we used in connection.",
        GINT_TO_POINTER(M_CONNECT_PASSWORD)},

    {"wanted_nick", (getter)PyConnect_get, NULL, 
        "Nick which we would prefer to use",
        GINT_TO_POINTER(M_CONNECT_WANTED_NICK)},

    {"username", (getter)PyConnect_get, NULL, 
        "User name",
        GINT_TO_POINTER(M_CONNECT_USERNAME)},

    {"realname", (getter)PyConnect_get, NULL, 
        "Real name",
        GINT_TO_POINTER(M_CONNECT_REALNAME)},

    {NULL}
};

PyTypeObject PyConnectType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "Connect",            /*tp_name*/
    sizeof(PyConnect),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)PyConnect_dealloc, /*tp_dealloc*/
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
    "PyConnect objects",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    0,                      /* tp_methods */
    0,                      /* tp_members */
    PyConnect_getseters,        /* tp_getset */
    &PyIrssiChatBaseType,          /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,      /* tp_init */
    0,                         /* tp_alloc */
    0,                 /* tp_new */
};


/* server connect factory function (managed == 0, don't do signal cleanup, 1 == do sig cleanup */
PyObject *pyconnect_sub_new(void *connect, PyTypeObject *subclass, int managed)
{
    static const char *CONNECT_TYPE = "SERVER CONNECT";
    PyConnect *pyconn = NULL;

    g_return_val_if_fail(connect != NULL, NULL);
    
    pyconn = py_instp(PyConnect, subclass);
    if (!pyconn)
        return NULL;

    pyconn->base_name = CONNECT_TYPE;
    pyconn->data = connect;
    
    if (managed)
    {
        //XXX: how to handle cleanup?
        //signal_add_last_data("server disconnected", connect_cleanup, pyconn);
        //pyconn->cleanup_installed = 1;
    }

    return (PyObject *)pyconn;
}

PyObject *pyconnect_new(void *connect, int managed)
{
    return pyconnect_sub_new(connect, &PyConnectType, managed);
}

int connect_object_init(void) 
{
    g_return_val_if_fail(py_module != NULL, 0);

    if (PyType_Ready(&PyConnectType) < 0)
        return 0;
    
    Py_INCREF(&PyConnectType);
    PyModule_AddObject(py_module, "Connect", (PyObject *)&PyConnectType);

    return 1;
}

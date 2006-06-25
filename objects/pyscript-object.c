#include <Python.h>
#include <structmember.h>
#include "pyscript-object.h"
#include "pyirssi.h"
#include "pysignals.h"
#include "pymodule.h"

/* handle cycles...
   Can't think of any reason why the user would put script into one of the lists
   but who knows. Call GC after unloading module.
*/
static int PyScript_traverse(PyScript *self, visitproc visit, void *arg)
{
    Py_VISIT(self->module);
    Py_VISIT(self->argv);
    Py_VISIT(self->modules);

    return 0;
}

static int PyScript_clear(PyScript *self)
{
    Py_CLEAR(self->module);
    Py_CLEAR(self->argv);
    Py_CLEAR(self->modules);

    return 0;
}

static void PyScript_dealloc(PyScript* self)
{
    PyScript_clear(self);
    pyscript_remove_signals((PyObject*)self);
    
    self->ob_type->tp_free((PyObject*)self);
}

static PyObject *PyScript_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyScript *self; 
    PyObject *argv, *modules;

    argv = PyList_New(0);
    if (!argv)
        return NULL; 

    modules = PyDict_New();
    if (!modules)
    {
        Py_DECREF(argv);
        return NULL; 
    }

    self = (PyScript *)type->tp_alloc(type, 0);
    if (!self)
    {
        Py_DECREF(argv);
        Py_DECREF(modules);
        return NULL;
    }
   
    self->argv = argv;
    self->modules = modules;
    
    return (PyObject *)self;
}

PyDoc_STRVAR(PyScript_command_bind_doc,
    "command_bind(command, func, catetory=None, priority=SIGNAL_PRIORITY_DEFAULT) -> None\n"
    "\n"
    "Add handler for a command\n"
);
static PyObject *PyScript_command_bind(PyScript *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"cmd", "func", "category", "priority", NULL};
    char *cmd;
    PyObject *func;
    char *category = NULL;
    int priority = SIGNAL_PRIORITY_DEFAULT; 

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "sO|zi", kwlist, 
                &cmd, &func, &category, &priority))
        return NULL;

    if (!PyCallable_Check(func))
        return PyErr_Format(PyExc_TypeError, "func must be callable");
  
    if (!pysignals_command_bind_list(&self->signals, cmd, func, category, priority))
        return PyErr_Format(PyExc_RuntimeError, "unable to bind command");
    
    Py_RETURN_NONE;
}

PyDoc_STRVAR(PyScript_signal_add_doc,
    "signal_add(signal, func, priority=SIGNAL_PRIORITY_DEFAULT) -> None\n"
    "\n"
    "Add handler for signal"
);
static PyObject *PyScript_signal_add(PyScript *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"signal", "func", "priority", NULL};
    char *signal;
    PyObject *func;
    int priority = SIGNAL_PRIORITY_DEFAULT; 

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "sO|i", kwlist, 
                &signal, &func, &priority))
        return NULL;

    if (!PyCallable_Check(func))
        return PyErr_Format(PyExc_TypeError, "func must be callable");

    if (!pysignals_signal_add_list(&self->signals, signal, func, priority))
        return PyErr_Format(PyExc_KeyError, "unable to find signal, '%s'", signal);
    
    Py_RETURN_NONE;
}

PyDoc_STRVAR(PyScript_signal_remove_doc,
    "signal_remove(signal, func=None) -> None\n"
    "\n"
    "Remove signal handler\n"
);
static PyObject *PyScript_signal_remove(PyScript *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"signal", "func", NULL};
    char *signal = "";
    PyObject *func = Py_None;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s|O", kwlist, 
           &signal, &func))
        return NULL;

    if (!PyCallable_Check(func) && func != Py_None)
        return PyErr_Format(PyExc_TypeError, "func must be callable or None");

    if (func == Py_None)
        func = NULL;
    
    if (!pysignals_remove_search(&self->signals, signal, func, PSG_SIGNAL))
        return PyErr_Format(PyExc_KeyError, "can't find signal");
    
    Py_RETURN_NONE;
}

PyDoc_STRVAR(PyScript_command_unbind_doc,
    "command_unbind(command, func=None) -> None\n"
    "\n"
    "Remove command handler\n"
);
static PyObject *PyScript_command_unbind(PyScript *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"command", "func", NULL};
    char *command = "";
    PyObject *func = Py_None;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s|O", kwlist, 
           &command, &func))
        return NULL;

    if (!PyCallable_Check(func) && func != Py_None)
        return PyErr_Format(PyExc_TypeError, "func must be callable or None");

    if (func == Py_None)
        func = NULL;

    if (!pysignals_remove_search(&self->signals, command, func, PSG_COMMAND))
        return PyErr_Format(PyExc_KeyError, "can't find command");

    Py_RETURN_NONE;
}

PyDoc_STRVAR(PyScript_signal_register_doc,
    "signal_register(signal, arglist) -> None\n"
    "\n"
    "Register a new dynamic signal for use with irssi_python\n"
    "arglist is a string of character codes representing the type of each argument\n"
    "of the signal handler function.\n"
    "\n"
    " Scalars\n"
    "   s -> char *\n"
    "   i -> int\n"
    "\n"
    " Chat objects\n"
    "   c -> CHATNET_REC\n"
    "   S -> SERVER_REC\n"
    "   C -> CHANNEL_REC\n"
    "   q -> QUERY_REC\n"
    "   n -> NICK_REC\n"
    "   W -> WI_ITEM_REC\n"
    "\n"
    " Irssi objects\n"
    "   d -> DCC_REC\n"
    "\n"
    " Other objects\n"
    "   r -> RECONNECT_REC\n"
    "   o -> COMMAND_REC\n"
    "   l -> LOG_REC\n"
    "   a -> RAWLOG_REC\n"
    "   g -> IGNORE_REC\n"
    "   b -> BAN_REC\n"
    "   N -> NETSPLIT_REC\n"
    "   e -> NETSPLIT_SERVER_REC\n"
    "   O -> NOTIFYLIST_REC\n"
    "   p -> PROCESS_REC\n"
    "   t -> TEXT_DEST_REC\n"
    "   w -> WINDOW_REC\n"
);
static PyObject *PyScript_signal_register(PyScript *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"signal", "arglist", NULL};
    static const char *good_codes = "sicSCqnWdrolagbNeOptw";
    char *signal = "";
    char *arglist = "";
    int i;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "ss", kwlist, 
           &signal, &arglist))
        return NULL;

    for (i = 0; arglist[i]; i++)
        if (!strchr(good_codes, arglist[i]))
            return PyErr_Format(PyExc_TypeError, "invalid code, %c", arglist[i]);

    if (i >= SIGNAL_MAX_ARGUMENTS)
        return PyErr_Format(PyExc_TypeError, 
                "arglist greater than SIGNAL_MAX_ARGUMENTS (%d)", 
                SIGNAL_MAX_ARGUMENTS);

    if (!pysignals_register(signal, arglist))
        return PyErr_Format(PyExc_TypeError, "signal present with different args");
   
    self->registered_signals = g_slist_append(self->registered_signals, 
            g_strdup(signal));
    
    Py_RETURN_NONE;
}

PyDoc_STRVAR(PyScript_signal_unregister_doc,
    "signal_unregister(signal) -> None\n"
    "\n"
    "Unregister dynamic signal\n"
);
static PyObject *PyScript_signal_unregister(PyScript *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"signal", NULL};
    char *signal = "";
    GSList *search;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, 
           &signal))
        return NULL;

    search = g_slist_find_custom(self->registered_signals, signal, (GCompareFunc)strcmp);
    if (!search)
        return PyErr_Format(PyExc_KeyError, "script has not registered that signal");
   
    g_free(search->data);
    self->registered_signals = g_slist_delete_link(self->registered_signals, search);
   
    if (!pysignals_unregister(signal))
        return PyErr_Format(PyExc_SystemError, 
                "script registered signal, but signal does not exist");
    
    Py_RETURN_NONE;
}

/* Methods for object */
static PyMethodDef PyScript_methods[] = {
    {"command_bind", (PyCFunction)PyScript_command_bind, METH_VARARGS | METH_KEYWORDS, 
        PyScript_command_bind_doc},
    {"signal_add", (PyCFunction)PyScript_signal_add, METH_VARARGS | METH_KEYWORDS,
        PyScript_signal_add_doc},
    {"signal_remove", (PyCFunction)PyScript_signal_remove, METH_VARARGS | METH_KEYWORDS,
        PyScript_signal_remove_doc},
    {"command_unbind", (PyCFunction)PyScript_command_unbind, METH_VARARGS | METH_KEYWORDS,
        PyScript_command_unbind_doc},
    {"signal_register", (PyCFunction)PyScript_signal_register, METH_VARARGS | METH_KEYWORDS,
        PyScript_signal_register_doc},
    {"signal_unregister", (PyCFunction)PyScript_signal_unregister, METH_VARARGS | METH_KEYWORDS,
        PyScript_signal_unregister_doc},
    {NULL}  /* Sentinel */
};

static PyMemberDef PyScript_members[] = {
    {"argv", T_OBJECT, offsetof(PyScript, argv), 0, "Script arguments"},
    {"module", T_OBJECT_EX, offsetof(PyScript, module), RO, "Script module"},
    {"modules", T_OBJECT_EX, offsetof(PyScript, modules), 0, "Imported modules"},
    {NULL}  /* Sentinel */
};

static PyTypeObject PyScriptType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "Script",               /*tp_name*/
    sizeof(PyScript),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)PyScript_dealloc, /*tp_dealloc*/
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
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC, /*tp_flags*/
    "PyScript objects",           /* tp_doc */
    (traverseproc)PyScript_traverse,		    /* tp_traverse */
    (inquiry)PyScript_clear,      /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    PyScript_methods,             /* tp_methods */
    PyScript_members,             /* tp_members */
    0,                      /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,                          /* tp_init */
    0,                         /* tp_alloc */
    PyScript_new,                 /* tp_new */
};

/* PyScript factory function */
PyObject *pyscript_new(PyObject *module, char **argv)
{
    PyObject *script = NULL;

    script = PyObject_CallFunction((PyObject*)&PyScriptType, "()"); 

    if (script)
    {
        PyScript *scr = (PyScript *)script;

        while (*argv)
        {
            PyObject *str = PyString_FromString(*argv);
            if (!str)
            {
                /* The destructor should DECREF argv */
                Py_DECREF(script);
                return NULL;
            }

            PyList_Append(scr->argv, str);
            Py_DECREF(str);

            *argv++;
        }

        Py_INCREF(module);
        scr->module = module;
    }
    
    return script;
}

void pyscript_remove_signals(PyObject *script)
{
    GSList *node;
    PyScript *self;
   
    g_return_if_fail(pyscript_check(script));
    
    self = (PyScript *) script;

    /* remove bound signals */
    pysignals_remove_list(self->signals);
    g_slist_free(self->signals);
    self->signals = NULL;

    /* remove registered signals */
    for (node = self->registered_signals; node; node = node->next)
    {
        pysignals_unregister(node->data);
        g_free(node->data);
    }

    g_slist_free(self->registered_signals);
    self->registered_signals = NULL;
}

void pyscript_clear_modules(PyObject *script)
{
    PyScript *self;

    g_return_if_fail(pyscript_check(script));

    self = (PyScript *) script;

    PyDict_Clear(self->modules);
}

int pyscript_init(void) 
{
    if (PyType_Ready(&PyScriptType) < 0)
        return 0;

    Py_INCREF(&PyScriptType);
    PyModule_AddObject(py_module, "Script", (PyObject *)&PyScriptType);

    return 1;
}


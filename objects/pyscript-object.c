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

//FIXME: add prioriety as arg
PyDoc_STRVAR(PyScript_command_bind_doc,
    "Bind a command"
);
static PyObject *PyScript_command_bind(PyScript *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"cmd", "func", "category", "priority", NULL};
    char *cmd;
    PyObject *func;
    char *category = NULL;
    int priority = SIGNAL_PRIORITY_DEFAULT; 
    PY_SIGNAL_REC *srec;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "sO|si", kwlist, 
                &cmd, &func, &category, &priority))
        return NULL;

    if (!PyCallable_Check(func))
        return PyErr_Format(PyExc_TypeError, "func must be callable");
  
    srec = pysignals_command_bind(cmd, func, category, priority);
    if (!srec)
        return PyErr_Format(PyExc_RuntimeError, "unable to bind command");
    
    /* add record to internal list*/
    self->signals = g_slist_append(self->signals, srec);

    Py_RETURN_NONE;
}

PyDoc_STRVAR(PyScript_signal_add_doc,
    "add signal"
);
static PyObject *PyScript_signal_add(PyScript *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"signal", "func", "category", "priority", NULL};
    char *signal;
    PyObject *func;
    char *category = NULL;
    int priority = SIGNAL_PRIORITY_DEFAULT; 
    PY_SIGNAL_REC *srec;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "sO|si", kwlist, 
                &signal, &func, &category, &priority))
        return NULL;

    if (!PyCallable_Check(func))
        return PyErr_Format(PyExc_TypeError, "func must be callable");

    srec = pysignals_signal_add(signal, func, priority);
    if (!srec)
        return PyErr_Format(PyExc_KeyError, "unable to find signal, '%s'", signal);
    
    self->signals = g_slist_append(self->signals, srec);

    Py_RETURN_NONE;
}

static PyMemberDef PyScript_members[] = {
    {"argv", T_OBJECT, offsetof(PyScript, argv), 0, "Script arguments"},
    {"module", T_OBJECT_EX, offsetof(PyScript, module), RO, "Script module"},
    {"modules", T_OBJECT_EX, offsetof(PyScript, modules), 0, "Imported modules"},
    {NULL}  /* Sentinel */
};

/* Methods for object */
static PyMethodDef PyScript_methods[] = {
    {"command_bind", (PyCFunction)PyScript_command_bind, METH_VARARGS | METH_KEYWORDS, 
        PyScript_command_bind_doc},
    {"signal_add", (PyCFunction)PyScript_signal_add, METH_VARARGS | METH_KEYWORDS,
        PyScript_signal_add_doc},
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

    for (node = self->signals; node != NULL; node = node->next) 
        pysignals_remove_generic((PY_SIGNAL_REC *)node->data);
  
    g_slist_free(self->signals);
    self->signals = NULL;
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


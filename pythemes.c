#include <Python.h>
#include "pythemes.h"
#include "pyirssi.h"
#include "factory.h"
#include "pymodule.h"
#include "pyloader.h"

/* Edited from Perl Themes.xs */
int pythemes_printformat(TEXT_DEST_REC *dest, const char *script, const char *format, PyObject *argtup) 
{
    char *arglist[MAX_FORMAT_PARAMS + 1];
    THEME_REC *theme;
    char *str;
    int formatnum;
    int i;

    formatnum = format_find_tag(script, format);
    if (formatnum < 0) {
         PyErr_Format(PyExc_KeyError, "unregistered format '%s'", format);
         return 0;
    }

    memset(arglist, 0, sizeof arglist);
    for (i = 0; i < MAX_FORMAT_PARAMS && i < PyTuple_Size(argtup); i++) {
        PyObject *obj = PyTuple_GET_ITEM(argtup, i);
        char *str;

        if (!PyString_Check(obj)) {
            PyErr_Format(PyExc_TypeError, "format argument list contains non-string data");
            return 0;
        }

        str = PyString_AsString(obj);
        if (!str)
            return 0;
        
        arglist[i] = str;
    }
    
    theme = window_get_theme(dest->window); 
    signal_emit("print format", 5, theme, script,
             dest, GINT_TO_POINTER(formatnum), arglist);

    str = format_get_text_theme_charargs(theme, script, dest, formatnum, arglist);
    if (*str != '\0') printtext_dest(dest, "%s", str);
    g_free(str);

    return 1;
}

static void py_destroy_format_list(FORMAT_REC *recs)
{
    int i;

    for (i = 0; recs[i].def; i++)
    {
        g_free(recs[i].def);
        g_free(recs[i].tag);
    }

    g_free(recs);
}

/* register a list of formats in this format:
 * [ (name, format), ... ]
 */
int pythemes_register(const char *script, PyObject *list)
{
    FORMAT_REC *formatrecs;
    int i;

    if (!PyList_Check(list))
    {
        PyErr_Format(PyExc_TypeError, "arg must be list");
        return 0;
    }
    
    if (PyList_Size(list) == 0)
    {
        PyErr_Format(PyExc_TypeError, "cannot register empty list");
        return 0;
    }

    if (g_hash_table_lookup(default_formats, script))
    {
        PyErr_Format(PyExc_KeyError, "format list already registered by script");
        return 0;
    }
    
    formatrecs = g_new0(FORMAT_REC, PyList_Size(list) + 2);
    formatrecs[0].tag = g_strdup(script);
    formatrecs[0].tag = g_strdup("Python script");

    for (i = 0; i < PyList_Size(list); i++)
    {
        FORMAT_REC *rec;
        PyObject *item;
        char *key, *value;

        rec = &formatrecs[i + 1];
        item = PyList_GET_ITEM(list, i);
        if (!PyArg_ParseTuple(item, "ss", &key, &value))
        {
            if (PyErr_ExceptionMatches(PyExc_TypeError))
            {
                PyErr_Clear();
                PyErr_Format(PyExc_TypeError, "format list must contain tuples of two strings");
            }
            py_destroy_format_list(formatrecs);
            return 0;
        }

        rec->tag = g_strdup(key);
        rec->def = g_strdup(value);
        rec->params = MAX_FORMAT_PARAMS;
    }

    theme_register_module(script, formatrecs);

    return 1;
}

void pythemes_unregister(const char *script)
{
    FORMAT_REC *formats;

    formats = g_hash_table_lookup(default_formats, script);
    if (!formats)
        return;

    py_destroy_format_list(formats);
    theme_unregister_module(script); 
}

PyDoc_STRVAR(py_printformat_doc,
    "printformat(level, format, ...) -> None\n"
);
static PyObject *py_printformat(PyObject *self, PyObject *all)
{
    int level = 0;
    char *format = "";
    PyObject *args = NULL, *varargs = NULL;
    TEXT_DEST_REC dest;
    char *script;

    args = PySequence_GetSlice(all, 0, 2);
    if (!args)
        goto error;

    varargs = PySequence_GetSlice(all, 2, PyTuple_Size(all));
    if (!varargs)
        goto error; 
    
    if (!PyArg_ParseTuple(args, "is", &level, &format))
        goto error; 

    script = pyloader_find_script_name();
    if (!script)
    {
        PyErr_Format(PyExc_RuntimeError, "No script found");
        goto error;
    }

    format_create_dest(&dest, NULL, NULL, level, NULL);
    if (!pythemes_printformat(&dest, script, format, varargs))
        goto error;

    Py_DECREF(args);
    Py_DECREF(varargs);
    
    Py_RETURN_NONE;

error:
    Py_XDECREF(args);
    Py_XDECREF(varargs);

    return NULL;
}

static int py_add_module_func(PyMethodDef *mdef)
{
    PyObject *func;

    g_return_val_if_fail(py_module != NULL, 0);

    func = PyCFunction_New(mdef, NULL);
    if (!func)
        return 0;

    if (PyModule_AddObject(py_module, mdef->ml_name, func) != 0)
    {
        Py_DECREF(func);
        return 0;
    }

    return 1;
}

static int py_add_method(PyTypeObject *type, PyMethodDef *mdef)
{
    int ret;
    PyObject *func;

    g_return_val_if_fail(type->tp_dict != NULL, 0);

    func = PyDescr_NewMethod(type, mdef);
    if (!func)
        return 0;

    ret = PyDict_SetItemString(type->tp_dict, mdef->ml_name, func);
    Py_DECREF(func);
    if (ret != 0)  
        return 0;

    return 1;
}

int pythemes_init(void)
{
    static PyMethodDef pfdef = {"printformat", (PyCFunction)py_printformat, 
        METH_VARARGS, py_printformat_doc};

    /* add function to main module and as member some types */
   
    if (!py_add_module_func(&pfdef))
        return 0;

    if (!py_add_method(&PyServerType, &pfdef))
        return 0;

    if (!py_add_method(&PyWindowType, &pfdef))
        return 0;

    if (!py_add_method(&PyWindowItemType, &pfdef))
        return 0;

    return 1;
}

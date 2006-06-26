#include <Python.h>
#include "pyirssi.h"
#include "pysignals.h"
#include "factory.h"

/* NOTE:
 * Signals must be registered to be accessible to Python scripts.
 *
 * Registering a dynamic signal adds a new SPEC_REC with a refcount of 1.
 * If another script registers the same signal, refcount is incremented.
 * Binding to the signal also increments its reference count. Likewise,
 * unregistering or unbinding a dynamic signal will decrement its refcount.
 * When refcount hits 0, the dynamic signal is removed from the list, and
 * scripts can no longer bind to it. Built-in signals in the sigmap are
 * never removed; it is an error for the refcount of any such signal entry 
 * to drop to 0.
 */

typedef struct _PY_SIGNAL_SPEC_REC 
{
    char *name;
    char *arglist;
    int refcount;
    int dynamic;
    int is_var; /* is this entry a prefix for a variable signal? */
} PY_SIGNAL_SPEC_REC;

#include "pysigmap.h"

#define SIGNAME(sig) (sig->command? sig->command : sig->signal->name)

/* hashtable for normal signals, tree for variable signal prefixes. */
static GHashTable *py_sighash = NULL;
static GTree *py_sigtree = NULL;

static void py_run_handler(PY_SIGNAL_REC *rec, void **args);
static void py_sig_proxy(void *p1, void *p2, void *p3, void *p4, void *p5, void *p6);
static void py_signal_ref(PY_SIGNAL_SPEC_REC *sig);
static int py_signal_unref(PY_SIGNAL_SPEC_REC *sig);
static void py_signal_add(PY_SIGNAL_SPEC_REC *sig);
static PY_SIGNAL_REC *py_signal_rec_new(const char *signal, PyObject *func, const char *command);
static void py_signal_rec_destroy(PY_SIGNAL_REC *sig);
static PyObject *py_mkstrlist(void *iobj);
static PyObject *py_i2py(char code, void *iobj);
static void *py_py2i(char code, PyObject *pobj, int arg);
static void py_getstrlist(GList **list, PyObject *pylist);
static int precmp(const char *spec, const char *test);
static PY_SIGNAL_SPEC_REC *py_signal_lookup(const char *name);
static void py_signal_remove(PY_SIGNAL_SPEC_REC *sig);

PY_SIGNAL_REC *pysignals_command_bind(const char *cmd, PyObject *func, 
        const char *category, int priority)
{
    PY_SIGNAL_REC *rec = py_signal_rec_new("send command", func, cmd);
    g_return_val_if_fail(rec != NULL, NULL);
    
    command_bind_full(MODULE_NAME, priority, cmd, 
            -1, category, (SIGNAL_FUNC)py_sig_proxy, rec);

    return rec;
}

int pysignals_command_bind_list(GSList **list, const char *command, 
        PyObject *func, const char *category, int priority)
{
    PY_SIGNAL_REC *rec = pysignals_command_bind(command, func, category, priority);
    if (!rec)
        return 0;

    *list = g_slist_append(*list, rec);
    return 1;
}

/* return NULL if signal is invalid */
PY_SIGNAL_REC *pysignals_signal_add(const char *signal, PyObject *func, int priority)
{
    PY_SIGNAL_REC *rec = py_signal_rec_new(signal, func, NULL);

    if (rec == NULL)
        return NULL;
   
    signal_add_full(MODULE_NAME, priority, SIGNAME(rec), 
            (SIGNAL_FUNC)py_sig_proxy, rec); 

    return rec;
}

int pysignals_signal_add_list(GSList **list, const char *signal, 
        PyObject *func, int priority)
{
    PY_SIGNAL_REC *rec = pysignals_signal_add(signal, func, priority);
    if (!rec)
        return 0;

    *list = g_slist_append(*list, rec);
    return 1;
}

void pysignals_command_unbind(PY_SIGNAL_REC *rec)
{
    g_return_if_fail(rec->is_signal == FALSE);
    g_return_if_fail(rec->command != NULL);

    command_unbind_full(rec->command, (SIGNAL_FUNC)py_sig_proxy, rec);
    py_signal_rec_destroy(rec);
}

void pysignals_signal_remove(PY_SIGNAL_REC *rec)
{
    g_return_if_fail(rec->is_signal == TRUE);

    signal_remove_full(SIGNAME(rec), (SIGNAL_FUNC)py_sig_proxy, rec);
    py_signal_rec_destroy(rec);
}

void pysignals_remove_generic(PY_SIGNAL_REC *rec)
{
    if (rec->is_signal)
        pysignals_signal_remove(rec);
    else
        pysignals_command_unbind(rec);
}

/* returns 1 when found and removed successfully */
int pysignals_remove_search(GSList **siglist, const char *name, 
        PyObject *func, PSG_TYPE type)
{
    GSList *node;

    for (node = *siglist; node != NULL; node = node->next)
    {
        PY_SIGNAL_REC *sig = node->data;

        if ((sig->is_signal && type == PSG_COMMAND) ||
                (!sig->is_signal && type == PSG_SIGNAL))
            continue;
        
        if ((strcmp(SIGNAME(sig), name) == 0) && 
                (func == NULL || func == sig->handler))
        {
            pysignals_remove_generic(sig);
            *siglist = g_slist_delete_link(*siglist, node);

            /* deleting node won't harm iteration because it quits here */
            return 1;
        }
    }

    return 0;
}

void pysignals_remove_list(GSList *siglist)
{
    GSList *node = siglist;

    for (node = siglist; node != NULL; node = node->next)
        pysignals_remove_generic(node->data);
}

static PyObject *py_mkstrlist(void *iobj)
{
    PyObject *list;
    GList *node, **ptr;
    ptr = iobj;

    list = PyList_New(0);
    if (!list)
        return NULL;
    
    for (node = *ptr; node != NULL; node = node->next)
    {
        int ret;
        PyObject *str;

        str = PyString_FromString(node->data);
        if (!str)
        {
            Py_DECREF(list);
            return NULL;
        }

        ret = PyList_Append(list, str);
        Py_DECREF(str);
        if (ret != 0)
        {
            Py_DECREF(list);
            return NULL;
        }
    }

    return list;
}

/* irssi obj -> PyObject */
static PyObject *py_i2py(char code, void *iobj)
{
    if (iobj == NULL)
        Py_RETURN_NONE;

    switch (code)
    {
        case '?':
            Py_RETURN_NONE;

        case 's':
            return PyString_FromString((char *)iobj);
        case 'u':
            return PyLong_FromUnsignedLong(*(unsigned long*)iobj);
        case 'I':
            return PyInt_FromLong(*(int *)iobj);
        case 'i':
            return PyInt_FromLong((int)iobj);

        case 'G':
            return py_mkstrlist(iobj);
        case 'L': /* list of nicks */
            return py_irssi_chatlist_new((GSList *)iobj, 1);

        case 'c':
        case 'S':
        case 'C':
        case 'q':
        case 'n':
        case 'W':
            return py_irssi_chat_new(iobj, 1);

        case 'd':
            return py_irssi_new(iobj, 1);

        case 'r':
            return pyreconnect_new(iobj);
        case 'o':
            return pycommand_new(iobj);
        case 'l':
            return pylog_new(iobj);
        case 'a':
            return pyrawlog_new(iobj);
        case 'g':
            return pyignore_new(iobj);
        case 'b':
            return pyban_new(iobj);
        case 'N':
            return pynetsplit_new(iobj);
        case 'e':
            return pynetsplit_server_new(iobj);
        case 'O':
            return pynotifylist_new(iobj);
        case 'p':
            return pyprocess_new(iobj);
        case 't':
            return pytextdest_new(iobj);
        case 'w':
            return pywindow_new(iobj);
    }

    return PyErr_Format(PyExc_TypeError, "unknown code %c", code);
}

/* PyObject -> irssi obj*/
static void *py_py2i(char code, PyObject *pobj, int arg)
{
    char *type;

    if (pobj == Py_None)
        return NULL;

    switch (code)
    {
        /* XXX: string doesn't (necssarily) persist */
        case 's':
            type = "str";
            if (PyString_Check(pobj)) return PyString_AsString(pobj);
            break;
        case 'i':
            type = "int";
            if (PyInt_Check(pobj)) return (void*)PyInt_AsLong(pobj);
            break;

        case 'L': /* list of nicks */
            /*FIXME*/
            return NULL;

        case 'c':
            type = "Chatnet";
            if (pychatnet_check(pobj)) return DATA(pobj);
            break;
        case 'S':
            type = "Server";
            if (pyserver_check(pobj)) return DATA(pobj);
            break;
        case 'C':
            type = "Channel";
            if (pychannel_check(pobj)) return DATA(pobj);
            break;
        case 'q':
            type = "Query";
            if (pyquery_check(pobj)) return DATA(pobj);
            break;
        case 'n':
            type = "Nick";
            if (pynick_check(pobj)) return DATA(pobj);
            break;
        case 'W':
            type = "WindowItem";
            if (pywindow_item_check(pobj)) return DATA(pobj);
            break;

        case 'd':
            type = "DCC";
            if (pydcc_check(pobj)) return DATA(pobj);
            break;

        case 'r':
            type = "Reconnect";
            if (pyreconnect_check(pobj)) return DATA(pobj);
            break;
        case 'o':
            type = "Command";
            if (pycommand_check(pobj)) return DATA(pobj);
            break;
        case 'l':
            type = "Log";
            if (pylog_check(pobj)) return DATA(pobj);
            break;
        case 'a':
            type = "Rawlog";
            if (pyrawlog_check(pobj)) return DATA(pobj);
            break;
        case 'g':
            type = "Ignore";
            if (pyignore_check(pobj)) return DATA(pobj);
            break;
        case 'b':
            type = "Ban";
            if (pyban_check(pobj)) return DATA(pobj);
            break;
        case 'N':
            type = "Netsplit";
            if (pynetsplit_check(pobj)) return DATA(pobj);
            break;
        case 'e':
            type = "NetsplitServer";
            if (pynetsplit_server_check(pobj)) return DATA(pobj);
            break;
        case 'O':
            type = "Notifylist";
            if (pynotifylist_check(pobj)) return DATA(pobj);
            break;
        case 'p':
            type = "Process";
            if (pyprocess_check(pobj)) return DATA(pobj);
            break;
        case 't':
            type = "TextDest";
            if (pytextdest_check(pobj)) return DATA(pobj);
            break;
        case 'w':
            type = "Window";
            if (pywindow_check(pobj)) return DATA(pobj);
            break;
        default:
            PyErr_Format(PyExc_TypeError, "don't know type code %c", code);
            return NULL;
    }

    PyErr_Format(PyExc_TypeError, "expected type %s for arg %d, but got %s", 
        type, arg, pobj->ob_type->tp_name);
    return NULL;
}

static void py_getstrlist(GList **list, PyObject *pylist)
{
    GList *out = NULL;
    int i;
    PyObject *str;
    char *cstr;

    for (i = 0; i < PyList_Size(pylist); i++)
    {
        str = PyList_GET_ITEM(pylist, i);
        if (!PyString_Check(str))
        {
            PyErr_SetString(PyExc_TypeError, "string list contains invalid elements");
            PyErr_Print();
            return;
        }

        cstr = g_strdup(PyString_AS_STRING(str));
        out = g_list_append(out, cstr);
    }

    g_list_foreach(*list, (GFunc)g_free, NULL);
    g_list_free(*list);
    *list = out;
}

static void py_run_handler(PY_SIGNAL_REC *rec, void **args)
{
    PyObject *argtup, *ret;
    char *arglist = rec->signal->arglist;
    int arglen, i, j;

    arglen = strlen(arglist);
    g_return_if_fail(arglen <= SIGNAL_MAX_ARGUMENTS);
    
    argtup = PyTuple_New(arglen);
    if (!argtup)
        goto error;

    for (i = 0; i < arglen; i++)
    {
        PyObject *arg = py_i2py(arglist[i], args[i]);
        if (!arg)
            goto error;

        PyTuple_SET_ITEM(argtup, i, arg);
    }
    
    ret = PyObject_CallObject(rec->handler, argtup);
    if (!ret)
        goto error;
  
    /*XXX: reference arg handling not well tested */
    for (i = 0, j = 0; i < arglen; i++)
    {
        GList **list;
        PyObject *pyarg = PyTuple_GET_ITEM(argtup, i);
        
        switch (arglist[i])
        {
            case 'G':
                list = args[i];
                py_getstrlist(list, pyarg);
                break;

            case 'I':
                if (ret != Py_None)
                {
                    PyObject *value;
                    int *intarg = args[i];

                    /* expect a proper return value to set reference arg.
                     * if return is a tuple, find the next item
                     */
                    if (PyTuple_Check(ret))
                        value = PyTuple_GET_ITEM(ret, j++);
                    else
                        value = ret;
                   
                    if (!PyInt_Check(value))
                        continue;

                    *intarg = PyInt_AS_LONG(value);
                }
                break;
        }
    }
    
    Py_XDECREF(ret);

error:
    Py_XDECREF(argtup);
    if (PyErr_Occurred())
        PyErr_Print();
}

static void py_sig_proxy(void *p1, void *p2, void *p3, void *p4, void *p5, void *p6)
{
    PY_SIGNAL_REC *rec = signal_get_user_data();
    void *args[6];

    args[0] = p1; args[1] = p2; args[2] = p3;
    args[3] = p4; args[4] = p5; args[5] = p6;
    py_run_handler(rec, args);
}

int pysignals_emit(const char *signal, PyObject *argtup)
{
    PY_SIGNAL_SPEC_REC *spec;
    void *args[6];
    char *arglist;
    int i;
    int maxargs;

    memset(args, 0, sizeof args);

    spec = py_signal_lookup(signal);
    if (!spec)
    {
        PyErr_Format(PyExc_KeyError, "signal not found");
        return 0;
    }

    arglist = spec->arglist;
    maxargs = strlen(arglist);

    for (i = 0; i < maxargs && i+1 < PyTuple_Size(argtup); i++)
    {
        args[i] = py_py2i(arglist[i], PyTuple_GET_ITEM(argtup, i+1), i+1);
        if (PyErr_Occurred()) /* XXX: any cleanup needed? */
            return 0;
    }

    signal_emit(signal, maxargs, 
            args[0], args[1], args[2],   
            args[3], args[4], args[5]);

    return 1;
}

/* returns NULL if signal is invalid, incr reference to func */
static PY_SIGNAL_REC *py_signal_rec_new(const char *signal, PyObject *func, const char *command)
{
    PY_SIGNAL_REC *rec;
    PY_SIGNAL_SPEC_REC *spec;

    g_return_val_if_fail(func != NULL, NULL);
    
    spec = py_signal_lookup(signal);
    if (!spec)
        return NULL;

    rec = g_new0(PY_SIGNAL_REC, 1);
    rec->signal = spec;
    rec->handler = func;
    Py_INCREF(func);

    if (command)
    {
        rec->is_signal = FALSE;
        rec->command = g_strdup(command);
    }
    else 
    {
        rec->is_signal = TRUE;
        /* handle variable signal. requested signal will be longer than spec->name, ie
         * signal = "var signal POOOM", spec->name = "var signal " 
         */
        if (strcmp(signal, spec->name) != 0)
            rec->command = g_strdup(signal);
    }

    py_signal_ref(spec);

    return rec;
}

static void py_signal_rec_destroy(PY_SIGNAL_REC *sig)
{
    py_signal_unref(sig->signal);
    Py_DECREF(sig->handler);
    g_free(sig->command);
    g_free(sig);
}

static void py_signal_add(PY_SIGNAL_SPEC_REC *sig)
{
    if (sig->is_var)
        g_tree_insert(py_sigtree, sig->name, sig);
    else
        g_hash_table_insert(py_sighash, sig->name, sig);
}

static void py_signal_remove(PY_SIGNAL_SPEC_REC *sig)
{
    int ret;

    if (sig->is_var)
        g_tree_remove(py_sigtree, sig->name);
    else
    {
        ret = g_hash_table_remove(py_sighash, sig->name);
        g_return_if_fail(ret != FALSE);
    }
}

static int precmp(const char *spec, const char *test)
{
    while (*spec == *test++)
        if (*spec++ == '\0')
            return 0;

    /* Variable event prefix matches (spec must never be empty string)*/
    /* precmp("var event ", "var event POOOM") -> 0 */
    if (*spec == '\0' && *(spec-1) == ' ')
        return 0;
    
    return *(const unsigned char *)(test - 1) - *(const unsigned char *)spec; 
}

static PY_SIGNAL_SPEC_REC *py_signal_lookup(const char *name)
{
    PY_SIGNAL_SPEC_REC *ret;

    /* First check the normal signals hash, then check the variable signal prefixes in the tree */
    ret = g_hash_table_lookup(py_sighash, name);
    if (!ret)
        ret = g_tree_search(py_sigtree, (GCompareFunc)precmp, name);

    return ret;
}

static void py_signal_ref(PY_SIGNAL_SPEC_REC *sig)
{
    g_return_if_fail(sig->refcount >= 0);

    sig->refcount++;
}

static int py_signal_unref(PY_SIGNAL_SPEC_REC *sig)
{
    g_return_val_if_fail(sig->refcount >= 1, 0);
    g_return_val_if_fail(sig->refcount > 1 || sig->dynamic, 0);

    sig->refcount--;

    if (sig->refcount == 0)
    {
        py_signal_remove(sig);

        /* freeing name also takes care of the key */
        g_free(sig->name);
        g_free(sig->arglist);
        g_free(sig);

        return 1;
    }

    return 0;
}

/* returns 0 when signal already exists, but with different args, or when
   similar signal prefix is already present. */
int pysignals_register(const char *name, const char *arglist)
{
    int len;
    PY_SIGNAL_SPEC_REC *spec;

    len = strlen(name);
    g_return_val_if_fail(len > 0, 0);
    
    spec = py_signal_lookup(name);
    if (!spec)
    {
        spec = g_new0(PY_SIGNAL_SPEC_REC, 1);
        spec->is_var = name[len-1] == ' '; /* trailing space means signal prefix */
        spec->dynamic = 1;
        spec->refcount = 0;
        spec->name = g_strdup(name);
        spec->arglist = g_strdup(arglist);
        
        py_signal_add(spec);
    }
    else if (strcmp(spec->arglist, arglist) || strcmp(spec->name, name))
        return 0;

    spec->refcount++;

    return 1;
}

/* returns 0 when name doesn't exist */
int pysignals_unregister(const char *name)
{
    PY_SIGNAL_SPEC_REC *spec;

    spec = py_signal_lookup(name);
    if (!spec)
        return 0;

    py_signal_unref(spec);
    return 1;
}

void pysignals_init(void)
{
    int i;
    
    g_return_if_fail(py_sighash == NULL);
    g_return_if_fail(py_sigtree == NULL);

    py_sigtree = g_tree_new((GCompareFunc)strcmp);
    py_sighash = g_hash_table_new(g_str_hash, g_str_equal);

    for (i = 0; i < py_sigmap_len(); i++)
    {
        py_sigmap[i].refcount = 1;
        py_sigmap[i].dynamic = 0;
        py_signal_add(&py_sigmap[i]);
    }
}

static int py_check_sig(char *key, PY_SIGNAL_SPEC_REC *value, void *data)
{
    /* shouldn't need to deallocate any script recs -- all remaining at 
       this point should not be dynamic. non dynamic signals should have
       no outstanding references */
    g_return_val_if_fail(value->dynamic == 0, FALSE);
    g_return_val_if_fail(value->refcount == 1, FALSE);

    return FALSE;
}

/* XXX: remember to remove all scripts before calling this deinit */
void pysignals_deinit(void)
{
    g_return_if_fail(py_sighash != NULL);
    g_return_if_fail(py_sigtree != NULL);
    
    g_tree_foreach(py_sigtree, (GTraverseFunc)py_check_sig, NULL); 
    g_hash_table_foreach_remove(py_sighash, (GHRFunc)py_check_sig, NULL);

    g_tree_destroy(py_sigtree);
    g_hash_table_destroy(py_sighash);
    py_sigtree = NULL;
    py_sighash = NULL;
}

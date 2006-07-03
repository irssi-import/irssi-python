#include <Python.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#include "pyirssi.h"
#include "pycore.h"
#include "pyloader.h"
#include "pymodule.h"
#include "pysignals.h"
#include "pythemes.h"
#include "factory.h"

/*XXX: copy parse into utils */
static void cmd_exec(const char *data)
{
    PyObject *co;
    PyObject *ret;
    PyObject *d;
    PyObject *m;
    char *cmd;

    if (!*data)
        cmd_return_error(CMDERR_NOT_ENOUGH_PARAMS);

    cmd = g_strconcat(data, "\n", NULL);
    
    m = PyImport_AddModule("__main__");
    if (!m)
        goto error;

    d = PyModule_GetDict(m);
    if (!d)
        goto error;

    co = Py_CompileString(cmd, "<stdin>", Py_single_input);
    if (!co)
        goto error;

    ret = PyEval_EvalCode((PyCodeObject *)co, d, d);
    Py_DECREF(co);
    Py_XDECREF(ret);

error:
    g_free(cmd);
    if (PyErr_Occurred())
        PyErr_Print();
}

static void cmd_load(const char *data)
{
    char **argv;

    argv = g_strsplit(data, " ", -1);
    if (*argv == NULL || **argv == '\0')
    {
        g_strfreev(argv);
        cmd_return_error(CMDERR_NOT_ENOUGH_PARAMS);
    }

    pyloader_load_script_argv(argv);
    g_strfreev(argv);
}

static void cmd_unload(const char *data)
{
    void *free_arg;
    char *script;

    if (!cmd_get_params(data, &free_arg, 1, &script))
        return;

    if (*script == '\0')
        cmd_param_error(CMDERR_NOT_ENOUGH_PARAMS);

    pyloader_unload_script(script); 
    
    cmd_params_free(free_arg);
}

static void cmd_list()
{
    char buf[128];
    GSList *list;

    list = pyloader_list();

    g_snprintf(buf, sizeof(buf), "%-15s %s", "Name", "File");

    if (list != NULL)
    {
        GSList *node;

        printtext_string(NULL, NULL, MSGLEVEL_CLIENTCRAP, buf);
        for (node = list; node != NULL; node = node->next)
        {
            PY_LIST_REC *item = node->data;
            g_snprintf(buf, sizeof(buf), "%-15s %s", item->name, item->file); 

            printtext_string(NULL, NULL, MSGLEVEL_CLIENTCRAP, buf);
        }
    }
    else
        printtext_string(NULL, NULL, MSGLEVEL_CLIENTERROR, "No python scripts are loaded");

    pyloader_list_destroy(&list);
}

#if 0
/* why doesn't this get called? */
static void intr_catch(int sig)
{
    printtext(NULL, NULL, MSGLEVEL_CLIENTERROR, "got sig %d", sig);
    PyErr_SetInterrupt();
}
#endif 

void irssi_python_init(void)
{
    Py_InitializeEx(0);

    pysignals_init();
    if (!pyloader_init() || !pymodule_init() || !factory_init() || !pythemes_init()) 
    {
        printtext(NULL, NULL, MSGLEVEL_CLIENTERROR, "Failed to load Python");
        return;
    }

    /*PyImport_ImportModule("irssi_startup");*/
    /* Install the custom output handlers, import hook and reload function */
    /* XXX: handle import error */
    PyRun_SimpleString(
            "import irssi_startup\n"
    );

    //assert(signal(SIGINT, intr_catch) != SIG_ERR); 
    
    command_bind("pyload", NULL, (SIGNAL_FUNC) cmd_load);
    command_bind("pyunload", NULL, (SIGNAL_FUNC) cmd_unload);
    command_bind("pylist", NULL, (SIGNAL_FUNC) cmd_list);
    command_bind("pyexec", NULL, (SIGNAL_FUNC) cmd_exec);
    module_register(MODULE_NAME, "core");
}

void irssi_python_deinit(void)
{
    command_unbind("pyload", (SIGNAL_FUNC) cmd_load);
    command_unbind("pyunload", (SIGNAL_FUNC) cmd_unload);
    command_unbind("pylist", (SIGNAL_FUNC) cmd_list);
    command_unbind("pyexec", (SIGNAL_FUNC) cmd_exec);

    pymodule_deinit();
    pyloader_deinit();
    pysignals_deinit();
    Py_Finalize();
}

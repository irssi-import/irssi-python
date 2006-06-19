#include <Python.h>
#include "pyirssi.h"
#include "pysignals.h"
#include "factory.h"

static void py_command_proxy(char *data, SERVER_REC *server, WI_ITEM_REC *witem);

/* crec should be owned by a Script object */
void py_command_bind(const char *category, PY_COMMAND_REC *crec)
{
    command_bind_full(MODULE_NAME, SIGNAL_PRIORITY_DEFAULT, crec->name, 
            -1, category, (SIGNAL_FUNC)py_command_proxy, crec);
}

void py_command_unbind(PY_COMMAND_REC *crec)
{
    command_unbind_full(crec->name, (SIGNAL_FUNC)py_command_proxy, crec);
}

/* This is just for testing. A complete version would use a signal map, a better
   wrapper object factory system, and a generic signal handler like in the Perl 
   bindings */ 
static void py_command_proxy(char *data, SERVER_REC *server, WI_ITEM_REC *witem)
{
    PY_COMMAND_REC *crec;
    PyObject *ret, *pyserver, *pywitem;

#if 0
    if (server)
    {
        CHAT_PROTOCOL_REC *chat = chat_protocol_find_id(server->chat_type);
        if (chat && !strcmp(chat->name, "IRC"))
            pyserver = pyirc_server_new(server);
        else
            pyserver = pyserver_new(server);
        g_assert(pyserver != NULL);
    }
    else
    {
        pyserver = Py_None;
        Py_INCREF(pyserver);
    }
    if (witem)
    {
        char *type = module_find_id_str("WINDOW ITEM TYPE", witem->type);
        g_assert(type != NULL);

        if (!strcmp("CHANNEL", type))
            pywitem = pychannel_new(witem);
        else if (!strcmp("QUERY", type))
            pywitem = pyquery_new(witem);
        else
            pywitem = pywindow_item_new(witem);

        g_assert(pywitem != NULL);
    }
    else
    {
        pywitem = Py_None;
        Py_INCREF(pywitem);
    }
#endif

    if (server)
    {
        pyserver = py_irssi_chat_new(server, 1);
        g_assert(pyserver != NULL);
    }
    else
    {
        pyserver = Py_None;
        Py_INCREF(Py_None);
    }

    if (witem)
    {
        pywitem = py_irssi_chat_new(witem, 1);
        g_assert(pywitem != NULL);
    }
    else
    {
        pywitem = Py_None;
        Py_INCREF(Py_None);
    }
    
    crec = signal_get_user_data();
    ret = PyObject_CallFunction(crec->handler, "(sOO)", data, pyserver, pywitem);
    if (!ret)
        PyErr_Print();
    else
        Py_DECREF(ret);

    Py_DECREF(pyserver);
    Py_DECREF(pywitem);
}

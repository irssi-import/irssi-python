#include <Python.h>
#include <frameobject.h>
#include "pymodule.h"
#include "pyirssi_irc.h"
#include "pyscript-object.h"
#include "factory.h"
#include "pyutils.h"
#include "pysignals.h"

/*
 * This module is some what different than the Perl's.
 * Script specific operations are handled by the Script object 
 * instead of by a function in this module.  command_bind, 
 * signal_bind, etc require data to be saved about the script 
 * for cleanup purposes, so I moved those functions to the script 
 * object.
 */

/* Main embedded module */
PyObject *py_module = NULL;

static PyObject *find_script(void);

/* Module functions */
/*XXX: prefix PY to avoid ambiguity with py_command function */
PyDoc_STRVAR(PY_command_doc,
    "Execute command"
);
static PyObject *PY_command(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"cmd", NULL};
    char *cmd = "";

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, 
           &cmd))
        return NULL;

    py_command(cmd, NULL, NULL);
    
    Py_RETURN_NONE;
}

PyDoc_STRVAR(py_prnt_doc,
    "print output"
);
/*XXX: print is a python keyword, so abbreviate it */
static PyObject *py_prnt(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = {"text", "msglvl", NULL};
    int msglvl = MSGLEVEL_CLIENTNOTICE;
    char *text = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s|i:prnt", kwlist, 
                &text, &msglvl))
        return NULL;

    printtext_string(NULL, NULL, msglvl, text);
    
    Py_RETURN_NONE;
}

PyDoc_STRVAR(py_get_script_doc,
    "Get Irssi script object"
);
static PyObject *py_get_script(PyObject *self, PyObject *args)
{
    PyObject *ret = find_script();

    /* XXX: type check */
    
    if (!ret)
        PyErr_SetString(PyExc_RuntimeError, "unable to find script object");
    else
        Py_INCREF(ret);
   
    return ret;
}

PyDoc_STRVAR(py_chatnet_find_doc,
    "Find chat network with name"
);
static PyObject *py_chatnet_find(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"name", NULL};
    char *name = "";

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, 
           &name))
        return NULL;

    return py_irssi_chat_new(chatnet_find(name), 1);
}

PyDoc_STRVAR(py_chatnets_doc,
    "Return a list of all chatnets"
);
static PyObject *py_chatnets(PyObject *self, PyObject *args)
{
    return py_irssi_chatlist_new(chatnets, 1);
}

PyDoc_STRVAR(py_reconnects_doc,
    "Return a list of all reconnects"
);
static PyObject *py_reconnects(PyObject *self, PyObject *args)
{
    return py_irssi_objlist_new(reconnects, 1, (InitFunc)pyreconnect_new);
}

PyDoc_STRVAR(py_servers_doc,
    "Return a list of all servers"
);
static PyObject *py_servers(PyObject *self, PyObject *args)
{
    return py_irssi_chatlist_new(servers, 1);
}

PyDoc_STRVAR(py_channels_doc,
    "Return channel list"
);
static PyObject *py_channels(PyObject *self, PyObject *args)
{
    return py_irssi_chatlist_new(channels, 1);
}

PyDoc_STRVAR(py_channel_find_doc,
    "Find channel from any server"
);
static PyObject *py_channel_find(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"name", NULL};
    char *name = "";

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, 
           &name))
        return NULL;

    return py_irssi_chat_new(channel_find(NULL, name), 1);
}

PyDoc_STRVAR(py_windows_doc,
    "Get a list of all windows"
);
static PyObject *py_windows(PyObject *self, PyObject *args)
{
    return py_irssi_objlist_new(windows, 1, (InitFunc)pywindow_new);
}

PyDoc_STRVAR(py_active_win_doc,
    "Return active window"
);
static PyObject *py_active_win(PyObject *self, PyObject *args)
{
    if (active_win)
        return pywindow_new(active_win);
    
    Py_RETURN_NONE;
}

PyDoc_STRVAR(py_active_server_doc,
    "Return active server"
);
static PyObject *py_active_server(PyObject *self, PyObject *args)
{
    if (active_win)
        return py_irssi_chat_new(active_win->active_server, 1);

    Py_RETURN_NONE;
}

PyDoc_STRVAR(py_window_find_name_doc,
    "Find window with name"
);
static PyObject *py_window_find_name(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"name", NULL};
    char *name = "";
    WINDOW_REC *win;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, 
           &name))
        return NULL;

    win = window_find_name(name);
    if (win)
        return pywindow_new(win);

    Py_RETURN_NONE;
}

PyDoc_STRVAR(py_window_find_refnum_doc,
    "Find window with reference number"
);
static PyObject *py_window_find_refnum(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"refnum", NULL};
    int refnum = 0;
    WINDOW_REC *win;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, 
           &refnum))
        return NULL;

    win = window_find_refnum(refnum);
    if (win)
        return pywindow_new(win);

    Py_RETURN_NONE;
}

PyDoc_STRVAR(py_window_refnum_prev_doc,
    "Return refnum for window that's previous in window list"
);
static PyObject *py_window_refnum_prev(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"refnum", "wrap", NULL};
    int refnum = 0;
    int wrap = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "ii", kwlist, 
           &refnum, &wrap))
        return NULL;

    return PyInt_FromLong(window_refnum_prev(refnum, wrap));
}

PyDoc_STRVAR(py_window_refnum_next_doc,
    "Return refnum for window that's next in window list"
);
static PyObject *py_window_refnum_next(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"refnum", "wrap", NULL};
    int refnum = 0;
    int wrap = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "ii", kwlist, 
           &refnum, &wrap))
        return NULL;

    return PyInt_FromLong(window_refnum_next(refnum, wrap));
}

PyDoc_STRVAR(py_windows_refnum_last_doc,
    "Return refnum for last window."
);
static PyObject *py_windows_refnum_last(PyObject *self, PyObject *args)
{
    return PyInt_FromLong(windows_refnum_last());
}

PyDoc_STRVAR(py_window_find_level_doc,
    "Find window with level."
);
static PyObject *py_window_find_level(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"level", NULL};
    int level = 0;
    WINDOW_REC *win;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, 
           &level))
        return NULL;

    win = window_find_level(NULL, level);
    if (win)
        return pywindow_new(win);

    Py_RETURN_NONE;
}

PyDoc_STRVAR(py_window_find_item_doc,
    "Find window which contains window item with specified name."
);
static PyObject *py_window_find_item(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"name", NULL};
    char *name = "";
    WINDOW_REC *win;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, 
           &name))
        return NULL;

    win = window_find_item(NULL, name);
    if (win)
        return pywindow_new(win);

    Py_RETURN_NONE;
}

PyDoc_STRVAR(py_window_find_closest_doc,
    "Find window that matches best to given arguments. `name' can be either"
    "window name or name of one of the window items."
);
static PyObject *py_window_find_closest(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"name", "level", NULL};
    char *name = "";
    int level = 0;
    WINDOW_REC *win;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "si", kwlist, 
           &name, &level))
        return NULL;

    win = window_find_closest(NULL, name, level);
    if (win)
        return pywindow_new(win);

    Py_RETURN_NONE;
}

PyDoc_STRVAR(py_window_item_find_doc,
    "Find window item that matches best to given arguments."
);
static PyObject *py_window_item_find(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"name", NULL};
    char *name = "";

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, 
           &name))
        return NULL;

    return py_irssi_chat_new(window_item_find(NULL, name), 1);
}

/*XXX: this could be __init__ for Window */
PyDoc_STRVAR(py_window_create_doc,
    "window_create(item=None, automatic=False) -> Window object\n"
    "\n"
    "Create a new window\n"
);
static PyObject *py_window_create(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"item", "automatic", NULL};
    PyObject *item = NULL;
    int automatic = 0;
    WI_ITEM_REC *witem = NULL;
    WINDOW_REC *win;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|Oi", kwlist, 
           &item, &automatic))
        return NULL;

    if (item)
    {
        if (pywindow_item_check(item))
        {
            witem = ((PyWindowItem*)item)->data;
            if (!witem)
                return PyErr_Format(PyExc_TypeError, "invalid window item");
            else if (witem->server != NULL)
                return PyErr_Format(PyExc_TypeError, "window item already assigned to window");
        }
        else if (item == Py_None)
            ;
        else
            return PyErr_Format(PyExc_TypeError, "item must be window item or None");
    }

    win = window_create(witem, automatic);
    if (win)
        return pywindow_new(win);

    Py_RETURN_NONE;
}

PyDoc_STRVAR(py_server_find_tag_doc,
    "Find server with tag"
);
static PyObject *py_server_find_tag(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"tag", NULL};
    char *tag = "";

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, 
           &tag))
        return NULL;

    return py_irssi_chat_new(server_find_tag(tag), 1);
}

PyDoc_STRVAR(py_server_find_chatnet_doc,
    "Find first server that is in chatnet"
);
static PyObject *py_server_find_chatnet(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"chatnet", NULL};
    char *chatnet = "";

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, 
           &chatnet))
        return NULL;

    return py_irssi_chat_new(server_find_chatnet(chatnet), 1);
}

PyDoc_STRVAR(py_queries_doc,
    "Return a list of open queries."
);
static PyObject *py_queries(PyObject *self, PyObject *args)
{
    return py_irssi_chatlist_new(queries, 1);
}

PyDoc_STRVAR(py_query_find_doc,
    "Find a query from any server."
);
static PyObject *py_query_find(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"nick", NULL};
    char *nick = "";

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, 
           &nick))
        return NULL;

    return py_irssi_chat_new(query_find(NULL, nick), 1);
}

PyDoc_STRVAR(py_mask_match_doc,
    "Return true if mask matches nick!user@host"
);
static PyObject *py_mask_match(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"mask", "nick", "user", "host", NULL};
    char *mask = "";
    char *nick = "";
    char *user = "";
    char *host = "";

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "ssss", kwlist, 
           &mask, &nick, &user, &host))
        return NULL;

    return PyBool_FromLong(mask_match(NULL, mask, nick, user, host));
}

PyDoc_STRVAR(py_mask_match_address_doc,
    "Return True if mask matches nick!address"
);
static PyObject *py_mask_match_address(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"mask", "nick", "address", NULL};
    char *mask = "";
    char *nick = "";
    char *address = "";

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "sss", kwlist, 
           &mask, &nick, &address))
        return NULL;

    return PyBool_FromLong(mask_match_address(NULL, mask, nick, address));
}

PyDoc_STRVAR(py_masks_match_doc,
    "Return True if any mask in the masks (string separated by spaces)\n"
    "matches nick!address\n"
);
static PyObject *py_masks_match(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"masks", "nick", "address", NULL};
    char *masks = "";
    char *nick = "";
    char *address = "";

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "sss", kwlist, 
           &masks, &nick, &address))
        return NULL;

    return PyBool_FromLong(masks_match(NULL, masks, nick, address));
}

PyDoc_STRVAR(py_rawlog_set_size_doc,
    "Set the default rawlog size for new rawlogs."
);
static PyObject *py_rawlog_set_size(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"lines", NULL};
    int lines = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, 
           &lines))
        return NULL;

    rawlog_set_size(lines);
    
    Py_RETURN_NONE;
}

PyDoc_STRVAR(py_logs_doc,
    "Return list of logs."
);
static PyObject *py_logs(PyObject *self, PyObject *args)
{
    return py_irssi_objlist_new(logs, 1, (InitFunc)pylog_new);
}

PyDoc_STRVAR(py_log_find_doc,
    "Find log by file name."
);
static PyObject *py_log_find(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"fname", NULL};
    char *fname = "";
    LOG_REC *log;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, 
           &fname))
        return NULL;

    log = log_find(fname);
    if (log)
        return pylog_new(log);
    
    Py_RETURN_NONE;
}

PyDoc_STRVAR(py_ignores_doc,
    "Return a list of ignore entries"
);
static PyObject *py_ignores(PyObject *self, PyObject *args)
{
    return py_irssi_objlist_new(ignores, 1, (InitFunc)pyignore_new);
}

PyDoc_STRVAR(py_ignore_check_doc,
    "Return True if ignore matches"
);
static PyObject *py_ignore_check(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"nick", "host", "channel", "text", "level", NULL};
    char *nick = "";
    char *host = "";
    char *channel = "";
    char *text = "";
    int level = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "ssssi", kwlist, 
           &nick, &host, &channel, &text, &level))
        return NULL;

    return PyBool_FromLong(ignore_check(NULL, nick, host, channel, text, level));
}

PyDoc_STRVAR(py_dccs_doc,
    "Return list of active DCCs"
);
static PyObject *py_dccs(PyObject *self, PyObject *args)
{
    return py_irssi_list_new(dcc_conns, 1);
}

PyDoc_STRVAR(py_dcc_register_type_doc,
    "???"
);
static PyObject *py_dcc_register_type(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"type", NULL};
    char *type = "";

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, 
           &type))
        return NULL;

    dcc_register_type(type);
    
    Py_RETURN_NONE;
}

PyDoc_STRVAR(py_dcc_unregister_type_doc,
    "???"
);
static PyObject *py_dcc_unregister_type(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"type", NULL};
    char *type = "";

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, 
           &type))
        return NULL;

    dcc_unregister_type(type);

    Py_RETURN_NONE;
}

PyDoc_STRVAR(py_dcc_find_request_latest_doc,
    "???"
);
static PyObject *py_dcc_find_request_latest(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"type", NULL};
    int type = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, 
           &type))
        return NULL;

    return py_irssi_new(dcc_find_request_latest(type), 1);
}

PyDoc_STRVAR(py_dcc_find_request_doc,
    "???"
);
static PyObject *py_dcc_find_request(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"type", "nick", "arg", NULL};
    int type = 0;
    char *nick = "";
    char *arg = "";

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "iss", kwlist, 
           &type, &nick, &arg))
        return NULL;

    return py_irssi_new(dcc_find_request(type, nick, arg), 1);
}

PyDoc_STRVAR(py_dcc_chat_find_id_doc,
    "???"
);
static PyObject *py_dcc_chat_find_id(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"id", NULL};
    char *id = "";

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, 
           &id))
        return NULL;

    return py_irssi_new(dcc_chat_find_id(id), 1);
}

PyDoc_STRVAR(py_dcc_str2type_doc,
    "???"
);
static PyObject *py_dcc_str2type(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"type", NULL};
    char *type = "";

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, 
           &type))
        return NULL;

    return PyInt_FromLong(dcc_str2type(type));
}

PyDoc_STRVAR(py_dcc_type2str_doc,
    "???"
);
static PyObject *py_dcc_type2str(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"type", NULL};
    int type = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, 
           &type))
        return NULL;

    RET_AS_STRING_OR_NONE(dcc_type2str(type));
}

PyDoc_STRVAR(py_dcc_get_download_path_doc,
    "???"
);
static PyObject *py_dcc_get_download_path(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"fname", NULL};
    char *fname = "";
    char *path;
    PyObject *pypath;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, 
           &fname))
        return NULL;

    path = dcc_get_download_path(fname);
    if (!path)
        Py_RETURN_NONE; /*XXX: how to handle? */
    
    pypath = PyString_FromString(path);
    g_free(path);

    return pypath;
}

PyDoc_STRVAR(py_notifies_doc,
    "Return list of notifies"
);
static PyObject *py_notifies(PyObject *self, PyObject *args)
{
    return py_irssi_objlist_new(notifies, 1, (InitFunc)pynotifylist_new); 
}

PyDoc_STRVAR(py_notifylist_add_doc,
    "notifylist_add(mask, ircnets=None, away_check=0, idle_time_check=0) -> Notifylist object\n"
    "\n"
    "Add new item to notify list\n"
);
static PyObject *py_notifylist_add(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"mask", "ircnets", "away_check", "idle_check_time", NULL};
    char *mask = "";
    char *ircnets = NULL;
    int away_check = 0;
    int idle_check_time = 0;
    NOTIFYLIST_REC *rec;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s|zii", kwlist, 
           &mask, &ircnets, &away_check, &idle_check_time))
        return NULL;

    rec = notifylist_add(mask, ircnets, away_check, idle_check_time);
    if (rec)
        return pynotifylist_new(rec);

    Py_RETURN_NONE;
}

PyDoc_STRVAR(py_notifylist_remove_doc,
    "Remove notify item from notify list"
);
static PyObject *py_notifylist_remove(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"mask", NULL};
    char *mask = "";

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, 
           &mask))
        return NULL;

    notifylist_remove(mask);
    
    Py_RETURN_NONE;
}

PyDoc_STRVAR(py_notifylist_ison_doc,
    "notifylist_ison(nick, serverlist="") -> IrcServer object\n"
    "\n"
    "Check if nick is in IRC. serverlist is a space separated list of server tags.\n"
    "If it's empty string, all servers will be checked\n"
);
static PyObject *py_notifylist_ison(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"nick", "serverlist", NULL};
    char *nick = "";
    char *serverlist = "";

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s|s", kwlist, 
           &nick, &serverlist))
        return NULL;

    return py_irssi_chat_new(notifylist_ison(nick, serverlist), 1);
}

PyDoc_STRVAR(py_notifylist_find_doc,
    "notifylist_find(mask, ircnet=None) -> Notifylist object\n"
    "\n"
    "Find notify\n"
);
static PyObject *py_notifylist_find(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"mask", "ircnet", NULL};
    char *mask = "";
    char *ircnet = NULL;
    NOTIFYLIST_REC *rec;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s|z", kwlist, 
           &mask, &ircnet))
        return NULL;

    rec = notifylist_find(mask, ircnet);
    if (rec)
        return pynotifylist_new(rec);
    
    Py_RETURN_NONE;
}

PyDoc_STRVAR(py_commands_doc,
    "Return a list of all commands."
);
static PyObject *py_commands(PyObject *self, PyObject *args)
{
    return py_irssi_objlist_new(commands, 1, (InitFunc)pycommand_new);
}

PyDoc_STRVAR(py_level2bits_doc,
    "Level string -> number"
);
static PyObject *py_level2bits(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"level", NULL};
    char *level = "";

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, 
           &level))
        return NULL;

    return PyLong_FromUnsignedLong(level2bits(level));
}

PyDoc_STRVAR(py_bits2level_doc,
    "Level number -> string"
);
static PyObject *py_bits2level(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"bits", NULL};
    unsigned bits;
    char *str;
    PyObject *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "I", kwlist, 
           &bits))
        return NULL;

    str = bits2level(bits);
    if (str)
    {
        ret = PyString_FromString(str);
        g_free(str);
        return ret;
    }

    Py_RETURN_NONE;
}

PyDoc_STRVAR(py_combine_level_doc,
    "Combine level number to level string ('+level -level'). Return new level number."
);
static PyObject *py_combine_level(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"level", "str", NULL};
    int level = 0;
    char *str = "";

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "is", kwlist, 
           &level, &str))
        return NULL;

    return PyLong_FromUnsignedLong(combine_level(level, str));
}

PyDoc_STRVAR(py_signal_emit_doc,
    "signal_emit(signal, *args) -> None\n"
    "\n"
    "Emit an Irssi signal with up to 6 arguments\n"
);
static PyObject *py_signal_emit(PyObject *self, PyObject *args)
{
    PyObject *pysig;

    if (PyTuple_Size(args) < 1 || PyTuple_Size(args) > SIGNAL_MAX_ARGUMENTS)
        return PyErr_Format(PyExc_TypeError, "need at least one argument for signal");

    pysig = PyTuple_GET_ITEM(args, 0);
    if (!PyString_Check(pysig))
        return PyErr_Format(PyExc_TypeError, "signal must be string");
    
    if (!pysignals_emit(PyString_AS_STRING(pysig), args))
        return NULL;

    Py_RETURN_NONE;
}

PyDoc_STRVAR(py_signal_stop_doc,
    "signal_stop() -> None\n"
    "\n"
    "Stop the signal that's currently being emitted.\n"
);
static PyObject *py_signal_stop(PyObject *self, PyObject *args)
{
    signal_stop();
    Py_RETURN_NONE;
}

PyDoc_STRVAR(py_signal_stop_by_name_doc,
    "signal_stop_by_name(signal) -> None\n"
    "\n"
    "Stop the signal, 'signal', thats currently being emitted by name\n"
);
static PyObject *py_signal_stop_by_name(PyObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"signal", NULL};
    char *signal = "";

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, 
           &signal))
        return NULL;

    signal_stop_by_name(signal);
    
    Py_RETURN_NONE;
}

static PyMethodDef ModuleMethods[] = {
    {"prnt", (PyCFunction)py_prnt, METH_VARARGS|METH_KEYWORDS, py_prnt_doc},
    {"get_script", (PyCFunction)py_get_script, METH_NOARGS, py_get_script_doc},
    {"chatnet_find", (PyCFunction)py_chatnet_find, METH_VARARGS | METH_KEYWORDS,
        py_chatnet_find_doc},
    {"chatnets", (PyCFunction)py_chatnets, METH_NOARGS,
        py_chatnets_doc},
    {"reconnects", (PyCFunction)py_reconnects, METH_NOARGS,
        py_reconnects_doc},
    {"servers", (PyCFunction)py_servers, METH_NOARGS,
        py_servers_doc},
    {"windows", (PyCFunction)py_windows, METH_NOARGS,
        py_windows_doc},
    {"active_win", (PyCFunction)py_active_win, METH_NOARGS,
        py_active_win_doc},
    {"active_server", (PyCFunction)py_active_server, METH_NOARGS,
        py_active_server_doc},
    {"window_find_name", (PyCFunction)py_window_find_name, METH_VARARGS | METH_KEYWORDS,
        py_window_find_name_doc},
    {"window_find_refnum", (PyCFunction)py_window_find_refnum, METH_VARARGS | METH_KEYWORDS,
        py_window_find_refnum_doc},
    {"window_refnum_prev", (PyCFunction)py_window_refnum_prev, METH_VARARGS | METH_KEYWORDS,
        py_window_refnum_prev_doc},
    {"window_refnum_next", (PyCFunction)py_window_refnum_next, METH_VARARGS | METH_KEYWORDS,
        py_window_refnum_next_doc},
    {"windows_refnum_last", (PyCFunction)py_windows_refnum_last, METH_NOARGS,
        py_windows_refnum_last_doc},
    {"window_find_level", (PyCFunction)py_window_find_level, METH_VARARGS | METH_KEYWORDS,
        py_window_find_level_doc},
    {"window_find_item", (PyCFunction)py_window_find_item, METH_VARARGS | METH_KEYWORDS,
        py_window_find_item_doc},
    {"window_find_closest", (PyCFunction)py_window_find_closest, METH_VARARGS | METH_KEYWORDS,
        py_window_find_closest_doc},
    {"window_item_find", (PyCFunction)py_window_item_find, METH_VARARGS | METH_KEYWORDS,
        py_window_item_find_doc},
    {"window_create", (PyCFunction)py_window_create, METH_VARARGS | METH_KEYWORDS,
        py_window_create_doc},
    {"server_find_tag", (PyCFunction)py_server_find_tag, METH_VARARGS | METH_KEYWORDS,
        py_server_find_tag_doc},
    {"server_find_chatnet", (PyCFunction)py_server_find_chatnet, METH_VARARGS | METH_KEYWORDS,
        py_server_find_chatnet_doc},
    {"command", (PyCFunction)PY_command, METH_VARARGS | METH_KEYWORDS,
        PY_command_doc},
    {"channels", (PyCFunction)py_channels, METH_NOARGS,
        py_channels_doc},
    {"channel_find", (PyCFunction)py_channel_find, METH_VARARGS | METH_KEYWORDS,
        py_channel_find_doc},
    {"query_find", (PyCFunction)py_query_find, METH_VARARGS | METH_KEYWORDS,
        py_query_find_doc},
    {"queries", (PyCFunction)py_queries, METH_NOARGS,
        py_queries_doc},
    {"mask_match", (PyCFunction)py_mask_match, METH_VARARGS | METH_KEYWORDS,
        py_mask_match_doc},
    {"mask_match_address", (PyCFunction)py_mask_match_address, METH_VARARGS | METH_KEYWORDS,
        py_mask_match_address_doc},
    {"masks_match", (PyCFunction)py_masks_match, METH_VARARGS | METH_KEYWORDS,
        py_masks_match_doc},
    {"rawlog_set_size", (PyCFunction)py_rawlog_set_size, METH_VARARGS | METH_KEYWORDS,
        py_rawlog_set_size_doc},
    {"logs", (PyCFunction)py_logs, METH_NOARGS,
        py_logs_doc},
    {"log_find", (PyCFunction)py_log_find, METH_VARARGS | METH_KEYWORDS,
        py_log_find_doc},
    {"ignores", (PyCFunction)py_ignores, METH_NOARGS,
        py_ignores_doc},
    {"ignore_check", (PyCFunction)py_ignore_check, METH_VARARGS | METH_KEYWORDS,
        py_ignore_check_doc},
    {"dccs", (PyCFunction)py_dccs, METH_NOARGS,
        py_dccs_doc},
    {"dcc_register_type", (PyCFunction)py_dcc_register_type, METH_VARARGS | METH_KEYWORDS,
        py_dcc_register_type_doc},
    {"dcc_unregister_type", (PyCFunction)py_dcc_unregister_type, METH_VARARGS | METH_KEYWORDS,
        py_dcc_unregister_type_doc},
    {"dcc_find_request_latest", (PyCFunction)py_dcc_find_request_latest, METH_VARARGS | METH_KEYWORDS,
        py_dcc_find_request_latest_doc},
    {"dcc_find_request", (PyCFunction)py_dcc_find_request, METH_VARARGS | METH_KEYWORDS,
        py_dcc_find_request_doc},
    {"dcc_chat_find_id", (PyCFunction)py_dcc_chat_find_id, METH_VARARGS | METH_KEYWORDS,
        py_dcc_chat_find_id_doc},
    {"dcc_str2type", (PyCFunction)py_dcc_str2type, METH_VARARGS | METH_KEYWORDS,
        py_dcc_str2type_doc},
    {"dcc_type2str", (PyCFunction)py_dcc_type2str, METH_VARARGS | METH_KEYWORDS,
        py_dcc_type2str_doc},
    {"dcc_get_download_path", (PyCFunction)py_dcc_get_download_path, METH_VARARGS | METH_KEYWORDS,
        py_dcc_get_download_path_doc},
    {"notifies", (PyCFunction)py_notifies, METH_NOARGS,
        py_notifies_doc},
    {"notifylist_add", (PyCFunction)py_notifylist_add, METH_VARARGS | METH_KEYWORDS,
        py_notifylist_add_doc},
    {"notifylist_remove", (PyCFunction)py_notifylist_remove, METH_VARARGS | METH_KEYWORDS,
        py_notifylist_remove_doc},
    {"notifylist_ison", (PyCFunction)py_notifylist_ison, METH_VARARGS | METH_KEYWORDS,
        py_notifylist_ison_doc},
    {"notifylist_find", (PyCFunction)py_notifylist_find, METH_VARARGS | METH_KEYWORDS,
        py_notifylist_find_doc},
    {"commands", (PyCFunction)py_commands, METH_NOARGS,
        py_commands_doc},
    {"level2bits", (PyCFunction)py_level2bits, METH_VARARGS | METH_KEYWORDS,
        py_level2bits_doc},
    {"bits2level", (PyCFunction)py_bits2level, METH_VARARGS | METH_KEYWORDS,
        py_bits2level_doc},
    {"combine_level", (PyCFunction)py_combine_level, METH_VARARGS | METH_KEYWORDS,
        py_combine_level_doc},
    {"signal_emit", (PyCFunction)py_signal_emit, METH_VARARGS,
        py_signal_emit_doc},
    {"signal_stop", (PyCFunction)py_signal_stop, METH_NOARGS,
        py_signal_stop_doc},
    {"signal_stop_by_name", (PyCFunction)py_signal_stop_by_name, METH_VARARGS | METH_KEYWORDS,
        py_signal_stop_by_name_doc},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

/*XXX: move to pyloader.c??*/
/* Traverse stack backwards to find the nearest _script object in globals */
static PyObject *find_script(void)
{
    PyFrameObject *frame;

    for (frame = PyEval_GetFrame(); frame != NULL; frame = frame->f_back)
    {
        g_return_val_if_fail(frame->f_globals != NULL, NULL);
        PyObject *script = PyDict_GetItemString(frame->f_globals, "_script");
        if (script)
        {
            /*
            PySys_WriteStdout("Found script at %s in %s, script -> 0x%x\n",
                    PyString_AS_STRING(frame->f_code->co_name), 
                    PyString_AS_STRING(frame->f_code->co_filename), script);
            */
            return script;
        }
    }

    return NULL;
}

int pymodule_init(void)
{
    g_return_val_if_fail(py_module == NULL, 0);

    py_module = Py_InitModule("_irssi", ModuleMethods);
    if (!py_module)
        return 0;
     
    return 1;
}

void pymodule_deinit(void)
{
    g_return_if_fail(py_module != NULL);

    Py_DECREF(py_module);
    py_module = NULL;
}

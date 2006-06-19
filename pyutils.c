#include <string.h>
#include "pyirssi.h"
#include "pyutils.h"
#include "settings.h"
#include "servers.h"

/* copy paste from perl bindings */
void py_command(const char *cmd, SERVER_REC *server, WI_ITEM_REC *item)
{
    const char *cmdchars;
    char *sendcmd = (char *) cmd;

    if (*cmd == '\0')
        return;

    cmdchars = settings_get_str("cmdchars");
    if (strchr(cmdchars, *cmd) == NULL) {
        /* no command char - let's put it there.. */
        sendcmd = g_strdup_printf("%c%s", *cmdchars, cmd);
    }

    signal_emit("send command", 3, sendcmd, server, item);
    if (sendcmd != cmd) g_free(sendcmd);
}

/* return the file extension for a file, or empty string
   don't free result */
char *file_get_ext(const char *file)
{
    const char *dot = NULL;

    while (*file)
    {
        if (*file == '.')
            dot = file;

        file++;
    }

    if (dot)
        return (char *) dot + 1;

    return (char *) file;
}

int file_has_ext(const char *file, const char *ext)
{
    const char *fext = file_get_ext(file);

    return !strcmp(fext, ext);
}


/* return whats in the braces -> /path/to/{filename}.py
   result must be freed */
char *file_get_filename(const char *path)
{
    const char *begin;
    const char *end;
    char *name;
    size_t len;

    begin = strrchr(path, '/');
    if (!begin)
        begin = path;
    else
        begin++;

    end = strrchr(begin, '.');
    if (end != NULL && end > begin)
        len = end - begin;
    else
        len = strlen(begin);

    name = g_strnfill(len, 0);

    strncpy(name, begin, len);

    return name;
}


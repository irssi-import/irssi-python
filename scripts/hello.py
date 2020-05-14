# type /pyload hello

import irssi

# data - contains the parameters for /HELLO
# server - the active server in window
# witem - the active window item (eg. channel, query)
#         or None if the window is empty
def cmd_hello(data, server, witem):
    if not server or not server.connected:
        irssi.prnt(b"Not connected to server")

    if data:
        server.command(b"MSG %s Hello!" % data)
    elif isinstance(witem, irssi.Channel) or isinstance(witem, irssi.Query):
        witem.command(b"MSG %s Hello!" % witem.name)
    else:
        irssi.prnt(b"Nick not given, and no active channel/query in window")

irssi.command_bind(b'hello', cmd_hello)

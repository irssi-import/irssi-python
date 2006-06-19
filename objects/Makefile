CC = gcc

PYTHON = /usr/include/python2.4
IRSSI = /usr/local/include/irssi
CFLAGS = -fpic -ggdb -Wall -I$(PYTHON) -I$(IRSSI) -I$(IRSSI)/src \
-I$(IRSSI)/src/fe-common/core -I$(IRSSI)/src/core -I$(IRSSI)/src/fe-text \
-I$(IRSSI)/src/irc -I$(IRSSI)/src/irc/core -I$(IRSSI)/src/irc/dcc -I.. \
`pkg-config glib-2.0 --cflags`

OBJ = pyscript-object.o base-objects.o window-item-object.o channel-object.o \
query-object.o server-object.o connect-object.o irc-server-object.o \
irc-connect-object.o irc-channel-object.o ban-object.o nick-object.o \
chatnet-object.o reconnect-object.o window-object.o textdest-object.o \
rawlog-object.o log-object.o logitem-object.o ignore-object.o \
dcc-object.o factory.o

pyobjects.a: $(OBJ)
	ar r pyobjects.a $(OBJ)

%.o: %.c
	$(CC) -c $< $(CFLAGS)

clean:
	rm -f *.o *.so *.a

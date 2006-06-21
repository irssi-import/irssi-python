CC = gcc

PYTHON = /usr/include/python2.4
IRSSI = /usr/local/include/irssi

CFLAGS = -fpic -ggdb -Wall -I$(PYTHON) -I$(IRSSI) -I$(IRSSI)/src \
-I$(IRSSI)/src/fe-common/core -I$(IRSSI)/src/core -I$(IRSSI)/src/fe-text \
-I$(IRSSI)/src/irc -I$(IRSSI)/src/irc/core -I$(IRSSI)/src/irc/dcc \
-I$(IRSSI)/src/irc/notifylist -I.. -I. -Iobjects \
`pkg-config glib-2.0 --cflags`

LDFLAGS = -fpic /usr/lib/libpython2.4.so

OBJ = pycore.o pyutils.o pymodule.o pyloader.o pysignals.o 

pyirssi: pyobjects.a $(OBJ)
	$(CC) -shared -o libirssi_python.so $(OBJ) objects/pyobjects.a $(LDFLAGS)

pyobjects.a: 
	cd objects/ && make

%.o: %.c
	$(CC) -c $< $(CFLAGS)

clean:
	rm -f *.o *.so
	cd objects/ && make clean


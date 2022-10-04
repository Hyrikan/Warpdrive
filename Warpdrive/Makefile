SERVER = warp_server.c
CLIENT = warp_client.c
TESTSERVER = test_warp_server.c
TESTCLIENT = test_warp_client.c

LIBS = -pthread -lucp -lucs -larrow-glib -lglib-2.0 -lgobject-2.0

FLAGS = -Wall

SEARCHPATH = -L/usr/lib/ucx/ -I/usr/include/glib-2.0/ -I/usr/lib/x86_64-linux-gnu/glib-2.0/include/ #-L/usr/lib/warpdrive/

OUTPUTDIR = warpdrive/
OBJSDIR = objs/

EXAMPLEOUT = ucp_example
SERVEROUT = warp_server
CLIENTOUT = warp_client
TESTSERVEROUT = test_warp_server
TESTCLIENTOUT = test_warp_client

COMPILER = gcc

all: staticlibs dynamiclibs testclient testserver

testserver: staticlibs
	$(COMPILER) $(FLAGS) $(TESTSERVER) -L$(OUTPUTDIR) -lwarp_server $(LIBS) $(SEARCHPATH) -o $(TESTSERVEROUT)

testclient: staticlibs
	$(COMPILER) $(FLAGS) $(TESTCLIENT) -L$(OUTPUTDIR) -lwarp_client $(LIBS) $(SEARCHPATH) -o $(TESTCLIENTOUT)

staticlibs: staticobjs
	ar -rcs $(OUTPUTDIR)lib$(CLIENTOUT).a $(OBJSDIR)static/$(CLIENTOUT).o
	ar -rcs $(OUTPUTDIR)lib$(SERVEROUT).a $(OBJSDIR)static/$(SERVEROUT).o

staticobjs:
	$(COMPILER) -c $(FLAGS) $(CLIENT) $(LIBS) -o $(OBJSDIR)static/$(CLIENTOUT).o $(SEARCHPATH)
	$(COMPILER) -c $(FLAGS) $(SERVER) $(LIBS) -o $(OBJSDIR)static/$(SERVEROUT).o $(SEARCHPATH)

dynamiclibs: dynamicobjs
	$(COMPILER) -shared $(OBJSDIR)shared/$(CLIENTOUT).o -o $(OUTPUTDIR)lib$(CLIENTOUT).so
	$(COMPILER) -shared $(OBJSDIR)shared/$(SERVEROUT).o -o $(OUTPUTDIR)lib$(SERVEROUT).so

dynamicobjs:
	$(COMPILER) -c $(FLAGS) -fPIC $(CLIENT) $(LIBS) -o $(OBJSDIR)shared/$(CLIENTOUT).o $(SEARCHPATH)
	$(COMPILER) -c $(FLAGS) -fPIC $(SERVER) $(LIBS) -o $(OBJSDIR)shared/$(SERVEROUT).o $(SEARCHPATH)

staticlibfromdynamicobjs: dynamicobjs
	ar -rcs $(OUTPUTDIR)lib$(CLIENTOUT).a $(OBJSDIR)shared/$(CLIENTOUT).o
	ar -rcs $(OUTPUTDIR)lib$(SERVEROUT).a $(OBJSDIR)shared/$(SERVEROUT).o

testrandomstuff:
	$(COMPILER) $(FLAGS) testrandomstuff.c $(LIBS) -o random $(SEARCHPATH)

testcompilewithsharedlib:
	$(COMPILER) $(FLAGS) $(TESTCLIENT) -lwarp_client -lucp -L/usr/lib/ucx -L./out/  -o $(OUTPUTDIR)sharedtest

testcompilewithstaticlib:
	$(COMPILER) $(FLAGS) -lwarp_client -L/usr/lib/warpdrive/ -I/usr/include/ $(TESTCLIENT) -o $(OUTPUTDIR)sharedtest

clean:
	rm $(OBJSDIR)static/*.o $(OBJSDIR)shared/*.o $(OUTPUTDIR)* random randomoutput.txt serveroutput.txt $(TESTCLIENTOUT) $(TESTSERVEROUT)
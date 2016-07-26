

bin_PROGRAMS=ipdupdetect

BUILT_SOURCES = MonitorManagerFunctions.i

MonitorManagerFunctions.i: MonitorManagerFunctions.gperf
	gperf -tCG MonitorManagerFunctions.gperf > MonitorManagerFunctions.i || rm -f MonitorManagerFunctions.i

ipdupdetect_CXXFLAGS=$(LIBLOGGER_CFLAGS) $(LIBCLIENTSERVER_CFLAGS)
ipdupdetect_LDFLAGS=$(LIBCLIENTSERVER_LIBS) $(LIBLOGGER_LIBS)
ipdupdetect_SOURCES=main.cpp \
			Interfaces.cpp \
			Monitor.cpp \
			MonitorManager.cpp \
			IPInfo.cpp



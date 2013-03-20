CXXFLAGS := -std=c++11 $(shell pkg-config --cflags xmms2-client-cpp xmms2-client-cpp-glib glib-2.0) $(CXXFLAGS)
LDFLAGS := $(LDFLAGS) $(shell pkg-config --libs xmms2-client-cpp xmms2-client-cpp-glib glib-2.0) -lespeak

xmms2-announce: xmms2-announce.cpp
	$(CXX) $(CXXFLAGS) xmms2-announce.cpp -o xmms2-announce $(LDFLAGS)

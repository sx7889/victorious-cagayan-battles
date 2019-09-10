CXXFLAGS=-O3 -fomit-frame-pointer -fPIC

ifeq ($(OS),Windows_NT)
CXX=x86_64-w64-mingw32-g++
AR=x86_64-w64-mingw32-ar
CXXFLAGS += -I../divide/include 
ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
	CXXFLAGS += -m64
else
ifeq ($(PROCESSOR_ARCHITECTURE),x86)
	CXXFLAGS += -m32
else
	CXXFLAGS += -m64
endif
endif
else
	CXXFLAGS += `sdl2-config --cflags`
endif

all: libcgame.a libsgame.a

CXXFLAGS += -I../divide/shared -I../divide/enet/include -I../divide/engine 

DEPS=game.h data.h

CLIENT_OBJS= \
	.obj/ai.o \
	.obj/client.o \
	.obj/entities.o \
	.obj/game.o \
	.obj/render.o \
	.obj/scoreboard.o \
	.obj/server.o \
	.obj/waypoint.o \
	.obj/weapon.o

SERVER_OBJS= \
	.obj/entities-standalone.o \
	.obj/server-standalone.o \

.obj/%.o: %.cpp $(DEPS) 
	@mkdir -p .obj	
	$(CXX) $(CXXFLAGS) -c -o $@ $<

.obj/%-standalone.o: %.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

libcgame.a: $(CLIENT_OBJS)
	$(AR) rcs $@ $(CLIENT_OBJS)

libsgame.a: CXXFLAGS += -DSTANDALONE
libsgame.a: $(SERVER_OBJS)
	$(AR) rcs $@ $(SERVER_OBJS)

clean:
	-$(RM) libcgame.a libsgame.a $(CLIENT_OBJS) $(SERVER_OBJS)
	 

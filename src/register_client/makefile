TARGET=libregister_client.so
SRC=$(wildcard x*.cpp)
#dir=$(notdir $(src))
OBJS=$(patsubst %.cpp,%.o,$(SRC) )
INCLUDE_PATH=/usr/include/xms
CPPFLAGS= -fPIC -I$(INCLUDE_PATH)
LIBS=-levent -lpthread -lxcom -lprotobuf -lssl
INSTALL_PATH=/usr/lib/
exist = $(shell if [ ! -d $(INCLUDE_PATH) ]; then mkdir $(INCLUDE_PATH); fi;)
$(TARGET):$(OBJS) 
	g++  -shared $+ -o $@ $(LIBS)
install:
	cp $(TARGET) $(INSTALL_PATH)
	@echo $(exist) "check include ..."
	cp *.h $(INCLUDE_PATH)
clean:
	rm -rf *.o  $(TARGET)



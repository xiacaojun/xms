TARGET=xauth
SRC=$(wildcard *.cpp)
OBJS=$(patsubst %.cpp,%.o,$(SRC) )
INCLUDE_PATH=/usr/include/xms
CPPFLAGS= -I$(INCLUDE_PATH)
INSTALL_PATH=/usr/bin/
INSTALL_LIB_PATH=/usr/lib/
LIBS=-levent -lpthread -lxcom -lprotobuf -lLXMysql -lmysqlclient -lssl -lregister_client -lconfig_client
LIB_TARGET=lib$(TARGET).so
exist = $(shell if [ ! -d $(INCLUDE_PATH) ]; then mkdir $(INCLUDE_PATH); fi;)

START_SHELL=start_$(TARGET)
STOP_SHELL=stop_$(TARGET)
WATCH=watch_$(TARGET)
all:$(TARGET) $(LIB_TARGET) $(WATCH)
	@echo "build all"
$(TARGET):$(OBJS)
	g++  $+ -o $@ $(LIBS)
$(LIB_TARGET):xauth_client.cpp
	g++ -fPIC -shared $+ -o $@ $(LIBS) $(CPPFLAGS)
$(WATCH):../xplatform/watchdog.cpp
	g++  $+ -o $@
install:$(WATCH)
	@echo "make start shell"
	# watch_register_server 3 register_server 20011
	echo "$(WATCH) 3 $(TARGET) \$$1 \$$2 \$$3 \$$4 \$$5" >$(START_SHELL)
	chmod +x $(START_SHELL)
	@echo "make stop shell"
	echo "killall $(WATCH)" > $(STOP_SHELL)
	echo "killall $(TARGET)" >> $(STOP_SHELL)
	chmod +x $(STOP_SHELL)
	cp $(STOP_SHELL) $(INSTALL_PATH)
	cp $(START_SHELL) $(INSTALL_PATH)
	cp $(TARGET) $(INSTALL_PATH)
	cp $(WATCH) $(INSTALL_PATH)
	@echo $(exist) "check include ..."
	cp *.h $(INCLUDE_PATH)
	cp $(LIB_TARGET) $(INSTALL_LIB_PATH)

uninstall:
	rm -rf *.o  $(TARGET) $(WATCH)  $(START_SHELL)  
	rm -rf *.o  $(TARGET) $(WATCH)  $(START_SHELL)  
	rm -rf $(INSTALL_PATH)$(TARGET)
	rm -rf $(INSTALL_PATH)$(WATCH)  
	rm -rf $(INSTALL_PATH)$(START_SHELL)  
	rm -rf $(INSTALL_LIB_PATH)$(LIB_TARGET)  
clean:
	rm -rf *.o  $(TARGET) $(WATCH) $(STOP_SHELL)  $(START_SHELL) $(LIB_TARGET)  



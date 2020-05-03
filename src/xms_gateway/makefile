TARGET=xms_gateway
SRC=$(wildcard *.cpp)
OBJS=$(patsubst %.cpp,%.o,$(SRC) ) 
CPPFLAGS= -g -I/usr/include/xms
LIBS=-levent -lpthread -lxcom -lprotobuf -lLXMysql -lmysqlclient -lssl -lconfig_client -lregister_client
INSTALL_PATH=/usr/bin/
WATCH=watch_$(TARGET)
START_SHELL=start_$(TARGET)
STOP_SHELL=stop_$(TARGET)

$(TARGET):$(OBJS) 
	g++  $+ -o $@ $(LIBS)

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
uninstall:
	rm -rf *.o  $(TARGET) $(WATCH)  $(START_SHELL)  
	rm -rf $(INSTALL_PATH)$(TARGET)
	rm -rf $(INSTALL_PATH)$(WATCH)  
	rm -rf $(INSTALL_PATH)$(START_SHELL)  
	rm -rf $(INSTALL_PATH)$(STOP_SHELL)  
clean:
	rm -rf *.o  $(TARGET) $(WATCH) $(STOP_SHELL)  $(START_SHELL)  





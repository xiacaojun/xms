TARGET=xms_add_user
SRC=$(wildcard *.cpp)
OBJS=$(patsubst %.cpp,%.o,$(SRC) ) 
CPPFLAGS= -I/usr/include/xms
LIBS=-levent -lpthread  -lprotobuf -lLXMysql -lmysqlclient -lxauth -lxcom
INSTALL_PATH=/usr/bin/
$(TARGET):$(OBJS)  
	g++  $+ -o $@ $(LIBS)
install:$(WATCH)
	@echo "make start shell"
	cp $(TARGET) $(INSTALL_PATH)
uninstall:
	rm -rf *.o  $(TARGET)
	rm -rf $(INSTALL_PATH)$(TARGET)
clean:
	rm -rf *.o  $(TARGET) 


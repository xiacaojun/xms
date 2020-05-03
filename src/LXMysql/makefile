SRC=$(wildcard *.cpp)
OBJS=$(patsubst %.cpp,%.o,$(SRC) )
TARGET=libLXMysql.so
CPPFLAGS= -fPIC  -I/usr/include/mysql
LIBS=-lpthread 
# apt-get install libmysqlclient-dev
INSTALL_PATH=/usr/lib/
exist = $(shell if [ ! -d $(INCLUDE_PATH) ]; then mkdir $(INCLUDE_PATH); fi;)

$(TARGET):$(OBJS)
	g++ -shared $+ -o $@ $(LIBS)
install:
	cp $(TARGET) $(INSTALL_PATH)
	@echo $(exist) "check include ..."
	cp *.h $(INCLUDE_PATH)
	@echo "install libLXMysql.so success!"
clean:
	rm -rf *.o $(TARGET)

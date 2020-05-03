protoc -I=./ --cpp_out=dllexport_decl=LIBPROTOBUF_EXPORTS:./  xmsg_com.proto
protoc -I=./ --cpp_out=dllexport_decl=LIBPROTOBUF_EXPORTS:./  xmsg_type.proto

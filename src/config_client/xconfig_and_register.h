#ifndef XCONFIG_AND_REGISTER_H
#define XCONFIG_AND_REGISTER_H

namespace google{namespace protobuf {
        class Message;
}}
class XConfigAndRegister
{
public:
    static bool Init(const char *service_name,
        const char *service_ip, int service_port,
        const char *register_ip, int register_port, google::protobuf::Message *conf_message );
};

#endif // !XCONFIG_AND_REGISTER_H
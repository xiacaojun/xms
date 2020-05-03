#pragma once

#include <QDialog>
#include "ui_config_edit.h"
namespace google
{
    namespace protobuf
    {
        class Message;
    }
}
namespace xmsg
{
    class XConfig;
}
class ConfigEdit : public QDialog
{
    Q_OBJECT

public:
    ConfigEdit(QWidget *parent = Q_NULLPTR);
    ~ConfigEdit();
    //bool LoadConfig(const char *ip,int port);
    bool LoadConfig(xmsg::XConfig *config);
    void LoadProto(const char *filename, const char *class_name);
    void InitGui(); 
signals:
    void AddLog(const char *log);
    //消息回调
    void MessageCBSig(bool is_ok, const char *msg);
public slots:
    void Save();
    ///选择proto文件，并加载动态编译
    void LoadProto();


    //消息回调
    void MessageCB(bool is_ok, const char *msg);
private:
    Ui::ConfigEdit ui;
    //基础配置信息，用于区分哪些是根据proto文件生成的
    int config_row_count_ = 0;
    //用于存储配置项
    google::protobuf::Message *message_ = 0;
    xmsg::XConfig *config_ = 0;

};

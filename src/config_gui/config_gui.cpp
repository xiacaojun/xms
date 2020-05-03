#include "config_gui.h"
#include <QMouseEvent>
#include <string>
#include <sstream>
#include "xtools.h"
#include "xconfig_client.h"
#include <QTime>
#include "config_edit.h"
#include <QMessageBox>
#include "xconfig_manager.h"
#include "xauth_client.h"
#include "xmsg_com.pb.h"
#include "login_gui..h"
using namespace xmsg;
using namespace std;
static ConfigGui *config_gui = 0;
static xmsg::XConfig config_tmp;
static mutex config_mutex;
static void  GetConfigResCB(xmsg::XConfig config)
{
    XMutex mux(&config_mutex);
    config_tmp.CopyFrom(config);
    config_gui->SEditConfCB(&config_tmp);
}
void  ConfigGui::EditConf()
{
    if (ui.tableWidget->rowCount() == 0)return;
    int row = ui.tableWidget->currentRow();
    if (row < 0)return;
    auto item_ip = ui.tableWidget->item(row, 1);
    auto item_port = ui.tableWidget->item(row, 2);
    if (!item_ip || !item_port)return;
    string ip = item_ip->text().toStdString();
    int port = atoi(item_port->text().toStdString().c_str());
    MCONF->LoadConfig(ip.c_str(), port);
}
void  ConfigGui::EditConfCB(void *config)
{
    XMutex mux(&config_mutex);
    ConfigEdit edit;
    edit.LoadConfig((XConfig*)config);
    //edit.LoadConfig(ip.c_str(), port);
    if (edit.exec() == QDialog::Accepted)
    {
        AddLog("新增配置成功");
    }
    Refresh();
}
ConfigGui::ConfigGui(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    config_gui = this;

    //去除原窗口边框
    setWindowFlags(Qt::FramelessWindowHint);

    //隐藏背景，用于圆角
    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);
    connect(this, SIGNAL(SEditConfCB(void*)), this, SLOT(EditConfCB(void*)));
    while (ui.tableWidget->rowCount() > 0)
        ui.tableWidget->removeRow(0);
    string server_ip = ui.server_ip_edit->text().toStdString();
    int server_port = ui.server_port_box->value();

    MCONF->set_server_ip(server_ip.c_str());
    MCONF->set_server_port(server_port);
    MCONF->set_auto_connect(true);

    MCONF->StartConnect();
    XAuthClient::Get()->set_server_ip(server_ip.c_str());
    XAuthClient::Get()->set_server_port(server_port);
    //设置自动重连
    XAuthClient::Get()->set_auto_connect(true);
    XAuthClient::Get()->StartConnect();

    //while (!XAuthClient::Get()->is_connected())
    //{
    //    cout << "-" << flush;
    //    //cout << XAuthClient::Get()->is_connecting() << endl; 
    //    //cout << XAuthClient::Get()->is_connected() << endl; 
    //    //cout << "." << flush;
    //    this_thread::sleep_for(100ms);
    //}
  /*  XAuthClient::Get()->LoginReq("root", "123456");
    this_thread::sleep_for(100ms);
    XLoginRes login;
    if (XAuthClient::Get()->GetLoginInfo("root", &login, 500))
    {
        cout << "login success!" << endl;
    }
    else
    {
        cout << "login failed!" << endl;
    }
    MCONF->set_login(login);*/
    MCONF->LoadConfigResCB = GetConfigResCB;
    Refresh();

}
//显示在日志列表中
void ConfigGui::AddLog(const char *log)
{
    //加入日期显示
    auto t = QTime::currentTime().toString("HH:mm:ss");
    QString str = t;
    str += " ";
    str += QString::fromLocal8Bit(log);
    LOGDEBUG(log);
    ui.log_list_Widget->insertItem(0, new QListWidgetItem(str));
}


//删除选中的配置
void ConfigGui::DelConf()
{
    if (ui.tableWidget->rowCount() == 0)return;
    int row = ui.tableWidget->currentRow();
    if (row < 0)return;
    auto item_name = ui.tableWidget->item(row, 0);
    auto item_ip = ui.tableWidget->item(row, 1);
    auto item_port = ui.tableWidget->item(row, 2);
    string name = item_name->text().toStdString();
    string ip = item_ip->text().toStdString();
    int port = atoi(item_port->text().toStdString().c_str());
    
    stringstream ss;
    ss << "您确认删除" << name << "|" << ip << ":" << port << " 微服务配置吗？";
    if (QMessageBox::information(0, "",
        QString::fromLocal8Bit(ss.str().c_str()),
        QMessageBox::Yes|QMessageBox::No
        ) == QMessageBox::No)
    {
        return;
    }
    MCONF->DeleteConfig(ip.c_str(), port);
    ss.clear();
    ss << "删除配置" << name << "|" << ip << ":" << port;
    AddLog(ss.str().c_str());
    //获取选中的配置name IP port 
    Refresh();
}
//新增配置
void ConfigGui::AddConf()
{
    //打开模态窗口，等待退出
    ConfigEdit edit;
    if (edit.exec() == QDialog::Accepted)
    {
        AddLog("新增配置成功");
    }
    Refresh();
}
bool ConfigGui::CheckLogin(std::string ip, int port)
{
    //验证登录是否有效，是否超时
    //超时前一分钟发送token延时命令
    static bool is_login = false;
    static std::string last_ip = "";
    static int last_port = 0;
    //要考虑更换服务器后的重新登录
    if (is_login && ip == last_ip&& last_port== port)
        return true;
    last_ip = ip;
    last_port = port;
    is_login = false;
    XAUTH->set_server_ip(ip.c_str());
    XAUTH->set_server_port(port);
    XAUTH->Close();
    if (!XAUTH->AutoConnect(1))
    {
        AddLog("验证服务连接失败");
        return false;
    }

    LoginGUI login;
    if (login.exec() == QDialog::Rejected)
    {
        return false;
    }
    is_login = true;
    return true;
}

//刷新显示配置列表
void ConfigGui::Refresh()
{
    while (ui.tableWidget->rowCount() > 0)
        ui.tableWidget->removeRow(0);
    //断开重连，如果修改配置中心的IP或者端口
    string server_ip = ui.server_ip_edit->text().toStdString();
    int server_port = ui.server_port_box->value();

    if (!CheckLogin(server_ip, server_port))
    {
        AddLog("验证登录失败");
        return;
    }
    //清理历史列表
    AddLog("清理历史列表");
    while (ui.tableWidget->rowCount() > 0)
        ui.tableWidget->removeRow(0);

    stringstream ss;
    ss << server_ip << ":" << server_port;
    LOGDEBUG(ss.str().c_str());
    
    //关闭之前的连接，重新建立连接
    MCONF->set_server_ip(server_ip.c_str());
    MCONF->set_server_port(server_port);
    MCONF->set_auto_delete(false);
    
    MCONF->Close();
    if (!MCONF->AutoConnect(1))
    {
    //LOGDEBUG("连接配置中心失败!");
        AddLog("连接配置中心失败");
        return;
    }
    //LOGDEBUG("连接配置中心成功!");
    AddLog("连接配置中心成功");



    //从配置中心获取配置列表
    auto confs = MCONF->GetAllConfig(1, 10000, 2);
    LOGDEBUG(confs.DebugString());

    //插入获取的列表
    ui.tableWidget->setRowCount(confs.config_size());
    for (int i = 0; i < confs.config_size(); i++)
    {
        auto conf = confs.config(i);
        ui.tableWidget->setItem(i, 0, new QTableWidgetItem(conf.service_name().c_str()));
        ui.tableWidget->setItem(i, 1, new QTableWidgetItem(conf.service_ip().c_str()));
        stringstream ss;
        ss << conf.service_port();
        ui.tableWidget->setItem(i, 2, new QTableWidgetItem(ss.str().c_str()));
    }
    AddLog("更新配置列表完成");
}

static bool mouse_press = false;
static QPoint mouse_point;
void ConfigGui::mouseMoveEvent(QMouseEvent *ev)
{
    //没有按下，处理原事件
    if (!mouse_press)
    {
        QWidget::mouseMoveEvent(ev);
        return;
    }
    auto cur_pos = ev->globalPos();
    this->move(cur_pos - mouse_point);
}
void ConfigGui::mousePressEvent(QMouseEvent *ev)
{
    //鼠标左键按下记录位置
    if (ev->button() == Qt::LeftButton)
    {
        mouse_press = true;
        mouse_point = ev->pos();
    }

}
void ConfigGui::mouseReleaseEvent(QMouseEvent *ev)
{
    mouse_press = false;
}
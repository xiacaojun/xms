#pragma once

#include <QWidget>
#include "ui_login_gui.h"
#include <string>
class LoginGUI : public QDialog
{
    Q_OBJECT

public:
    LoginGUI(QDialog *parent = Q_NULLPTR);
    ~LoginGUI();
    void set_server_ip(std::string ip) { server_ip_ = ip; }
    void set_server_port_(int port) { server_port_ = port; }

    std::string username() { return username_; }
    std::string password() { return password_; }
public slots:
    void Login();
private:
    std::string server_ip_ = "";
    int server_port_ = 0;
    std::string username_ = "";
    std::string password_ = "";
    Ui::LoginGUI ui;
};

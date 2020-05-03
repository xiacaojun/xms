#pragma once

#include <QtWidgets/QWidget>
#include "ui_config_gui.h"

class ConfigGui : public QWidget
{
    Q_OBJECT

public:
    ConfigGui(QWidget *parent = Q_NULLPTR);
    void mouseMoveEvent(QMouseEvent *ev);
    void mousePressEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);

signals:
    void SEditConfCB(void *config);
public slots:
    //刷新显示配置列表
    void Refresh();

    //新增配置
    void AddConf();

    //删除选中的配置
    void DelConf();

    //编辑配置
    void EditConf();

    void EditConfCB(void *config);

    //显示在日志列表中
    void AddLog(const char *log);
private:
    Ui::ConfigGuiClass ui;

    bool CheckLogin(std::string ip,int port);

};

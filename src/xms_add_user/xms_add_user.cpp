#include <iostream>
#include <string>
#include "xauth_client.h"
#include <thread>
using namespace std;
int main(int argc, char *argv[])
{
	string username = "";
	string rolename = "";
	string password = "";
	cout << "Username:";
	cin >> username;
	cout << "Rolename:";
	cin >> rolename;
	cout << "Password:";
	cin >> password;
	cout << username << "/" << password << endl;
	XAuthClient::RegMsgCallback();
	XAuthClient::Get()->set_server_ip("127.0.0.1");
	XAuthClient::Get()->set_server_port(AUTH_PORT);
	XAuthClient::Get()->StartConnect();
	while (!XAuthClient::Get()->is_connected())
	{
		this_thread::sleep_for(100ms);
	}
	xmsg::XAddUserReq adduser;
	adduser.set_username(username);
	adduser.set_password(password);
	adduser.set_rolename(rolename);
	XAuthClient::Get()->AddUserReq(&adduser);
	this_thread::sleep_for(500ms);
	return 0;
}

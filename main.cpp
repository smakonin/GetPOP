/*
    GetPOP - Get Pop Mail Uitility
    Copyright (C) 1999-2002 Stephen Makonin. All rights reserved.
*/

#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <vector>
#include <algorithm>
using namespace std;

#include <windows.h>
#include <winbase.h>
#include <winsock.h>
#include <process.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

string help("\nGet Pop Mail Uitility\nCopyright (C) 1999-2002 Stephen Makonin.\n\nUSAGE:  getpop.exe [mail server ip] [account name] [password] [directory]\n\n\tmail server ip - IP Address or URL of Mail Server\n\taccount name   - Mailbox Account User Name\n\tpassword       - Password of Account User\n\tdirectory      - (Optional) Directory to store messages in\n\n");
string eom = "\r\n.\r\n";
const long max_recv = 4096;
const long max_block_size = max_recv * 100;
const long max_timeout = 5;
const long max_filename_len = _MAX_PATH-5;

string long_to_str(long num)
{
	char buffer[20];
	_ltoa(num, buffer, 10);
	return buffer;
}

unsigned long get_ip_address(string address)
{
	char ip_addr[33];
	char *ptr;
	unsigned long ip;
	unsigned char a1;
	unsigned char a2;
	unsigned char a3;
	unsigned char a4;
	unsigned long count;

	strcpy(ip_addr, address.c_str());
	for(count = 0; count < strlen(ip_addr); count++)
		if(!isdigit(ip_addr[count]) && ip_addr[count] != '.')
			return 0;

	ptr = strtok(ip_addr, ".");
	a1 = (unsigned char)atoi(ptr);
	ptr = strtok(NULL, ".");
	a2 = (unsigned char)atoi(ptr);
	ptr = strtok(NULL, ".");
	a3 = (unsigned char)atoi(ptr);
	ptr = strtok(NULL, ".");
	a4 = (unsigned char)atoi(ptr);	

	ip = (unsigned long)(a4 << 24) + (unsigned long)(a3 << 16) + (unsigned long)(a2 << 8) + a1;

	return ip;
}

string http_error_text(int error_code)
{
	string error_text;

	switch(error_code)
	{
	case 100:
		error_text = "Not connected to server.";
		break;
	case 101:
		error_text = "Invalid connect/disconnect level.";
		break;
	case 102:
		error_text = "POP Account Name and/or Password invalid.";
		break;
	case 103:
		error_text = "Directory to save messages does not exist.";
		break;
	case 334:
		error_text = "Cannot create file. May be write protected or disk full.";
		break;

	case WSAEACCES: //(10013) 
		error_text = "Permission denied.";
		break;

	case WSAEADDRINUSE: //(10048) 
		error_text = "Address already in use.";
		break;

	case WSAEADDRNOTAVAIL: //(10049) 
		error_text = "Cannot assign requested address.";
		break;

	case WSAEAFNOSUPPORT: //(10047) 
		error_text = "Address family not supported by protocol family.";
		break;

	case WSAEALREADY: //(10037) 
		error_text = "Operation already in progress.";
		break;

	case WSAECONNABORTED: //(10053) 
		error_text = "Software caused connection abort."; 
		break;

	case WSAECONNREFUSED: //(10061) 
		error_text = "Connection refused.";
		break;

	case WSAECONNRESET: //(10054) 
		error_text = "Connection reset by peer."; 
		break;

	case WSAEDESTADDRREQ: //(10039) 
		error_text = "Destination address required.";
		break;

	case WSAEFAULT: //(10014) 
		error_text = "Bad address.";
		break;

	case WSAEHOSTDOWN: //(10064) 
		error_text = "Host is down.";
		break;

	case WSAEHOSTUNREACH: //(10065) 
		error_text = "No route to host.";
		break;

	case WSAEINPROGRESS: //(10036) 
		error_text = "Operation now in progress.";
		break;

	case WSAEINTR: //(10004) 
		error_text = "Interrupted function call.";
		break;

	case WSAEINVAL: //(10022) 
		error_text = "Invalid argument.";
		break;

	case WSAEISCONN: //(10056) 
		error_text = "Socket is already connected.";
		break;

	case WSAEMFILE: //(10024) 
		error_text = "Too many open files.";
		break;

	case WSAEMSGSIZE: //(10040) 
		error_text = "Message too long.";
		break;

	case WSAENETDOWN: //(10050) 
		error_text = "Network is down.";
		break;

	case WSAENETRESET: //(10052) 
		error_text = "Network dropped connection on reset.";
		break;

	case WSAENETUNREACH: //(10051) 
		error_text = "Network is unreachable.";
		break;

	case WSAENOBUFS: //(10055) 
		error_text = "No buffer space available.";
		break;

	case WSAENOPROTOOPT: //(10042) 
		error_text = "Bad protocol option.";
		break;

	case WSAENOTCONN: //(10057) 
		error_text = "Socket is not connected.";
		break;

	case WSAENOTSOCK: //(10038) 
		error_text = "Socket operation on non-socket."; 
		break;

	case WSAEOPNOTSUPP: //(10045) 
		error_text = "Operation not supported.";
		break;

	case WSAEPFNOSUPPORT: //(10046) 
		error_text = "Protocol family not supported.";
		break;

	case WSAEPROCLIM: //(10067) 
		error_text = "Too many processes.";
		break;

	case WSAEPROTONOSUPPORT: //(10043) 
		error_text = "Protocol not supported.";
		break;

	case WSAEPROTOTYPE: //(10041) 
		error_text = "Protocol wrong type for socket.";
		break;

	case WSAESHUTDOWN: //(10058) 
		error_text = "Cannot send after socket shutdown.";
		break;

	case WSAESOCKTNOSUPPORT: //(10044) 
		error_text = "Socket type not supported.";
		break;

	case WSAETIMEDOUT: //(10060) 
		error_text = "Connection timed out.";
		break;

	case WSAEWOULDBLOCK: //(10035) 
		error_text = "Resource temporarily unavailable.";
		break;

	case WSAHOST_NOT_FOUND: //(11001) 
		error_text = "Host not found.";
		break;

	case WSANOTINITIALISED: //(10093) 
		error_text = "Successful WSAStartup not yet performed.";
		break;

	case WSANO_DATA: //(11004) 
		error_text = "Valid name, no data record of requested type.";
		break;

	case WSANO_RECOVERY: //(11003) 
		error_text = "This is a non-recoverable error.";
		break;

	case WSASYSNOTREADY: //(10091) 
		error_text = "Network subsystem is unavailable.";
		break;

	case WSATRY_AGAIN: //(11002) 
		error_text = "Non-authoritative host not found.";
		break;

	case WSAVERNOTSUPPORTED: //(10092) 
		error_text = "WINSOCK.DLL version out of range.";
		break;

	case WSAEDISCON: //(10094) 
		error_text = "Graceful shutdown in progress.";
		break;

	default:
		error_text = "Unknown Error Number" + error_code;
		break;
	}

	return error_text;
}

string dir_exits(string dir)
{
	struct _stat buf;
	char full[_MAX_PATH];

	full[0] = 0;

	if(dir.substr(dir.length()-1, 1) == "/" || dir.substr(dir.length()-1, 1) == "\\")
		dir.erase(dir.length()-1, 1);

	if(_fullpath(full, dir.c_str(), _MAX_PATH) == NULL)
	{
		dir = "";
	}
	else
	{
		if(_stat(full, &buf) == -1)
			dir = "";
		else
			dir = full;
	}

	return dir;
}

int socket_connect(SOCKET *my_socket, string host, int port)
{
	static SOCKADDR_IN in_address;
	WORD ver_request = MAKEWORD(1,1);
	WSADATA ws_data;
    LPHOSTENT host_entry;
	unsigned long addr;
	int return_code;
	int error_code;

	error_code = 0;
	return_code = WSAStartup(ver_request, &ws_data);
	if(ws_data.wVersion != ver_request)
	{
		error_code = WSAVERNOTSUPPORTED;
	}
	else
	{
		addr = get_ip_address(host);
		if(!addr)
		{
			host_entry = gethostbyname(host.c_str());
			if(!host_entry)
			{
				error_code = WSAGetLastError();
			}
			else
			{
				if(!addr)
				{
					in_address.sin_family = AF_INET;
					in_address.sin_addr = *((LPIN_ADDR)*host_entry->h_addr_list);
					in_address.sin_port = htons((u_short)port);
				}
				else
				{
					in_address.sin_family = AF_INET;
					in_address.sin_addr = *((LPIN_ADDR)&addr);
					in_address.sin_port = htons((u_short)port);
				}
			}
		}
	}

	*my_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(*my_socket == INVALID_SOCKET)
	{
		error_code = WSAGetLastError();
	}
	else
	{
		return_code = connect(*my_socket, (LPSOCKADDR)&in_address, sizeof(struct sockaddr));
		if(return_code == SOCKET_ERROR)
			error_code = WSAGetLastError();
	}

	return error_code;
}

int socket_disconnect(SOCKET my_socket)
{
	int error_code;

	error_code = 0;

	shutdown(my_socket, 0x02);
	closesocket(my_socket);

	WSACleanup();

	return error_code;
}

int socket_send(SOCKET my_socket, string data)
{
	int return_code;
	int error_code;

	error_code = 100;
	if(my_socket)
	{
		error_code = 0;

		return_code = send(my_socket, data.c_str(), (int)data.length(), 0);
		if(return_code == SOCKET_ERROR)
			error_code = WSAGetLastError();
	}

	return error_code;
}

int socket_receive(SOCKET my_socket, string *data, string end_mark, int length, bool clear_data=true)
{
	string current_end;
	char bytes[max_recv + 1];
	int len, i, total_downloaded;
	int error_code;
	vector<char> raw_data;

	FD_SET fd = {1, my_socket};
	TIMEVAL tv = {max_timeout, 0};

	if(clear_data)
		data->clear();

	error_code = 0;
	total_downloaded = 0;
	len = 1;
	current_end.append(end_mark.size(), ' ');
	while(true) 
	{
		memset(bytes, 0, max_recv + 1);

		if(!total_downloaded && length > 0)
		{
			if(max_recv > length)
				len = length;
			else
				len = max_recv;
		}
		else if(length == -1)
		{
			len = 1;
		}
		else
		{
			len = length - total_downloaded;
			if(len > max_recv)
				len = max_recv;
		}

		if (len > 0) 
			if (!select(0, &fd, NULL, NULL, &tv))
				return error_code;

		len = recv(my_socket, bytes, len, 0);
		if(len == SOCKET_ERROR)
		{
			error_code = WSAGetLastError();
			break;
		}
		
		total_downloaded += len;

		if(len == 1)
		{
			raw_data.push_back(bytes[0]);

			current_end += bytes[0];
			current_end.erase(0, 1);				
		}
		else if(len > 0)
		{
			for(i = 0; i < len; i++)
				raw_data.push_back(bytes[i]);
		}

		if(end_mark == current_end && current_end != "")
			break;

		if((total_downloaded >= length && length > 0) || len == 0)
			break;
	}

	for(i = 0; i < (int)raw_data.size(); i++)
		*data += (char)raw_data[i];

	raw_data.clear();

	return error_code;
}

void replace_substr(string *data, string old_str, string new_str)
{
	string var_code;
	long pos;

	while((pos = (long)data->find(old_str, 0)) != string::npos)
	{
		data->erase(pos, old_str.length());
		data->insert(pos, new_str);
	}
}

void save_message(string filename, string data)
{
	replace_substr(&data, "\r\n", "\n");
	ofstream out(filename.c_str());
	out << data;
	out.close();
}

string get_date(int type)
{
	string buffer;
	time_t ltime;
	tm *today;
	char tmpbuf[129];

	time(&ltime);
    today = localtime(&ltime);
    switch(type)
	{
	case 1:
		strftime(tmpbuf, 128, "%Y%m%d", today);
		break;
	case 2:
		strftime(tmpbuf, 128, "%y%m%d", today);
		break;
	case 3:
		strftime(tmpbuf, 128, "%Y%j%H%M%S", today);
		break;
	case 4:
		strftime(tmpbuf, 128, "%Y %m %d", today);
		break;
	}
	buffer = tmpbuf;
	return buffer;
}

string create_run_id()
{
	return get_date(3) + long_to_str(_getpid());
}

int main(int argc, char *argv[])
{
	SOCKET my_socket;
	int error_code, msg_count;
	string pop_server, account_name, account_pssswd, storage_dir;
	string cmd, data, status, filename, id;

	//check params and set defaults
	if(argc < 4)
	{
		cout << help.c_str();
		return 0;
	}
	id = create_run_id();
	pop_server = argv[1];
	account_name = argv[2];
	account_pssswd = argv[3];
	if(argc == 5)
		storage_dir = argv[4];
	else
		storage_dir = "./";

	// check if dir exists
	storage_dir = dir_exits(storage_dir);
	if(storage_dir == "")
	{
		error_code = 103;
		cout << "ERROR: (" << error_code << ") " << http_error_text(error_code).c_str() << endl;
		return -1;
	}
	
	//connect to pop
	socket_connect(&my_socket, pop_server, 110);
	socket_receive(my_socket, &status, "\r\n", -1);

	//login with account name and password
	cmd = "USER " + account_name + "\r\n";
	socket_send(my_socket, cmd);
	socket_receive(my_socket, &status, "\r\n", -1);

	cmd = "PASS " + account_pssswd + "\r\n";
	socket_send(my_socket, cmd);
	status = "";
	while(status == "")
		socket_receive(my_socket, &status, "\r\n", -1);

	if(status.substr(0, 4) == "-ERR")
	{
		error_code = 102;
		cout << "ERROR: (" << error_code << ") " << http_error_text(error_code).c_str() << endl;
		return -1;
	}

	msg_count = 1;
	while(true)
	{
		//get the next massage
		cmd = "RETR " + long_to_str(msg_count) + "\r\n";
		socket_send(my_socket, cmd);
	
		status = "";
		while(status == "")
			socket_receive(my_socket, &status, "\r\n", -1);

		//if -ERR status = no more messages
		if(status.substr(0, 4) == "-ERR")
			break;

		socket_receive(my_socket, &data, eom, -1);
		data.erase(data.find(eom), eom.length());

		filename = "message-" + id + "-" + long_to_str(msg_count) + ".txt";
		save_message(storage_dir + "/" + filename, data);

		cmd = "DELE " + long_to_str(msg_count) + "\r\n";
		socket_send(my_socket, cmd);
		socket_receive(my_socket, &status, "\r\n", -1);

		msg_count++;
	}

	//quit and disconnect
	cout << long_to_str(msg_count-1) << " message(s) downloaded." << endl;
	cmd = "QUIT\r\n";
	socket_send(my_socket, cmd);
	socket_receive(my_socket, &status, "\r\n", -1);
	socket_disconnect(my_socket);
}

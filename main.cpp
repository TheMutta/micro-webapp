#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcgiapp.h>

#include <iostream>
#include <sstream>
#include <map>
#include <string>

std::map<std::string, std::size_t> users;
std::map<std::string, bool> loggedIn;

void create_login_page(FCGX_Request *request, std::string method) {
	std::string title = "Login";
	std::string status = "200 OK";
	std::string content_type = "text/html";

	std::string form;
	if (method == "GET") {
		form = "<form method='POST' action=''>"
			" <input id='username' name='username' type='text'></input>"
			" <input id='password' name='password' type='password'></input>"
			" <input type='submit' value='Submit'>"
			"</form>";
	} else if (method == "POST") {
		int content_length =
			atoi(FCGX_GetParam(
						"CONTENT_LENGTH",
						request->envp
					  ));

		char *content_buffer = new char[content_length + 1];

		FCGX_GetStr(content_buffer, content_length, request->in);
		content_buffer[content_length]='\0';

		printf("%s\n", content_buffer);

		std::string username, password;

		// Use istringstream to read the query string
		std::istringstream iss(content_buffer);

		// Extract parameters until the '&' character or the end of the stream is reached
		std::getline(iss, username, '&');
		std::getline(iss, password, '&');

		// Further split username and password into key-value pairs using '=' as delimiter
		size_t equalPos;
		if ((equalPos = username.find('=')) != std::string::npos) {
			username.erase(0, equalPos + 1);
		}
		if ((equalPos = password.find('=')) != std::string::npos) {
			password.erase(0, equalPos + 1);
		}

		// Now you have separate strings for username and password
		std::cout << "Username: " << username << std::endl;
		std::cout << "Password: " << password << std::endl;

		std::size_t hashedPassword = std::hash<std::string>{}(password);

		if (users.find(username) != users.end() &&
				users[username] == hashedPassword) {
			loggedIn[username] = true;
			form = "Hello, " + username + " Secret data here";
		} else {
			status = "401 Unauthorized";
			form = "Wrong credentials";
		}



		delete[] content_buffer;
	}


	FCGX_FPrintF(
			request->out,
			"Status: %s\r\n"
			"Content-type: %s\r\n\r\n"
			"<h1>%s</h1>"
			"<div>%s</div>",
			status.c_str(),
			content_type.c_str(),
			title.c_str(),
			form.c_str()
		    );

}

void return_not_found(FCGX_Request *request, std::string path, std::string method) {
	std::string title = "404";
	std::string status = "404 Not found";
	std::string content_type = "text/html";

	std::string form;
	if (method == "GET") {
		form = "<p>The requested page at " + path + " has not been found</p>";
	}

	FCGX_FPrintF(
			request->out,
			"Status: %s\r\n"
			"Content-type: %s\r\n\r\n"
			"<h1>%s</h1>"
			"<div>%s</div>",
			status.c_str(),
			content_type.c_str(),
			title.c_str(),
			form.c_str()
		    );
}

int main() {
	FCGX_Init();

	int socket = FCGX_OpenSocket("127.0.0.1:2016", 1024);
	FCGX_Request request;
	FCGX_InitRequest(&request, socket, 0);

	users["jessica"] = std::hash<std::string>{}("password");

	while(FCGX_Accept_r(&request) >= 0) {
		printf("Accept\n");

		std::string path =
			FCGX_GetParam(
				"SCRIPT_NAME",
				request.envp
			);

		std::string method =
			FCGX_GetParam(
				"REQUEST_METHOD",
				request.envp
			);

		if (path == "/login") {
			create_login_page(&request, method);
		} else {
			return_not_found(&request, path, method);
		}

		FCGX_Finish_r(&request);
	}

	return 0;
}

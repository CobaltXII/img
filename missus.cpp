/*

for CobaltXII's personal usage
hot images via telnet

*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <ctime>
#include <random>
#include <vector>
#include <string>
#include <cfloat>
#include <sstream>
#include <fstream>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "../stb/stb_image.h"

#include "xterm_256.h"
#include "make_image.hpp"
#include "make_image256.hpp"
#include "make_image8.hpp"

#define PORT 6969

// The list of images.
std::vector<std::string> pics;

// Get all the images in a directory.
void get_pics_in(std::string path) {
	DIR* d;
	struct dirent* dir;
	d = opendir(path.c_str());
	if (d) {
		while ((dir = readdir(d)) != NULL) {
			if (strcmp(strrchr(dir->d_name, '.'), ".txt")) {
				pics.push_back(path + dir->d_name);
			}
		}
		closedir(d);
	}
}

// Get all the images.
void get_pics() {
	std::vector<std::string> usernames;
	std::ifstream users("../rarchive/users.txt");
	std::string user;
	while (std::getline(users, user)) {
		usernames.push_back(user);
	}
	users.close();
	for (int i = 0; i < usernames.size(); i++) {
		get_pics_in("../rarchive/rarchive/" + usernames[i] + "/");
	}
}

// Connection handler.
void* connection_handler(void* vnew_socket) {
	int new_socket = *(int*)vnew_socket;

	char buffer[1024] = {0};

	// Send a greeting.
	const char* msg1 = "Welcome! Type 24 for true-color images (only certain terminals  \n"
					   "support this), type 256 for 256-color images (many terminals    \n"
					   "support this), or type 8 for 8-color images (almost every       \n"
					   "terminal supports this). Once you choose a mode, you will be    \n"
					   "presented with an image (very NSFW!), after which you can type  \n"
					   "anything besides 'quit' to keep getting more. Enjoy!            \n";
	send(new_socket, msg1, strlen(msg1), 0);

	// Read the next command.
	if (read(new_socket, buffer, 1024) < 0) {
		close(new_socket);
		return NULL;
	}

	// Parse the next command.
	int mode;
	if (buffer[0] == '2') {
		if (buffer[1] == '4') {
			mode = 24;
		} else if (buffer[1] == '5') {
			mode = 256;
		} else {
			close(new_socket);
			return NULL;
		}
	} else if (buffer[0] == '8') {
		mode = 8;
	} else {
		close(new_socket);
		return NULL;
	}

	while (1) {
		// Pick a random image for the client.
		std::string img = pics[rand() % pics.size()];

		// Generate an image for the client.
		char* image;
		if (mode == 24) {
			image = make_image(img);
		} else if (mode == 256) {
			image = make_image256(img);
		} else {
			image = make_image8(img);
		}

		// Send the image to the client.
		if (send(new_socket, image, strlen(image), 0) < 0) {
			break;
		}
		std::cout << "sent an image!" << std::endl;

		// Read the next command.
		if (read(new_socket, buffer, 1024) < 0) {
			break;
		}

		// Parse the next command.
		if (buffer[0] == 'q' &&
			buffer[1] == 'u' &&
			buffer[2] == 'i' &&
			buffer[3] == 't')
		{
			// Close the connection to the client.
			close(new_socket);
			return NULL;
		}
	}

	return NULL;
}

// Entry point.
int main() {
	for (int i = 0; i < 256; i++) {
		std::cout << "\u001b[48;5;" << i << "m ";
	}
	std::cout << "\033[0m\n";

	signal(SIGPIPE, SIG_IGN);
	srand(time(NULL));

	get_pics();
	std::cout << pics.size() << " pics!" << std::endl;

	int server_fd, new_socket;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);

	if (!(server_fd = socket(AF_INET, SOCK_STREAM, 0))) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
		perror("bind");
		exit(EXIT_FAILURE);
	}

	if (listen(server_fd, 3) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}

	while (1) {
		if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
			perror("accept");
			exit(EXIT_FAILURE);
		}

		// Log the client's IP address.
		std::cout << "new client: " << inet_ntoa(address.sin_addr) << std::endl;

		// Create a new thread to handle the connection.
		pthread_t thread_id;
		if (pthread_create(&thread_id, NULL, connection_handler, (void*)&new_socket) < 0) {
			perror("pthread_create");
		}
	}
}
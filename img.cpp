/*

command line images

*/

#include <string>
#include <cfloat>
#include <sstream>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "../stb/stb_image.h"

#include "xterm_256.h"
#include "make_image.hpp"
#include "make_image256.hpp"
#include "make_image8.hpp"

int main(int argc, char** argv) {
	// Parse command line arguments.
	if (argc != 2 && argc != 3) {
		std::cerr << "usage: img <image> [-256 -8]" << std::endl;
		std::cerr << "\t-256\t256-color mode" << std::endl;
		std::cerr << "\t-8\t8-color mode" << std::endl;
		return 1;
	} else if (argc == 3) {
		if (std::string(argv[2]) == "-256") {
			// Print a 256-color image.
			std::cout << make_image256(argv[1]) << std::endl;
			return 0;
		} else if (std::string(argv[2]) == "-8") {
			// Print an 8-color image.
			std::cout << make_image8(argv[1]) << std::endl;
			return 0;
		} else {
			std::cerr << "usage: img <image> [-256 -8]" << std::endl;
			std::cerr << "\t-256\t256-color mode" << std::endl;
			std::cerr << "\t-8\t8-color mode" << std::endl;
			return 1;
		}
	}
	// Print a true-color image.
	std::cout << make_image(argv[1]) << std::endl;

	// Exit successfully.
	return 0;
}
char* make_image(std::string path) {
	// Load the image.
	int xres;
	int yres;
	unsigned char* img = stbi_load(path.c_str(), &xres, &yres, NULL, 3);

	// Check if the image failed to load.
	if (!img) {
		std::cerr << "could not load image \'" << path << "\'" << std::endl;
		exit(2);
	}

	// Create a new image.
	int scale = xres / 128;
	int new_xres = xres / scale;
	int new_yres = yres / scale;
	unsigned char* new_img = new unsigned char[new_xres * new_yres * 3];

	// Check if the new image failed to allocate.
	if (!new_img) {
		std::cerr << "could not allocate image" << std::endl;
		exit(3);
	}

	// Scale the image.
	int scale2 = scale * scale;
	for (int i = 0; i < new_xres; i++) {
		for (int j = 0; j < new_yres; j++) {
			int x = i * scale;
			int y = j * scale;
			unsigned long long r = 0;
			unsigned long long g = 0;
			unsigned long long b = 0;
			for (int u = 0; u < scale; u++) {
				for (int v = 0; v < scale; v++) {
					int m = x + u;
					int n = y + v;
					unsigned char* offset = img + (n * xres + m) * 3;
					r += offset[0];
					g += offset[1];
					b += offset[2];
				}
			}
			unsigned char* offset = new_img + (j * new_xres + i) * 3;
			offset[0] = r / scale2;
			offset[1] = g / scale2;
			offset[2] = b / scale2;
		}
	}

	// Print the image to an std::stringstream.
	std::stringstream buffer;
	for (int y = 0; y < new_yres; y += 2) {
		for (int x = 0; x < new_xres; x++) {
			// Get the background color.
			unsigned char* bg_offset = new_img + (y * new_xres + x) * 3;
			int bg_r = bg_offset[0];
			int bg_g = bg_offset[1];
			int bg_b = bg_offset[2];
			// Get the foreground color.
			unsigned char* fg_offset = new_img + ((y + 1) * new_xres + x) * 3;
			int fg_r = fg_offset[0];
			int fg_g = fg_offset[1];
			int fg_b = fg_offset[2];
			// Print the ANSI escape codes.
			buffer << "\033[38;2;" << fg_r << ";" << fg_g << ";" << fg_b << "m";
			buffer << "\033[48;2;" << bg_r << ";" << bg_g << ";" << bg_b << "m";
			// Print the lower half block Unicode character.
			buffer << "\u2584";
		}
		buffer << "\033[0m\n";
	}
	buffer << '\n';

	// Return the buffer as a C string.
	return (char*)buffer.str().c_str();
}
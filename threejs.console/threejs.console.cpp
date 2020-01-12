#include "stdafx.h"

#include "threejs.core.h"

#include <iostream>

int main(int argc, char **argv)
{
	std::cout << "You have entered " << argc
		<< " arguments:" << "\n";

	for (int i = 0; i < argc; ++i)
		std::cout << argv[i] << "\n";

	if (argc != 2) {
		std::cout << "USAGE: threejs.Console.exe <input>\n";
		return 1;
	}

	auto input_filename = argv[1];
	auto bufferGeomRef = ParseThreeBufferGeometry(input_filename);

	return 0;
}


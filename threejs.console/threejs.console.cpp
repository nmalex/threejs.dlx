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

	SerializeThreeBufferGeometry("CD0F346A-CCF4-4169-AB22-A255BC6181E2", "C:\\Temp\\1.json", bufferGeomRef);

	return 0;
}


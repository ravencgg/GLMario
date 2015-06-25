#include "helper.h"


char* load_text_file(char* filename)
{
	char* result = nullptr;

	struct stat st;

	if (stat(filename, &st) == 0)
	{
		result = new char[st.st_size + 1]; // NOTE(chris): This does not clear to zero, and the + 1 is for the null terminator at the end of it. 
		result[st.st_size] = '\0'; // This null terminator

		std::ifstream input(filename, std::ios::binary);

		if (input.is_open())
		{
			input.read(result, st.st_size);
			input.close();
		}
	}

	return result;
}

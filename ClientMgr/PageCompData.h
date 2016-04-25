#pragma once

#include "glm/glm.hpp"

#include <vector>
#include <string>

struct PCDConsole {
	static int const size_max = 256;
	int size_text;
	int num_showing;
	int index = 0;
	int size;
	bool is_dirty;

	std::vector< std::string > list_strings;
	std::vector< glm::ivec2 > list_pos;
};

struct PCDCommand { 
	static int const size_max = 16;
	int size_text;
	int index = 0;
	int index_history = 0;
	bool is_input;
	bool is_shift;

	glm::ivec2 vect_pos;

	std::string str_command;
	std::vector< std::string > list_strings;
};

struct PCDStatic {
	static int const size_max = 64;
	int size_text;
	int num_showing;
	int index = 0;
	int size;
	bool is_dirty;

	std::vector< std::string > list_strings;
	std::vector< glm::ivec2 > list_pos;
};

struct PCDEditable { 
	bool is_editable;
};

struct PCDTextField {
	glm::ivec2 * ptr_vec_pos;
	glm::ivec2 vec_offset;
	int size_text;
	std::string * ptr_str;
};
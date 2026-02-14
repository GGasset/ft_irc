#ifndef FILE_HPP
#define FILE_HPP

#include "Server.hpp"
#include <fstream>
#include <cstring>

#define CHUNK_SIZE 512  // Tamaño de cada chunk transmitido

class File
{
	public:
		std::string	name;
		std::string path;
		std::string sender;
		std::string target;
		unsigned long file_size;
		unsigned long bytes_sent;
		std::ifstream file_stream;

	public:
		File();
		File(std::string _name, std::string _path, std::string _sender, std::string _target);
		File(const File &f);
		~File();
		File & operator = (const File &f);
		
		bool open_file();
		bool close_file();
		unsigned long get_file_size();
		std::string read_chunk();
		bool is_complete() const;
		void reset_transfer();
};

#endif
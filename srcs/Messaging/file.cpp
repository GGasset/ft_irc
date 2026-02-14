#include "File.hpp"
#include <sys/stat.h>

File::File(): file_size(0), bytes_sent(0) {}

File::File(std::string _name, std::string _path, std::string _sender, std::string _target): 
	name(_name), path(_path), sender(_sender), target(_target), file_size(0), bytes_sent(0) {};

File::File(const File &x) { *this = x; };

File::~File() { 
	close_file();
};

File & File::operator=(const File &f)
{
	if (this == &f)
		return (*this);
	this->name = f.name;
	this->path = f.path;
	this->sender = f.sender;
	this->target = f.target;
	this->file_size = f.file_size;
	this->bytes_sent = f.bytes_sent;
	return (*this);
};

// Obtiene el tamaño del archivo
unsigned long File::get_file_size()
{
	struct stat stat_buf;
	if (stat(path.c_str(), &stat_buf) == 0)
		return stat_buf.st_size;
	return 0;
}

// Abre el archivo para lectura
bool File::open_file()
{
	file_stream.open(path.c_str(), std::ios::binary);
	if (!file_stream.is_open())
		return false;
	file_size = get_file_size();
	bytes_sent = 0;
	return true;
}

// Cierra el archivo
bool File::close_file()
{
	if (file_stream.is_open())
	{
		file_stream.close();
		return true;
	}
	return false;
}

// Lee un chunk del archivo
std::string File::read_chunk()
{
	if (!file_stream.is_open())
		return "";
	
	char buffer[CHUNK_SIZE];
	file_stream.read(buffer, CHUNK_SIZE);
	std::streamsize bytes_read = file_stream.gcount();
	
	if (bytes_read > 0)
	{
		bytes_sent += bytes_read;
		return std::string(buffer, bytes_read);
	}
	return "";
}

// Verifica si la transferencia está completa
bool File::is_complete() const
{
	return bytes_sent >= file_size && file_size > 0;
}

// Reinicia la transferencia
void File::reset_transfer()
{
	bytes_sent = 0;
	if (file_stream.is_open())
		file_stream.seekg(0);
}
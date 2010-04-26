/* UTIL */

#include "utilio.h"

long fsize(const char *path)
{
	AutoFile file(path, "rb");

	fseek(file.get(), 0, SEEK_END);
	
	return ftell(file.get());
	// AutoFile closes file
}

AutoFile::AutoFile(const char * filename, const char * mode)
:	_file(fopen(filename, mode))
{
	if (!_file)
		throw io_error("Could not open file.");
}

void AutoFile::close()
{
	if (_file)
		fclose(_file);
}

AutoFile::~AutoFile()
{
	close();
}

AutoFile::AutoFile(const AutoFile &)
{
	// never called
}

io_error::io_error(const std::string& what)
:	runtime_error(what)
{
}

bad_data_error::bad_data_error(const std::string& what)
:	runtime_error("There was a problem with the scenario data: " + what)
{
}

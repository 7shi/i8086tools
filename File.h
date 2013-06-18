#pragma once
#include <string>

struct FileBase {
	int fd;
	int count;
	std::string path;

	FileBase(int fd, const std::string &path);
	virtual ~FileBase();

	virtual int read(void *buf, int len) = 0;
	virtual int write(void *buf, int len) = 0;
};

struct File: public FileBase {
	File(int fd, const std::string &path);
	File(const std::string &path, int flag, int mode);
	virtual ~File();

	virtual int read(void *buf, int len);
	virtual int write(void *buf, int len);
};

#pragma once
#include <stdint.h>
#include <string>

extern std::string rootpath;

extern std::string regs8[];
extern std::string regs16[];
extern std::string sregs[];

inline uint16_t read16(uint8_t *mem) {
	return mem[0] | (mem[1] << 8);
}

inline uint32_t read32(uint8_t *mem) {
	return mem[0] | (mem[1] << 8) | (mem[2] << 16) | (mem[3] << 24);
}

extern std::string hex(int v, int len = 0);
extern std::string hexdump(uint8_t *mem, int len);
void setroot(std::string root);
std::string convpath(const std::string &path);
bool startsWith(const std::string &s, const std::string &prefix);
bool endsWith(const std::string &s, const std::string &suffix);
std::string replace(const std::string &src, const std::string &s1, const std::string &s2);

#ifdef WIN32
extern std::string getErrorMessage(int err);
#endif

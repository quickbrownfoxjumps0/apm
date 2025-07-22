#include "utils.h"
#include <string.h>

bool is_ascii(const char *str)
{
	if (!str)
		return false;
	while (*str) {
		if ((unsigned char) *str > 127)
			return false;
		str++;
	}
	return true;
}

const char *get_filename_from_path(const char *path)
{
	const char *slash = strrchr(path, '\\');
	if (slash)
		return slash + 1;
	slash = strrchr(path, '/');
	if (slash)
		return slash + 1;
	return path;
}


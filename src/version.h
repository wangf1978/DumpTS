#ifndef __VERSION_H__
#define __VERSION_H__

#define VER_STR(value)					#value
#define VER_STRINGIZE(value)			VER_STR(value)

#define MAJOR_VERSION		0
#define MINOR_VERION		3
#define BUILD_VERSION		1
#define REVISION_NUMBER		"9352ebe9f96c593f972b0c4647483f556ff0ccaa"

#define APP_VERSION					\
	VER_STRINGIZE(MAJOR_VERSION) "." \
	VER_STRINGIZE(MINOR_VERION) "." \
	VER_STRINGIZE(MAJOR_VERSION) "." REVISION_NUMBER

#endif

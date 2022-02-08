#ifndef __VERSION_H__
#define __VERSION_H__

#define VER_STR(value)					#value
#define VER_STRINGIZE(value)			VER_STR(value)

#define MAJOR_VERSION		0
#define MINOR_VERION		3
#define REVISION_NUMBER		"82f2057174af621a67bef463431c716cbc0ee56b"

#define APP_VERSION					\
	VER_STRINGIZE(MAJOR_VERSION) "." \
	VER_STRINGIZE(MINOR_VERION) "." REVISION_NUMBER

#endif

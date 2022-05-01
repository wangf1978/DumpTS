/*
	the different compiler may have different implementation for C/C++ standard, some allow this one, some do NOT allow it.
	For example, strcpy, it is allowed in default gcc/g++ compiler, but in MSVC it is not allowed at default
	this header file will try to provide a unified MACRO definition for the different compiler or platform based on the standard C/C++ interface
*/
#ifndef _PLATFORM_DEF_H_
#define _PLATFORM_DEF_H_

#ifdef _WIN32
#include <io.h>
#include <conio.h>
#include <Windows.h>

/* Values for the second argument to access.
   These may be OR'd together.  */
#define R_OK    4       /* Test for read permission.  */
#define W_OK    2       /* Test for write permission.  */
#define X_OK    1       /* Test for execute permission.  */
#define F_OK    0       /* Test for existence.  */

#elif defined(__linux__) || defined(__APPLE__) && defined(__MACH__)
#include <unistd.h>
#include <winerror.h>
#include <termios.h>
#include <errno.h>
#include <minwindef.h>
#include <basetyps.h>
#include <wtypes.h>
#include <Unknwn.h>
#include <dlfcn.h>

#if __cplusplus > 199711L
#define register      // Deprecated in C++11.
#endif  // #if __cplusplus > 199711L

#define _MAX_PATH   260 // max. length of full pathname
#define _MAX_DRIVE  3   // max. length of drive component
#define _MAX_DIR    256 // max. length of path component
#define _MAX_FNAME  256 // max. length of file name component
#define _MAX_EXT    256 // max. length of extension component

#define _access							access
#define _strnicmp						strncasecmp
#define _stricmp						strcasecmp
#define strcpy_s(ds, l, ss)				strcpy(ds, ss)
#define strcat_s(ds, l, ss)				strcat(ds, ss)
#define strncpy_s(ds, l, ss, n)			strncpy(ds,ss,n)
#define sprintf_s(s,l,f,...)			sprintf(s, f, ##__VA_ARGS__)	
#define _fseeki64						fseeko
#define _ftelli64						ftello
#define fread_s(p, l, es, ec, fp)		fread(p, es, ec, fp)
#define _getch							getch
#define _unlink							unlink


#define _countof(a)						(sizeof((a))/sizeof((a)[0]))

typedef int								errno_t;

inline errno_t fopen_s(FILE** pFile, const char *filename, const char *mode)
{
	errno = 0;
	FILE* fp = fopen(filename, mode);
	if (pFile != NULL)
		*pFile = fp;
	return errno;
}

inline errno_t localtime_s(struct tm* _Tm, time_t const* const _Time)
{
	struct tm* ret_tm = localtime(_Time);
	if (_Tm)
		memcpy(_Tm, ret_tm, sizeof(struct tm));
	return errno;
}

inline char getch() {
	/*#include <unistd.h>   //_getch*/
	/*#include <termios.h>  //_getch*/
	char buf = 0;
	struct termios old = { 0 };
	fflush(stdout);
	if (tcgetattr(0, &old)<0)
		perror("tcsetattr()");
	old.c_lflag &= ~ICANON;
	old.c_lflag &= ~ECHO;
	old.c_cc[VMIN] = 1;
	old.c_cc[VTIME] = 0;
	if (tcsetattr(0, TCSANOW, &old)<0)
		perror("tcsetattr ICANON");
	if (read(0, &buf, 1)<0)
		perror("read()");
	old.c_lflag |= ICANON;
	old.c_lflag |= ECHO;
	if (tcsetattr(0, TCSADRAIN, &old)<0)
		perror("tcsetattr ~ICANON");
	printf("%c\n", buf);
	return buf;
}

inline errno_t _splitpath_s(
	char const* _FullPath,
	char*       _Drive,
	size_t      _DriveCount,
	char*       _Dir,
	size_t      _DirCount,
	char*       _Filename,
	size_t      _FilenameCount,
	char*       _Ext,
	size_t      _ExtCount
)
{
	// Check parameter
	if ((_FullPath == nullptr) ||
		(_Drive == nullptr && _DriveCount != 0) ||
		(_Filename == nullptr && _FilenameCount != 0) ||
		(_Ext == nullptr && _ExtCount != 0))
		return EINVAL;

	size_t max_size_t = (size_t)~((size_t)0);

	// Check the full path size
	size_t fullpath_count = 0;
	const char* p = _FullPath;
	for (; *p != '\0' && fullpath_count < max_size_t; p++, fullpath_count++);

	// Reset the component buffer
	if (_DriveCount > 0)
		memset(_Drive, 0, _DriveCount);

	if (_DirCount > 0)
		memset(_Dir, 0, _DirCount);

	if (_FilenameCount > 0)
		memset(_Filename, 0, _FilenameCount);

	if (_ExtCount > 0)
		memset(_Ext, 0, _ExtCount);

	if (_Drive != nullptr)
		*_Drive = '\0';

	const char* pDir = _FullPath;

	p = strrchr(_FullPath, '/');
	size_t real_dir_count = p == nullptr ? 0 : (size_t)(p + 1 - _FullPath);
	const char* pFilename = p == nullptr ? _FullPath : (p + 1);

	p = strrchr(p == nullptr ? _FullPath : p, '.');

	size_t real_filename_count = p == nullptr ? fullpath_count : (size_t)(p - pFilename);
	const char* pExt = p;

	size_t real_ext_count = p == nullptr ? 0 : _FullPath + fullpath_count - p;

	if (_DirCount != 0 && _DirCount <= real_dir_count)
		return ERANGE;

	if (_FilenameCount != 0 && _FilenameCount <= real_filename_count)
		return ERANGE;

	if (_ExtCount != 0 && _ExtCount <= real_ext_count)
		return ERANGE;

	if (_DirCount != 0 && real_dir_count != 0)
		memcpy(_Dir, pDir, real_dir_count);

	if (_FilenameCount != 0 && real_filename_count != 0)
		memcpy(_Filename, pFilename, real_filename_count);

	if (_ExtCount != 0 && real_ext_count > 0)
		memcpy(_Ext, pExt, real_ext_count);

	return 0;
}

/////////////////////////////////////////////
//inter lock part
/////////////////////////////////////////////

#ifdef USE_MIPS
extern __inline__ void LNatomic_add(int i, long * v)
{
	unsigned long temp;

	__asm__ __volatile__(
		"1:   ll      %0, %1      # atomic_add\n"
		"     addu    %0, %2                  \n"
		"     sc      %0, %1                  \n"
		"     beqz    %0, 1b                  \n"
		: "=&r" (temp), "=m" (*v)
		: "Ir" (i), "m" (*v));
}

extern __inline__ void LNatomic_sub(int i, long * v)
{
	unsigned long temp;

	__asm__ __volatile__(
		"1:   ll      %0, %1      # atomic_sub\n"
		"     subu    %0, %2                  \n"
		"     sc      %0, %1                  \n"
		"     beqz    %0, 1b                  \n"
		: "=&r" (temp), "=m" (*v)
		: "Ir" (i), "m" (*v));
}


extern __inline__ unsigned long LNxchg_u32(long * m, unsigned long val)
{
	unsigned long dummy;

	__asm__ __volatile__(
		".set\tpush\t\t\t\t# xchg_u32\n\t"
		".set\tnoreorder\n\t"
		".set\tnomacro\n\t"
		"ll\t%0, %3\n"
		"1:\tmove\t%2, %z4\n\t"
		"sc\t%2, %1\n\t"
		"beqzl\t%2, 1b\n\t"
		" ll\t%0, %3\n\t"
		"sync\n\t"
		".set\tpop"
		: "=&r" (val), "=m" (*m), "=&r" (dummy)
		: "R" (*m), "Jr" (val)
		: "memory");

	return val;
}
#endif //USE_MIPS

inline long InterlockedIncrement(long volatile *pLong)
{
#ifndef NO_ASM
#ifdef USE_MIPS
	LNatomic_add(1, pLong);
	return *pLong;
#else
	register int __res;
	__asm__ __volatile__("movl    $1,%0\n\t"
		"lock    xadd %0,(%1)\n\t"
		"inc     %0\n\t"
		: "=a" (__res), "=r" (pLong)
		: "1" (pLong));
	return __res;
#endif
#else
#ifndef USE_MIPS
	/*long retVal = 0;
	__asm {
	mov ecx, pLong;
	mov eax, 1;
	lock xadd dword ptr [ecx], eax;
	inc eax;
	mov retVal, eax;
	}
	return retVal;*/
	return __sync_add_and_fetch(pLong, 1);
#endif
	return ++(*pLong);
#endif
}

inline long InterlockedDecrement(long volatile *pLong)
{
#ifndef NO_ASM
#ifdef USE_MIPS
	LNatomic_sub(1, pLong);
	return *pLong;
#else
	register int __res;
	__asm__ __volatile__("movl    $0xffffffff,%0\n\t"
		"lock    xadd %0,(%1)\n\t"
		"dec     %0\n\t"
		: "=a" (__res), "=r" (pLong)
		: "1" (pLong));
	return __res;
#endif
#else
#ifndef USE_MIPS
	/*long retVal = 0;
	__asm {
	mov ecx, pLong;
	mov eax, 0FFFFFFFFh;
	lock xadd dword ptr [ecx], eax;
	dec eax;
	mov retVal, eax;
	}
	return retVal;*/
	return __sync_sub_and_fetch(pLong, 1);
#endif
	return --(*pLong);
#endif
}

inline long InterlockedExchange(
	long volatile *Target, // pointer to the value to exchange
	long Value     // new value for Target
)
{
#ifndef NO_ASM
#ifdef USE_MIPS
	return (long)LNxchg_u32(Target, (unsigned long)Value);
#else
	register int __res;
	__asm__ __volatile__("movl    (%2),%0\n\t"
		"1:\n\t"
		"lock    cmpxchgl %3,(%1)\n\t"
		"jne 1b\n\t"
		: "=a" (__res), "=c" (Target) :
		"1" (Target), "d" (Value));
	return __res;
#endif
#else
#ifndef USE_MIPS
	/*long retVal = 0;
	__asm {
	mov ecx,	Target;
	mov edx, Value;
	mov eax, [ecx];
	lab1:
	lock cmpxchg dword ptr [ecx], edx;
	jne lab1
	mov retVal, eax;
	}
	return retVal;*/
	return __sync_lock_test_and_set(Target, Value);
#endif
	long oldvalue = *Target;
	*Target = Value;
	return oldvalue;
#endif
}

inline long InterlockedExchangeAdd(
	long volatile *pLong,
	long Value
)
{
#ifndef NO_ASM
#ifdef USE_MIPS
	if (Value >= 0)
		LNatomic_add((unsigned long)Value, pLong);
	else
		LNatomic_sub((unsigned long)Value, pLong);
	return *pLong;
#else
	register int __res;
	__asm__ __volatile__("movl    (%2),%0\n\t"
		"lock    xadd %3,(%2)\n\t"
		: "=a" (__res), "=c" (pLong)
		: "1" (pLong), "d" (Value));
	return __res;
#endif
#else
	/*#ifndef USE_MIPS
	long retVal = 0;
	__asm __volatile__{
	mov ecx, pLong;
	mov eax, Value;
	lock xadd [ecx], eax;
	mov retVal, eax;
	}
	return retVal;
	#endif*/
	*pLong += Value;
	return *pLong;
#endif
}

#endif	// _WIN32

#endif

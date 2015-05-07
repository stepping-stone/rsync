#ifndef MINGW_H
#define MINGW_H

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utime.h>
#include <malloc.h>
#include <stdio.h>
#include <dirent.h>
#include <windows.h>
#include <wchar.h>


#undef MAXPATHLEN
#define MAXPATHLEN 4096
#undef MAX_PATH
#define MAX_PATH 4096


#undef OFF_T
#undef STRUCT_STAT
#undef USE_STAT64_FUNCS
#define OFF_T off64_t
#define STRUCT_STAT struct _stati64
#define USE_STAT64_FUNCS 1

#define HAVE_SOCKADDR_STORAGE 1

#if 0
struct var_info_t
{
	void* var;
	int is_pointer;
	int size;
};

struct file_list;
struct file_struct;
#endif

/*
#define stat64(...) __stati64(__VA_ARGS__)
#define fstat64(...) _fstati64(__VA_ARGS__)
*/

#define utimbuf _utimbuf

int not_implemented(const char* fun, const char* file, int line);
#define NOT_IMPLEMENTED(fun) not_implemented(fun, __FILE__, __LINE__)
#define NOT_IMPLEMENTED2(rettype, fun) (rettype) not_implemented(fun, __FILE__, __LINE__)

#define getgrgid(...) NOT_IMPLEMENTED2(struct group *, "getgrgid")
#define getgrnam(...) NOT_IMPLEMENTED2(struct group *, "getgrnam")
#define getpwnam(...) NOT_IMPLEMENTED2(struct passwd *, "getpwnam")
#define setuid(...) NOT_IMPLEMENTED("setuid")
#define setgid(...) NOT_IMPLEMENTED("setgid")
#define kill(...) NOT_IMPLEMENTED("kill")
#define chroot(...) NOT_IMPLEMENTED("chroot")
#define getpwuid(...) NOT_IMPLEMENTED2(struct passwd*, "getpwuid")
#define minor(...) NOT_IMPLEMENTED("minor")
#define major(...) NOT_IMPLEMENTED("major")

#define openlog(...) NOT_IMPLEMENTED("openlog")
#define closelog(...) NOT_IMPLEMENTED("closelog")
#define syslog(...) NOT_IMPLEMENTED("syslog")
#define chown(...) NOT_IMPLEMENTED("chown")
#define symlink(...) NOT_IMPLEMENTED("symlink")

#define EAI_BADHINTS  12
#define EAI_PROTOCOL  13
#define EAI_MAX   14

#define AI_MASK   (AI_PASSIVE | AI_CANONNAME | AI_NUMERICHOST)

#define S_ISUID	04000
#define S_ISGID 02000

#define WNOHANG 1
#define SIGUSR1 10
#define SIGUSR2 12

#define FNDELAY 04000
#define F_GETFL 3
#define F_SETFL 4
#define F_WRLCK 1
#define F_SETLK 6

#define SIGHUP    1
#define SIGCHLD   17
#define SIGPIPE   13

#define	LOG_PID		0x01
#define	LOG_WARNING	4
#define	LOG_INFO	6

#define	IN_EXPERIMENTAL(a)	((((long int) (a)) & 0xf0000000) == 0xf0000000)

#define SIGACTMASK(n,h)

struct passwd
{
	char *pw_name;    /* Username.  */
	char *pw_passwd;    /* Password.  */
	unsigned int pw_uid;   /* User ID.  */
	unsigned int pw_gid;   /* Group ID.  */
	char *pw_gecos;   /* Real name.  */
	char *pw_dir;     /* Home directory.  */
	char *pw_shell;   /* Shell program.  */
};

struct group
{
	char *gr_name;    /* Group name.  */
	char *gr_passwd;    /* Password.  */
	unsigned int gr_gid;   /* Group ID.  */
	char **gr_mem;    /* Member list. */
};


struct flock
{
	short int l_type; /* Type of lock: F_RDLCK, F_WRLCK, or F_UNLCK.  */
	short int l_whence; /* Where `l_start' is relative to (like `lseek').  */
	long int l_start;  /* Offset where the lock begins.  */
	long int l_len;  /* Size of the locked area; zero means until EOF.  */
	int l_pid;  /* Process holding the lock.  */
};

struct FILE_PIPE_LOCAL_INFORMATION {
	ULONG NamedPipeType;
	ULONG NamedPipeConfiguration;
	ULONG MaximumInstances;
	ULONG CurrentInstances;
	ULONG InboundQuota;
	ULONG ReadDataAvailable;
	ULONG OutboundQuota;
	ULONG WriteQuotaAvailable;
	ULONG NamedPipeState;
	ULONG NamedPipeEnd;
};

typedef struct _IO_STATUS_BLOCK {
		union {
				LONG Status;
				PVOID Pointer;
		};
		ULONG_PTR Information;
} IO_STATUS_BLOCK;

typedef enum _FILE_INFORMATION_CLASS {
	FileDirectoryInformation = 1,
	FileFullDirectoryInformation,
	FileBothDirectoryInformation,
	FileBasicInformation,
	FileStandardInformation,
	FileInternalInformation,
	FileEaInformation,
	FileAccessInformation,
	FileNameInformation,
	FileRenameInformation,
	FileLinkInformation,
	FileNamesInformation,
	FileDispositionInformation,
	FilePositionInformation,
	FileFullEaInformation,
	FileModeInformation,
	FileAlignmentInformation,
	FileAllInformation,
	FileAllocationInformation,
	FileEndOfFileInformation,
	FileAlternateNameInformation,
	FileStreamInformation,
	FilePipeInformation,
	FilePipeLocalInformation,
	FilePipeRemoteInformation,
	FileMailslotQueryInformation,
	FileMailslotSetInformation,
	FileCompressionInformation,
	FileObjectIdInformation,
	FileCompletionInformation,
	FileMoveClusterInformation,
	FileQuotaInformation,
	FileReparsePointInformation,
	FileNetworkOpenInformation,
	FileAttributeTagInformation,
	FileTrackingInformation,
	FileIdBothDirectoryInformation,
	FileIdFullDirectoryInformation,
	FileValidDataLengthInformation,
	FileShortNameInformation,
	FileMaximumInformation
} FILE_INFORMATION_CLASS, *PFILE_INFORMATION_CLASS;


char* __getcwd(char *buffer, int maxlen);
int __mkdir (const char *path, mode_t mode);
int __rename(const char *oldname, const char *newname);
int __open(const char *filename, int oflag, ...);
FILE* __fopen(const char *path, const char *mode);
int __unlink(const char *filename);
int __rmdir(const char *dirname);
int __chdir(const char *dirname);
int __utime(const char *filename, struct _utimbuf *times);
DIR* __opendir (const char* dir);
struct dirent* __readdir(DIR*);
int __closedir(DIR*);
int __stati64(const char *path, struct _stati64 *buffer);
unsigned long long int makedev (unsigned int __major, unsigned int __minor);
wchar_t* unixToWindows(const char* unixPath);
char* windowsToUnix(const wchar_t* windowsPath);
void wtrim_trailing_slashes(wchar_t *name);

int getuid();
int getgid();

extern void out_of_memory(const char *str);

LONG __stdcall NtQueryInformationFile (HANDLE, IO_STATUS_BLOCK *, VOID *, DWORD, DWORD);

#define getcwd(...) __getcwd(__VA_ARGS__)
#define open(...) __open(__VA_ARGS__)
#define chdir(...) __chdir(__VA_ARGS__)
#define rmdir(...) __rmdir(__VA_ARGS__)
#define unlink(...) __unlink(__VA_ARGS__)
#define utime(...) __utime(__VA_ARGS__)

void get_utf8_arguments(int* pargc, char*** pargv);

int fork(void);

#endif

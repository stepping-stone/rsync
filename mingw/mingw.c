#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdarg.h>
#include <time.h>
#include <direct.h>

#define DEF_REN 1
#include "rsync.h"
#include "mingw.h"

extern int am_sender;
extern int am_generator;

char currentDir[MAXPATHLEN] = "";

BOOLEAN WINAPI RtlTimeToSecondsSince1970(PLARGE_INTEGER Time, PULONG ElapsedSeconds);

#define TOUL(x) (ULONGLONG)(x)
static const ULONGLONG WCEXE = TOUL('e') << 32 | TOUL('x') << 16 | TOUL('e');
static const ULONGLONG WCBAT = TOUL('b') << 32 | TOUL('a') << 16 | TOUL('t');
static const ULONGLONG WCCMD = TOUL('c') << 32 | TOUL('m') << 16 | TOUL('d');
static const ULONGLONG WCCOM = TOUL('c') << 32 | TOUL('o') << 16 | TOUL('m');

struct chmod_mode_struct {
  struct chmod_mode_struct *next;
  int ModeAND, ModeOR;
  char flags;
};

#include "global.h"

#define FILETIME_1970		(116444736000000000LL)	/* 100-nanoseconds between 1/1/1601 and 1/1/1970 */
#define HECTONANOSEC_PER_SEC	(10000000)

int not_implemented(const char* fun, const char* file, int line)
{
	rprintf(FINFO, "%s not implemented, called in %s at line %d\n", fun, file, line);
	exit(43);

	return 0;
}

int getuid()
{
	return 0;
}

int geteuid()
{
	return 0;
}

int pipe (int __pipedes[2])
{
	return _pipe(__pipedes, 65536, _O_BINARY);
}

int fcntl (int fd, int cmd, ...)
{
	switch (cmd)
	{
		case F_GETFL:     /* Get file status flags and file access modes */	
			return 0;
		case F_SETFL:     /* Set file status flags */
			return 0;
		default:
		 	errno = EINVAL;
		  rprintf(FINFO, "unimplemented fnctl\n");
			return -1;
	}

	errno = ENOSYS;
	rprintf(FINFO, "unimplemented fnctl\n");

	return -1;
}

unsigned long long int makedev (unsigned int __major, unsigned int __minor)
{
	return ((__minor & 0xff) | ((__major & 0xfff) << 8)
		| (((unsigned long long int) (__minor & ~0xff)) << 12)
		| (((unsigned long long int) (__major & ~0xfff)) << 32));
}

int waitpid(int pid, int *stat_loc, int options)
{
	DWORD timeout = INFINITE;
	if (options & WNOHANG) timeout = 0;
	switch (WaitForSingleObject((HANDLE) pid, timeout))
	{
		case WAIT_TIMEOUT:
			return 0;
		case WAIT_FAILED:
			return -1;
		default:
			GetExitCodeProcess((HANDLE) pid, (LPDWORD) stat_loc);
			if (!WIFEXITED(*stat_loc)) *stat_loc = 0;
			return pid;
	}
}

int __select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
	struct timeval start_time;
	struct timeval time;
	int i = 0;
	int fds = 0;
	fd_set new_readfds;
	fd_set new_writefds;
	fd_set new_exceptfds;
	DWORD bytesAvail = 0;

	gettimeofday(&start_time, NULL);

	FD_ZERO(&new_exceptfds);
	FD_ZERO(&new_readfds);
	FD_ZERO(&new_writefds);

	for (;;)
	{
		if (readfds)
		{
			for (i = 0; i < nfds; i++)
			{
				if (FD_ISSET(i, readfds))
				{
					bytesAvail = 0;
					if (PeekNamedPipe((HANDLE)_get_osfhandle(i), NULL, 0, NULL, &bytesAvail, NULL))
					{
						if (bytesAvail > 0)
						{
							FD_SET((unsigned) i, &new_readfds);
							fds++;
						}
					}
					else if (GetLastError() == ERROR_INVALID_FUNCTION || GetLastError() == ERROR_BROKEN_PIPE)
					{
						FD_SET((unsigned) i, &new_readfds);
						fds++;
					}
					else
					{
						FD_SET((unsigned) i, &new_exceptfds);
						fds++;
					}
				}
			}
		}

		if (writefds)
		{
			for (i = 0; i < nfds; i++)
			{
				if (FD_ISSET(i, writefds))
				{
          IO_STATUS_BLOCK iosb;
					struct FILE_PIPE_LOCAL_INFORMATION fpli;
					if(NtQueryInformationFile ((HANDLE) _get_osfhandle(i), &iosb, &fpli, sizeof (fpli), FilePipeLocalInformation))
					{
						FD_SET((unsigned) i, &new_writefds);
						fds++;
					}
					else if (fpli.WriteQuotaAvailable >= 4096)
					{
						FD_SET((unsigned) i, &new_writefds);
						fds++;
					}
				}
			}
		}

		if (fds > 0) break;

		Sleep(10);

		if (timeout != NULL)
		{
			gettimeofday(&time, NULL);
			if (time.tv_sec - start_time.tv_sec > timeout->tv_sec || (time.tv_sec - start_time.tv_sec == timeout->tv_sec && time.tv_usec - start_time.tv_usec >= timeout->tv_usec)) break;
		}
	}

	if (readfds != NULL)
	{
		 memcpy(readfds, &new_readfds, sizeof(fd_set));
	}
  if (writefds != NULL)
	{
	 memcpy(writefds, &new_writefds, sizeof(fd_set));
	}
  if (exceptfds != NULL)
	{
	 memcpy(exceptfds, &new_exceptfds, sizeof(fd_set));
	}

	return fds;
}

int gettimeofday (struct timeval *tv, void* tz)
{
	FILETIME fti;
	LARGE_INTEGER li;
	GetSystemTimeAsFileTime(&fti);
	memcpy(&li, &fti, sizeof(FILETIME));

	tv->tv_usec = (long) ((li.QuadPart / 10LL) % 1000000LL);
	tv->tv_sec = (long) ((li.QuadPart - 116444736000000000LL) / 10000000LL);

	return (0);
}

int __rename(const char *oldname, const char *newname)
{
	const wchar_t* oldWindowsName = unixToWindows(oldname);
	const wchar_t* newWindowsName = unixToWindows(newname);

	int result = _wrename(oldWindowsName, newWindowsName);
	if (result && errno == EEXIST)
	{
		result = _wunlink(newWindowsName);
		if (result) return result;
		result = _wrename(oldWindowsName, newWindowsName);
	}

	return result;
}

int __mkdir (const char *path, mode_t mode)
{
	const wchar_t* windowsPath = unixToWindows(path);
	if (_wmkdir(windowsPath)) {
		return -1;
	}
	return _wchmod(windowsPath, mode);
}

BOOL WINAPI CtrlHandler(DWORD dwCtrlType)
{
	msleep(400);
	if (dwCtrlType)
	{
		if (!am_sender && !am_generator )
		{
			sigusr2_handler(0);
		}
	}
	else
	{
		exit_cleanup(RERR_SIGNAL);
	}

	return TRUE;
}

size_t sizeOfVars(const struct var_info_t* var_info)
{
	size_t result = 0;
	size_t i;
	char* var;
	for (i = 0; var_info[i].var; i++)
	{
		if (var_info[i].is_pointer)
		{
			result += sizeof(int);
			var = *((char**) var_info[i].var);
			if (var == NULL) continue;

			if (var_info[i].size)
			{
				result += var_info[i].size;							
			}
			else
			{
				result += strlen(var) + 1;
			}
		}
		else
		{
			result += var_info[i].size;
		}
	}

	return result;
}

/**
 * Saves global and local variables described by global_var_info and local_var_info to shared memory.
 */
HANDLE saveVars(const struct var_info_t* local_var_info)
{
	HANDLE hMapping = NULL;
	SECURITY_ATTRIBUTES securityAttributes = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
	hMapping = CreateFileMapping(INVALID_HANDLE_VALUE, &securityAttributes, PAGE_READWRITE, 0, sizeOfVars(global_var_info) + sizeOfVars(local_var_info) + file_list_size(the_file_list), NULL);
	void* shared = MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	char* sharedVar = shared;
	char* var;
	size_t varSize;
	const struct var_info_t* var_info[] = { global_var_info, local_var_info, NULL };
	size_t i, j;

	if (verbose > 3) rprintf(FINFO, "copying variables to shared memory\n");

	for (i = 0; var_info[i]; i++)
	{
		for (j = 0; var_info[i][j].var; j++)
		{
			if (var_info[i][j].is_pointer)
			{
				var = *((char**) var_info[i][j].var);
				*((int*) sharedVar) = (var == NULL);
				sharedVar += sizeof(int);

				if (var == NULL) continue;

				if (var_info[i][j].size)
				{
					varSize = var_info[i][j].size;
				}
				else
				{
					varSize = strlen(var) + 1;
				}
			}
			else
			{
				var = var_info[i][j].var;
				varSize = var_info[i][j].size;
			}

			memcpy(sharedVar, var, varSize);
			sharedVar += varSize;
		}
	}

	if (verbose > 3) rprintf(FINFO, "copying file list\n");
  
	save_file_list(sharedVar, the_file_list);

	UnmapViewOfFile(shared);

	if (verbose > 3) rprintf(FINFO, "variables copied\n");

	return hMapping;
}


void restoreVars(HANDLE hMapping, const struct var_info_t* local_var_info)
{
	void* shared = MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	char* sharedVar = shared;
	char** varPointer;
	char* var;
	size_t varSize;
	const struct var_info_t* var_info[] = { global_var_info, local_var_info, NULL };
	size_t i, j;

	if (verbose > 3) rprintf(FINFO, "restoring variables from shared memory\n");

	for (i = 0; var_info[i]; i++ )
	{
		for (j = 0; var_info[i][j].var; j++)
		{
			if (var_info[i][j].is_pointer)
			{
				varPointer = (char**) var_info[i][j].var;
				int isNull = *((int*) sharedVar);
				sharedVar += sizeof(int);

				if (isNull)
				{
					*varPointer = NULL;
					continue;
				}

				if (var_info[i][j].size)
				{
					varSize = var_info[i][j].size;
				}
				else
				{
					varSize = strlen(sharedVar) + 1;
				}
				var = *varPointer = malloc(varSize);
			}
			else
			{
				var = var_info[i][j].var;
				varSize = var_info[i][j].size;
			}

			memcpy(var, sharedVar, varSize);
			sharedVar += varSize;
		}
	}

	if (verbose > 3) rprintf(FINFO, "restoring file list\n");
	restore_file_list(the_file_list, sharedVar);
	free( local_var_info[2].var);
	*((struct file_list**) local_var_info[2].var) = the_file_list;

	UnmapViewOfFile(shared);
	CloseHandle(hMapping);

	if (verbose > 3) rprintf(FINFO, "variables restored\n");
}

size_t file_list_size(struct file_list* flist)
{
	size_t result = 0;
	int i;
	for (i = 0; i < flist->count; i++)
	{
		result += file_struct_size(flist->files[i]);
	}

	return result;
}

size_t file_struct_size(struct file_struct* file)
{
	size_t result = file_struct_len;
	if (file->u.sum) result += strlen(file->u.sum) + 1;
	if (file->basename) result += strlen(file->basename) + 1;
	if (file->dirname) result += strlen(file->dirname) + 1;

	return result;
}

size_t save_string(void** shared, const char* str)
{
	size_t len = 0;
	if (str)
	{
		len = strlen(str) + 1;
		memcpy(*shared, str, len);
		*shared += len;
	}

	return len;
}

void restore_string(char** str, void** shared)
{
	size_t len;

	if (*str)
	{
		len = strlen(*str) + 1;
		memcpy(*str, *shared, len);
		*shared += len;
	}
}

void save_file_list(void* shared, struct file_list* flist)
{
	struct file_struct* file;
	int i;
	for (i = 0; i < flist->count; i++)
	{
		file = (struct file_struct*) shared;
		memcpy(file, flist->files[i], file_struct_len);
		shared += file_struct_len;
		file->u.sum = (char*) save_string(&shared, flist->files[i]->u.sum);
		file->basename = (char*) save_string(&shared, flist->files[i]->basename);
		file->dirname = (char*) save_string(&shared, flist->files[i]->dirname);
	}
}

void restore_file_list(struct file_list* flist, void* shared)
{
	struct file_struct* file;
	void* pool;

	flist->files = new_array(struct file_struct *, flist->malloced);
	flist->file_pool = pool_create(FILE_EXTENT, 0,out_of_memory, POOL_INTERN);

	size_t len;
	int i;
	for (i = 0; i < flist->count; i++)
	{
		file = (struct file_struct*) shared;

		len = file_struct_len + (size_t) file->u.sum + (size_t) file->basename + (size_t) file->dirname;
		flist->files[i] = pool_alloc(flist->file_pool, len, "restore_file_list");
		memcpy(flist->files[i], shared, len);

		if (file->u.sum) flist->files[i]->u.sum = (char*) flist->files[i] + file_struct_len;
		if (file->basename) flist->files[i]->basename = (char*) flist->files[i] + file_struct_len + (size_t) file->u.sum;
		if (file->dirname) flist->files[i]->dirname = (char*) flist->files[i] + file_struct_len + (size_t) file->u.sum + (size_t) file->basename;

		shared += len;
	}
}

char* __getcwd(char *buffer, int maxlen)
{
	wchar_t wbuf[MAXPATHLEN];
	if (_wgetcwd(wbuf, maxlen) != NULL)
	{
		strcpy(buffer, windowsToUnix(wbuf));
		return buffer;
	}
	else
	{
		return NULL;
	}
}

int __open(const char *filename, int oflag, ...) 
{
	int mode = 0;

	if (oflag & O_CREAT)
	{
		va_list arg;
		va_start (arg, oflag);
		mode = va_arg (arg, int);
		va_end (arg);

		return _wopen(unixToWindows(filename), oflag, mode);
  }
	else
	{
		return _wopen(unixToWindows(filename), oflag);
	}
}

FILE *__fopen(const char *path, const char *mode)
{
	wchar_t wmode[16];
	MultiByteToWideChar(CP_UTF8, 0, mode, -1, wmode, sizeof(wmode) / sizeof(wchar_t));
	return _wfopen(unixToWindows(path), wmode);
}

/**
 * The Windows implementation of _wstati64 does not support UNC paths.
 * This function has been copied from the wine sources and modified.
 */
int __stati64(const char *unixPath, struct _stati64 *buf)
{
	wchar_t* path = unixToWindows(unixPath);
  DWORD dw;
  WIN32_FILE_ATTRIBUTE_DATA hfi;
  unsigned short mode = S_IREAD;
  int plen;

  if (!GetFileAttributesExW(path, GetFileExInfoStandard, &hfi))
  {
      errno = ERROR_FILE_NOT_FOUND;
      return -1;
  }

  memset(buf,0,sizeof(struct _stati64));

  /* FIXME: rdev isn't drive num, despite what the docs says-what is it? */
  if (iswalpha(*path))
    buf->st_dev = buf->st_rdev = towupper(*path - 'A'); /* drive num */
  else
    buf->st_dev = buf->st_rdev = _getdrive() - 1;

  plen = wcslen(path);

  /* Dir, or regular file? */
  if ((hfi.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ||
      (path[plen-1] == '\\'))
    mode |= (S_IFDIR | S_IEXEC);
  else
  {
    mode |= S_IFREG;
    /* executable? */
    if (plen > 6 && path[plen-4] == '.')  /* shortest exe: "\x.exe" */
    {
      ULONGLONG ext = towlower(path[plen-1]) | (towlower(path[plen-2]) << 16) |
                               ((ULONGLONG)towlower(path[plen-3]) << 32);
      if (ext == WCEXE || ext == WCBAT || ext == WCCMD || ext == WCCOM)
        mode |= S_IEXEC;
    }
  }

  if (!(hfi.dwFileAttributes & FILE_ATTRIBUTE_READONLY))
    mode |= S_IWRITE;

  buf->st_mode  = mode;
  buf->st_nlink = 1;
  buf->st_size  = ((__int64)hfi.nFileSizeHigh << 32) + hfi.nFileSizeLow;
  RtlTimeToSecondsSince1970((LARGE_INTEGER *)&hfi.ftLastAccessTime, &dw);
  buf->st_atime = dw;
  RtlTimeToSecondsSince1970((LARGE_INTEGER *)&hfi.ftLastWriteTime, &dw);
  buf->st_mtime = buf->st_ctime = dw;

  return 0;
}

int __unlink(const char *filename)
{
	return _wunlink(unixToWindows(filename));
}

int __rmdir(const char *dirname)
{
	return _wrmdir(unixToWindows(dirname));
}

int __chdir(const char *dirname)
{
	strcpy(currentDir, dirname);
	return _wchdir(unixToWindows(dirname));
}

int __utime(const char *filename, struct _utimbuf *times)
{
	return _wutime(unixToWindows(filename), times);
}

/**
 * Modified function from mingw.
 */
DIR* __opendir (const char* dir)
{
  _WDIR *nd;
  unsigned int rc;
  wchar_t szFullPath[MAX_PATH];
	const wchar_t* szPath = unixToWindows(dir);
  
  errno = 0;

  if (!dir)
    {
      errno = EFAULT;
      return NULL;
    }

  if (dir[0] == '\0')
    {
      errno = ENOTDIR;
      return NULL;
    }

  /* Attempt to determine if the given path really is a directory. */
  rc = GetFileAttributesW(szPath);
  if (rc == (unsigned int)-1)
    {
      /* call GetLastError for more error info */
      errno = ENOENT;
      return NULL;
    }
  if (!(rc & FILE_ATTRIBUTE_DIRECTORY))
    {
      /* Error, entry exists but not a directory. */
      errno = ENOTDIR;
      return NULL;
    }

  /* Make an absolute pathname.  */
  _wfullpath (szFullPath, szPath, MAX_PATH);

  /* Allocate enough space to store DIR structure and the complete
   * directory path given. */
  nd = (_WDIR *) malloc (sizeof (_WDIR) + (wcslen (szFullPath)
             + wcslen (L"\\")
             + wcslen (L"*") + 1)
            * sizeof (wchar_t));

  if (!nd)
    {
      /* Error, out of memory. */
      errno = ENOMEM;
      return NULL;
    }

 /* Create the search expression. */
  wcscpy (nd->dd_name, szFullPath);

  /* Add on a slash if the path does not end with one. */
  if (nd->dd_name[0] != L'\0'
      && wcsrchr (nd->dd_name, L'/') != nd->dd_name
              + wcslen (nd->dd_name) - 1
      && wcsrchr (nd->dd_name, L'\\') != nd->dd_name
                     + wcslen (nd->dd_name) - 1)
    {
      wcscat (nd->dd_name, L"\\");
    }

  /* Add on the search pattern */
  wcscat (nd->dd_name, L"*");

  /* Initialize handle to -1 so that a premature closedir doesn't try
   * to call _findclose on it. */
  nd->dd_handle = -1;

  /* Initialize the status. */
  nd->dd_stat = 0;

  /* Initialize the dirent structure. ino and reclen are invalid under
   * Win32, and name simply points at the appropriate part of the
   * findfirst_t structure. */
  nd->dd_dir.d_ino = 0;
  nd->dd_dir.d_reclen = 0;
  nd->dd_dir.d_namlen = 0;
  memset (nd->dd_dir.d_name, 0, FILENAME_MAX);

  return (DIR*) nd;
}

int __closedir (DIR * dir)
{
	_WDIR* dirp = (_WDIR*) dir;

  int rc;

  errno = 0;
  rc = 0;

  if (!dirp)
    {
      errno = EFAULT;
      return -1;
    }

  if (dirp->dd_handle != -1)
    {
      rc = _findclose (dirp->dd_handle);
    }

  /* Delete the dir structure. */
  free (dirp);

  return rc;
}

struct dirent * __readdir(DIR* dir)
{
	_WDIR* dirp = (_WDIR*) dir;
	static struct dirent entry;

  errno = 0;

  /* Check for valid DIR struct. */
  if (!dirp)
    {
      errno = EFAULT;
      return NULL;
    }

  if (dirp->dd_stat < 0)
    {
      /* We have already returned all files in the directory
       * (or the structure has an invalid dd_stat). */
      return NULL;
    }
  else if (dirp->dd_stat == 0)
    {
      /* We haven't started the search yet. */
      /* Start the search */
      dirp->dd_handle = _wfindfirst (dirp->dd_name, &(dirp->dd_dta));

      if (dirp->dd_handle == -1)
  {
    /* Whoops! Seems there are no files in that
     * directory. */
    dirp->dd_stat = -1;
  }
      else
  {
    dirp->dd_stat = 1;
  }
    }
  else
    {
      /* Get the next search entry. */
      if (_wfindnext (dirp->dd_handle, &(dirp->dd_dta)))
  {
    /* We are off the end or otherwise error. 
       _findnext sets errno to ENOENT if no more file
       Undo this. */ 
    DWORD winerr = GetLastError ();
    if (winerr == ERROR_NO_MORE_FILES)
      errno = 0;  
    _findclose (dirp->dd_handle);
    dirp->dd_handle = -1;
    dirp->dd_stat = -1;
  }
      else
  {
    /* Update the status to indicate the correct
     * number. */
    dirp->dd_stat++;
  }
    }

  if (dirp->dd_stat > 0)
    {
      /* Successfully got an entry. Everything about the file is
       * already appropriately filled in except the length of the
       * file name. */
			entry.d_ino = dirp->dd_dir.d_ino;
			entry.d_reclen = dirp->dd_dir.d_reclen;
			WideCharToMultiByte(CP_UTF8, 0, dirp->dd_dta.name, -1, entry.d_name, MAX_PATH, NULL, NULL);
			entry.d_namlen = strlen(entry.d_name);
			return &entry;
    }

  return NULL;
}


/**
 * Converts paths of the form /x/some/path to \\?\x:\some\path.
 */
wchar_t* unixToWindows(const char* unixPath)
{
	static wchar_t windowsPath[2][MAXPATHLEN];
  static int i = 0;

  int pathLen = strlen(unixPath);
	wchar_t* path;

  i = (i + 1) % 2;
	if (strlen(currentDir) == 0 && strcmp(unixPath, "/dev/null") == 0)
	{
		wcscpy(windowsPath[i], L"nul");
		return windowsPath[i];
	}

	wcscpy(windowsPath[i], L"\\\\?\\");
	if (strcmp(currentDir, "/") == 0 && ((pathLen == 1 && isalpha(unixPath[0])) || (pathLen > 1 && isalpha(unixPath[0]) && unixPath[1] == '/')))
	{  // curentDir is / and unixPath of the form x or x/some/path
		windowsPath[i][4] = unixPath[0];
    windowsPath[i][5] = ':';
		MultiByteToWideChar(CP_UTF8, 0, unixPath + 1, -1, windowsPath[i] + 6, MAXPATHLEN - 6);
	}
	else if ((pathLen == 2 && unixPath[0] == '/' && isalpha(unixPath[1])) || (pathLen > 2 && unixPath[0] == '/' && isalpha(unixPath[1]) && unixPath[2] == '/'))
	{  // unixPath is of the form /x or /x/some/path
		windowsPath[i][4] = unixPath[1];
		windowsPath[i][5] = ':';
		MultiByteToWideChar(CP_UTF8, 0, unixPath + 2, -1, windowsPath[i] + 6, MAXPATHLEN - 6);
	}
	else if (unixPath[0] == '/')
	{  // unixPath is of the form /some/path
    windowsPath[i][4] = L'A' + _getdrive() - 1;
    windowsPath[i][5] = L':';
		MultiByteToWideChar(CP_UTF8, 0, unixPath, -1, windowsPath[i] + 6, MAXPATHLEN - 6);
	}
	else
	{  // unixPath if of the form some/path
		_wgetcwd(windowsPath[i] + 4, MAXPATHLEN - 4);
		if (wcslen(windowsPath[i]) > 6 && windowsPath[i][6] == L'?')
		{
      wmemmove(windowsPath[i], windowsPath[i] + 4, wcslen(windowsPath[i]) + 1);
		}
		if (windowsPath[i][wcslen(windowsPath[i]) - 1] != L'\\') wcscat(windowsPath[i], L"\\");
		if (strcmp(unixPath, ".") != 0) MultiByteToWideChar(CP_UTF8, 0, unixPath, -1, windowsPath[i] + wcslen(windowsPath[i]), MAXPATHLEN - wcslen(windowsPath[i]));
	}

	wtrim_trailing_slashes(windowsPath[i]);

	path = windowsPath[i];
	while (*path)
	{
		if (*path == '/') *path = L'\\';
		path++;
	}

	if (wcslen(windowsPath[i]) == 6 && iswalpha(windowsPath[i][4]) && windowsPath[i][5] == ':') wcscat(windowsPath[i], L"\\");

	return windowsPath[i];
}

/*
 * Convert paths of the form x:\some\path to /x/some/path.
 */
char* windowsToUnix(const wchar_t* windowsPath)
{
	static char unixPath[2][MAXPATHLEN];
	static int i = 0;
	char* path;

  i = (i + 1) % 2;
  WideCharToMultiByte(CP_UTF8, 0, windowsPath, -1, unixPath[i], MAX_PATH, NULL, NULL);
	if (strlen(unixPath[i]) >= 2 && isalpha(unixPath[i][0]) && unixPath[i][1] == ':')
	{  // path begins with x:
		unixPath[i][1] = unixPath[i][0];
		unixPath[i][0] = '/';
	}
	path = unixPath[i];
	while (*path)
	{
		if (*path == '\\') *path = '/';
		path++;
	}

	return unixPath[i];
}

void wtrim_trailing_slashes(wchar_t *name)
{
  int l;

  /* Don't change empty string; and also we can't improve on
   * "/" */

  l = wcslen(name);
  while (l > 1) {
    if (name[--l] != '/')
      break;
    name[l] = '\0';
  }
}

/**
 * Retrieves the unicode command line encoded in UTF-8.
 */
void get_utf8_arguments(int* pargc, char*** pargv)
{
	int i, bytesWritten;
  char* arg;
  LPWSTR* arglist = CommandLineToArgvW(GetCommandLineW(), pargc);
	size_t size = 0;

	for (i = 0; i < *pargc; i++)
	{
		size += WideCharToMultiByte(CP_UTF8, 0, arglist[i], -1, NULL, 0, NULL, NULL);
	}

	*pargv = (char**) malloc(size + *pargc * sizeof(char**));
	if (*pargv == NULL)
	{
		LocalFree(arglist);
		out_of_memory("get_utf8_arguments");
	}

	arg = ((char*) *pargv) + *pargc * sizeof(char**);	
 	for (i = 0; i < *pargc; i++)
  {
		bytesWritten = WideCharToMultiByte(CP_UTF8, 0, arglist[i], -1, arg, size, NULL, NULL);
		(*pargv)[i] = arg;
    arg += bytesWritten;
		size -= bytesWritten;
  }

  LocalFree(arglist);
}

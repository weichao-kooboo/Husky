#ifndef HKY_FILES_H_INCLUDED
#define HKY_FILES_H_INCLUDED

typedef int                      hky_fd_t;

#define HKY_INVALID_FILE -1
#define HKY_FILE_ERROR -1

#ifdef __CYGWIN__

#ifndef HKY_HAVE_CASELESS_FILESYSTEM
#define HKY_HAVE_CASELESS_FILESYSTEM
#endif // HKY_HAVE_CASELESS_FILESYSTEM

#define hky_open_file(name,mode,create,access) open((const char*)name,model|create|O_BINARY,access)

#else
#define hky_open_file(name,mode,create,access) open((const char*)name,mode|create,access)
#endif // __CYGWIN__


#define hky_open_file_n          "open()"

#define HKY_FILE_RDONLY          O_RDONLY
#define HKY_FILE_WRONLY          O_WRONLY
#define HKY_FILE_RDWR            O_RDWR
#define HKY_FILE_CREATE_OR_OPEN  O_CREAT
#define HKY_FILE_OPEN            0
#define HKY_FILE_TRUNCATE        (O_CREAT|O_TRUNC)
#define HKY_FILE_APPEND          (O_WRONLY|O_APPEND)
#define HKY_FILE_NONBLOCK        O_NONBLOCK

#if (HKY_HAVE_OPENAT)
#define HKY_FILE_NOFOLLOW        O_NOFOLLOW

#if defined(O_DIRECTORY)
#define HKY_FILE_DIRECTORY       O_DIRECTORY
#else
#define HKY_FILE_DIRECTORY       0
#endif

#if defined(O_SEARCH)
#define HKY_FILE_SEARCH          (O_SEARCH|HKY_FILE_DIRECTORY)

#elif defined(O_EXEC)
#define HKY_FILE_SEARCH          (O_EXEC|HKY_FILE_DIRECTORY)

#elif (HKY_HAVE_O_PATH)
#define HKY_FILE_SEARCH          (O_PATH|O_RDONLY|HKY_FILE_DIRECTORY)

#else
#define HKY_FILE_SEARCH          (O_RDONLY|HKY_FILE_DIRECTORY)
#endif

#endif /* HKY_HAVE_OPENAT */

#define HKY_FILE_DEFAULT_ACCESS 0644
#define HKY_FILE_OWNER_ACCESS 0600

#define hky_close_file close
#define hky_close_file_n "close()"

#define hky_delete_file(name)   unlink((const char*)name)
#define hky_delete_file_n   "unlink()"

static hky_inline ssize_t
hky_write_fd(hky_fd_t fd,void *buf,size_t n){
    return write(fd,buf,n);
}

#define hky_realpath(p, r)       (u_char *) realpath((char *) p, (char *) r)
#define hky_realpath_n           "realpath()"
#define hky_getcwd(buf, size)    (getcwd((char *) buf, size) != NULL)
#define hky_getcwd_n             "getcwd()"
#define hky_path_separator(c)  ((c)=='/')

#if defined(PATH_MAX)

#define HKY_HAVE_MAX_PATH        1
#define HKY_MAX_PATH             PATH_MAX

#else

#define HKY_MAX_PATH             4096

#endif

#define hky_write_console hky_write_fd

#define hky_linefeed(p)     *p++=LF;
#define HKY_LINEFEED_SIZE 1
#define HKY_LINEFEED "\x0a"

#define hky_stdout STDOUT_FILENO
#define hky_stderr STDERR_FILENO
#define hky_set_stderr(fd) dup2(fd,STDERR_FILENO)
#define hky_set_stderr_n "dup2(STDERR_FILENO)"

#endif // HKY_FILES_H_INCLUDED

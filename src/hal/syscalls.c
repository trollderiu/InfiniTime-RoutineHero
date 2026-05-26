#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

register char *stack_ptr asm("sp");

caddr_t _sbrk(int incr) {
    extern char end; // symbol from linker script
    static char *heap_end;
    char *prev_heap_end;

    if (heap_end == 0) {
        heap_end = &end;
    }
    prev_heap_end = heap_end;
    heap_end += incr;

    return (caddr_t)prev_heap_end;
}

void _exit(int status) { while(1); }
int _kill(int pid, int sig) { return -1; }
int _getpid(void) { return 1; }
int _write(int fd, const void *buf, size_t count) { return count; }
int _close(int fd) { return -1; }
int _lseek(int fd, int offset, int whence) { return 0; }
int _read(int fd, void *buf, size_t count) { return 0; }
int _fstat(int fd, struct stat *st) { st->st_mode = S_IFCHR; return 0; }
int _isatty(int fd) { return 1; }

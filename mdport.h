/*
    mdport.h  -  Machine Dependent Code for Porting Unix/Curses games

    Copyright (C) 2007 Nicholas J. Kisseberth
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
    3. Neither the name(s) of the author(s) nor the names of other contributors
       may be used to endorse or promote products derived from this software
       without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) AND CONTRIBUTORS ``AS IS'' AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR(S) OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
*/

#ifdef HAVE_CONFIG_H
#ifdef PDCURSES
#undef HAVE_UNISTD_H
#undef HAVE_LIMITS_H
#undef HAVE_MEMORY_H
#undef HAVE_STRING_H
#endif
#include "config.h"
#elif defined(__DJGPP__)
#define HAVE_SYS_TYPES_H 1
#define HAVE_PROCESS_H 1
#define HAVE_PWD_H 1
/* #define HAVE_UNISTD_H 1 */
#define HAVE_TERMIOS_H 1
#define HAVE_SETGID 1
#define HAVE_GETGID 1
#define HAVE_SETUID 1
#define HAVE_GETUID 1
#define HAVE_GETPASS 1
#define HAVE_SPAWNL 1
#define HAVE_ALARM 1
#elif defined(_WIN32)
#define HAVE_CURSES_H
#define HAVE_TERM_H
#define HAVE__SPAWNL
#define HAVE_SYS_TYPES_H
#define HAVE_PROCESS_H
#define HAVE_VSNPRINTF 1
#define HAVE_IO_H 1
#define HAVE_TIME_H 1
#elif defined(__CYGWIN__)
#define HAVE_VSNPRINTF 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_PWD_H 1
#define HAVE_PWD_H 1
#define HAVE_SYS_UTSNAME_H 1
#define HAVE_ARPA_INET_H 1
#define HAVE_UNISTD_H 1
#define HAVE_TERMIOS_H 1
#define HAVE_NCURSES_TERM_H 1
#define HAVE_ESCDELAY
#define HAVE_SETGID 1
#define HAVE_GETGID 1
#define HAVE_SETUID 1
#define HAVE_GETUID 1
#define HAVE_GETPASS 1
#define HAVE_GETPWUID 1
#define HAVE_WORKING_FORK 1
#define HAVE_ALARM 1
#define HAVE_SPAWNL 1
#define HAVE__SPAWNL 1
#else /* POSIX */
#define HAVE_TIME_H 1
#define HAVE_VSNPRINTF 1
#define HAVE_SYS_TYPES_H 1
#undef HAVE_PROCESS_H
#define HAVE_PWD_H 1
#define HAVE_PWD_H 1
#define HAVE_SYS_UTSNAME_H 1
#define HAVE_ARPA_INET_H 1
#define HAVE_UNISTD_H 1
#define HAVE_TERMIOS_H 1
#define HAVE_TERM_H 1
#define HAVE_CURSES_H 1
#define HAVE_TERM_H 1
#undef HAVE_NCURSES_TERM_H
#undef HAVE_ESCDELAY
#define HAVE_SETGID 1
#define HAVE_GETGID 1
#define HAVE_SETUID 1
#define HAVE_GETUID 1
#define HAVE_SETREUID 1
#define HAVE_SETREGID 1
#undef HAVE_SETRESUID
#undef HAVE_SETRESGID
#define HAVE_GETPASS 1
#define HAVE_GETPWUID 1
#define HAVE_WORKING_FORK 1
#undef HAVE_SPAWNL
#undef HAVE__SPAWNL
#undef HAVE_NLIST_H
#undef HAVE_NLIST
#undef HAVE_LOADAV
#ifndef _AIX
#define HAVE_GETLOADAVG 1
#endif
#define HAVE_ALARM 1
#endif

#ifdef NO_MDPORT_CURSES
#undef HAVE_CURSES_H
#undef HAVE_TERM_H
#endif

#ifdef __DJGPP__
#undef HAVE_GETPWUID /* DJGPP's limited version doesn't even work as documented */
#endif

int  md_chmod(char *filename, int mode);
char *md_crypt(char *key, char *salt);
int  md_dsuspchar();
int  md_erasechar();
char *md_gethomedir();
char *md_getusername();
int  md_getuid();
char *md_getpass(char *prompt);
int  md_getpid();
char *md_getrealname(int uid);
void md_init();
int  md_killchar();
void md_normaluser();
void md_raw_standout();
void md_raw_standend();
int  md_readchar();
int  md_setdsuspchar(int c);
int  md_shellescape();
void md_sleep(int s);
int  md_suspchar();
int  md_hasclreol();
int  md_unlink(char *file);
int  md_unlink_open_file(char *file, FILE *inf);
void md_tstpsignal();
void md_tstphold();
void md_tstpresume(void (*tstp)(int sig));
void md_ignoreallsignals();
void md_start_checkout_timer(int time);
void md_stop_checkout_timer(void);
void md_onsignal_autosave();
void md_onsignal_exit();
void md_onsignal_default();
int  md_issymlink(char *sp);
unsigned int md_htonl(unsigned int);
unsigned int md_ntohl(unsigned int);
int md_shellescape(void);
unsigned long md_memused(void);
int md_rand(void);
void md_srand(int seed);
extern char *md_getusername();
extern char *md_gethomedir();
extern void md_flushinp();
extern char *md_getshell();
extern char *md_gethostname();
extern void md_dobinaryio();
extern char *md_getpass();
extern char *md_crypt();
extern char *md_getroguedir();
extern void md_init();
extern int md_exec(const char *path, const char *arg0, const char *arg1);
#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

typedef struct EFILE
{
	FILE *fp;
	unsigned int efp_cksum;
	unsigned int efp_seed;
	int efp_iomode;
	int efp_error;
} EFILE;

void efclearerr(EFILE *efp);
void efseterr(EFILE *efp, int err);
int eferror(EFILE *efp);
EFILE *efopen(const char *filename, const char *mode);
int efclose(EFILE *efp);
size_t efread(void *ptr, size_t size, size_t nitems, EFILE *efp);
size_t efwrite(const void *ptr, size_t size, size_t nitems, EFILE *efp);
size_t efwriten(const void *ptr, size_t size, EFILE *efp);
size_t efreadn(void *ptr, size_t size, EFILE *efp);

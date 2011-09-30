/*
    save.c  -  save and restore routines
   
    Last Modified: Dec 30, 1990

    UltraRogue
    Copyright (C) 1984, 1985, 1986, 1987, 1990 Herb Chong
    All rights reserved.
    
    Based on "Advanced Rogue"
    Copyright (C) 1983, 1984 Michael Morgan, Ken Dalka and AT&T
    All rights reserved.

    Based on "Super-Rogue"
    Copyright (C) 1982, 1983 Robert D. Kindelberger
    All rights reserved.

    Based on "Rogue: Exploring the Dungeons of Doom"
    Copyright (C) 1980, 1981 Michael Toy, Ken Arnold and Glenn Wichman
    All rights reserved.
    
    See the file LICENSE.TXT for full copyright and licensing information.
*/

#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include "rogue.h"

typedef struct stat STAT;

extern char *sys_errlist[], version[];
extern int  errno;

char    *sbrk();

STAT    sbuf;

save_game()
{
    int savefd;
    int c;
    char    buf[2 * LINELEN];

    /*
     * get file name
     */
    mpos = 0;
    if (file_name[0] != '\0') {
	msg("Save file (%s)? ", file_name);
	do {
	    c = getchar() & 0177;
	    if (c == ESCAPE)
		return (0);
	} while (c != 'n' && c != 'N' && c != 'y' && c != 'Y');
	mpos = 0;
	if (c == 'y' || c == 'Y')
	    goto gotfile;
    }

    do {
	msg("File name: ");
	mpos = 0;
	buf[0] = '\0';
	if (get_str(buf, cw) == QUIT) {
	    msg("");
	    return FALSE;
	}
	strcpy(file_name, buf);
gotfile:
	wclear(hw);
	wmove(hw, LINES - 1, 0);
	wrefresh(hw);
	if ((savefd = creat(file_name, 0400)) == -1)
	    msg(sys_errlist[errno]);    /* fake perror() */
    } while (savefd == -1);

    /*
     * write out encrpyted file (after a stat) The fwrite is to force
     * allocation of the buffer before the write
     */
    save_file(savefd);
    return TRUE;
}

save_file(savefd)
int savefd;
{
    wmove(cw, LINES - 1, 0);
    wrefresh(cw);
    fstat(savefd, &sbuf);
    write(savefd, "junk", 5);
    lseek(savefd, 0L, 0);
    encwrite(version, sbrk(0) - version, savefd);
    close(savefd);
}

restore(file, envp)
char    *file;
char    **envp;
{
    int infd;
    extern char **environ;
    char    *sp, *getenv();
    char    buf[2 * LINELEN];
    STAT    sbuf2;

    if (strcmp(file, "-r") == 0)
	file = file_name;
    if ((infd = open(file, 0)) < 0) {
	perror(file);
	return FALSE;
    }

    fflush(stdout);
    encread(buf, strlen(version) + 1, infd);
    if (strcmp(buf, version) != 0) {
	printf("Save Game Version: %s\nReal Game Version: %s\n",
	    buf, version);
	printf("Sorry, saved game is out of date.\n");
	return FALSE;
    }

    fstat(infd, &sbuf2);
    fflush(stdout);
    brk(version + sbuf2.st_size);
    lseek(infd, 0L, 0);
    encread(version, (int) sbuf2.st_size, infd);

    /*
     * we do not close the file so that we will have a hold of the inode
     * for as long as possible
     */

    mpos = 0;
    mvwprintw(cw, 0, 0, "%s: %s", file, ctime(&sbuf2.st_mtime));

    if (unlink(file) < 0) {
	printf("Cannot unlink file\n");
	return FALSE;
    }

    environ = envp;
    if ((sp = getenv("OPTIONS")) != NULL)
	parse_opts(sp);

    strcpy(file_name, file);

    setup();
    clearok(curscr, TRUE);
    touchwin(cw);
    noecho();
    nonl();
    srand(1234);
    playit();
}

/*****************************************************************
 *
 *           encwrite, encread: encoded read/write routines
 *
 * The encwrite/encread routines implement a synchronous stream Vernam
 * cipher using a linear congruential random number generator to
 * simulate a one-time pad.  The random seed is encoded and stored in
 * the file using data diffusion (putword,getword).
 *
 * Usage:
 *  encwrite (start, size, outfd);
 *  unsigned char *start; long size; int outfd;
 *
 *  encread (start, size, infd);
 *  unsigned char *start; long size; int infd;
 *
 * HISTORY
 *
 * 07-May-86  Michael Laman (laman) at NCR Rancho Bernardo
 *  Modified for UltraRogue.  Removed all uses of standard I/O from
 *  data encryption routines.  This is most portable solution around
 *  a bug of limiting how many times a game may be saved.
 *
 * 03-Mar-85  Michael Mauldin (mlm) at Carnegie-Mellon University
 *  Modified for UltraRogue.  Allowed multiple encwrites to a file by
 *  removing lseek calls and computing checksum while reading.
 *
 * 20-Dec-82  Michael Mauldin (mlm) at Carnegie-Mellon University
 *  Created as a replacement for the encwrite/encread in
 *  Rogue 5.2, which are very easily broken.
 *
 *****************************************************************/

/* Constants for key generation */
#define OFFSET  667818L
#define MODULUS 894871L
#define MULT    2399L
#define ENCHAR  ((unsigned char) ((seed= ((seed*MULT+OFFSET)%MODULUS)) >> 10))

/* Constants for checksumming */
#define INITCK  1232531L
#define CKMOD   6506347L

struct wb {
    int w1, w2, w3, w4;
};

/*****************************************************************
 * Encwrite: write a buffer to a file using data encryption
 *****************************************************************/

encwrite(start, size, outfd)
unsigned char   *start;
long    size;
int outfd;
{
    unsigned char   *buf_p, *end_p;
    int ch;
    long    cksum = INITCK, cnt, seed;
    char    buffer[512 * 20];

    srandom(time((long *) 0));  /* Build a random seed */
    seed = (random() & 0x7fff);

    putword(seed, outfd);       /* Write the random seed */
    putword(size, outfd);       /* Write the file length */

# ifdef DEBUG
    fprintf(stderr, "Encwrite: size %ld, seed %ld.\n", size, seed);
# endif

    /* Now checksum, encrypt, and write out the buffer. */
    while (size) {
	buf_p = (unsigned char *) buffer;
	cnt = min(size, sizeof buffer);
	end_p = (unsigned char *) &buffer[cnt];
	while (buf_p < end_p) {
	    cksum = ((cksum << 8) + (ch = *start++)) % CKMOD;
	    *buf_p++ = ch ^ ENCHAR;
	}
	if (write(outfd, buffer, cnt) != cnt)
	    return 0;
	size -= cnt;
    }

    putword(cksum, outfd);  /* Write out the checksum */

# ifdef DEBUG
    fprintf(stderr, "Checksum is %ld.\n", cksum);
# endif
    return 1;
}

/*****************************************************************
 * Encread: read a block of encrypted text from a file descriptor
 *****************************************************************/

encread(start, size, infd)
unsigned char   *start;
long    size;
int infd;
{
    int length, ch;
    int stored;
    long    seed, cnt, getword(), cksum = INITCK;

    seed = getword(infd);
    stored = getword(infd);

# ifdef DEBUG
    fprintf(stderr, "Encread: size %ld, seed %ld, stored %ld.\n",
	size, seed, stored);
# endif

    if ((length = read(infd, start, (int) min(size, stored))) > 0) {
	for (cnt = length; cnt--;) {
	    ch = (unsigned char) (*start++ ^= ENCHAR);
	    cksum = ((cksum << 8) + ch) % CKMOD;
	}

	if ((length == stored) && (getword(infd) != cksum)) {
# ifdef DEBUG
	    fprintf(stderr, "Computed checksum %ld is wrong.\n",
		cksum);
# else
	    fprintf(stderr, "Sorry, file has been touched.\n");
# endif
	    while (length--)
		*--start = '\0';    /* Zero the buffer */
	}
    }

    return (length);
}

/*****************************************************************
 * putword: Write out an encoded int word
 *****************************************************************/

putword(word, file)
long    word;
int file;
{
    struct wb   w;

    w.w1 = rand();
    w.w2 = rand();
    w.w3 = rand();
    w.w4 = w.w1 ^ w.w2 ^ w.w3 ^ word;

    return write(file, (char *) &w, sizeof w);
}

/*****************************************************************
 * getword: Read in an encoded int word
 *****************************************************************/
long getword(fd)
int fd;
{
    struct wb   w;

    if (read(fd, (char *) &w, sizeof w) == sizeof w)
	return (w.w1 ^ w.w2 ^ w.w3 ^ w.w4);
    else
	return (0);
}

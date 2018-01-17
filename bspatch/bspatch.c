/*-
 * Copyright 2003-2005 Colin Percival
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted providing that the following conditions 
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "bzlib.h"
#include "xmd5.h"

#define O_RDONLY        0x0000
#define O_WRONLY        0x0001
#define O_CREAT         0x0100
#define O_TRUNC         0x0200

#define	CTRL_BLOCK_OFFSET	40
#define DIFF_BLOCK_OFFSET	48
#define NEW_SIZE_OFFSET		56
#define HEADER_SIZE			64

typedef int  ssize_t;
typedef long off_t;

void err(int code, const char *fmt, ...)
{
	va_list args; va_start(args, fmt);
	vprintf_s(fmt, args);
	va_end(args);
	exit(code);
}

static off_t offtin(u_char *buf)
{
	off_t y;

	y=buf[7]&0x7F;
	y=y*256;y+=buf[6];
	y=y*256;y+=buf[5];
	y=y*256;y+=buf[4];
	y=y*256;y+=buf[3];
	y=y*256;y+=buf[2];
	y=y*256;y+=buf[1];
	y=y*256;y+=buf[0];

	if(buf[7]&0x80) y=-y;

	return y;
}

int main(int argc,char * argv[])
{
	FILE * f, * cpf, * dpf, * epf;
	BZFILE * cpfbz2, * dpfbz2, * epfbz2;
	int cbz2err, dbz2err, ebz2err;
	FILE* fd;
	ssize_t oldsize,newsize;
	ssize_t bzctrllen,bzdatalen;
	u_char header[64],buf[8];
	char md5sum[33];
	u_char *old, *newb;
	off_t oldpos,newpos;
	off_t ctrl[3];
	off_t lenread;
	off_t i;

	if(argc!=4) err(1,"usage: %s oldfile newfile patchfile\n",argv[0]);

	/* Open patch file */
	if ((f = fopen(argv[3], "rb")) == NULL)
		err(1, "fopen(%s)", argv[3]);

	/*
	File format:
		0	8	"BSDIFF40"
		8	8	X
		16	8	Y
		24	8	sizeof(newfile)
		32	X	bzip2(control block)
		32+X	Y	bzip2(diff block)
		32+X+Y	???	bzip2(extra block)
	with control block a set of triples (x,y,z) meaning "add x bytes
	from oldfile to x bytes from the diff block; copy y bytes from the
	extra block; seek forwards in oldfile by z bytes".
	*/

	/* Read header */
	if (fread(header, 1, HEADER_SIZE, f) < HEADER_SIZE) {
		if (feof(f))
			err(1, "Corrupt patch 1\n");
		err(1, "fread(%s)", argv[3]);
	}

	/* Check for appropriate magic */
	if (memcmp(header, "BSDIFF40", 8) != 0)
		err(1, "Corrupt patch BSDIFF40\n");

	/* Read lengths from header */
	bzctrllen=offtin(header+CTRL_BLOCK_OFFSET);
	bzdatalen=offtin(header+DIFF_BLOCK_OFFSET);
	newsize=offtin(header+NEW_SIZE_OFFSET);
	//printf("%x %x %x\n", bzctrllen, bzdatalen, newsize);
	if((bzctrllen<0) || (bzdatalen<0) || (newsize<0))
		err(1,"Corrupt patch\n");
	if (fseek(f, 40, SEEK_SET))
		err(1, "fseek");
	if (md5sum_fp_to_string(f, md5sum) == 0)
		err(1, "md5sum");
	if (memcmp(md5sum, header+8, 32) != 0)
		err(1, "md5sum is not eq");

	/* Close patch file and re-open it via libbzip2 at the right places */
	if (fclose(f))
		err(1, "fclose(%s)", argv[3]);
	if ((cpf = fopen(argv[3], "rb")) == NULL)
		err(1, "fopen(%s)", argv[3]);
	if (fseek(cpf, HEADER_SIZE, SEEK_SET))
		err(1, "fseeko(%s, %lld)", argv[3],
		    (long long)HEADER_SIZE);
	if ((cpfbz2 = BZ2_bzReadOpen(&cbz2err, cpf, 0, 0, NULL, 0)) == NULL)
		err(1, "BZ2_bzReadOpen, bz2err = %d", cbz2err);
	if ((dpf = fopen(argv[3], "rb")) == NULL)
		err(1, "fopen(%s)", argv[3]);
	if (fseek(dpf, HEADER_SIZE + bzctrllen, SEEK_SET))
		err(1, "fseeko(%s, %lld)", argv[3],
		    (long long)(HEADER_SIZE + bzctrllen));
	if ((dpfbz2 = BZ2_bzReadOpen(&dbz2err, dpf, 0, 0, NULL, 0)) == NULL)
		err(1, "BZ2_bzReadOpen, bz2err = %d", dbz2err);
	if ((epf = fopen(argv[3], "rb")) == NULL)
		err(1, "fopen(%s)", argv[3]);
	if (fseek(epf, HEADER_SIZE + bzctrllen + bzdatalen, SEEK_SET))
		err(1, "fseeko(%s, %lld)", argv[3],
		    (long long)(HEADER_SIZE + bzctrllen + bzdatalen));
	if ((epfbz2 = BZ2_bzReadOpen(&ebz2err, epf, 0, 0, NULL, 0)) == NULL)
		err(1, "BZ2_bzReadOpen, bz2err = %d", ebz2err);

	if(((fd=fopen(argv[1],"rb"))==NULL) ||
		(fseek(fd,0,SEEK_END)!=0) ||
		((oldsize=ftell(fd))<0) ||
		((old=malloc(oldsize+1))==NULL) ||
		(fseek(fd,0,SEEK_SET)!=0) ||
		(fread(old,1,oldsize,fd)!=oldsize) ||
		(fclose(fd)==-1)) err(1,"%s",argv[1]);
	if((newb=malloc(newsize+1))==NULL) err(1,NULL);

	oldpos=0;newpos=0;
	while(newpos<newsize) {
		/* Read control data */
		for(i=0;i<=2;i++) {
			lenread = BZ2_bzRead(&cbz2err, cpfbz2, buf, 8);
			if ((lenread < 8) || ((cbz2err != BZ_OK) &&
			    (cbz2err != BZ_STREAM_END)))
				err(1, "Corrupt patch\n");
			ctrl[i]=offtin(buf);
		};

		/* Sanity-check */
		if(newpos+ctrl[0]>newsize)
			err(1,"Corrupt patch\n");

		/* Read diff string */
		lenread = BZ2_bzRead(&dbz2err, dpfbz2, newb + newpos, ctrl[0]);
		if ((lenread < ctrl[0]) ||
		    ((dbz2err != BZ_OK) && (dbz2err != BZ_STREAM_END)))
			err(1, "Corrupt patch\n");

		/* Add old data to diff string */
		for(i=0;i<ctrl[0];i++)
			if((oldpos+i>=0) && (oldpos+i<oldsize))
				newb[newpos+i]+=old[oldpos+i];

		/* Adjust pointers */
		newpos+=ctrl[0];
		oldpos+=ctrl[0];

		/* Sanity-check */
		if(newpos+ctrl[1]>newsize)
			err(1,"Corrupt patch\n");

		/* Read extra string */
		lenread = BZ2_bzRead(&ebz2err, epfbz2, newb + newpos, ctrl[1]);
		if ((lenread < ctrl[1]) ||
		    ((ebz2err != BZ_OK) && (ebz2err != BZ_STREAM_END)))
			err(1, "Corrupt patch\n");

		/* Adjust pointers */
		newpos+=ctrl[1];
		oldpos+=ctrl[2];
	};

	/* Clean up the bzip2 reads */
	BZ2_bzReadClose(&cbz2err, cpfbz2);
	BZ2_bzReadClose(&dbz2err, dpfbz2);
	BZ2_bzReadClose(&ebz2err, epfbz2);
	if (fclose(cpf) || fclose(dpf) || fclose(epf))
		err(1, "fclose(%s)", argv[3]);

	/* Write the new file */
	if ((fd = fopen(argv[2], "wb")) == NULL)
		err(1, "%s", argv[2]);
	if (fwrite(newb, newsize, 1, fd) != 1)
		err(1, "fwrite(%s)", argv[2]);
	if (fclose(fd))
		err(1, "fclose");

	free(newb);
	free(old);

	printf("Success");
	return 0;
}

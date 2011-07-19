#include <cprint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define FMTMARK 	"\e[%dm"

int cprintf(FgColor fc, BgColor bc, Ptype t
		, const char *fmt,...)
{
	va_list ap;
	va_start(ap, fmt);
	int r = fvcprintf(STDOUT_FILENO, fc, bc, t, fmt, ap);
	va_end(ap);
	return r;
}

int fcprintf(int fd, FgColor fc, BgColor bc, Ptype t
		, const char *fmt,...)
{
	va_list ap;
	va_start(ap, fmt);
	int r = fvcprintf(fd, fc, bc, t, fmt, ap);
	va_end(ap);
	return r;
}

int fvcprintf(int fd, FgColor fc, BgColor bc, Ptype t
		, const char *fmt, va_list ap)
{
	char *nfmt = NULL;
	int fmt_len = strlen(fmt);
	int nfmt_len = fmt_len + 25;
	int idx = 0;
	
	nfmt = malloc(sizeof(char) * nfmt_len);
	memset(nfmt, '\0', nfmt_len * sizeof(char));
	
	if(fc != UNSET_F){
		idx += sprintf(nfmt + idx, FMTMARK, fc);
	}
	
	if(bc != UNSET_B){
		idx += sprintf(nfmt + idx, FMTMARK, bc);
	}
	
	if(t != UNSET_T){
		idx += sprintf(nfmt + idx, FMTMARK, t);
	}
	
	idx += sprintf(nfmt + idx, "%s", fmt);
	sprintf(nfmt + idx, "\e[0m");
	
	//maybe this is enough.
	int strlen = 40960;
	char *str = malloc(strlen * sizeof(char));
	memset(str, '\0', strlen * sizeof(char));
	
	/*
	 * print the new fmt
	 */
	vsnprintf(str, strlen, nfmt, ap);
	free(nfmt);
	
	int rt = -1;
	if(-1 == (rt = write(fd, str, strlen))){
		free(str);
		return -1;
	}
	free(str);
	
	return rt;
}

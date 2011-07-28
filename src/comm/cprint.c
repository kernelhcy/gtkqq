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
	char nfmt[500];
	int idx = 0;
	
	if(fc != UNSET_F){
		idx += snprintf(nfmt + idx, 500 - idx, FMTMARK, fc);
	}
	
	if(bc != UNSET_B){
		idx += snprintf(nfmt + idx, 500 -idx, FMTMARK, bc);
	}
	
	if(t != UNSET_T){
		idx += snprintf(nfmt + idx, 500 -idx, FMTMARK, t);
	}
	
	idx += snprintf(nfmt + idx, 500 - idx, "%s", fmt);
	snprintf(nfmt + idx, 500 - idx, "\e[0m");
	
	char str[5000];
	/*
	 * print the new fmt
	 */
	int strlen = vsnprintf(str, 5000, nfmt, ap);
	
	int rt = -1;
	if(-1 == (rt = write(fd, str, strlen))){
		return -1;
	}
	
	return rt;
}

#ifndef STRINGFOOS_H
#define STRINGFOOS_H

#include <matypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <fstream>
#include <sstream>
#include <consts.h>


typedef int (*pFNic)(int);

inline int noting(int c)
{
    return c;
}

//’ to ‘int* (*)(int)’|

inline char* str_up2chr(char*& p, char peos, pFNic tou=0)
{
    register char* pp = p;
    register char eos = peos;
    while(*p && *p!=eos) {
        if(tou) {
            *p = (char)tou((int)*p);
        }
        ++(p);
    };
    if(*p!=0) { //!eos
        *p=0;
        ++(p);
    }
    return pp;
}
/*
inline kchar* str_up2any(char** p, kchar* eoss , pFNic tou=0)
{
    char* pp = *p;
    while(**p && !_ttstrchr(eoss,**p)) {
        if(tou) {
            **p = (char)tou((int)**p);
        }
        ++(*p);
    };
    if(**p!=0) { //!eos
        **p=0;
        (*p+=strlen(eoss));
    }
    return pp;
}
*/

inline kchar* str_after(char* buff, kchar*  key, char delim)
{
    char* p = (char*)strstr(buff, key);
    if(p){
        p += strlen(key);

        while(*p == delim && *p)p++;
        kchar* ps = p;
        while(*p != delim && *p)p++;
        if(*p)
           *p=0;
        return ps;
    }
    return 0;
}

inline kchar* str_up2any(char*& p, kchar* peoss , pFNic tou=0)
{
    register char * pp = p;
    register kchar* eoss = peoss;
    while(*p!='\0' && !strchr(eoss, *p) )
    {
        if(tou){
            *p = tou((int)*p);
        }
        ++(p);
    };
     if(*p!=0){  //!eos
        *p=0;
        p+=strlen(eoss);
    }
    return pp;
}

inline kchar* str_deleol(char* p)
{
    char* end = p + strlen(p)-1;
    while((*end=='\r' || *end=='\n') && end > p )
        *(end--)='\0';
    return p;
}

inline kchar* str_up2str(char** p, kchar* pss)
{
    register char* pp = *p;
    register kchar* ss = pss;
    register char* ps = _ttstrstr(pp, ss);
    if(ps!=0){
        *ps = 0;
        *p = ps + strlen(ss);
    }else{//put it to eos
        *p = pp + (strlen(pp));
    }
    return pp;
}


inline kchar* str_printf(char* d, int len, char* f, ...)
{
    va_list args;
    va_start(args, f);
    ::vsnprintf(d, len, f, args);
    va_end(args);
    return d;
}

inline void str_int2mode (char* retval, int mode)
{
    int j=0;
    static char perms[]="xwrxwrxwr-";
    for(int k=0x1;k<0x100; k<<=1)
    {
        if(mode & k)
            retval[j]=perms[j];
        else
            retval[j]='-';
        ++j;
    }
    retval[j]=0;
}

#define DEC2INT(x) (isdigit(x) ? x - '0' : x - 'W')
inline size_t str_urldecode(char* dst, kchar* src, bool form)
{
    size_t len=0;
    while(*src) {
        kchar& c = *src;
        if(c=='%' && *(src+1) && *(src+2)) {
            if(isxdigit(*(src+1)) &&
                    isxdigit(*(src+2))) {
                *dst++ = (char)(DEC2INT(::tolower(*(src+1)))<<4 | DEC2INT(::tolower(*(src+2))));
                ++len;
            }
        } else if(form && c=='+') {
            *dst++ = ' ';
            ++len;
        } else {
            *dst++=c;
            ++len;
        }
        ++src;
    }
    *dst=0;
    return len;
}


inline kchar* str_getfile_ext(kchar* pf, kchar* def)
{
    kchar* prev = strrchr(pf,'.');
    if(0 == prev) return def;
    return prev;
}




//-----------------------------------------------------------------------------
// replaces in string d all u's with t's
//-----------------------------------------------------------------------------
// utility function
inline kchar* str_crepl(char* d, kchar w, char t)
{
    if(d==0)return 0;
    while(*d) {
        if(*d==w)
            *d=t;
        ++d;
    }
    *d = 0;
    return d;
}

//-----------------------------------------------------------------------------
// copy s in d excluding all occ's
//-----------------------------------------------------------------------------
inline kchar* str_ccpy(char* d, kchar* s, char occ)
{
    if(s==0)return 0;
    while(*s && *s!=occ)
        *d++=*s++;
    *d = 0;
    return s;
}

inline void str_prepline(char* s)
{
    while(*s++)
    {
        if(*s=='\t'||*s=='\r'||*s=='\n')
            *s=' ';
    }
}

//-----------------------------------------------------------------------------
// copy s in d excluding any of occ's
//-----------------------------------------------------------------------------
inline char* str_scpy(char* d, char* s, kchar* occ)
{
    if(s==0)return 0;
    while(*s && !_ttstrchr(occ,*s))
        *d++=*s++;
    *d=0;
    return s;
}

//-----------------------------------------------------------------------------
// trims all c's from p
//-----------------------------------------------------------------------------
inline void  str_trimall(char* p, char c)
{
    register char *d=p;
    while(*p) {
        if(*p!=c) {
            *d++=*p;
        }
        ++p;
    }
    *d=0;
}

inline kchar* str_lrtim(char* p)
{
    kchar* orig=p;
    char* ps = p;
    while(*p==' ')++p;
    while(*p){*ps++=*p++;}
    *ps = 0;--ps;
    while(ps > orig && *ps==' ')
    {
        *ps = '\0';
        --ps;
    }
    return p;
}



inline int str_cmp(kchar* s1, kchar* s2)
{
    return strcmp(s1,s2);

}

inline kchar* str_days(int idx)
{
    static char* days[]={_CC("Sun"), _CC("Mon"), _CC("Tue"),
                         _CC("Wed"), _CC("Thu"), _CC("Fri"), _CC("Sat")};
    return days[idx];
}

inline kchar* str_months(int idx)
{
    static char* months[]={_CC("Jan"), _CC("Feb"), _CC("Mar"),
                           _CC("Apr"), _CC("May"), _CC("Jun"),
                           _CC("Jul"), _CC("Aug"), _CC("Sep"),
                           _CC("Oct"), _CC("Nov"), _CC("Dec")};
    return months[idx];
}

inline kchar*  str_time()
{
    static char timestamp[128];

    time_t  curtime = time(0);
    strcpy(timestamp, ctime(&curtime));
    char *pe = strchr(timestamp,'\r');
    if(pe)*pe=0;
    pe = strchr(timestamp,'\n');
    if(pe)*pe=0;
    return timestamp;
}



#define FT() FTR tr(__PRETTY_FUNCTION__)

#endif // STRINGFOOS_H

/**
# Copyright (C) 2012-2014 Chincisan Octavian-Marius(mariuschincisan@gmail.com) - getic.net - N/A
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
*/

#include <assert.h>
#include "config.h"
#include "strops.h"

//ugli string ops wit some garbage from previous ...
namespace so{

    bool    strbegin(const char* content, const char* strset)
    {
        const char* pr = ::strstr(content, strset);
        if( pr == (char*)content)
            return 1;
        char locoset[128];
        ::strcpy(locoset, (char*)strset);
        strupper(locoset);
        return ::strstr(content, locoset) == content;
    }

    const char*   stristr(const char* content, const char* strset)
    {
        const char* ret = ::strstr(content, strset);
        if(ret)
            return ret;
        char locoset[1024];
        ::strcpy(locoset, (char*)strset);
        strupper(locoset);
        return ::strstr(content, locoset);
    }
    const char*   strstr(const char* content, const char* strset)
    {
        const char* ret = ::strstr(content, strset);
        if(ret)
            return ret;
        char locoset[512];
        ::strcpy(locoset, (char*)strset);
        strupper(locoset);
        return ::strstr(content, locoset);
    }
    void   trimcrlf(char* p)
    {
        int   d  = 0;
        char* pd = p;
        while(*p)
        {
            if(*p!='\r' && *p!='\n')
            {
                pd[d++]=*p;
            }
            ++p;
        }
        pd[d++]=0;
    }

    void strlower(char* a)
    {
	    char* pc = a;
        int    diff = 'a'-'A';
		while(*pc)
		{

			if(*pc>='A'&& *pc<='Z')
				*pc += diff;
			++pc;
		}
    }

	void strupper(char* a)
	{
	    char* pc = a;
        int diff = 'a'-'A';
		while(*pc)
		{
			if(*pc>='a'&& *pc<='z')
				*pc -= diff;
			++pc;
		}

	}

    bool isdigit(char c)
    {
        return (c >= '0' && c <= '9');
    }
    char *strnew(const char *psz)
    {
        char *res = new char[strlen(psz) + sizeof(char)];
        ::strcpy(res, (char*)psz);
        return res;
    }
    int chartohex (char c)
    {
        if (c >= '0' && c <= '9')
            return (c - '0');
        else if (c >= 'a' && c <= 'f')
            return (c - 'a' + 10);
        else if (c >= 'A' && c <= 'F')
            return (c - 'A' + 10);
        else
            return -1;
    }


    char* strlastchr(char* s, char p)
    {
        char* pdot = s;
        char* pdp = pdot;
        while(1){
            pdot = pdp;
            pdp = strchr(pdot,p);
            if(pdp)++pdp;
            else break;
        }
        return pdot-1;
    }

    char* strprevchr(char* start, char* s, char p)
    {
        char* cp = s;
        while(cp!=start && *cp != p)--cp;
        return cp;
    }

    char* str_chr(char* s, char p)
    {
        while(*s && *s!=p)++s;
        return s;
    }

    int   str_count(const char* s, char cc)
    {
        int c = 0;
        const char* p = s;
        while(*p)if(*p++==cc)++c;
        return c;
    }

    char* strnchr(char* s, char p, int pos)
    {
        char* pdot = s;
        char* pdp = pdot;
        while(1){
            pdot = pdp;
            pdp = strchr(pdot,p);
            if(pdp && --pos)
                ++pdp;
            else
                break;
        }
        return pdp ? pdp : pdot-1;
    }

    const char* strccpy(char* d, const char* s, char occ)
    {
        while(*s!=occ && *s)
            *d++=*s++;
        *d = 0;
        return s;
    }

    char* strscpy(char* d, char* s, const char* occ)
    {
        while(*s && !strchr(occ,*s))
            *d++=*s++;
        *d=0;
        return s;
    }

    bool is_alnum(const char c)
    {
        return  (c>='0' && c<='9') ||  (c<='z' && c>='A');
    }

    char* strskipalnum(char *s)
    {
        while(*s && *s>0 && is_alnum(*s))++s;
        return s;
    }

    bool  strcat(char* d, char*s, int szl)
    {
        if((strlen(d) + strlen(s)) < (size_t)(szl-1))
        {
            ::strcat(d,s);
            return 1;
        }
        return 0;
    }

    void  keep_alphanum(char* p)
    {
        int   d  = 0;
        char* pd = p;
        while(*p)
        {
            if(is_alnum(*p))
            {
                pd[d++]=*p;
            }
            ++p;
        }
        pd[d++]=0;
    }

	void replace_urlfmt(char* p)
	{
	    while(*p)
        {
            if(*p=='%')
				*p='_';
            ++p;
        }
        *p=0;
	}

    void  format_ascii(char* p){
        while(*p)
        {
            if(*p=='/')
                *p='(';
			else if(*p=='\\')
				*p=')';
            else if(*p=='?')
                *p='[';
            else if(*p=='*')
                *p='^';
            else if(*p=='&')
                *p=',';
            else if(*p=='\"')
                *p='\'';
			else if(*p=='&')
				*p='+';
            ++p;
        }
        *p=0;
    }

    char* trimfront(char* p)
    {
        while(*p && (*p==' ' || *p=='\t'))
            ++p;
        return p;
    }

    void   trimrightsp(char* p)
    {
        char* plast = p+strlen(p)-1;
        while(plast!=p && (*plast==' '||*plast=='\t'))
            --plast;
        *(plast+1)=0;
    }

    void  trimright(char* p, char c)
    {
        char* plast = p+strlen(p)-1;
        while(plast!=p && *plast==c)
            --plast;
        *(plast+1)=0;
    }

    void  trimeol(char* p)
    {
        char* plast = p+strlen(p)-1;
        while(plast!=p && (*plast=='\r' || *plast=='\n'))
            --plast;
        *(plast+1)=0;
    }


    void  trimnonsql(char* p)
    {
        char *d=p;

        while(*p)
        {
            if(*p==',')
                *d++=' ';
            else if((*p >= 'a' && *p <= 'z') || (*p >='A' && *p<= 'Z'))
            {
                *d++=*p;
            }
            ++p;
        }
        *d=0;
    }

    void  trimall(char* p, char c)
    {
        char *d=p;
        while(*p)
        {
            if(*p!=c)
            {
                *d++=*p;
            }
            ++p;
        }
        *d=0;
    }

    char* strcpy(char* d, char* s, size_t destsz)
    {
        if(destsz){
            while(*s && --destsz)
                *d++=*s++;
            *d=0;
        }
        else
        {
            while(*s)
                *d++=*s++;
            *d=0;
        }
        return d;
    }

    bool strend(const char* hay, const char* ned)
    {
        size_t hl = strlen(hay);
        size_t ed = strlen(ned);
        if(hl > ed)
        {
            hay += (hl-ed);
            return !::strcmp(hay, ned);
        }
        return 0;
    }

    void    manglename(char* p)
    {
        while(*p)
        {
            if(*p=='?')
                *p='#';
            ++p;
        }
        *p = 0;
    }

    int  str_findlast(const char* a, char chr)
    {
        int pla = (int)strlen(a);
        while(--pla>-1)
        {
            if(a[pla]==chr)
                return (int)pla;
        }
        return 0;
    }

    //-----------------------------------------------------------------------------
    void replace_all(string &content, const string &from, const string &to)
    {
        size_t i = 0;
        size_t j;
        while((j = content.find(from, i)) != string::npos)
        {
            content.replace(j, from.size(), to);
            i = j + to.size();
        }
    }

    bool str_mysql_malf(const string& in)
    {
       return ( in.find("\r")!=string::npos ||
                in.find("\n")!=string::npos ||
                in.find("\"")!=string::npos ||
                in.find("=")!=string::npos ||
                in.find("`")!=string::npos ||
                in.find("%")!=string::npos ||
                in.find("(")!=string::npos ||
                in.find(")")!=string::npos ||
                in.find(",")!=string::npos ||
                in.find("\'")!=string::npos ||
                in.find("\\")!=string::npos);
    }

    //-----------------------------------------------------------------------------
    void url_normalize(string& sname)
    {
        if(sname.find("://")==string::npos )
        {
            string loco=sname;
            sname=string("http://") + loco; // perhaps
        }
        so::replace_all(sname,"www.","");
    }


    string& strip_tags(string &html)
    {
        bool inflag = false;
        bool done = false;
        size_t i, j;
        while (!done)
        {
            if (inflag)
            {
                i = html.find('>');
                if(i==(size_t)-1)
                {
                    if (i != string::npos)
                    {
                        inflag = false;
                        html.erase(0, i+1);
                    }
                    else
                    {
                        done = true;
                        html.erase();
                    }
                }
            }
            else
            {
                i = html.find('<');
                if (i != string::npos)
                {
                    j = html.find('>');
                    if (j == string::npos)
                    {
                        inflag = true;
                        done = true;
                        html.erase(i);
                    }
                    else
                    {
                        html.erase(i, j-i+1);
                    }
                }
                else
                {
                    done = true;
                }
            }
        }
        return html;
    }

    //-----------------------------------------------------------------------------
    bool bad_link(string  s)
    {
        if(s.find("(")!=string::npos ||
           s.find("{")!=string::npos ||
           s.find("#")!=string::npos ||
           s.find(",")!=string::npos ||
           s.find("|")!=string::npos ||
           s.find("@")!=string::npos ||
           s.find("+")!=string::npos)
           return true;

        so::lower_it(s);
        if(s.find(".mpg")!=string::npos ||
           s.find(".mp3")!=string::npos ||
           s.find(".mp")!=string::npos ||
           s.find(".ra")!=string::npos ||
           s.find(".og")!=string::npos ||
           s.find(".gz")!=string::npos ||
           s.find(".zip")!=string::npos ||
           s.find(".jpg")!=string::npos ||
           s.find(".gif")!=string::npos ||
           s.find(".png")!=string::npos ||
           s.find(".avi")!=string::npos ||
           s.find(".wav")!=string::npos ||
           s.find(".tar")!=string::npos||
           s.find(".pdf")!=string::npos ||
           s.find(".bmp")!=string::npos ||
           s.find(".7z")!=string::npos ||
           s.find(".ico")!=string::npos)
           return true;

        return false;
   }

    void clean_uri(string& uri)
    {
        //so::lower_it(uri);
        //TODO place them in config
        pcrecpp::RE("&amp(:|;)").GlobalReplace("&", &uri);
        pcrecpp::RE("(\\?|&)(m|c|i|zenid|oscsid|osCsid|SID|sid|s|S|jsessionid|sessionid)=[0-9a-zA-Z]+").GlobalReplace("", &uri);
        pcrecpp::RE("(\\?|&)[0-9a-zA-Z]+(m|c|i|zenid|oscsid|osCsid|SID|sid|s|S|jsessionid|sessionid)=[0-9a-zA-Z]+").GlobalReplace("", &uri);
        pcrecpp::RE("(\\?|&)^(m|c|i|zenid|oscsid|osCsid|SID|sid|s|S|jsessionid|sessionid)=[0-9a-zA-Z]+").GlobalReplace("", &uri);
        pcrecpp::RE("$\\?").GlobalReplace("", &uri);
        pcrecpp::RE("#+").GlobalReplace("", &uri);
        pcrecpp::RE("^/").GlobalReplace("", &uri);
        pcrecpp::RE("index\.(php|html|asp|htm)$").GlobalReplace("", &uri);
        pcrecpp::RE("[/'.]+$").GlobalReplace("", &uri);
        pcrecpp::RE("[/\".]+$").GlobalReplace("", &uri);
        pcrecpp::RE("[\/]+$").GlobalReplace("", &uri);
    }

    bool is_common(const string& word)
    {
        // TODO should read from some file
        static string reject="ok in at to eh duh ha yes this wmv tar"
                             "that where here not or and under var gz"
                             "then time other near by lol own org"
                             "contact mail website if out dif href html asp gif png img net"
                             "web w3c with such because neither either way "
                             "when well who what will do done which was there here because tell top"
                             "var int function copy http zip php pdf"
                             "www size height width while above below"
                             "under you me his our their him mine your upper left right from into";
        if(word.size()<3)
            return false;
        if(reject.find(word)!=string::npos)
            return false;
        return true;

    }


    bool prep_kws(string& word)
    {

        if(word.find("<")!=string::npos && word.find(">")!=string::npos)
            return false;
        if(word.find("{")!=string::npos && word.find("}")!=string::npos)
            return false;

        pcrecpp::RE("&(quote|amp|nbsp|copy|divide|cent|gt|lt|pound|reg|sect|trade|yen|aacute|iquest|quot|rdquo|lsquo|rsquo|euro|eacute|middot)").GlobalReplace(" ", &word);
        pcrecpp::RE("[^A-Za-z0-9-.]").GlobalReplace(" ", &word);
        pcrecpp::RE("[\r\n]").GlobalReplace("", &word);
        pcrecpp::RE("[\_\]+").GlobalReplace(" ", &word);
        pcrecpp::RE("[\\-\]+").GlobalReplace(" ", &word);
        pcrecpp::RE("\\\'").GlobalReplace("", &word);
        pcrecpp::RE("\\\"").GlobalReplace("", &word);
        pcrecpp::RE("[/\\/]").GlobalReplace(" ", &word);
        pcrecpp::RE("[/./]+").GlobalReplace(".", &word);
        pcrecpp::RE("[/,/]+").GlobalReplace(",", &word);
        pcrecpp::RE("[\\t\]+").GlobalReplace(" ", &word);
        pcrecpp::RE("[\ \]+").GlobalReplace(" ", &word);

        if(word.length()<2)
            return false;
        so::lower_it(word);
        return true;
    }

    void prep_dsc(string& val)
    {
        so::lower_it(val);
        pcrecpp::RE("/[^a-zA-Z0-9]+/").GlobalReplace(" ", &val);
        pcrecpp::RE("[\r\n]").GlobalReplace("", &val);
        pcrecpp::RE("[\,:;|]").GlobalReplace(" ", &val);
        pcrecpp::RE("[\ \]+").GlobalReplace(" ", &val);
        pcrecpp::RE("[\t\]+").GlobalReplace(" ", &val);
    }

    bool prep_kw(string& word)
    {
        pcrecpp::RE("^[\ ]+").GlobalReplace("", &word);
        pcrecpp::RE("^[\.]+").GlobalReplace("", &word);
        pcrecpp::RE("^[\-]+").GlobalReplace("", &word);
        pcrecpp::RE("[\ ]+$").GlobalReplace("", &word);
        pcrecpp::RE("[/\.]+$").GlobalReplace("", &word);
        if(word.length()<2)
            return false;
        return true;
    }


    void    url_parse(string& uri, string& proto, string& host, string& path,string& port, string& qry)
    {

        if(uri.compare(0,4,"http")!=0)
        {
            GLOGE("Uri for parsing has uparsable format: " << uri);
            return;
        }

        string* pwhat = &proto;
        string::const_iterator b = uri.begin();
        for(; b != uri.end(); ++b)
        {
            switch(*b)
            {
            case ':':
                if(host.empty())
                {
                    if(proto.empty())
                        return;
                    proto+="://";
                    b+=2;
                    pwhat=&host;
                }
                else
                {
                    if(proto.empty()||host.empty())
                        return;
                    pwhat=&port;
                    port+=":";//put in the port so we dont test it
                }
                break;
            case '/':
                if(path.empty())
                    pwhat=&path;
                else
                    pwhat->append(1,*b);
                break;
            case '?':
                pwhat=&qry;
                pwhat->append("?"); //
                break;
            default:
                pwhat->append(1,*b);
                break;
            }
        }
        if(uri[uri.length() - 1 ]=='/' )
        {
            uri = uri.substr(0,uri.length() - 1);
        }
    }

    //test some of the assrts. TODO....
    int64_t evalasset(const string& asset)
    {
        int64_t l=0;

        if(asset.find("rss")!=string::npos)  l |= 0x1;
        if(asset.find("js")!=string::npos)  l |= 0x2;
        if(asset.find("javascript")!=string::npos)  l |= 0x2;
        if(asset.find("zip")!=string::npos)  l |= 0x4;
        if(asset.find("mp")!=string::npos)  l |= 0x8;
        if(asset.find("ajax")!=string::npos)  l |= 0x10;
        if(asset.find("tar")!=string::npos)  l |= 0x20;
        if(asset.find("flv")!=string::npos)  l |= 0x40;
        return l;
    }


};

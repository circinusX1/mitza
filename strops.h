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

#ifndef __PARSER_H__
#define __PARSER_H__

#include <string>
#include <algorithm>
#include <string.h>
#include <pcrecpp.h>

#define __nill (void*)0


using namespace std;
namespace so
{
bool strbegin(const char* beg, const char* str);
bool strend(const char* hay, const char* str);
const char* strstr(const char* beg, const char* str);
const char* stristr(const char* beg, const char* str);
char *strnew(const char *psz);
bool isdigit(char c);
void strlower(char* a);
int chartohex (char c) ;
bool is_alnum(const char c);
void   trimrightsp(char* s);
char* strlastchr(char* s, char p);
char* strprevchr(char* s, char p);
char* strprevchr(char* start, char* s, char p);
char* str_chr(char* s, char p);
int   str_count(const char* s, char p);
void   trimcrlf(char* s);
char* strnchr(char* s, char p, int pos);
char* strcpy(char* d, char* s, int destsz=0);
const char* strccpy(char* d, const char* s, char occ);
char* strscpy(char* d, char* s, const char* occ);
char* strskipalnum(char *s) ;
bool  strcat(char* d, char* s, int szl);
void  keep_alphanum(char* p);
void  format_ascii(char* p);
char* trimfront(char* p);
void  trimall(char* p, char c);
void  trimright(char* p, char c);
void  trimnonsql(char* p);
void  trimeol(char* p);
void   manglename(char* file);
void strupper(char* a);
int  str_findlast(const char* a, char chr);
void replace_urlfmt(char*);
void replace_all(string &content, const string &from, const string &to);
void url_normalize(string& sname);
string& strip_tags(string &html);
bool str_mysql_malf(const string& in);
inline void lower_it(string& s)
{
    std::transform(s.begin(), s.end(), s.begin(),::tolower);
}

void    url_parse(string& uri, string& proto, string& host, string& path,string& port, string& qry);
inline  void rep_all(const char* w, const char* t, string& s)
{
    pcrecpp::RE(w).GlobalReplace(t, &s);
}
void clean_uri(string& uri);
bool bad_link(string s);
bool is_common(const string& word);
void prep_dsc(string& s);
bool prep_kws(string& word);
bool prep_kw(string& word);
int64_t evalasset(const string& asset);

};



#endif //

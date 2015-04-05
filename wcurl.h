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

#ifndef WCURL_H
#define WCURL_H

#include <assert.h>
#include <string.h>
#include <sys/socket.h>
#include <curl/curl.h> //sudo aptitude install libcurl-dev && sudo apt-get install libcurl4-gnutls-dev

using namespace std;

class wcurl
{
public:
    wcurl(const string& uer);
    ~wcurl();
    void reuse();
    int32_t header(string& storage);
    int32_t content(string& storage, bool with_hdr=false);
private:
    void _close();
    size_t _fetch_url();
    static size_t writeCallbackHdr(char* buf, size_t size, size_t nmemb, void* up);
    static size_t writeCallback(char* buf, size_t size, size_t nmemb, void* up);
private:
    string* _payload;
    string  _url;
    string  _redirect;
    bool    _used;
    bool    _bscript;
    string  _robots;
    CURL*   _curl;
    int     _socket;
public:
    string  _mime;
    string  _date;
    string  _ip;
    int32_t _code;
};

#endif //WCURL_H

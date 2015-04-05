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
#include <string.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <pcrecpp.h>
#include "strops.h"
#include "wcurl.h"
#include "main.h"

static string _dummy;

//-----------------------------------------------------------------------------
wcurl::wcurl(const string& url):_url(url),_used(false),_bscript(0)
{
    _curl = curl_easy_init();
}

//-----------------------------------------------------------------------------
wcurl::~wcurl()
{
    _close();
}

//-----------------------------------------------------------------------------
void wcurl::_close()
{
    if(_curl)
        curl_easy_cleanup(_curl);
    _curl = 0;

}

//-----------------------------------------------------------------------------
size_t wcurl::writeCallbackHdr(char* buf, size_t size, size_t nmemb, void* up)
{
    wcurl* pthis = reinterpret_cast<wcurl*>(up);
    pthis->_payload->append(buf, size*nmemb);
    return size*nmemb; //tell curl how many bytes we handled
}

//-----------------------------------------------------------------------------
size_t wcurl::writeCallback(char* buf, size_t size, size_t nmemb, void* up)
{
    wcurl* pthis = reinterpret_cast<wcurl*>(up);
    pthis->_payload->append(buf, size * nmemb);
    return size*nmemb; //tell curl how many bytes we handled
}

//-----------------------------------------------------------------------------
void wcurl::reuse()
{
    curl_easy_cleanup(_curl);
    _curl = curl_easy_init();
    _used=false;
}

//-----------------------------------------------------------------------------
int32_t wcurl::header(string& p)
{
    _payload=&p;
    assert(false==_used); //one query per instance

    //curl_easy_setopt( _curl, CURLOPT_CUSTOMREQUEST, "HEAD" );
    curl_easy_setopt(_curl, CURLOPT_NOBODY, 1L);

    curl_easy_setopt( _curl, CURLOPT_USERAGENT, _AGENT);
    curl_easy_setopt( _curl, CURLOPT_HEADER, 0 );
    curl_easy_setopt (_curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(_curl, CURLOPT_HEADERFUNCTION, &wcurl::writeCallbackHdr);
    curl_easy_setopt(_curl, CURLOPT_WRITEHEADER, this);
    curl_easy_setopt(_curl, CURLOPT_FILETIME, 1L);
    curl_easy_setopt(_curl, CURLOPT_NOSIGNAL, 1);       //disable signals to use with threads

    //curl_easy_setopt(_curl, CURLOPT_COOKIEFILE, "");    //keep cookies in memory
    curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYPEER, 0); //don't verify peer against cert
    curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYHOST, 0); //don't verify host against cert
    curl_easy_setopt(_curl, CURLOPT_FOLLOWLOCATION, 1); //bounce through login to next page

    curl_easy_setopt( _curl, CURLOPT_URL, _url.c_str() );
    CURLcode res = curl_easy_perform( _curl );

    _used = true;
    _code = 404;
    if (CURLE_OK == res)
    {
        char* ip;
        curl_easy_getinfo(_curl, CURLINFO_PRIMARY_IP, &ip);
        _ip=ip;
        short s;
        curl_easy_getinfo(_curl, CURLINFO_RESPONSE_CODE, &s);
        _code=s;

        time_t fileTime = -1;
        curl_easy_getinfo(_curl, CURLINFO_FILETIME, &fileTime);
        if (fileTime != -1)   //otherwise unknown
        {
            char st[64];
            sprintf (st, "%s",ctime(&fileTime));
            st[strlen(st)-1]=0;
            _date = st;
        }
        char* ct = 0;
        curl_easy_getinfo(_curl, CURLINFO_CONTENT_TYPE, &ct);
        _mime = ct ? ct : "";
    }
    return _code;
}

//-----------------------------------------------------------------------------
int32_t wcurl::content(string& p, bool with_hdr)
{
    //time_t fileTime = 1;
    _payload=&p;
    assert(false==_used); //one query per instance
    //curl_easy_setopt( curl, CURLOPT_CUSTOMREQUEST, "GET" );
    curl_easy_setopt( _curl, CURLOPT_HEADER, with_hdr );
    curl_easy_setopt (_curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(_curl, CURLOPT_FILETIME, 1L);
    curl_easy_setopt(_curl, CURLOPT_NOSIGNAL, 1);       //disable signals to use with threads

    curl_easy_setopt( _curl, CURLOPT_USERAGENT, _AGENT);
    curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, &wcurl::writeCallback);
    curl_easy_setopt(_curl, CURLOPT_WRITEDATA, this);

    curl_easy_setopt(_curl, CURLOPT_COOKIEFILE, "");    //keep cookies in memory
    curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYPEER, 0); //don't verify peer against cert
    curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYHOST, 0); //don't verify host against cert
    curl_easy_setopt(_curl, CURLOPT_FOLLOWLOCATION, 1); //bounce through login to next page
#ifdef CURLOPT_HTTPAUTH
// curl_easy_setopt(ch, CURLOPT_HTTPAUTH, CURLAUTH_ANY);
#endif
    curl_easy_setopt( _curl, CURLOPT_URL, _url.c_str() );
    CURLcode res = curl_easy_perform( _curl );
    _used = true;
    _code = 404;

    if (CURLE_OK == res)
    {
        char* ip;
        curl_easy_getinfo(_curl, CURLINFO_PRIMARY_IP, &ip);
        _ip=ip;
        short s;
        curl_easy_getinfo(_curl, CURLINFO_RESPONSE_CODE, &s);
        _code=s;

        time_t fileTime = -1;
        curl_easy_getinfo(_curl, CURLINFO_FILETIME, &fileTime);
        if (fileTime != -1)   //otherwise unknown
        {
            char st[64];
            sprintf (st, "%s", ctime(&fileTime));
            st[strlen(st)-1]=0;
            _date = st;
        }
        char* ct = 0;
        curl_easy_getinfo(_curl, CURLINFO_CONTENT_TYPE, &ct);
        _mime = ct ? ct : "text/html";
    }
    return _code;
}

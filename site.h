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

#ifndef SITEX_H
#define SITEX_H

#include <iostream>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <algorithm>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <memory>
#include <os.h>
#include <htmlcxx/html/ParserDom.h>
#include "main.h"
#include "htpargs.h"
#include "config.h"
#include "glb.h"

#define _C <<","<<
#define _S(v) "'" << v << "'"
#define _SC(v) "'" << v << "',"
#define STR(v) "'" << hs[v] << "'"
#define INT(v) hs(v)


//-----------------------------------------------------------------------------
#define _DOMAIN_SCORE  100000 // TODO

//-----------------------------------------------------------------------------
using namespace htmlcxx;
using namespace std;
class mysqlcls;

//-----------------------------------------------------------------------------
class wsite : public glb::qitm
{
public:
    struct score{
        uint16_t    _weight;
        uint16_t    _count;
    };

    wsite(mysqlcls* pdb, const glb::qitm& itm);
    virtual ~wsite();
    bool db_create();
    bool robots_omit(const string& path);
    void db_remove();
    bool add_link();
    void rem_link(){--_nlinks;}
    bool cango_extern(const string& fullurl);
    bool cango_down(uint32_t dpth);
private:
    void _update_server();
    void _db_clean_unrefs();

public:

    mysqlcls*               _p_dbase;
    uint32_t                _siteid;
    bool                    _fetched;
    string                  _title;
    string                  _ip;
    string                  _logo;
    string                  _firstimg;
    string                  _srvname;
    string                  _robots;
    map<string, uint16_t>   _scores;
};


//-----------------------------------------------------------------------------
template <typename T>
class ref_count
{
public:
    ref_count():_ptr(0),_prefs(0) {

        AutoLock l(&_m);
        _prefs=new int32_t;
        *_prefs=0;
    }

    ref_count(T* pt):_ptr(0),_prefs(0) {
        AutoLock l(&_m);

        _prefs=new int32_t;
        (*_prefs)++;
        _ptr=pt;
    }
    ref_count(const ref_count& r): _ptr(r._ptr), _prefs(r._prefs) {
        AutoLock l(&_m);
        (*_prefs)++;
    }
    ref_count<T>& operator = ( ref_count& r) {
        if(this!=&r) {
            AutoLock l(&_m);
            /*
            if(--(*r._prefs) == 0)
            {
                delete _prefs;
                delete _ptr;
            }
            */
            _ptr = r._ptr;
            _prefs = r._prefs;
            ++(*_prefs);
        }
        return *this;
    }

    ref_count<T>& operator = ( T* r) {
        if(this->_ptr!=r && r) {
            AutoLock l(&_m);
            _ptr = r;
            ++(*_prefs);
        }
        return *this;
    }

    virtual ~ref_count() {
        if(--(*_prefs) == 0) {
            delete _prefs;
            delete _ptr;
        }
    }

    T& operator* () {
        return *_ptr;
    }

    T* operator-> () {
        return _ptr;
    }
    T* ptr() {
        return _ptr;    //debig only
    }
private:
    T*          _ptr;
    int32_t*    _prefs;
    mutex       _m;
};

#endif // SITE_H

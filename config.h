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

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <os.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <map>

//-----------------------------------------------------------------------------
using namespace std;
typedef int (*pfatoi) (const char *);

//-----------------------------------------------------------------------------
typedef std::map<string, string> Mapss;
typedef std::map<string, string>::iterator MapssIt;

//-----------------------------------------------------------------------------
class Conf
{
    struct StrRule {
        char s[32];
        int   val;
    };

    void _assign( const char* pred, const char* val, int line);
    void _bind(const char* lpred, const char* vname,
               size_t& val, const char* kv ) {
        _bind(lpred, vname, (int&) val, kv );
    }
    void _bind(const char* lpred, const char* vname,
               std::string& val, const char* kv) {

        if(vname) {
            if(!strcmp(lpred,vname)) {
                val = kv;
                return;
            }
            return;
        }
        val = kv;
    }

    void _bind(const char* lpred, const char* vname,
               int& val, const char* kv, StrRule* r=0 ) {
        if(!strcmp(lpred,vname)) {
            if(r) {
                StrRule* pr = r;
                while(pr->s && *pr->s) {
                    if(!strcmp(pr->s, kv)) {
                        val = pr->val;
                        //throw(1);
                    }
                    pr++;
                }
            } else {
                if(kv[0]==':') {
                    int a,b,c,d;
                    sscanf(&kv[1],"%d.%d.%d.%d",&a,&b,&c,&d);
                    a&=0xff;
                    b&=0xff;
                    c&=0xff;
                    d&=0xff;
                    val = (d<<24|c<<16|b<<8|a);
                } else {
                    val = ::_ttatoi(kv);
                }
                //throw(1);
            }

        }
    }
    void _bind(const char* lpred, const char* vname,
               std::set<string>& val, const char* kv) {
        if(!strcmp(lpred,vname)) {
            std::istringstream iss(kv);
            std::string token;
            while(getline(iss, token, ',')) {
                val.insert(token);
            }
            //throw(1);
        }
    }
public:
    struct Mitza {
        Mitza():linksperpage(128),linkspersite(2048),leavesite(0),reindex(0),
                pagelen(32768),threads(1),depth(3),delay(10),blog("EW"),
                sitefile("sites.txt"),cache(512),preque(32),dbsleep(10)
        {
        }
        int32_t             linksperpage;
        int32_t             linkspersite;
        int32_t             leavesite;
        int32_t             reindex;
        int32_t             pagelen;
        int32_t             threads;
        int32_t             depth;
        int32_t             delay;
        string              blog;
        string              sitefile;
        int32_t             cache;
        int32_t             preque;
        int32_t             dbsleep;
        string              host;
        string              database;
        string              user;
        string              password;
        string              qdir;

        string              urlregex;
    } _mitza;
    int _depth;
    int _links;
    int _pages;

public:
    Conf();
    ~Conf();
public:
    void load(const char*);
    void  check_log_size();
    void  rollup_logs(const char* rootName);
public:
    std::string         _section;
    mutex               _m;
    size_t              _blog;
};

//-----------------------------------------------------------------------------------
template <typename T>inline void fix(T& val, T minv, T maxv)
{
    if(val < minv) {
        val = minv;
        return;
    }
    if(val > maxv) {
        val = maxv;
        return;
    }
}

//-----------------------------------------------------------------------------
extern Conf* GCFG;

//-----------------------------------------------------------------------------
#define __cfMari    GCFG->_glbl
#define __cfPool    GCFG->_pool
#define PORT        GCFG->_listeners


//-----------------------------------------------------------------------------
#define GLOGI(x) if(GCFG->_blog & 0x1) \
do{\
    AutoLock __a(&GCFG->_m); \
    std::cout << hex <<pthread_self() <<dec <<" I: " << x << "\n";\
}while(0);

//-----------------------------------------------------------------------------
#define GLOGW(x) if(GCFG->_blog & 0x2) \
do{\
    AutoLock __a(&GCFG->_m); \
    std::cout << hex <<pthread_self() <<dec <<" W: " << x << "\n";\
}while(0);

#define GLOGE(x) if(GCFG->_blog & 0x4) \
do{\
    AutoLock __a(&GCFG->_m); \
    std::cout << __FILE__ <<":"<<__LINE__ << ":" << hex <<pthread_self() <<dec <<" E: " << x << "\n";\
}while(0);

//-----------------------------------------------------------------------------
#define GLOGT(x) if(GCFG->_blog & 0x8) \
do{\
    AutoLock __a(&GCFG->_m); \
    std::cout << hex <<pthread_self() <<dec <<" T: " << x << "\n";\
}while(0);

//-----------------------------------------------------------------------------
#define GLOGD(x) if(GCFG->_blog & 0x10) \
do{\
    AutoLock __a(&GCFG->_m); \
    std::cout << hex <<pthread_self() <<dec <<" D: " << x << "\n";\
}while(0);

//-----------------------------------------------------------------------------
#define GLOGX(x) if(GCFG->_blog & 0x20) \
do{\
    AutoLock __a(&GCFG->_m); \
    std::cout << hex <<pthread_self() <<dec <<" X: " << x << "\n";\
}while(0);


#endif //_CONFIG_H_

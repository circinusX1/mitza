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

#ifndef HTPARGS_H
#define HTPARGS_H

#include <string.h>
#include <stdlib.h>
#include <map>
#include <string>
#include <assert.h>

using namespace std;

class htpargs
{
public:

    htpargs(const char* args=0) {
        if(args)
            parse (args);
    };

    void parse (const char* args) {
        char loco[1024];
        char* pl = loco;
        ::strcpy(loco,args);
        const char* tok="&=";
        const char* pk, *pv;
        while((pk = ::strtok(pl,tok))) {
            pl=0;
            pv = ::strtok(pl,tok);
            if(0==pv)break;
            _args[pk] = pv;
        }
    }
    virtual ~htpargs();
    const char* operator[](const char* k)const {
        map<string,string>::const_iterator f = _args.find(k);
        if(f == _args.end())
            return "";
        return (*f).second.c_str();
    }
    int operator()(const char* k)const {
        map<string,string>::const_iterator f = _args.find(k);
        if(f == _args.end())
            return 0;
        return ::atoi((*f).second.c_str());
    }
protected:
private:

    map<string,string>  _args;
};

#endif // HTPARGS_H

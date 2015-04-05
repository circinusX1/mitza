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

#ifndef MAIN_H
#define MAIN_H


#include <signal.h>
#include <os.h>
#include <assert.h>
#include <mysql/mysql.h>
#include "htpargs.h"
#include "database.h"
#include "glb.h"

#define _AGENT      "mitza-1.0.0"
#define _MAX_LINKS  4000
#define _MAX_UNIQ   100

//#define MULTIPLE_KWS     32
#define _I(k_)      (int32_t)::atol(k_)
#define _U(k_)      (uint32_t)::atol(k_)

using namespace std;

extern bool     __alive;


inline void _assert(string s, bool c )
{
    if(!c) {
        cout << s << "\n";
        assert(c);
    }
}

//-----------------------------------------------------------------------------
inline glb& glb_wset()
{
    static glb   lb;
    return lb;
}


inline mutex* db_lock()
{
    static mutex   mut;
    return &mut;
}


#endif // MAIN_H

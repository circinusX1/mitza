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

#ifndef THRX_H
#define THRX_H

#include <signal.h>
#include <os.h>
#include <mysql/mysql.h>
#include "htpargs.h"
#include "database.h"

using namespace std;

class cthread : public os_thread
{
public:
    cthread();
    virtual ~cthread();
    void thread_main();
    char* strbuff(size_t sz);
    static bool is_idling(){return _winstance==0;}
private:
    void _process(mysqlcls& db, glb::qitm& itm);
    void _working();
    void _idling();

private:
    char    *_tmp_str;
    size_t   _cap;
    static   int _winstance;
};



#endif // THR_H

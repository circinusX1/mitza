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


#include <openssl/md5.h>
#include <htmlcxx/html/ParserDom.h>
#include <memory>
#include <pcrecpp.h>
#include "main.h"
#include "database.h"
#include "site.h"
#include "webpage.h"
#include "wcurl.h"
#include "config.h"
#include "thr.h"
#include "glb.h"

int cthread::_winstance = 0;

//-----------------------------------------------------------------------------
cthread::cthread():_tmp_str(0),_cap(0)
{
}

//-----------------------------------------------------------------------------
cthread::~cthread()
{
    delete[] _tmp_str;
}

//-----------------------------------------------------------------------------
// shared between webpages for sql-iazeation
char* cthread::strbuff(size_t sz)
{
    if(sz > _cap)
    {
        delete[] _tmp_str;
        _tmp_str = new char[sz];
        _cap = sz;
    }
    return _tmp_str;
}

//-----------------------------------------------------------------------------
void cthread::_process(mysqlcls& db, glb::qitm& itm)
{
    webpage* p = 0;
    try{
        p = new webpage(this, &db, itm);
        if(p && p->is_good())
        {
            p->dig();
        }
    }
    catch(...)
    {
        GLOGE("Exception unknown...");
        __alive=false;
    }
    delete p;
    usleep(GCFG->_mitza.delay);
}

//-----------------------------------------------------------------------------
void cthread::_working()
{
      AutoLock  l(db_lock());
      ++cthread::_winstance;
}

//-----------------------------------------------------------------------------
void cthread::_idling()
{
    AutoLock  l(db_lock());
    --cthread::_winstance;

    if(0 == cthread::_winstance)
    {
        glb_wset().clear();
    }
}

//-----------------------------------------------------------------------------
void cthread::thread_main()
{
    mysqlcls    db;
    bool        flip=false;
    glb::qitm   itm(_EMPTY_STR);
    string      link;

    db.connect(GCFG->_mitza.host.c_str(),GCFG->_mitza.user.c_str(),GCFG->_mitza.password.c_str());
    usleep(1000);
    while(__alive && !is_stopped())
    {
        bool dequeued=false;
        do{
            AutoTr  l(db_lock(), &db);
            dequeued=glb_wset().dequeue_itm(&db, itm);
        }while(0);

        if(dequeued)
        {
            if(false==flip)
            {
                flip=true;
                _working();
            }
            // siteurl, is a  page link, then whe the page is built, this holds the site only.
            link = itm._siteurl; // this get changed.. design issue TODO
            if(glb_wset().add_2_ws(link))
            {
                _process(db, itm);
                glb_wset().remove_4_ws(link);
            }
        }
        else
        {
            sleep(2);
            if(flip)
            {
                flip=false;
                GLOGT("idling/working:" << cthread::_winstance);
                _idling();
            }
         }
    }
    db.disconnect();
    GLOGT("T exiting");
}

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

#ifndef WEBPAGE_H
#define WEBPAGE_H

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
#include <htmlcxx/html/ParserDom.h>  //sudo apt-get install libhtmlcxx*-dev
#include "main.h"
#include "htpargs.h"
#include "config.h"
#include "site.h"

#define _C <<","<<
#define _S(v) "'" << v << "'"
#define _SC(v) "'" << v << "',"
#define STR(v) "'" << hs[v] << "'"
#define INT(v) hs(v)

//-----------------------------------------------------------------------------
using namespace htmlcxx;
using namespace std;
class mysqlcls;

//-----------------------------------------------------------------------------
class cthread;
class webpage
{
    typedef enum _PAGE_ST
    {
        NEW_LINK=0,
        CHANGED_LINK,
        REFR_LINK,
        CHNG_LINK,
        PG_DONE,
        DB_LINK,
        PG_404,
        EMPTY,
    } PAGE_ST;
    struct score{
        uint16_t    _weight;
        uint16_t    _count;
    };

public:
    webpage(mysqlcls* db);
    webpage(cthread* pt, mysqlcls* db,  glb::qitm& itm);
    ~webpage();
    void dig();
    bool is_good()
    {
        return _uri.length() > 9 && _psite;
    }
private:
    bool _db_create(uint32_t siteid);
    bool _changed();
    PAGE_ST _touched();
    bool  _digpage();
    int  _db_update_link(bool update=false);
    void  _db_finalize_page();
    void  _finalize_words();
    void _finalize_site();
    void _finalize_links();
    void _finalize_assets();
    bool _add_page(string uri, const string& name, uint32_t sid, uint32_t dpth, int32_t parentid);
    void _add_links_from_dbpage();
    bool _db_delete_children();
    int32_t  _score(const char* tag)const;
    bool _add_word(string word, int32_t score, bool entire, bool incontent);
    const char* _sqlize(const string& in);
    bool _check_header();
    bool _check_content();
    void _inherit(const string& uri, webpage* root, string& out);
    void _get_site_forpage(const glb::qitm & itm);
    string _from_parent( string& siteurl);
    bool _try_to_add(const string& link, const string& name);
    void _enqueue_links();

public:
    wsite*      _psite;
    mysqlcls*   _p_dbase;
    cthread*    _pthr;
    uint32_t    _depthdown;
    int32_t     _pageid;
    int32_t     _ppageid;
    int         _bkls;
    bool        _deleted;
    int         _links_count;
    bool        _updated;
    int32_t     _accum;
    int64_t     _flags;
    PAGE_ST     _state;
    string      _port;
    string      _qry;
    string      _path;
    string      _proto;
    string      _host;
    string      _uri;
    string      _hostname;
    string      _header;
    string      _content;
    string      _location;
    string      _mime;
    string      _date;
    string      _md5;
    string      _title;
    string      _desc;
    string      _link_name;
    string      _compressed;
    set<string>       _assets;
    map<string,score> _scores;
    vector<glb::qitm> _links;
};  // uri



#endif // WEBPAGE

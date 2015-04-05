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

#ifndef THR_H
#define THR_H


#include <os.h>



class wsite;
class webpage;
class mysqlcls;
class glb
{
public:

    struct qitm
    {
        qitm(const qitm& r):_siteurl(r._siteurl),
                            _catid(r._catid),
                            _reindex(r._reindex),
                            _maxdepth(r._maxdepth),
                            _maxlinks(r._maxlinks),
                            _goextern(r._goextern),
                            _sessid(r._sessid),
                            _plinkid(r._plinkid),
                            _curdepth(r._curdepth),
                            _nlinks(r._nlinks){
            ;
        }
        qitm(const string& loc,
             uint32_t cat=0,
            bool re=false,
            uint32_t de=0,
            uint32_t ml=0,
            bool goex=false,
            uint32_t sid=0,
             int32_t pid=0,
            uint32_t lev=0,
            uint32_t nl=0):_siteurl(loc),
                            _catid(cat),
                            _reindex(re),
                            _maxdepth(de),
                            _maxlinks(ml),
                            _goextern(goex),
                            _sessid(sid),
                            _plinkid(pid),
                            _curdepth(lev),
                            _nlinks(nl){
                                ;
        }
        string      _siteurl;
        uint32_t    _catid;
        bool        _reindex;
        uint32_t    _maxdepth;
        uint32_t    _maxlinks;
        bool        _goextern;
        uint32_t    _sessid;
        int32_t     _plinkid;
        uint32_t    _curdepth;
        uint32_t    _nlinks;
        bool operator==(const qitm& r){return _siteurl==r._siteurl;}
    };

    struct sites
    {
        wsite*  ps;
        size_t  refs;
    };

    glb():_dbqsize(0),_items(0),_lastsnap(time(0)),_pgpeemin(0),_pgpeemin2(0){}
    bool add_2_ws(  const string& uri);
    void remove_4_ws(  const string& uri);
    size_t size()
    {
        return _uris.size();
    }
    size_t buzzy(mysqlcls* pdbase);
    bool dequeue_itm(mysqlcls* pdbase, qitm& itm);
    bool enqueue_itm(mysqlcls* pdbase, const qitm& itm );
    void wset(string& s);
    bool inset(const string& uri);
    wsite*  is_site(const string& s);
    bool  new_site(wsite* ws);
    void  del_site(const string& s);
    void  clear(){
        assert(_proccache.size()==0);
        assert(_dbcache.size()==0);
        assert(_wsites.size()==0);
        assert(_uris.size()==0);
        _recent.clear();
    }
    void pageadd();
    void flush(mysqlcls* pdbase);
private:

    bool _dequeue_db(mysqlcls* pdbase, qitm& itm);
    bool _enqueue_db(mysqlcls* pdbase, const qitm& itm );

    bool _dequeue_file(qitm& itm);
    bool _enqueue_file(const qitm& itm );
public:
    set<string>          _uris;    //to keep alive
    deque<string>        _recent; //hold here
    map<string, sites >  _wsites;
    mutex                _m;
    int32_t              _dbqsize;
    vector<qitm>         _proccache;
    vector<qitm>         _dbcache;
    int                  _items;
    time_t               _lastsnap;
    int32_t              _pgpeemin;
    int32_t              _pgpeemin2;
};

extern string _EMPTY_STR;

#endif // THR_H

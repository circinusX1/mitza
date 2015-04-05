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

#include <memory>
#include "main.h"
#include "strops.h"
#include "database.h"
#include "webpage.h"
#include "wcurl.h"
#include "config.h"
#include "thr.h"
#include "glb.h"

string _EMPTY_STR="";
//-----------------------------------------------------------------------------
// threads store working uri here for fast rejection to avoid db access
bool glb::add_2_ws(const string& uri)
{
    AutoLock l(&_m);

    set<string>::const_iterator f = _uris.find(uri);
    if(f == _uris.end() )
    {
        //check last used items
        if(std::find(_recent.begin(), _recent.end(), uri)!=_recent.end())
        {
            GLOGD("[RECENT]: found in recent: " << uri << " elems: " << _recent.size());
            return false; //most recent used
        }

        _recent.push_back(uri);
        if(_recent.size()>(size_t)GCFG->_mitza.cache)
        {
            _recent.pop_front();
        }
        GLOGT( "[WS]<-------<<{{" << uri<<"}}. elems:" << _uris.size());
        _uris.insert(uri);
        return true;
    }
    GLOGD(" found in working set: " << uri);
    return false;
}

//-----------------------------------------------------------------------------
void glb::remove_4_ws(const string& uri)
{
    AutoLock l(&_m);

    set<string>::const_iterator f = _uris.find(uri);
    if(f != _uris.end() )
    {
        _uris.erase(uri);
        GLOGT("[WS]---------->>{{" << uri<<"}}. elems:" << _uris.size());
    }
}
//-----------------------------------------------------------------------------
// see if the uri is in working set
bool glb::inset(const string& uri)
{
    AutoLock l(&_m); //glb lock

    set<string>::const_iterator f = _uris.find(uri);
    return f != _uris.end();
}

//-----------------------------------------------------------------------------
// a long string with uris
void glb::wset(string& s)
{
    AutoLock l(&_m); //glb lock

    set<string>::iterator b = _uris.begin();
    for(; b!= _uris.end(); ++b)
    {
        s += *b + "\n";
    }
}

//-----------------------------------------------------------------------------
// site array. many pages share one site form this array
bool glb::new_site(wsite* s)
{
    AutoLock l(&_m); //glb lock

    map<string,sites>::iterator f = _wsites.find(s->_siteurl);
    if(f != _wsites.end())
    {
        sites& st= f->second;
        ++st.refs;
    }
    else
    {
        sites st;
        st.refs=1;
        st.ps=s;
        _wsites[s->_siteurl]=st;
    }
    return true;
}

//-----------------------------------------------------------------------------
// dereference
void glb::del_site(const string&  s)
{
    AutoLock l(&_m); //glb lock

    map<string,sites>::iterator f = _wsites.find(s);
    if(f != _wsites.end())
    {
        sites& st= f->second;
        --st.refs;
        if(0==st.refs)
        {
            delete st.ps;
            _wsites.erase(f);
        }
    }
}

//------------------------------------------------------------------------------
// obtain a site
wsite* glb::is_site(const string& s)
{
    AutoLock l(&_m); //glb lock

    map<string,sites>::iterator f = _wsites.find(s);
    if(f != _wsites.end())
    {
        sites& s= f->second;
        ++s.refs;
        return s.ps;
    }
    return 0;
}

//-----------------------------------------------------------------------------
// place a working page on the queue. TODO pass a structure her
bool glb::enqueue_itm(mysqlcls* pdbase,const qitm& item)
{
    if(GCFG->_mitza.qdir.empty())
    {
        return _enqueue_db(pdbase, item);
    }
    return _enqueue_file(item);
}

//-----------------------------------------------------------------------------
// get a working item from queue
bool glb::dequeue_itm(mysqlcls* pdbase,qitm& item)
{
    if(__alive == false)
    {
        return false;
    }
    if(GCFG->_mitza.qdir.empty())
    {
        return _dequeue_db(pdbase, item);
    }
    return _dequeue_file(item);
}

bool glb::_dequeue_db(mysqlcls* pdbase, qitm& itm)
{
    if(_proccache.size())
    {
        itm=_proccache.back();
        _proccache.pop_back();
        GLOGT("Q1:---------------->:" << itm._siteurl);
        return true;
    }

    // fill up the vector up to
    // avoid db exauhst
    stringstream    s;

    itm._siteurl.clear();
    s << "SELECT * FROM que LIMIT " << GCFG->_mitza.preque;
    SR rset(pdbase->qry(s, true));
    MYSQL_ROW r2;
    while((r2 = rset->fetch()))
    {
        itm._siteurl = r2[1];
        itm._catid = (uint32_t)::atol(r2[2]);
        itm._plinkid = (uint32_t)::atol(r2[3]);
        itm._curdepth = (uint32_t)::atol(r2[4]);
        itm._sessid = (uint32_t)::atol(r2[5]);
        itm._reindex = (bool)::atol(r2[6]);
        itm._maxdepth = (uint32_t)::atol(r2[7]);
        itm._maxlinks = (uint32_t)::atol(r2[8]);
        itm._goextern = (bool)::atol(r2[9]);
        itm._nlinks = (uint32_t)::atol(r2[10]);
        _proccache.push_back(itm);

        //GLOGT("[PV]:<--------------[DB]" << itm._siteurl);
    }

    if(_proccache.size())
    {
        s.str("");
        s << "DELETE FROM que LIMIT " << GCFG->_mitza.preque;
        pdbase->qry(s);

        itm=_proccache.back();
        _proccache.pop_back();
        GLOGT("[Q2]:-------------->" << itm._siteurl);
        return true;
    }
    if(_dbcache.size())
    {
         itm=_dbcache.back();
        _dbcache.pop_back();
        GLOGT("[Q3]:-------------->" << itm._siteurl);
        return true;
    }
    return false;
}

bool glb::_enqueue_db(mysqlcls* pdbase, const qitm& itm )
{
    stringstream    s;
    //check if the item is in queue
    s.str("");
    s << "SELECT ppid FROM que WHERE url="<<_S(itm._siteurl) << " LIMIT 1";
    SR rset(pdbase->qry(s, true));
    if(rset->rows()==0)
    {
        //not in queue
        if(_dbcache.size() < (size_t)GCFG->_mitza.preque)
        {
            // check if is in que in cahce
            if(std::find(_dbcache.begin(),_dbcache.end(),itm._siteurl)==_dbcache.end())
            {
                //place it in db cahce
                _dbcache.push_back(itm);
                GLOGT("[Q1]:<--------------:" << itm._siteurl);
                return true;
            }
            GLOGT("[Q1]:X--------------:" << itm._siteurl);
            return false;
        }

        // we reached the cache maxim item count
        //flush them in db
        vector<qitm>::const_iterator b=_dbcache.begin();
        s.str("");
        s << "INSERT INTO que (url,catid,ppid,level,sessid,reindex,depth,maxlnks,externals,nlinks) VALUES \n";
        for(;b!=_dbcache.end();++b)
        {
            const qitm& itm = *b;
            s   << "("<< _S(itm._siteurl) <<","
                << itm._catid <<","
                << itm._plinkid <<","
                << itm._curdepth <<","
                << itm._sessid <<","
                << (int32_t)itm._reindex <<","
                << itm._maxdepth <<","
                << itm._maxlinks <<","
                << (int32_t)itm._goextern<<","
                << (int32_t)itm._nlinks <<")";
                if(b < _dbcache.end()-1)
                    s << ",\n";
        }
        pdbase->qry(s);
        GLOGT("[Q2]:<--------------[DV]:" << itm._siteurl);
        _dbcache.clear();
    }
    usleep(1024);
    return true;
}

bool glb::_dequeue_file(qitm& itm)
{
    return false;
}

bool glb::_enqueue_file(const qitm& itm )
{
    return false;
}

size_t glb::buzzy(mysqlcls* pdbase)
{
    int32_t rows=0;
    if(_uris.size() + _proccache.size() + _dbcache.size() == 0)
    {
        stringstream    s;
        s << "SELECT qid FROM que LIMIT 1;";
        SR rset(pdbase->qry(s, true));
        return rset->rows();
    }
    return 1;
}


void glb::pageadd()
{
    AutoLock l(&_m); //glb lock
    if(time(0) - _lastsnap < 60)
    {
        ++_pgpeemin;
    }
    else
    {
        _pgpeemin2=_pgpeemin;
        _lastsnap=time(0);
        _pgpeemin=0;
        GLOGI("*** Performing with "<<_pgpeemin2<<" pages/min ***");
    }
}


void glb::flush(mysqlcls* pdbase)
{
    stringstream s;
    int          itms=0;
    vector<qitm>::const_iterator b=_dbcache.begin();
    s.str("");
    s << "INSERT INTO que (url,catid,ppid,level,sessid,reindex,depth,maxlnks,externals,nlinks) VALUES \n";
    for(;b!=_dbcache.end();++b)
    {
        const qitm& itm = *b;
        s   << "("<< _S(itm._siteurl) <<","
            << itm._catid <<","
            << itm._plinkid <<","
            << itm._curdepth <<","
            << itm._sessid <<","
            << (int32_t)itm._reindex <<","
            << itm._maxdepth <<","
            << itm._maxlinks <<","
            << (int32_t)itm._goextern<<","
            << (int32_t)itm._nlinks <<")";
            if(b < _dbcache.end()-1)
                s << ",\n";
            ++itms;
    }
    if(itms)
        pdbase->qry(s);
    _dbcache.clear();
    usleep(1024);
    b=_proccache.begin();
    s.str("");
    itms=0;
    s << "INSERT INTO que (url,catid,ppid,level,sessid,reindex,depth,maxlnks,externals,nlinks) VALUES ";
    for(;b!=_proccache.end();++b)
    {
        const qitm& itm = *b;
        s   << "("<< _S(itm._siteurl) <<","
        << itm._catid <<","
        << itm._plinkid <<","
        << itm._curdepth <<","
        << itm._sessid <<","
        << (int32_t)itm._reindex <<","
        << itm._maxdepth <<","
        << itm._maxlinks <<","
        << (int32_t)itm._goextern<<","
        << (int32_t)itm._nlinks <<")";
        if(b < _dbcache.end()-1)
            s << ",";
        ++itms;
    }
    if(itms)
        pdbase->qry(s);
    _proccache.clear();
    usleep(1024);
}

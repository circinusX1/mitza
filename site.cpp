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

#include "strops.h"
#include "main.h"
#include "database.h"
#include "site.h"
#include "thr.h"
#include "config.h"
#include "wcurl.h"

//-----------------------------------------------------------------------------
wsite::wsite(mysqlcls* pdb, const glb::qitm& itm):glb::qitm(itm),
                                                    _p_dbase(pdb),
                                                    _siteid(0),
                                                    _fetched(false)
{
    GLOGX("create wsite" << itm._siteurl);
}

//-----------------------------------------------------------------------------
wsite::~wsite()
{
    GLOGX("destroy ~wsite:" << _siteurl);
    _update_server();
}

//-----------------------------------------------------------------------------
bool wsite::db_create()
{
    stringstream s;
    uint32_t sessidl;

    s << "SELECT siteid,sessid,catid,title,depth,logo,extrn,nlinks,srvip FROM domain WHERE name="<<_S(_siteurl);
    SR rezs(_p_dbase->qry(s, true));
    if(rezs->rows())
    {
        MYSQL_ROW row = rezs->fetch();
        _siteid        = _U(row[0]);
        sessidl         = _U(row[1]);
        _catid          = _U(row[2]);
        _title          = row[3];
        _maxdepth       = _U(row[4]);
        _logo           = row[5];
        _goextern       = bool(_U(row[6]));
        _nlinks         = _U(row[7]);
        _ip             = row[8];

        if(sessidl != _sessid)
        {
            s.str("");
            s << "UPDATE domain SET sessid="<<_sessid<<" WHERE siteid="<<_siteid;
            _p_dbase->qry(s);
        }

        GLOGI("GETTING SITE:" << _siteurl << " SITEID:" << _siteid);

        return _p_dbase->err()==0;
    }

    s.str("");
    s << "INSERT INTO domain (sessid,catid,name,depth,extrn) VALUES (" << _sessid <<","<< _catid<<","<<_S(_siteurl)<<","<< _maxdepth <<","<< _goextern<<")";
    _p_dbase->qry(s, true);
    _siteid = _p_dbase->lastid();


    GLOGI("CREATING SITE:" << _siteurl << " SITEID:" << _siteid);


    return _p_dbase->err()==0;
}


//-----------------------------------------------------------------------------
bool wsite::robots_omit(const string& path)
{
    if(_fetched==false && _robots.empty())
    {
         _fetched=true; //once
        string      urldoc = _siteurl + "/robots.txt";
        string      robots; robots.reserve(256);
        wcurl       curl(urldoc);
        int         code = curl.content(_robots);

        if(_ip.empty())
        {
            _ip = curl._ip;
        }

        if(code == 404 )
        {
            GLOGW("site:" << _siteurl << " has no robots");
            return false; //allowed. no robits file
        }
    }

    // parse robots.
    stringstream    ss(_robots);
    string          s;
    bool            section = false;

    while (getline(ss, s, '\n'))
    {
        bool bua = s.find("User-agent")!=string::npos;
        if(bua)
        {
            section = ((s.find("*")!=string::npos ||s.find(_AGENT)!=string::npos ));
            continue;
        }
        if(section)
        {
            if(path == "/")
            {
                if (s=="Dissalow: /")
                {
                    return true;
                }
                continue;
            }
            else if(s.find(path)!=string::npos)
            {
                if(s.find("Dissalow")!=string::npos)
                {
                    return true;
                }
                continue;
            }
        }
    }
    return false;
}

//-----------------------------------------------------------------------------
void wsite::db_remove()
{
    if(!_reindex)
        return;

    stringstream    s;
    AutoTr l(db_lock(), _p_dbase);


    s << "SELECT siteid FROM domain WHERE name="<<_S(_siteurl) << " AND sessid <>"<< _sessid;
    SR rezs(_p_dbase->qry(s, true));
    if(rezs->rows())
    {
        GLOGD("deleting " << _siteurl <<"\n");
        int32_t siteid = _I(rezs->fetch(0));

        GLOGI("DELETING SITE:" << _siteurl << " SITEID:" << siteid);
#ifdef MULTIPLE_KWS
        for(int32_t tbl = 0; tbl < MULTIPLE_KWS; ++tbl)
        {
            ::sprintf(tbl_name,"words%d",tbl);
            s.str("");
            s << "DELETE FROM "<<tbl_name<<" WHERE siteid="<<siteid;
            _p_dbase->qry(s);
            usleep(1024);
        }
#else
        s.str("");
        s << "DELETE FROM words WHERE siteid="<<siteid;
        _p_dbase->qry(s);
        usleep(1024);
#endif
        s.str("");
        s << "DELETE FROM links WHERE siteid="<<siteid << " AND sessid <>"<< _sessid;
        _p_dbase->qry(s);
        usleep(1024);
        s.str("");
        s << "DELETE FROM domain WHERE siteid="<<siteid << " AND sessid <>"<< _sessid;
        _p_dbase->qry(s);
        usleep(1024);
    }
}

//-----------------------------------------------------------------------------
void wsite::_db_clean_unrefs()
{
    AutoTr l(db_lock(), _p_dbase);
    stringstream s;
    s << "DELETE FROM links WHERE siteid="<<_siteid << " AND sessid != "<<_sessid;
    _p_dbase->qry(s);
}


//-----------------------------------------------------------------------------
void wsite::_update_server()
{
    stringstream s;
    AutoTr l(db_lock(), _p_dbase);

    if(_ip.empty())
        return;

    s << "SELECT ip FROM srv WHERE ip="<<_S(_ip);
    usleep(1024);
    SR rezs(_p_dbase->qry(s, true));
    size_t rows = rezs->rows();
    if(0 == rows)
    {
        s.str("");
        s << "INSERT INTO srv (ip,srvname) VALUES ("<<_S(_ip)<<","<<_S(_srvname)<<")";
        _p_dbase->qry(s, false);
    }
}


bool wsite::add_link()
{
    if(_maxlinks && ++_nlinks > _maxlinks)
    {
        GLOGD("links/page:"<<_nlinks <<"< max-links/page" << _maxlinks);
        return false;
    }
    return true;
}

bool wsite::cango_extern(const string& fullurl)
{
    if(!_goextern)
    {
        if(fullurl.compare(0, _siteurl.length(), _siteurl))
        {
             GLOGD("External link: " << fullurl << " while saiting: " << _siteurl);
             return false;
        }
    }
    return true;
}


 bool wsite::cango_down(uint32_t dpth)
 {
    if(_maxdepth && dpth >_maxdepth)
    {
        GLOGD("depth:"<<dpth<<" > max depth:"<<_maxdepth);
        return false;
    }
    return true;
 }

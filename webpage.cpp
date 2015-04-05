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

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/md5.h>
#include "strops.h"
#include "site.h"
#include "webpage.h"
#include "main.h"
#include "database.h"
#include "wcurl.h"
#include "thr.h"
#include "glb.h"
#include "config.h"


//-----------------------------------------------------------------------------
webpage::webpage(mysqlcls* db):_psite(0),
                            _p_dbase(db),
                            _pthr(0),
                            _depthdown(0),
                            _pageid(0),
                            _ppageid(-1),
                            _bkls(0),
                            _deleted(false),
                            _links_count(0),
                            _updated(false),
                            _accum(0),
                            _flags(0),
                            _link_name("index")

{
    //dummy used for parsing uri's
}

//-----------------------------------------------------------------------------
webpage::webpage(cthread* pt, mysqlcls* db,  glb::qitm& itm):_psite(0),
                                            _p_dbase(db),
                                            _pthr(pt),
                                            _depthdown(itm._curdepth),
                                            _pageid(0),
                                            _ppageid(itm._plinkid),
                                            _bkls(0),
                                            _deleted(false),
                                            _links_count(0),
                                            _updated(false),
                                            _accum(0),
                                            _link_name("index")
{
    if( pt)
    {
        GLOGT("new webpage:" <<itm._siteurl);
    }
    if(_ppageid>0)
    {
        webpage root(db);
        if(root._db_create(_ppageid))
        {
            _inherit(itm._siteurl, &root, _uri);
        }
    }
    else
    {
        so::url_normalize(itm._siteurl);
        _uri=itm._siteurl;
    }
    if(!_uri.empty())
    {
        so::url_parse(_uri,_proto,_host,_path,_port,_qry);
        _hostname = _proto + _host + _port;
        itm._siteurl = _proto + _host + _port; // kind of okward....TODO

        if(_pthr && !_hostname.empty()) // ptht is the heap one
        {
            _get_site_forpage(itm);
        }
    }
}

//-----------------------------------------------------------------------------
webpage::~webpage()
{
    if(!_hostname.empty() && _pthr/*heap objects*/)
    {
        glb_wset().del_site(_hostname);
    }
    if(_pthr)
        GLOGT("del ~webpage:" <<_uri);
}

//-----------------------------------------------------------------------------
void webpage::_get_site_forpage(const glb::qitm& itm)
{
    _psite = glb_wset().is_site(_hostname);
    if(0 == _psite)
    {
        _psite = new wsite(_p_dbase, itm);
        if(_psite)
        {
            if(_ppageid==-1 && itm._reindex)
            {
                _psite->db_remove();
            }
            _psite->db_create();
            glb_wset().new_site(_psite);
        }
        else{
            GLOGE("failed to allocate memory for new site:" << _hostname << ". exiting");
            __alive=false;
        }
    }
}

//-----------------------------------------------------------------------------
void webpage::dig( )
{
    if(_psite->robots_omit(_path))
    {
        GLOGW("page: " << _uri << " rejected by robots");
        return;
    }
     if(!_check_header())
    {
        GLOGW("page: " << _uri << " rejected by header");
        return;
    }
    if(!_check_content())
    {
        GLOGW("page: " << _uri << " rejected by content-or-not changed.");
        _enqueue_links();
        return;
    }
    if(!_content.empty())
    {
        if(_db_update_link())
        {
            return;
        }
        time_t startt = time(0);
        time_t parsed = startt;
        GLOGI( "DIGGING [" <<_pageid<<"/"<< _uri << "] level:" << _depthdown << " pid:" << _ppageid);
        if(_digpage())
        {
            parsed=time(0);
            _db_finalize_page();
            _enqueue_links();
        }
        time_t d = time(0)-startt;
        time_t p =time(0)-parsed;
        GLOGI( "DIGGED  [" << _uri << "] } found-links:" <<
                    _links_count << "in:(d/p)" <<d<<"/"<<p<< "secs.");

    }
    else
        GLOGW("content of the: " << _uri << ", is empty. spkippinig parsing");
}

//-----------------------------------------------------------------------------
bool webpage::_changed()
{
    stringstream    ss(_header);
    string          s;
    bool            moved=false;
    bool            noname = _psite->_srvname.empty();

    while (getline(ss, s, '\r'))
    {
        if(s=="\n")
            break;    //header done
        else if(s.find("404")!=string::npos) // no link_page
        {
            GLOGT("header found 404: " << _uri);
            ++_bkls;
            _deleted = true; // TO FIX
            moved = true;
        }
        if(s.find("Location:")!=string::npos ) // redirect
        {
            _location = s.substr(11);
            if(!_location.empty())
            {
                glb::qitm   it(_location);
                webpage     pl(0,0,it);

                if(pl._uri != _uri)
                {
                    GLOGT(_uri <<"site: "<<_uri<<" moved to: " << pl._uri);
                    moved =  true;
                }
            }
        }
        if(noname)
        {
            if(s.find("Server")!=string::npos  || s.find("Powered")!=string::npos)
            {
                size_t col=s.find(":");
                if(col != string::npos)
                {
                    _psite->_srvname += s.substr(col+1) + " " ;
                }
            }
        }
    }
    return moved;
}


//-----------------------------------------------------------------------------
webpage::PAGE_ST webpage::_touched()
{
    stringstream    s;
    AutoTr l(db_lock(), _p_dbase);

    // check in db
    s << "SELECT linkid, md5, sessid, ldat FROM links WHERE siteid="<<_psite->_siteid<<" AND url="<<_S(_uri);
    SR rezs(_p_dbase->qry(s, true));
    if(rezs->rows() == 0)
    {
        GLOGT(_uri << " is new link because was not found in db at all")
        return NEW_LINK; // not even in DB
    }

    if(rezs->rows() != 1)
    {
        int32_t rws = rezs->rows() ;
        GLOGE("There are " << rws << " records on links for:" << _uri );
    }

    // check session
    MYSQL_ROW  row = rezs->fetch();
    assert(row);
    _pageid = _U(row[0]);

    assert(_pageid);
    if(_psite->_sessid == (uint32_t)_I(row[2]))
    {
        GLOGW(_uri << _uri<<": was parsed in this sess. skipping");
        return PG_DONE;
    }

    // chek page date db.date ==  hdr.date
    const char* pdate = row[3];
    if(_date == pdate)          //date same
    {
        GLOGW(_uri << ": page has same date. skipping");
        return DB_LINK;
    }

    // check md5 of the  page

    wcurl   curl(_uri);
    _content.reserve(4096);
    if( 404 == curl.content(_content))
    {
        GLOGW( _uri << ": http error: " << curl._code);
        return PG_404;
    }

    if(!_content.empty())
    {
        unsigned char result[MD5_DIGEST_LENGTH+4];
        char out[16];

        MD5((const unsigned char*)_content.c_str() , _content.length(), result);
        _md5.clear();
        for(int k=0; k<MD5_DIGEST_LENGTH; k++)
        {
            sprintf(out,"%02X", (char)result[k]);
            _md5+=out;
        }
        if(_md5 == (const char*)row[1])
        {
            GLOGT( _uri << ": page crc did not changed. falling to add subsequent links from database");
            return DB_LINK;
        }
        return CHANGED_LINK;
    }
    return PG_DONE;
}

//-----------------------------------------------------------------------------
// page did not chnage so add for subsequest nlinks from the
// things we have in the db
void webpage::_add_links_from_dbpage()
{
    stringstream    s;
    AutoTr l(db_lock(), _p_dbase);

    s << "SELECT url,name,dpth FROM links WHERE siteid="<< _psite->_siteid<<
                                            " AND plinkid="<< _pageid <<
                                            " AND sessid ="<< _psite->_sessid <<
                                            " AND checkit=1";

    SR          rezl(_p_dbase->qry(s, true));
    MYSQL_ROW   row;
    while((row = rezl->fetch())!=0)
    {
        _add_page(row[0], row[1], _psite->_sessid, _I(row[2]), _pageid);
        GLOGT(row[0]);
    }
}

bool webpage::_db_delete_children()
{
    char            tbl_name[16];
    stringstream    s;
    AutoTr l(db_lock(), _p_dbase);

    GLOGT("{ "<<_uri <<": deleting children");
    s << "DELETE FROM links WHERE siteid="<< _psite->_siteid<<" AND plinkid="<< _pageid <<" AND sessid="<< _psite->_sessid;
    _p_dbase->qry(s);
    usleep(2048);
    s.str("");
    s << "DELETE FROM assets WHERE plinkid="<<_pageid<<" AND sessid="<< _psite->_sessid;
#ifdef MULTIPLE_KWS
    for(int32_t tbl = 0; tbl < MULTIPLE_KWS; ++tbl)
    {
        ::sprintf(tbl_name,"words%d",tbl);
        s.str("");
        s << "DELETE FROM "<<tbl_name<<" WHERE linkid="<<_pageid;
        _p_dbase->qry(s);
        usleep(2048);
    }
#else
    s.str("");
    s << "DELETE FROM words WHERE linkid="<<_pageid;
    _p_dbase->qry(s);
    usleep(2048);
#endif
    return true;
}

//-----------------------------------------------------------------------------
bool webpage::_check_header()
{
    if(_header.empty())
    {
        wcurl   curl(_uri);
        int     code = curl.header(_header);

        if(_psite->_ip.empty())
        {
            _psite->_ip = curl._ip;
        }
        if( 404 == code)
        {
            GLOGW("page " << _uri << ": HTML CODE" << curl._code);
            return false;
        }
        _mime   = curl._mime;
        _date   = curl._date;
        //GLOGT("Header:" << _header);
    }
    if(_mime.find("text")==string::npos)
    {
        GLOGW("page is not text/*: " << _uri << ", mime:" << _mime);
        return false;
    }

    bool bm = _changed();
    if(bm) // this parsed the header entities
    {
        if(_deleted)
        {
            GLOGW(_uri << " http error:  404");
            return false;
        }
        if(!_location.empty())
        {
            so::rep_all("[\r\n]","",_location);
            if(!_location.empty())
            {
                GLOGW(_uri <<": page is redirected to: " << _location );
                _add_page(_location, "", _psite->_sessid, _depthdown, _ppageid);
            }
        }
        return false; //redirected
    }
    return true;
}

//-----------------------------------------------------------------------------
bool webpage::_check_content()
{
    PAGE_ST state = NEW_LINK;
    state = _touched();
    if(state==PG_DONE)
    {
        return false;
    }

    if(state==DB_LINK)
    {
        _db_update_link(true);//couse did dont change by date or content refresh sessid
        _add_links_from_dbpage();
        return false;
    }

    if(state==CHANGED_LINK)
    {
        _db_update_link(true);//couse did dont change by date or content refresh sessid
        _db_delete_children();
    }

    if(_content.empty())
    {
        char            out[8];
        wcurl           curl(_uri);
        unsigned char   result[MD5_DIGEST_LENGTH];
        _content.reserve(4096);
        if( 404 == curl.content(_content))
        {
            GLOGW(_uri << ": http error: " << curl._code);
            return false;
        }
        MD5((const unsigned char*)_content.c_str() , _content.length(), result);
        _md5.clear();
        for(int k=0; k<MD5_DIGEST_LENGTH; k++)
        {
            sprintf(out,"%02X", (char)result[k]);
            _md5+=out;
        }
    }
    return !_content.empty();
}

//-----------------------------------------------------------------------------
int webpage::_db_update_link(bool update)
{
//    wsite* ps = _psite.ptr();
    stringstream s;
    AutoTr l(db_lock(), _p_dbase);

    if(_pageid==0)
    {
        assert(update==false); //called to update only. cheking flag
        pthread_t tid = pthread_self();

        s << "INSERT INTO links (siteid, sessid, plinkid, url, title, dsc, name,txt, idat, size, md5,vis,dpth,nlinks,blks,ldat,tid,checkit) VALUES (" <<
            _psite->_siteid <<","<<_psite->_sessid <<","<<
            _ppageid <<","<< _S(_sqlize(_uri)) <<","<<
            _S(_sqlize(_title)) <<","<<
            _S(_sqlize(_desc)) <<","<<
            _S(_sqlize(_link_name)) <<",'content'" << ",NOW()," << 0 <<","<<
            _S(_md5) <<","<< 1 <<","<< _depthdown <<","<< _links_count <<
            "," << _bkls << ","<< _S(_date) "," << tid <<",1)";
        _p_dbase->qry(s);
        _pageid = _p_dbase->lastid();

        GLOGT(_uri << ": INSERT links: id="<<_pageid<<"siteid=" << _psite->_siteid <<",sessid="<< _psite->_sessid <<",plinkid="<< _ppageid);
    }
    else
    {
        //update this with site id
        s << "UPDATE links SET" <<
            " siteid="<<_psite->_siteid<<
            ",sessid="<<_psite->_sessid<<
            ",ldat="<<_S(_date)<<
            ",checkit=0 "<<
            ",flags=" << _flags<<
            " WHERE linkid="<<_pageid;
        _p_dbase->qry(s);

        //updte childrens
        s.str("");
        s << "UPDATE links SET" <<
            " siteid="<<_psite->_siteid<<
            ",sessid="<<_psite->_sessid<<
            ",ldat="<<_S(_date)<<
            ",checkit=1" <<
            " WHERE plinkid="<<_pageid;
        _p_dbase->qry(s);
    }
    if(_p_dbase->err()==1)
    {
        GLOGT(s);
        _assert(s.str(), _pageid>0);
    }
    return     _p_dbase->err();
}

//-----------------------------------------------------------------------------
inline int32_t __calc_wkey(const string& word)
{
    #define KSUM_VALUE 256
    int32_t sum = 0;
    for(size_t i=0; i<word.length(); ++i)
    {
        sum += (unsigned char)word[i];
    }
#ifdef MULTIPLE_KWS
    return (sum % MULTIPLE_KWS);
#else
    return sum % KSUM_VALUE;
#endif
}

//-----------------------------------------------------------------------------
void webpage::_db_finalize_page()
{
    glb_wset().pageadd();

    AutoTr       l(db_lock(), _p_dbase);
    struct autocomit
    {
        mysqlcls* _pmy;
        autocomit(mysqlcls* pmy):_pmy(pmy){
            _pmy->qry("START TRANSACTION");
        }
        ~autocomit(){
            _pmy->qry("COMMIT");
        }
    }            cc(_p_dbase);

    GLOGT(_uri << " finalizing{");
    _finalize_words();
    _finalize_site();
    _finalize_links();
    _finalize_assets();
    GLOGT(_uri << " finalized}");
}

//-----------------------------------------------------------------------------
void webpage::_finalize_site()
{
    stringstream s;
    s.str("");
    if(_psite->_logo.empty())
        _psite->_logo=_psite->_firstimg;

    s << "UPDATE domain SET" <<
        " nlinks="<<_psite->_nlinks<<
        ", srvip="<<_S(_psite->_ip)<<
        ", name="<<_S(_psite->_siteurl)<<
        ", title="<<_S(_psite->_title)<<
        ", idat=NOW()"<<
        ", logo="<<_S(_psite->_logo)<<
        " WHERE siteid="<<_psite->_siteid;
    _p_dbase->qry(s);

}

//-----------------------------------------------------------------------------
void webpage::_finalize_links()
{
    stringstream s;
    s << "UPDATE links SET" <<
            " nlinks="<< _links_count<<
            ",blks="<< _bkls<<
            ",txt="<<  _S(_sqlize(_compressed))<<
            ",title="<<  _S(_sqlize(_title))<<
            ",dsc="<<  _S(_sqlize(_desc))<<
            ",name="<< _S(_sqlize(_link_name))<<
            ",checkit=0"
            " WHERE linkid="<<_pageid;
    _p_dbase->qry(s);
    _compressed.clear();
}

//-----------------------------------------------------------------------------
void webpage::_finalize_words()
{
#ifdef MULTIPLE_KWS
    stringstream    ss[MULTIPLE_KWS+1];
#else
    stringstream    ss;
#endif
    const char*     wordd=0;
    int32_t         kid = 0;
    int32_t         ksum=0;
    size_t          elems=0;
    stringstream    s;
    map<const char*, uint32_t> _kwids;
    map<string,score>::const_iterator b = _scores.begin();

    for(; b!=_scores.end(); ++b)
    {
        wordd=b->first.c_str();
        if(*wordd==0)continue;

        ksum = __calc_wkey(wordd);
        s.str("");
        s << "INSERT IGNORE INTO dict (wrd,ksum) values ('"<<b->first<<"',"<<ksum<<")";
        _p_dbase->qry(s);
        kid = _p_dbase->lastid();
        if(0 == kid)
        {
            s.str("");
            s << "SELECT wordid FROM dict WHERE ksum="<<ksum<<" AND "<<"wrd='"<<b->first<<"';";
            SR rezs(_p_dbase->qry(s, true));
            if(rezs.ok() && rezs->rows())
            {
                kid = _U(rezs->fetch(0));
            }
            else
            {
                GLOGE("cannot fetch:" << s.str());
                kid=-1;
            }
        }
        _kwids[b->first.c_str()]=kid;
        ++elems;
    }

    b =  _scores.begin();
#ifdef MULTIPLE_KWS
    for(size_t ct=0; ct < elems && b!=_scores.end(); ++b,++ct)
    {
         tid = __calc_wkey(b->first);
        ::sprintf(tblname,"words%d",tid);


        if(ss[tid].str().empty())
        {
            ss[tid] << "INSERT INTO "<<tblname<<" (linkid,wordid,weight,count,siteid) VALUES ";
            ss[tid] << "("<<_pageid << "," <<
                        _kwids[b->first.c_str()] << "," <<
                        b->second._weight << "," <<
                        b->second._count << "," <<
                        _psite->_siteid <<")";
            continue;
        }
        ss[tid] << ",("<<_pageid << "," <<
                    _kwids[b->first.c_str()] << "," <<
                    b->second._weight << "," <<
                    b->second._count << "," <<
                    _psite->_siteid <<")";
    }
    _scores.clear();

    _p_dbase->enable_delay(false);
    for(int i=0;i<MULTIPLE_KWS;++i)
    {
        if(!ss[i].str().empty())
        {
            //GLOGT( ss[i].str() );
            _p_dbase->qry(ss[i]);
            usleep(100);
        }
    }
    _p_dbase->enable_delay(true);
#else
    for(size_t ct=0; ct < elems && b!=_scores.end(); ++b,++ct)
    {
        ksum = __calc_wkey(b->first.c_str());
        if(ss.str().empty())
        {
            ss << "INSERT INTO words (linkid,wordid,weight,count,siteid,ksum) VALUES ";
            ss << "("<<_pageid << "," <<
                    _kwids[b->first.c_str()] << "," <<
                    b->second._weight << "," <<
                    b->second._count << "," <<
                    _psite->_siteid <<","
                    <<ksum<<")";
            continue;
        }
        ss << ",("<<_pageid << "," <<
                    _kwids[b->first.c_str()] << "," <<
                    b->second._weight << "," <<
                    b->second._count << "," <<
                    _psite->_siteid <<","<<
                    ksum<<")";
    }
    _scores.clear();

    if(!ss.str().empty())
    {
        _p_dbase->qry(ss);
        usleep(100);
    }
#endif

}

//-----------------------------------------------------------------------------
void webpage::_finalize_assets()
{
    stringstream s;
    //
    s << "DELETE FROM assets WHERE plinkid="<<_pageid<<" AND sessid="<< _psite->_sessid;
    _p_dbase->qry(s);

    s.str("");
    set<string>::const_iterator b = _assets.begin();
    for(; b!= _assets.end();++b)
    {
        if(s.str().empty())
        {
            s << "INSERT INTO assets (siteid,sessid,plinkid,href) ";
            s << "VALUES(" <<_psite->_siteid <<","<<_psite->_sessid<<","<<this->_pageid<<","<<_S(_sqlize(*b))<<")\n";
            continue;
        }
        s << ",("<<_psite->_siteid <<","<<_psite->_sessid<<","<<this->_pageid<<","<<_S(_sqlize(*b))<<")\n";
    }
    if(!s.str().empty())
    {
        _p_dbase->qry(s);
        usleep(2048);
    }
}

//-----------------------------------------------------------------------------
//return a copy...
string webpage::_from_parent(string& siteurl)
{
    string      fullurl;

    so::clean_uri(siteurl);
    _inherit(siteurl, this, fullurl);
    glb::qitm       itm(fullurl);
    webpage         pl(0,0,itm);
    return pl._uri;
}

//-----------------------------------------------------------------------------
bool webpage::_add_page(string siteurl, const string& name, uint32_t sid, uint32_t dpth, int32_t parentid)
{

    if(!so::bad_link(siteurl) )
    {
        string fullurl = _from_parent(siteurl);

        if(_psite->cango_extern(fullurl))
        {
            if(glb_wset().inset(fullurl))
            {
                GLOGD("Working on it "<< _uri);
                return false;
            }
            glb::qitm   itm(fullurl,_psite->_catid,
                                    _psite->_reindex,
                                    _psite->_maxdepth,
                                    _psite->_maxlinks,
                                    _psite->_goextern,
                                    _psite->_sessid,
                                    parentid,
                                    dpth,
                                    _psite->_nlinks);
            _links.push_back(itm);
            //glb_wset().enqueue_itm(_p_dbase, itm);m
            return true;
        }
        else
        {
            GLOGD("Extern: "<< _uri);
        }
    }
    //cout <<"ASSET:" << _uri << "\n";

    GLOGD("Bad link: "<< _uri);
    return false;
}

//-----------------------------------------------------------------------------
int32_t webpage::_score(const char* tag)const
{
    static struct TgScore
    {
        char tagL[16];
        int32_t weight;
    } tgss[]=
    {
        {"h1", 1000},
        {"h2", 950},
        {"h3", 900},
        {"h4", 850},
        {"h5", 800},
        {"href", 850},
        {"a", 850},
        {"src", 850},
        {"img", 850},
        {"title", 1000},
        {"meta", 900},
        {"b", 800},
        {"strong", 700},
        {"",0}
    };

    float weight = 100.0;

    for(int k=0; tgss[k].weight; ++k)
    {
        if(!strcmp(tag, tgss[k].tagL))
        {
            weight = tgss[k].weight;
            break;
        }
    }
    weight *= 1.0/(1+_depthdown);
    return (int32_t)weight;
}

//-----------------------------------------------------------------------------
bool webpage::_add_word(string word, int32_t weight, bool expression, bool incontent)
{
    if(!so::prep_kws(word))
        return false;

    if(word.find(' ')!=string::npos )
    {
        int                                 addon;
        std::stringstream                   strstr(word);
        std::istream_iterator<std::string>  it(strstr);
        std::istream_iterator<std::string>  end;
        std::vector<std::string>            result(it, end);

        std::vector<std::string>::iterator b = result.begin();
        for(; b!= result.end(); ++b)
        {
            if((*b).empty())  continue;

            string word = *b;
            if(so::is_common(word) && so::prep_kw(word))
            {
                //calc per site weight
                addon = 0;
                map<string,uint16_t>::iterator it = _psite->_scores.find(word);
                if(it != _psite->_scores.end())
                {
                    addon = it->second;
                }
                _scores[word]._weight = weight + addon;
                _scores[word]._count++;


                if(weight > _DOMAIN_SCORE)
                {
                    _psite->_scores[word] = (weight/100);
                }


            }
        }
    }
    else if(so::is_common(word))
    {
        _scores[word]._weight = weight;
        _scores[word]._count++;

        if(weight > _DOMAIN_SCORE)
        {
            _psite->_scores[word]=weight;
        }


    }

    if(_accum < GCFG->_mitza.pagelen)
    {
        _compressed+= word+" ";
        _accum += word.length() + 1;
    }
    return true;
}


bool webpage::_try_to_add(const string& link, const string& name)
{
    bool digged=false;

    if(_psite->cango_down(_depthdown+1) && _psite->add_link())
    {
        if(_add_page(link,  name.empty() ? link : name, _psite->_sessid, _depthdown+1, _pageid))
        {
            GLOGT("ENQUED<---" << link);
            digged=true;
            ++_links_count;
        }
        else
        {
            GLOGT("PARSED XX<---" << link);
            _psite->rem_link();
        }
    }
    return digged;
}

//-----------------------------------------------------------------------------
const char* webpage::_sqlize(const string& in)
{
    if( so::str_mysql_malf(in))
    {
        //reuse this buffer from thread instead bouncing in out of heap
        char* sqbuff = _pthr->strbuff(in.size() * 2 + 32);
        assert(sqbuff);
        ::mysql_real_escape_string(&_p_dbase->_mysql, sqbuff, in.c_str(), in.length());
        return sqbuff;
    }
    return in.c_str();
}

//-----------------------------------------------------------------------------
bool webpage::_db_create(uint32_t linkid)
{
    stringstream s;
    AutoTr       l(db_lock(), _p_dbase);

    s << "SELECT url FROM links WHERE linkid="<<linkid;
    SR rezl(_p_dbase->qry(s, true));
    if(rezl->rows())
    {
        MYSQL_ROW r = rezl->fetch();
        if(r)
        {
            _uri = r[0];
            so::url_parse(_uri,_proto,_host,_path,_port,_qry);
            return true;
        }
    }
    return false;
}

//-----------------------------------------------------------------------------
//            uri can be http://asdfasd or just doc
void webpage::_inherit(const string& uri, webpage* root, string& out)
{
    size_t  s =  uri.find("://");
    size_t  r = uri.find(root->_proto);
    if(r==string::npos && s==string::npos) //if has http://
    {
        out=root->_proto; //appenf http
        if(!root->_port.empty())
        {
            out+=":"+root->_port; //apend port
        }
        // if has no host
        if(uri.find(root->_host)==string::npos) //if ! has host, no proto no host
        {
            if(!uri.empty())
                out+=root->_host+"/";
            else
                out+=root->_host;
        }
    }
    out+=uri;
    so::replace_all(out,"www.","");
}

void webpage::_enqueue_links()
{
    AutoTr       l(db_lock(), _p_dbase);

    vector<glb::qitm>::const_iterator b=_links.begin();
    for(;b != _links.end();++b)
    {
        glb_wset().enqueue_itm(_p_dbase, *b);
    }
    _links.clear();
}

//-----------------------------------------------------------------------------
// longest function, google style...
bool webpage::_digpage()
{
    bool                        digged=false;
    bool                        content=false;
    int32_t                     weight = 0;
    int32_t                     linksperpage = 0;
    string                      tag,link,val,attr,name,metaw;
    HTML::ParserDom             parser;
    tree<HTML::Node>            dom = parser.parseTree(_content);
    tree<HTML::Node>::iterator  it = dom.begin();
    tree<HTML::Node>::iterator  end = dom.end();

    _content.clear();
    for (; it != end && __alive; ++it)
    {
        if(it->isTag())
        {
            tag = it->tagName();
            if(tag.empty())
                continue;
            so::lower_it(tag);

            weight+=_score(tag.c_str());

            if(tag=="link" ||
               tag=="script" ||
               tag=="img" ||
               tag=="a")
            {
                const string& t = it->text();
                if(t[0]=='<')
                {
                    _flags|=so::evalasset(t);
                    _assets.insert(t);
                }
            }
            if((tag=="a") || (tag=="meta") || (tag=="img"))
            {
                it->parseAttributes();
                const std::map<std::string, std::string>& attrs = it->attributes();
                if(attrs.size())
                {
                    std::map<std::string, std::string>::const_iterator b = attrs.begin();
                    for(; b!=attrs.end(); b++)
                    {
                        attr=(*b).first;
                        if(attr.empty()) continue;
                        so::lower_it(attr);
                        if(tag=="img" && b->first=="src")
                        {
                            link=b->second;
                            content=false;
                        }
                        else if(tag=="img" && b->first=="alt")
                        {
                            val=b->second;
                            content=true;
                        }
                        else if(tag=="a" &&   b->first=="href")
                        {
                            link=b->second;
                            content=true;
                        }
                        if(tag=="meta")
                        {
                            if( attr=="name")
                            {
                                if(b->second=="description")
                                {
                                    try{
                                        _desc = attrs.at("content");
                                        so::prep_dsc(_desc);
                                        content=false;

                                    }catch(std::out_of_range& r){}
                                }
                                else if(b->second=="keywords")
                                {
                                    try{
                                        content=false;
                                        val=attrs.at("content"); //this page desc
                                        so::prep_dsc(val);

                                        content=true;
                                    }catch(std::out_of_range& r){}
                                }
                            }
                            else if(attr=="http-equiv")
                            {
                                //not a freking shit library works properly in linuxoze messs
                                val = it->text();
                                size_t p =val.find("url=");
                                if(p!=string::npos)
                                {
                                    val=val.substr(p+4);  // http://www.airsoftdepot.ca/catalog
                                    p=val.find("\"");
                                    if(p==string::npos)
                                        p=val.find("\'");
                                    if(p!=string::npos)
                                    {
                                        val=val.substr(0,p);
                                        if(val!=_uri)
                                        {
                                            digged|=_try_to_add(val,val);
                                        }
                                    }
                                }
                                content=false;
                            }
                        }
                    }
                }
            }
        }
        else
        {
            if(tag=="title")
            {
                _title=it->text(); //this page title
                so::prep_dsc(_title);
                content=true;
            }
            if(tag=="a" && attr=="href")
            {
                name=it->text();  // next page link name
                content=false;
            }
            if(val.empty())
            {
                val = it->text();
                so::prep_dsc(val);
                content=true;
            }

            if(!link.empty() && link[0]!='#' && link!="/")
            {
                //cout <<"ASSET:" << link << "\n";

                if(tag=="img")
                {
                    //hold on and if logo not found, use this image.
                    if(_psite->_firstimg.empty())
                    {
                        _psite->_firstimg = link;
                    }
                    //const char* pl=_psite->_logo.c_str();
                    if(_psite->_logo.empty() && (val.find("logo")!=string::npos || link.find("logo")!=string::npos))
                    {
                        _psite->_logo = link;
                    }
                    digged|=_add_word(val, weight, false/*is a name*/, content);

                }
                else
                {
                    if(GCFG->_mitza.linksperpage &&
                       linksperpage < GCFG->_mitza.linksperpage)
                    {
                        if(_try_to_add(link,  name.empty() ? link : name))
                        {
                            ++linksperpage;
                            digged=true;
                        }
                    }
                    else
                    {
                        GLOGW("reached maximum linkes per page: ignoring:"<<link);
                    }
                    if(!name.empty())
                        digged|=_add_word(name, weight, true/*is a name*/, content);
                }
            }
            else if(!val.empty() && val[0]!='<' && val.length()>2)
            {
                if(tag!="script")
                    digged|=_add_word(val, weight, false, content);
            }

            val.clear();
            link.clear();
            tag.clear();
            weight=0;
            content=false;
        }
    }

    return digged;
}



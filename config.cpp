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

#include <assert.h>
#include <iostream>
#include <stdarg.h>
#include <limits.h>
#include "config.h"
#include <strutils.h>
#include "database.h"

//-----------------------------------------------------------------------------
Conf* GCFG;

//-----------------------------------------------------------------------------
Conf::Conf()
{
    _depth=3;
    _links=1024;
    _pages=8;
}

//-----------------------------------------------------------------------------
Conf::~Conf()
{
}

void Conf::load(const char* cfgFile)
{
    string loco = cfgFile;
    loco += ".conf";

    FILE* pf = _ttfopen(loco.c_str(), "rb");
    if(pf)
    {
        AutoCall<int (*)(FILE*), FILE*>    _a(::fclose, pf);
        char        pred[128];
        char        val[128];
        char        line[256];
        bool        inComment = false;

        try
        {
            while(!feof(pf))
            {
                if(fgets(line,255,pf))
                {
                    str_prepline(line);
                    if(*line==0)
                        continue;

                    if(inComment || *line=='#')
                        continue;

                    const char* pnext = str_ccpy(pred, line,'=');

                    if(*pred=='[')
                    {
                        str_lrtim(pred);
                        _section = pred;
                        continue;

                    }
                    else if(*pred=='}')
                    {
                        _assign("}", " ", 0);
                    }
                    if(pnext && *pnext)
                    {
                        str_scpy(val, (char*)pnext+1, "#");

                        str_lrtim(val);
                        str_lrtim(pred);
                        _assign(pred, val, 0);
                    }
                 }
                else
                    break;
                if(feof(pf))
                {
                    break;
                }
            }
        }
        catch(int& err)
        {
        }
    }
    else
    {
        printf( "Cannot find configuration file in curent folder\r\n");
        exit(0);
    }
    _mitza.depth++;
    _blog = 0;
    _blog |= _mitza.blog.find('I') == string::npos ? 0 : 0x1;
    _blog |= _mitza.blog.find('W') == string::npos ? 0 : 0x2;
    _blog |= _mitza.blog.find('E') == string::npos ? 0 : 0x4;
    _blog |= _mitza.blog.find('T') == string::npos ? 0 : 0x8;
    _blog |= _mitza.blog.find('D') == string::npos ? 0 : 0x10;
    _blog |= _mitza.blog.find('X') == string::npos ? 0 : 0x20;
     _mitza.delay *=  1000; //ms to us
     _mitza.dbsleep *= 1000; //
     if(!_mitza.qdir.empty())
        mkdir(_mitza.qdir.c_str(), S_IWUSR|S_IRUSR);

    //cout <<     _mitza.urlregex << "\n";
    if(_mitza.urlregex.empty())
    {
        _mitza.urlregex="(\\?|&)(oscsid|osCsid|SID|sid|s|S|jsessionid|sessionid)=[0-9a-zA-Z]+\n";
        _mitza.urlregex+="(\\?|&)[0-9a-zA-Z]+(oscsid|osCsid|SID|sid|s|S|jsessionid|sessionid)=[0-9a-zA-Z]+\n";
        _mitza.urlregex+="(\\?|&)^(oscsid|osCsid|SID|sid|s|S|jsessionid|sessionid)=[0-9a-zA-Z]+\n";
        _mitza.urlregex+="$\\?\n";
        _mitza.urlregex+="#+\n";
        _mitza.urlregex+="^/\n";
        _mitza.urlregex+="index\.(php|html|asp|htm)$\n";
        _mitza.urlregex+="[/'.]+$\n";
        _mitza.urlregex+="[/\".]+$\n";
        _mitza.urlregex+="[\/]+$";
    }
    if(_mitza.threads > 64)         _mitza.threads=64;
    if(_mitza.pagelen > 256000)     _mitza.pagelen = 256000;
    if(_mitza.preque > 200)         _mitza.preque=200;
    if(_mitza.cache > 4000)         _mitza.cache=4000;

}
//-----------------------------------------------------------------------------
#define BIND(mem_,name_)     _bind(lpred, #name_,  mem_.name_, val);

//-----------------------------------------------------------------------------
void Conf::_assign( const char* pred, const char* val, int line)
{
    char    lpred[256];
    char    loco[256];

    strcpy(loco,val);
    strcpy(lpred,pred);

    try
    {
        // read all the configuration values from mitza.config
        if(_section == "[global]")
        {
            BIND(_mitza,  threads);
            BIND(_mitza,     host);
            BIND(_mitza, database);
            BIND(_mitza,     user);
            BIND(_mitza, password);
            BIND(_mitza, blog);
            BIND(_mitza, pagelen);
            BIND(_mitza, sitefile);
            BIND(_mitza, depth);
            BIND(_mitza, delay);
            BIND(_mitza, cache);
            BIND(_mitza, qdir);
            BIND(_mitza, preque);
            BIND(_mitza, urlregex);
            BIND(_mitza, dbsleep);
            BIND(_mitza, linksperpage);
            BIND(_mitza, linkspersite);
            BIND(_mitza, leavesite);
            BIND(_mitza, reindex);
        }
    }
    catch(int done)
    {

    }
}


//-----------------------------------------------------------------------------
// not used.
void    Conf::check_log_size()
{

}

//-----------------------------------------------------------------------------
// not used
void Conf::rollup_logs(const char* rootName)
{
    char oldFn[256];
    char newFn[256];

    int iold = 40;
    sprintf(oldFn,"%s.log20",rootName);
    unlink(oldFn);

    while(iold-- > 0)
    {
        sprintf(newFn, "%s.log%d",rootName ,iold);
        sprintf(oldFn, "%s.log%d",rootName, iold-1);
        rename(oldFn, newFn);
    }
}

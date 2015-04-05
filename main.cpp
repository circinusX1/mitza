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
#include "os.h"
#include "strops.h"
#include "main.h"
#include "database.h"
#include "webpage.h"
#include "wcurl.h"
#include "config.h"
#include "thr.h"

//-----------------------------------------------------------------------------
bool __alive = true;
void ControlC (int i)
{
    __alive = false;
    printf("Control C. Exiting...(just wait ... stopping threads...) \n");
    usleep(0x1FFFFF);
}

//-------------------------------------------------------------------------------
void ControlP (int i)
{
    __alive=false;
    sleep(2);
}


//-----------------------------------------------------------------------------
int main(int nargs, char* vargs[])
{
    mysqlcls    db;

    signal(SIGINT,  ControlC);
    signal(SIGABRT, ControlC);
    signal(SIGKILL, ControlC);
    signal(SIGTRAP, SIG_IGN);
    signal(SIGPIPE, SIG_IGN); //this is crap

    if(nargs==1)
    {
        cout << "to create tables ./mitza --create\n to run ./mitza --run\n";
        return -1;
    }

    Conf        config;

    GCFG=&config;
    config.load(vargs[0]);
    db.connect(config._mitza.host.c_str(),config._mitza.user.c_str(),config._mitza.password.c_str());
    if(nargs>1 && !::strcmp(vargs[1],"--create"))
    {
        cout << "creating database and fresh tables. Now run --run\n";
        db.createtables();
        return 0;
    }

#ifdef _DEBUG
    db.createtables();
#endif

    vector<cthread*>  mt;
    struct  stat      st;
    uint32_t          sessid = 0; //db.sess_id();
    stringstream      s;

    //site file passed in
    if(nargs>1 )
    {
        if(::access(vargs[1], 0)==0)
        {
            config._mitza.sitefile=vargs[1];
        }

        if(::strstr(vargs[1],"http"))
        {
            FILE* pf = fopen("/tmp/mitza-last-sites-file.txt","wb");
            if(pf)
            {
                fprintf(pf, "%s,-1,1,5,4096,0\n",vargs[1]);
                //http://www.site.com,cat,reindex,max-depth,max-links-persite,go-out-ofsite,
                ::fclose(pf);
                config._mitza.sitefile="/tmp/mitza-last-sites-file.txt";
            }
        }

    }

    s << "SELECT sessid FROM que LIMIT 1";
    SR rezl(db.qry(s, true));
    MYSQL_ROW row = rezl->fetch();
    if(row && row[0])
    {
        sessid = _U(row[0]); // continue with queue
    }
    else
    {
        sessid = db.sess_id();
    }
    FILE* pf=0;
    if(stat(config._mitza.sitefile.c_str(), &st)==0 &&
       st.st_size > 10 &&
       (pf = ::fopen(config._mitza.sitefile.c_str(),"rb")))
    {
        char    line[256];

        while(!feof(pf) && __alive)
        {
            fgets(line,255,pf);
            so::trimcrlf(line);
            if(line[0]==0)
                continue;
            if(::strstr(line,"STOP"))
            {
                __alive=false;
                break;
            }
            if(line[0]!='#')
            {
                do{

                    string      site;
                    int32_t     catid=0;
                    int32_t     reindex=GCFG->_mitza.reindex;
                    int32_t     maxdepth=GCFG->_mitza.depth;
                    int32_t     maxlnks=GCFG->_mitza.linkspersite;
                    int32_t     externals=GCFG->_mitza.leavesite;

                    char* p = strtok(line,","); if(0==p)goto FINALLY;
                    site=p;
                    p = strtok(0,","); if(0==p)goto FINALLY;
                    catid=::atoi(p);
                    p = strtok(0,","); if(0==p)goto FINALLY;
                    reindex=::atoi(p);
                    p = strtok(0,","); if(0==p)goto FINALLY;
                    maxdepth=::atoi(p);
                    p = strtok(0,","); if(0==p)goto FINALLY;
                    maxlnks=::atoi(p);
                    p = strtok(0,","); if(0==p)goto FINALLY;
                    externals=::atoi(p);
                // the line
                FINALLY:
                GLOGI("MAIN ADDING : "<<site<<"cat:"<<catid<<" reindex:"<<reindex<<" maxdepth:"<<maxdepth);
                    glb::qitm i(site,catid,reindex,maxdepth,maxlnks,externals,sessid,-1,0,0);
                    glb_wset().enqueue_itm(&db, i);
                }while(0);
            }
            if(::feof(pf))
                break;
        }
        fclose(pf);
        sleep(1);
        pf = ::fopen(config._mitza.sitefile.c_str(),"ab");
        if(!pf)
            pf = ::fopen(config._mitza.sitefile.c_str(),"wb");
        if(pf)
        {
            ::fputs("#http://www.site.com,cat,reindex,max-depth,max-links-persite,go-out-ofsite,#\n",pf);
            ::fclose(pf);
        }
    }

    if(glb_wset().buzzy(&db))
    {
        for(int i=0;i<config._mitza.threads;i++)
        {
            cthread* pt = new cthread();
            pt->start_thread();
            mt.push_back(pt);
        }
        sleep(2);
        while(glb_wset().buzzy(&db) && __alive)
        {
            sleep(4);
        }
        do{
            AutoTr  l(db_lock(), &db);

            glb_wset().flush(&db);
            sleep(2);
        }while(0);

        GLOGI("Stopping Threads...\n");
        for(int i=0;i<(int)mt.size();i++)
        {
            mt[i]->stop_thread();
            sleep(1);
            delete mt[i];
        }
    }
    GLOGI("done.\n");
}


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

#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include "os.h"
#include "database.h"
#include "config.h"
#include "main.h"


using namespace std;

//-----------------------------------------------------------------------------
static mutex  Gdbm;

//-----------------------------------------------------------------------------
bool mysqlcls::createtables()
{
    stringstream s;

    s << "drop database " << GCFG->_mitza.database;
    qry(s);
    s.str("");
    s << "create database " << GCFG->_mitza.database;
    qry(s);
    qry("commit");
    s.str("");  s << "use " << GCFG->_mitza.database;
    qry(s);

    string sys("mysql --user=");
    sys += GCFG->_mitza.user +
            " --password="+
            GCFG->_mitza.password +
            " "+
            GCFG->_mitza.database +
            "< sql/tables.sql";
    system( sys.c_str() );
    qry(s);
    SR r(qry("SELECT id FROM sess;", true));
    if(r->rows() == 0)
    {
        qry("INSERT INTO sess (id) values (1);");
    }
    qry("ALTER TABLE domain AUTO_INCREMENT = 1");
    qry("ALTER TABLE links AUTO_INCREMENT = 1");
    qry("ALTER TABLE dict AUTO_INCREMENT = 1");
    qry("ALTER TABLE que AUTO_INCREMENT = 1");
    qry("commit");
    return true;
}

//-----------------------------------------------------------------------------
uint32_t mysqlcls::sess_id()
{
    qry("UPDATE sess SET id=id+1");
    SR r(qry("SELECT id FROM sess;", true));
    return ::atol(r->fetch()[0]);
}

//-----------------------------------------------------------------------------
mysqlcls::mysqlcls():_connected(false),_bdelay(true)
{

}

//-----------------------------------------------------------------------------
mysqlcls::~mysqlcls()
{
    disconnect();
}

bool     mysqlcls::_reconnect(int tout)
{
    disconnect();
    sleep(tout);
    return connect(_h.c_str(), _u.c_str(), _p.c_str());
}

//-----------------------------------------------------------------------------
bool     mysqlcls::connect(const char* hostname, const char* username, const char* password)
{
    mysql_thread_init();
    mysql_init(&_mysql); // link with -lmysqlclient

    mysql_options(&_mysql,MYSQL_READ_DEFAULT_GROUP,"mitza");
    mysql_options(&_mysql,MYSQL_OPT_COMPRESS,0);
    if (!mysql_real_connect(&_mysql,hostname,username,password,"",0,NULL,0))
    {
        error(); //install: mysql-server-5.5 mysqlclient-dev
        return 0;
    }
    if (!mysql_thread_safe())
    {
        GLOGE( "mysql is not thread safe \n");
        assert(0); // Major problem: libmysqlcompiled ThreadSafe?
    }

    _connected=true;
    qry("use mitza;");
    _h=hostname;
    _u=username;
    _p=password;
    return 1;

}

//-----------------------------------------------------------------------------
void    mysqlcls::disconnect()
{
    if(_connected)
    {
        mysql_close(&_mysql);
        mysql_thread_end() ;
    }
    _connected=false;
}

//-----------------------------------------------------------------------------
void mysqlcls::error()
{
    GLOGE(_lastq);
    GLOGE("ERROR: " << _err <<": "<< mysql_error(&_mysql) << "\n");
    if(_err == 1){
        cout << "Giving up...\n";
        __alive=false;
    }
    sleep(4);
}

//-----------------------------------------------------------------------------
dbresult* mysqlcls::qry(const stringstream& s, bool ret)
{
    return qry(s.str().c_str(), ret);
}

//-----------------------------------------------------------------------------
dbresult* mysqlcls::qry(const string& s, bool ret)
{
    return qry(s.c_str(), ret);
}


//-----------------------------------------------------------------------------
dbresult* mysqlcls::qry(const char* q, bool ret)
{
    if(_bdelay)
        usleep(GCFG->_mitza.dbsleep);
    _err=0;
    _lastq = q;
    do{
        _err =  mysql_query(&_mysql, q);
        if(0==_err)
        {
            if(ret)
            {
                return new dbresult(mysql_store_result(&_mysql));
            }
            return 0;
        }
    }while(0);
    error();
    return 0;
}

void mysqlcls::use_db (const char* s)
{
    _lastq = s;
    mysql_select_db(&_mysql,s);
}

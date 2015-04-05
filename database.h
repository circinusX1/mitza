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


#ifndef DBASEPRX_H
#define DBASEPRX_H

#include <os.h>
#include <assert.h>
#include <string>
#include <iostream>
#include <sstream>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <mysql/mysql.h> // sudo apt-get install libmysqlclient-dev

//-----------------------------------------------------------------------------
using namespace std;

//-----------------------------------------------------------------------------
class dbresult
{
    MYSQL_RES* _prez;
public:
    dbresult(MYSQL_RES* p):_prez(p) {}
    dbresult():_prez(0) {}
    ~dbresult() {
        if(_prez)
            mysql_free_result(_prez);
    }

    inline uint32_t rows() {
        if(_prez)
            return  mysql_num_rows(_prez);
        return 0;
    }

    MYSQL_ROW fetch() {
        static char* S[]= {_CC("0"),};
        if(_prez)
            return mysql_fetch_row(_prez);
        return S;
    }
    char* fetch(int index) {
        static char SS[]="0";
        if(_prez)
            return mysql_fetch_row(_prez)[index];
        return SS;
    }
};

//-----------------------------------------------------------------------------
class SR
{

public:
    SR(dbresult* pr):_pr(pr) {}
    ~SR() {
        if(_pr) delete _pr;
    }
    dbresult* operator->() {
        static dbresult sr;
        return _pr  ? _pr : & sr;
    }
    SR(SR& r):_pr(r._pr) {
        r._pr=0;
    };
    void detach() {
        if(_pr)
            delete _pr;
        _pr = 0;
    }

    SR& operator=(dbresult* pr) {
        delete _pr;
        _pr = pr;
        return *this;
    }
    bool ok(){return _pr!=0;}
private:

    SR& operator=(const SR& r);
    dbresult* _pr;
};

//---------------------------------------------------------------------------------------
class mysqlcls
{
    bool _connected;
public:
    mysqlcls();
    virtual ~mysqlcls();
    inline my_ulonglong lastid() {
        return mysql_insert_id(&_mysql);
    }
    bool connect(const char* hostname, const char* username, const char* password);
    void disconnect();
    void error();
    int  err()const {
        return _err;
    }
    dbresult* qry(const stringstream& s, bool ret=false);
    dbresult* qry(const string& s, bool ret=false);
    dbresult* qry(const char* q, bool ret=false);
    void use_db (const char* s);
    bool createtables();
    void enable_delay(bool b){_bdelay=b;}
    uint32_t sess_id();
private:
    bool    _reconnect(int tout);
public:
    MYSQL   _mysql;
    string  _lastq;
    int     _err;
    string   _h,_u,_p;
    bool    _bdelay;
};


class AutoTr
{
public:
    mutex* _pm;
    mysqlcls* _pc;
    public:
        AutoTr(mutex* m, mysqlcls* pc):_pm(m),_pc(pc){
          //  pc->qry("COMMIT;");
            _pm->mlock();

        }
        ~AutoTr(){
            _pm->munlock();
           // _pc->qry("START TRANSACTION;");
        }

};




#endif // DBASEPRX_H

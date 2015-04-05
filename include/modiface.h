#ifndef MOD_IFACE_H
#define MOD_IFACE_H

#include <matypes.h>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stddef.h>

#include <sys/time.h>
//------------------------------------------------------------------------------------
#define NON_COPYABLE(T_)     T_(){}\
                             T_(const T_&){};\
                             T_& operator=(const T_&){return *this;}
//------------------------------------------------------------------------------------
#define DLL_EXPORT __attribute__((visibility("default")))
#define DLL_LOCAL __attribute__((visibility("hidden")))
//------------------------------------------------------------------------------------

typedef enum _CAN_RW
{
    CAN_ZERO,
    CAN_READ=0x1,
    CAN_WRITE=0x2,
    CAN_REWR=0x3,
    CAN_READR=0x4,
    CAN_WRITER=0x8,
    CAN_REWRR=0xC
}CAN_RW;


typedef enum _HASH_METH
{
    eGET=153,
    ePOST=246,
    eHEAD=202,
    ePUT=169,
    eDEL=367,
    eOPTIONS=477,
    eTRACE=283,
    eCONNECT=455,
}HASH_METH;
//data for the module
struct KeyValx
{
    kchar* key;         //disiminated GET
    kchar* val;
};

//------------------------------------------------------------------------------------
class AdminModule;
class  CtxMod //<<<< move HttpRequest data here
{
    NON_COPYABLE(CtxMod);
public:
           CtxMod(int):_settings(0),_url_doc(0),_url_path(0),_dir_home(0),_dir_cache(0),
                       _doc_index(0),_start_time(time(0)),_hdrs(0),
                       _get(0),_cookies(0),_tempfile(0),_templen(0){}

    virtual ~CtxMod(){}

    friend class AdminModule;

    virtual int  socket_write(kchar*)=0;
    virtual int  socket_write(kchar*, size_t len)=0;
    virtual int  socket_read(char* ,size_t len)=0;

    virtual void    log_string(kchar*, ...)=0;
    int             _settings;
    kchar*     _url_doc;                   //  path/THIS  from GET
    kchar*     _url_path;                  // THIS/index.html from GET
    kchar*     _dir_home;                  //  var/www           from config
    kchar*     _dir_cache;
    kchar*     _doc_index;                 // config
    kchar*     _cli_ip;
    time_t          _start_time;
    HASH_METH       _emethod;
    const KeyValx*  _hdrs;                      // ptr to headers. last is 0
    const KeyValx*  _get;
    const KeyValx*  _cookies;
          FILE*     _tempfile;
          int64_t   _templen;
protected:
    virtual void get_report(std::ostringstream& ss)=0; //okward, but...

};


typedef enum _FD_OPS{
    eFD_SET,
    eFD_IS,
    eFD_CLEAR,
}FD_OPS;
//handler not quite an interface, but works
class CtxesThread;
class ClientThread;
class IModule
{
protected:
    NON_COPYABLE(IModule);
public:
    friend class CtxesThread;
    friend class Context;
    friend class ClientThread;


    IModule(kchar* cook):_pdata(0) {::strcpy(_cookie,cook);}
    virtual ~IModule(){}
    /// called once before going to call handle (repeatetly)
    /// one time initialisation here.
    virtual bool construct(CtxMod* pctx)=0;
    /// called to handle the context
    virtual int  work(CtxMod* pctx, CAN_RW rw)=0;
    /// should put the object in after-construction state.
    /// if the instance is reused (somehow pooled) init will be called again.
    /// clean up one time initilisation here. erlease all objs allocated in handle
    virtual void   destroy() = 0;

    /// if the IHandler is singleton, either threaded or not should return true
    virtual bool is_singleton() = 0;

    virtual bool uses_fds()const=0;
    virtual int  fd_action (FD_OPS op, fd_set& rd, fd_set& wr)=0;

private: // not for the module
    void  __set_data(const void * pdata){_pdata = 0;};
    void* __get_data()const{return _pdata;};
    kchar* __cookie()const{return _cookie;};
private:

    char  _cookie[256];
    void* _pdata;
};

//factory-----------------------------------------------------------------------------

typedef IModule* (*pFn_getFoo)(kchar* by_ext);
typedef void  (*pFn_releaseFoo)(IModule*, kchar* by_ext);

extern "C"
{
    IModule* factory_get(kchar* );
    void     factory_release(IModule* pm, kchar* );
    class    IModule;
}
#endif // MODIFACE_H

#ifndef CONSTS_H
#define CONSTS_H

#include <os.h>
#include <matypes.h>
#include <string.h>

#define _CC const_cast<char *>
struct Mimes
{
    char* k;
    char* v;
};

inline kchar* extractMime(kchar* by_ext)
{
	static struct Mimes __mime[]=
	{
		{_CC(".html"),_CC("text/html")},
		{_CC(".htm"),_CC("text/html")},
		{_CC(".png"),_CC("image/png")},
		{_CC(".css"),_CC("text/css")},
		{_CC(".js"), _CC("application/x-javascript")},
		{_CC(".ico"),_CC("image/x-icon")},
		{_CC(".gif"),_CC("image/gif")},
		{_CC(".jpg"),_CC("image/jpeg")},
		{_CC(".jpeg"),_CC("image/jpeg" )},
		{_CC(".mpg"),_CC("video/mpeg")},
		{_CC(".mpeg"),_CC("video/mpeg")},
		{_CC(".asf"),_CC("video/x-ms-asf")},
		{_CC(".avi"),_CC("video/x-msvideo")},
		{_CC(".wav"),_CC("audio/x-wav")},
		{_CC(".zip"),_CC("application/x-zip-compressed")},
		{_CC(".tgz"),_CC("application/x-tar-gz")},
		{_CC(".tar"),_CC("application/x-tar")},
		{_CC(".rar"),_CC("application/x-arj-compressed")},
		{_CC(".gz"), _CC("application/x-gunzip")},
		{_CC(".bmp"),_CC("image/bmp")},
		{_CC(".shtm"),_CC("text/html")},
		{_CC(".shtml"),_CC("text/html")},
		{_CC(".svg"),_CC("image/svg+xml")},
		{_CC(".torrent"),_CC("application/x-bittorrent")},
		{_CC(".mp"),_CC("audio/x-mp")},
		{_CC(".mid"),_CC("audio/mid")},
		{_CC(".mu"),_CC("audio/x-mpegurl")},
		{_CC(".ram"),_CC("audio/x-pn-realaudio")},
		{_CC(".xml"),_CC("text/xml")},
		{_CC(".xslt"),_CC("application/xml")},
		{_CC(".ra"), _CC("audio/x-pn-realaudio")},
		{_CC(".doc"),_CC("application/msword")},
		{_CC(".exe"),_CC("application/octet-stream")},
		{_CC(".xls"),_CC("application/excel")},
		{_CC(".arj"), _CC("application/x-arj-compressed")},
		{_CC(".rtf"), _CC("application/rtf")},
		{_CC(".pdf"), _CC("application/pdf")},
		{_CC(".swf"), _CC("application/x-shockwave-flash")},
		{0, 0}
	};

    for(int k=0; __mime[k].k; ++k)
    {
        if(__mime[k].k[0] == by_ext[0] && !strcmp(by_ext+1, __mime[k].k+1) )
        {
            return __mime[k].v;
            break;
        }
    }
    return "text/html";
}

inline u_int64_t extractRanges(kchar* pz, u_int64_t b, u_int64_t& s, u_int64_t& e)
{
    if(0 == pz) return b;
    kchar*   pd = _ttstrchr(pz,'-');
    if(*(pd+1))
    {
        sscanf(pz, "bytes=%luu-%luu", (u_int64_t*)&s, (u_int64_t*)&e);
        return e-s+1;
    }
    sscanf(pz, "bytes=%zu-", (u_int64_t*)&s);
    return b-s;
}

#define APP_ERRBASE     0x8000
#define APP_LASTERROR               APP_ERRBASE+18
struct Mex
{
private:
    int     _code;
    int     _line;
    char    _sfile[256];
    char    _ext[64];
public:

    Mex(int code, kchar* f, int l, kchar* ext=0):_code(code),_line(l)
    {
        char fn[256] = {0};
        strncpy(fn, f, 254);
        ::sprintf(_sfile, "%s", (char*)basename(fn));
        ::memset(_ext,0,sizeof(_ext));
        if(ext) strncpy(_ext, ext, sizeof(_ext)-1); else _ext[0]=' ';
    }
    int code()const{return _code;}
    kchar* desc()const{

        static __thread char msg[256];
        sprintf(msg,"%s:%s@%d (%s)", http_err(_code), _sfile, _line, _ext);
        return msg;
    }

    static kchar* http_err(int code)
    {
        static __thread char retstr[256];

        static  kchar*  err400[40]={
                    _CC("400 Bad Request"),
                    _CC("401 Unauthorized"),
                    _CC("402 Payment Required"),
                    _CC("403 Forbidden"),
                    _CC("404 Not Found"),
                    _CC("405 get_meth_kv Not Allowed"),
                    _CC("406 Not Acceptable"),
                    _CC("407 Proxy Authentication Required"),
                    _CC("408 Request Timeout"),
                    _CC("409 Conflict"),
                    _CC("410 Gone"),
                    _CC("411 Length Required"),
                    _CC("412 Precondition Failed"),
                    _CC("413 Request Entity Too Large"),
                    _CC("414 Request-URI Too Long"),
                    _CC("415 Unsupported Media Type"),
                    _CC("416 Requested Range Not Satisfiable"),
                    _CC("417 Expectation Failed")};

        static const  char*  err500[32]={
                    _CC("500 Internal Server Error"),
                    _CC("501 Not Implemented"),
                    _CC("502 Bad Gateway"),
                    _CC("503 Service Unavailable"),
                    _CC("504 Gateway Timeout"),
                    _CC("505 HTTP Version Not Supported")};


        static const  char*  errPROXY[32]={
                                        _CC(" CONTEX_BUFF_FULL"),
                                        _CC(" REMOTE_CLOSED_UNESPECTED"),
                                        _CC(" BLOCKED_HOST"),
                                        _CC(" CONNECTION_FAILED"),
                                        _CC(" INVALID_4_HEADER"),
                                        _CC(" CANNOT_CONNECT"),
                                        _CC(" IP_REJECTED"),
                                        _CC(" REMOTE_CLOSED_ONSEND"),
                                        _CC(" INVALID_5_HEADER"),
                                        _CC(" NO_5_KNOWNMETHODS"),
                                        _CC(" INVALID_5_COMMAND"),
                                        _CC(" NOT_5_IMPLEMENTEDIPV6"),
                                        _CC(" CANNOT_PARSE_HTTP"),
                                        _CC(" CIRCULAR_LINK"),
                                        _CC(" OUT_OF_MEMORY"),
                                        _CC(" CONTEXT_DONE"),
                                        _CC(" CLIENT_CLOSED"),
                                        _CC(" SSH_NO_HNAME"),
                                        _CC(" SSH_NO_HNAME"),
                                        _CC(" SSH_NO_HNAME"),
                                      };

        if(code >= 500 && code <= 505 ){
            return err500[code-500];
        }
        else if(code >= 400 && code <= 417 ){
            return err400[code-400];
        }
        else if(code >= APP_ERRBASE && code < APP_LASTERROR){
            return errPROXY[code - (APP_ERRBASE+1)];
        }   // is errno
        strerror_r(code, retstr, sizeof(retstr)-1);
        return retstr;
    }
};



#define __noop  (void(0))

#endif //CONSTS_H

/*
    Author: Octavian-Marius Chincisan
*/
#ifndef __OS_H__
#define __OS_H__

#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <inttypes.h>
#include <matypes.h>


#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <set>
#include <vector>
#include <deque>
#include <algorithm>

#ifdef MA_UNICODE
#   include <wchar.h>
#   define _ttfopen      _wfopen
#   define _ttstrchr     wcschr
#   define _ttstrstr     wcsstr
#   define _ttsnprintf   wsnprintf
#   define _ttatoi       _wtoi
#   define _ttstrtok     wcstok
#   define _ttgetenv     _wgetenv
#   define _ttsystem     _wsystem
#   define _ttasctime    _wasctime
#   define _ttremove     _wremove
#   define _ttrename     _wrename
#else
#   define _ttfopen      fopen
#   define _ttstrchr     strchr
#   define _ttstrstr     strstr
#   define _ttsnprintf   snprintf
#   define _ttatoi       atoi
#   define _ttstrtok     strtok
#   define _ttgetenv     getenv
#   define _ttsystem     system
#   define _ttasctime    asctime
#   define _ttremove     remove
#   define _ttrename     rename
#endif

#define UNUS(x_)   (void)(x_)

#define _CC const_cast<char *>
//-----------------------------------------------------------------------------
class mutex;
class condition
{
public:
    condition()
    {
        pthread_cond_init(&_cond, NULL);
        pthread_mutex_init(&_mutex  ,NULL);
    }
    ~condition()
    {
        pthread_cond_signal(&_cond);
        pthread_mutex_unlock(&_mutex);

        pthread_cond_destroy(&_cond);
        pthread_mutex_destroy(&_mutex);
    }
    void lock()
    {
        pthread_mutex_lock(&_mutex);
    }
    void signal()
    {
        pthread_cond_signal(&_cond);
    }
    void broadcast()
    {
        pthread_cond_broadcast(&_cond);
    };

    void wait()
    {
        pthread_cond_wait(&_cond, &_mutex);
    }
    void unlock()
    {
        pthread_mutex_unlock(&_mutex);
    }
private:

    pthread_cond_t _cond;
    pthread_mutex_t _mutex;
};
//-------;----------------------------------------------------------------------
class mutex
{
    mutable pthread_mutex_t _mut;
public:
    mutex()
    {
        pthread_mutexattr_t     attr;

        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&_mut,&attr);
        pthread_mutexattr_destroy(&attr);
    }

    virtual ~mutex()
    {
        pthread_mutex_unlock(&_mut);
        pthread_mutex_destroy(&_mut);
    }

    int mlock() const
    {
        return pthread_mutex_lock(&_mut);
    }

    int try_lock() const
    {
        return pthread_mutex_trylock(&_mut);
    }

    int munlock() const
    {
        return pthread_mutex_unlock(&_mut);
    }
};

//-----------------------------------------------------------------------------
class semaphore
{
    sem_t _sem;

public:
    semaphore( int init = 0 )
    {
        sem_init(&_sem,0,init);
    }

    virtual ~semaphore()
    {
        sem_destroy(&_sem);
    }

    int swait(int to=-1) const
    {
        UNUS(to);
        sem_wait((sem_t *)&_sem);
        return 1;
    }

    int stry_wait(int to=-1) const
    {
        UNUS(to);
        return (sem_trywait((sem_t *)&_sem)?errno:0);
    }

    int snotify() const
    {
        return (sem_post((sem_t *)&_sem) ? errno : 0);
    }

    int value() const
    {
        int val = -1;
        sem_getvalue((sem_t *)&_sem,&val);
        return val;
    }
    void reset( int init = 0 )
    {
        sem_destroy(&_sem);
        sem_init(&_sem,0,init);
    }
};


//-----------------------------------------------------------------------------
class AutoLock
{
public:
    AutoLock(const mutex* m):_mutex((mutex*)m)
    {
        if(_mutex)
        _mutex->mlock();
    }
    AutoLock(mutex* mutex):_mutex(mutex)
    {
        if(_mutex)
        _mutex->mlock();
    }
    ~AutoLock()
    {
        if(_mutex)
        _mutex->munlock();
    }
private:
    mutex* _mutex;
};

//-----------------------------------------------------------------------------
class os_thread
{
public:
    os_thread()
    {
        _bstop   = 1;
        _hthread = 0;
        _joined  = false;
        _init = -1;
    }

    virtual ~os_thread()
    {
        if(!_stopped)
        {
            stop_thread();
        }

        if(_joined && _hthread)
        {
            usleep(1000);
            ::pthread_detach(_hthread);
        }
        _hthread = 0;

    }
    virtual void _post_thread_foo()
    {
        ;
    }
    virtual bool _pre_thread_foo()
    {
        return true;
    }
    virtual int  start_thread()
    {
        _bstop   = 0;

        _init=pthread_attr_init(&_attr);
        pthread_attr_setdetachstate(&_attr, PTHREAD_CREATE_JOINABLE);

        if (pthread_create(&_hthread, &_attr, SFoo, this) != 0)
        {
            _hthread=0;
            pthread_attr_destroy(&_attr);
            return errno;
        }
        pthread_attr_destroy(&_attr);
        usleep(1000);
        _start.swait();
        return 0;
    }

    virtual void signal_to_stop()
    {
        _bstop = 1;
    }
    virtual void    stop_thread()
    {
        if(!_stopped)
        {
            _bstop = 1;
            usleep(10000);
        }
        t_join();
    }

    bool  is_stopped()
    {
        return _bstop;
    }

    int t_join()
    {
        if(!_joined && _hthread)
        {
            _joined = true;
            //printf("\njoining thread %p \n", _hthread);//mem leak fix valgrind
            return pthread_join(_hthread,0);
        }
        return 0;
    }

    int kill()
    {
        return 0;
    }
    int detach()
    {
        return 0;
    }

    void set_prio(int boost)
    {
        if(_hthread)
        {
           // pthread_setschedprio(_hthread, 32);
        }
    }
protected:
    virtual void thread_main() {};
    int         _bstop;
    THANDLE     _hthread;
private:
    int         _init;
    mutex       _mutex;
    semaphore   _start;
    pthread_attr_t  _attr;

    int         _stopped;
    bool        _joined;
    static void* SFoo(void* pData)
    {

        os_thread* pT = reinterpret_cast<os_thread*>(pData);
        pT->_stopped = 0;
        pT->_start.snotify();
        if(pT-> _pre_thread_foo())
        {
            pT->thread_main();
            pT->_stopped = 1;
            pT->_post_thread_foo();
            // no member access couse can be deleted
        }
        return 0;
    }
};


template < typename T, class P>struct AutoCall
{
    AutoCall(T f, P p):_f(f),_p(p) {}
    ~AutoCall()
    {
        _f(_p);
    }
    T _f;
    P _p;
};

using namespace std;


#define _PLATFORM_ "LINUX"
#endif // !__OS_H__


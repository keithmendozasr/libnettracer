#include <iostream>
#include <fstream>
#include <cerrno>
#include <cstring>
#include <sstream>
#include <cstdlib>
#include <map>
#include <stdexcept>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#ifndef __USE_GNU
    #define __USE_GNU
    #include <dlfcn.h>
    #undef __USE_GNU
#else
    #include <dlfcn.h>
#endif

#ifdef PRINT_DBG
    #define DEBUG(a) a
#else
    #define DEBUG(a)
#endif

using namespace std;

ofstream outFile;
map<int, string>fdList; 

void saveData(int __fd, __const void *buf, size_t __n, const string &dir)
{
    try
    {
        auto val = fdList.at(__fd);
        if(outFile.is_open())
        {
            outFile<<val<<" "<<dir<<"=";
            const char *b = (const char *)buf;
            for(int i=0; i<(__n/sizeof(char)); i++)
                outFile.put(b[i]);
            outFile<<endl;
        }
        else
        {
            cout<<"FD="<<__fd<<"\tdata="
                <<buf<<endl;
        }
    }
    catch(out_of_range &e)
    {
        DEBUG(cout<<"Not tracking "<<__fd<<endl);
    }
}

int connect (int __fd, __CONST_SOCKADDR_ARG __addr, socklen_t __len)
{
    typedef int (*REAL_FUNC)(int, __CONST_SOCKADDR_ARG, socklen_t);
    static REAL_FUNC real_func = NULL;

    char *filePath = getenv("NETTRACE_FILE_PATH");

    if(outFile.is_open() == false && filePath != NULL)
    {
        outFile.open(filePath);
        if(outFile.fail())
        {
            int err = errno;
            cerr<<"Failed to open "<<filePath<<". Cause: "<<strerror(err)<<endl;
        }
    }
    
    if(real_func == NULL)
    {
        DEBUG(cout<<"Getting old connect"<<endl);
        void * t = dlsym(RTLD_NEXT, "connect");
        real_func = *reinterpret_cast<REAL_FUNC *>(&t);
    }

    char dest[INET_ADDRSTRLEN];
    if(__addr->sa_family == AF_INET)
    {
        const sockaddr_in * s = reinterpret_cast<const sockaddr_in *>(__addr);
        if(inet_ntop(AF_INET, reinterpret_cast<const void *>(&(s->sin_addr)), dest, INET_ADDRSTRLEN) != dest)
        {
            int err = errno;
            cerr<<"Failed to convert IP. Cause: "<<strerror(err)<<endl;
        }
        else
        {
            ostringstream tmp;
            tmp<<dest<<":"<<ntohs(s->sin_port);
            auto ret = fdList.insert(pair<int, string>(__fd, tmp.str()));
            if(ret.second == false)
            {
                DEBUG(cout<<"Replacing "<<ret.first->first<<" with "<<tmp.str()<<endl);
                ret.first->second = tmp.str();
            }
        }
    }
    else
        DEBUG(cout<<"Not proccessing connection"<<endl);

    return real_func(__fd, __addr, __len);
}

ssize_t send (int __fd, __const void *__buf, size_t __n, int __flags)
{
    typedef ssize_t(*REAL_FUNC)(int, __const void *, size_t __n, int __flags);
    static REAL_FUNC real_func = NULL;

    if(real_func == NULL)
    {
        DEBUG(cout<<"Getting old send"<<endl);
        void *t = dlsym(RTLD_NEXT, "send");
        real_func = *reinterpret_cast<REAL_FUNC *>(&t);
    }

    saveData(__fd, __buf, __n, "to");

    return real_func(__fd, __buf, __n, __flags);
}

ssize_t recv (int __fd, void *__buf, size_t __n, int __flags)
{
    typedef ssize_t(*REAL_FUNC)(int, __const void *, size_t __n, int __flags);
    static REAL_FUNC real_func = NULL;

    if(real_func == NULL)
    {
        DEBUG(cout<<"Getting old send"<<endl);
        void *t = dlsym(RTLD_NEXT, "recv");
        real_func = *reinterpret_cast<REAL_FUNC *>(&t);
    }

    saveData(__fd, __buf, __n, "from");

    return real_func(__fd, __buf, __n, __flags);
}

ssize_t read (int __fd, void *__buf, size_t __n)
{
    typedef ssize_t(*REAL_FUNC)(int, __const void *, size_t __n);
    static REAL_FUNC real_func = NULL;

    if(real_func == NULL)
    {
        DEBUG(cout<<"Getting old send"<<endl);
        void *t = dlsym(RTLD_NEXT, "read");
        real_func = *reinterpret_cast<REAL_FUNC *>(&t);
    }

    saveData(__fd, __buf, __n, "from");

    return real_func(__fd, __buf, __n);
}

ssize_t write (int __fd, void *__buf, size_t __n)
{
    typedef ssize_t(*REAL_FUNC)(int, __const void *, size_t __n);
    static REAL_FUNC real_func = NULL;

    if(real_func == NULL)
    {
        DEBUG(cout<<"Getting old send"<<endl);
        void *t = dlsym(RTLD_NEXT, "write");
        real_func = *reinterpret_cast<REAL_FUNC *>(&t);
    }

    saveData(__fd, __buf, __n, "to");

    return real_func(__fd, __buf, __n);
}

ssize_t sendto (int __fd, __const void *__buf, size_t __n, int __flags, __CONST_SOCKADDR_ARG __addr, socklen_t __addr_len)
{
    typedef ssize_t(*REAL_FUNC)(int, __const void *, size_t, int, __CONST_SOCKADDR_ARG, socklen_t);
    static REAL_FUNC real_func = NULL;

    if(real_func == NULL)
    {
        DEBUG(cout<<"Getting old send"<<endl);
        void *t = dlsym(RTLD_NEXT, "sendto");
        real_func = *reinterpret_cast<REAL_FUNC *>(&t);
    }

    saveData(__fd, __buf, __n, "to");

    return real_func(__fd, __buf, __n, __flags, __addr, __addr_len);
}

ssize_t recvfrom (int __fd, void *__restrict __buf, size_t __n, int __flags, __SOCKADDR_ARG __addr, socklen_t *__restrict __addr_len)
{
    typedef ssize_t(*REAL_FUNC)(int, void *__restrict, size_t, int, __CONST_SOCKADDR_ARG, socklen_t *__restrict);
    static REAL_FUNC real_func = NULL;

    if(real_func == NULL)
    {
        DEBUG(cout<<"Getting old send"<<endl);
        void *t = dlsym(RTLD_NEXT, "recvfrom");
        real_func = *reinterpret_cast<REAL_FUNC *>(&t);
    }

    saveData(__fd, __buf, __n, "from");

    return real_func(__fd, __buf, __n, __flags, __addr, __addr_len);
}

int close(int fd)
{
    typedef int(*REAL_FUNC)(int);
    static REAL_FUNC real_func = NULL;

    if(real_func == NULL)
    {
        DEBUG(cout<<"Getting old close"<<endl);
        void *t = dlsym(RTLD_NEXT, "close");
        real_func = *reinterpret_cast<REAL_FUNC*>(&t);
    }

    try
    {
        auto val = fdList.at(fd);
        if(outFile.is_open())
            outFile<<"Connection to "<<val<<" closed"<<endl;
        else
            cout<<"Connection to "<<val<<" closed"<<endl;
    }
    catch(out_of_range &e)
    {
        DEBUG(cout<<"Not tracking "<<fd<<endl);
    }

    return real_func(fd);
}

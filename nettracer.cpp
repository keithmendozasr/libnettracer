#include <iostream>
#include <fstream>
#include <cerrno>
#include <cstring>
#include <sstream>
#include <cstdlib>

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

#define PRINT_DBG

#ifdef PRINT_DBG
    #define DEBUG(a) a
#else
    #define DEBUG(a)
#endif

using namespace std;

ofstream outFile;

void saveData(int __fd, __const void *buf, size_t __n)
{
   if(outFile.is_open())
   {
        outFile<<"FD="<<__fd<<"\tdata=";
        outFile.write((const char *)buf, (__n/sizeof(char)));
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
            ostringstream msg;
            msg<<"FD="<<__fd<<"\tdest="<<dest<<":"<<ntohs(s->sin_port);
            if(outFile.is_open())
                outFile<<msg.str()<<endl;
            else
                cout<<msg.str()<<endl;
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

    saveData(__fd, __buf, __n);

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

    saveData(__fd, __buf, __n);

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

    saveData(__fd, __buf, __n);

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

    saveData(__fd, __buf, __n);

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

    saveData(__fd, __buf, __n);

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

    saveData(__fd, __buf, __n);

    return real_func(__fd, __buf, __n, __flags, __addr, __addr_len);
}

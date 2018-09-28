#ifndef EASYTCPDEF_H
#define EASYTCPDEF_H

namespace EasyTCP
{
#ifdef __linux__

typedef int SOCKET;

#define INVALID_SOCKET	(SOCKET)(~0)
#define closesocket(fd)  ::close((fd))

#else
#include <winsock2.h>
typedef int socklen_t;
typedef SOCKET SOCKET;
#endif
}

#endif // EASYTCPDEF_H

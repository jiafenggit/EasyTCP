#ifndef EASYTCP_H
#define EASYTCP_H

#include "EasyTcpIConnection.h"
#include "EasyTcpIClient.h"
#include "EasyTcpIServer.h"

#ifdef __linux__
#include "EasyTcpClient_linux.h"
#include "EasyTcpServer_linux.h"
#else
#include "EasyTcpClient_win.h"
#include "EasyTcpServer_win.h"
#endif

#endif // EASYTCP_H

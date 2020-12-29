// CSocket.h

#ifndef _INC_CSOCKET_H
#define _INC_CSOCKET_H
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "common.h"

#include <winsock.h>
typedef int socklen_t;

struct CSocketAddressIP : public in_addr
{
	// Just the ip address. Not the port.
#define SOCKET_LOCAL_ADDRESS 0x0100007f
	// INADDR_ANY              (u_long)0x00000000
	// INADDR_LOOPBACK         0x7f000001
	// INADDR_BROADCAST        (u_long)0xffffffff
	// INADDR_NONE             0xffffffff

	DWORD GetAddrIP() const
	{
		return(s_addr);
	}
	void SetAddrIP(DWORD dwIP)
	{
		s_addr = dwIP;
	}
	LPCTSTR GetAddrStr() const
	{
		return inet_ntoa(*this);
	}
	void SetAddrStr(LPCTSTR pszIP)
	{
		// NOTE: This must be in 1.2.3.4 format.
		s_addr = inet_addr(pszIP);
	}
	bool IsValidAddr() const
	{
		// 0 and 0xffffffff=INADDR_NONE
		return(s_addr != INADDR_ANY && s_addr != INADDR_BROADCAST);
	}
	bool IsLocalAddr() const
	{
		return(s_addr == 0 || s_addr == SOCKET_LOCAL_ADDRESS);
	}

	bool IsSameIP(const CSocketAddressIP& ip) const;
	bool IsMatchIP(const CSocketAddressIP& ip) const;

	struct hostent* GetHostStruct() const
	{
		// try to reverse lookup a name for this IP address.
		// NOTE: This is a blocking call !!!!
		return gethostbyaddr((char*)&s_addr, sizeof(s_addr), AF_INET);
	}
	bool SetHostStruct(const struct hostent* pHost)
	{
		// Set the ip from the address name we looked up.
		if (pHost == NULL ||
			pHost->h_addr_list == NULL ||
			pHost->h_addr == NULL)	// can't resolve the address.
		{
			return(false);
		}
		SetAddrIP(*((DWORD*)(pHost->h_addr))); // 0.1.2.3
		return true;
	}

	bool SetHostStr(LPCTSTR pszHostName)
	{
		// try to resolve the host name with DNS for the true ip address.
		if (pszHostName[0] == '\0')
			return(false);
		if (isdigit(pszHostName[0]))
		{
			SetAddrStr(pszHostName); // 0.1.2.3
			return(true);
		}
		// NOTE: This is a blocking call !!!!
		return SetHostStruct(gethostbyname(pszHostName));
	}
	bool GetHostStr(TCHAR* pszHostName, int iLenMax)
	{
		// try to resolve the host name with DNS for the true ip address.
		// NOTE: This is a blocking call !!!!
		ASSERT(pszHostName);
		struct hostent* pHost = GetHostStruct();
		if (pHost)
		{
			strcpy(pszHostName, pHost->h_name);
			return(true);
		}
		else
		{
			// See CGSocket::GetLastError();
			strcpy(pszHostName, GetAddrStr());
			return(false);
		}
	}
	bool operator==(CSocketAddressIP ip) const
	{
		return(IsSameIP(ip));
	}
	CSocketAddressIP()
	{
		s_addr = INADDR_BROADCAST;
	}
	CSocketAddressIP(DWORD dwIP)
	{
		s_addr = dwIP;
	}
};


#endif
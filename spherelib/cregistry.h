#pragma once

class CGRegKey
{
public:
	CGRegKey() { throw "not implemented"; }
	CGRegKey(HKEY hKey) { throw "not implemented"; }

	LONG Open(LPCTSTR pszName, DWORD mode) { throw "not implemented"; }
	void Attach(HKEY hKey) { throw "not implemented"; }
	LONG QueryValue(LPCTSTR pszName, DWORD dwType, TCHAR *szValue, DWORD lSize) { throw "not implemented"; }
};

#pragma once
#include "CExpression.h"
#include "CScriptConsole.h"

#define CResourceObjPtr CResourceObj*
class CResourceObj : public CScriptObj
{
private:
	HASH_INDEX m_dwHashIndex;

public:
	CResourceObj(HASH_INDEX dwHashIndex)
	{
		m_dwHashIndex = dwHashIndex;
	}

	virtual HRESULT s_PropGet(LPCTSTR pszKey, CGVariant& vValRet, CScriptConsole* pSrc) { throw "not implemented"; }
	virtual HRESULT s_Method(LPCTSTR pszKey, CGVariant& vArgs, CGVariant& vValRet, CScriptConsole* pSrc) { throw "not implemented"; }

	int GetRefCount() { throw "not implemented"; }
	HASH_INDEX GetUIDIndex() const { return m_dwHashIndex; }
};
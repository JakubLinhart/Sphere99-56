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

struct CUIDArray
{
	CGRefArray<CResourceObj> m_UIDs;	// all the UID's in the World. CChar and CItem.

	DWORD GetUIDCount() const
	{
		return(m_UIDs.GetCount());
	}
#define UID_PLACE_HOLDER (CResourceObj*)0xFFFFFFFF
	CResourceObj* FindUIDObj(DWORD dwIndex) const
	{
		if (!dwIndex || dwIndex >= GetUIDCount())
			return(NULL);
		if (m_UIDs[dwIndex] == UID_PLACE_HOLDER)	// unusable for now. (background save is going on)
			return(NULL);
		return(m_UIDs[dwIndex]);
	}
	void FreeUID(CResourceObj* pObj)
	{
		// Can't free up the UID til after the save !
		m_UIDs.SetAt(pObj->GetUIDIndex(), UID_PLACE_HOLDER);
	}
	DWORD AllocUID(DWORD dwIndex, CResourceObj* pObj) { throw "not implemented"; }
};
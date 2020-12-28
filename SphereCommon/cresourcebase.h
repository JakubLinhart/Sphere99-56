//
// cresourcebase.h
// Copyright 1996 - 2001 Menace Software (www.menasoft.com)

#ifndef _INC_CRESOURCEBASE_H
#define _INC_CRESOURCEBASE_H
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "csphereexp.h"

struct CResourceQty
{
	// This can be used to "weight" any resource.
public:
	void SetResourceID( UID_INDEX rid, int iQty )
	{
		m_rid = rid;
		m_iQty = iQty;
	}
	CSphereUID GetResourceID() const
	{
		return( m_rid );
	}
	RES_TYPE GetResType() const
	{
		return( m_rid.GetResType());
	}
	int GetResIndex() const
	{
		return( m_rid.GetResIndex());
	}
	int GetResQty() const
	{
		return( m_iQty );
	}
	void SetResQty( int wQty )
	{
		m_iQty = wQty;
	}

	bool LoadResQty( LPCTSTR& pszCmds );
	int WriteKey( TCHAR* pszArgs ) const;
	int WriteNameSingle( TCHAR* pszArgs ) const;

private:
	CSphereUID m_rid;	// A RES_Skill, RES_ItemDef, or RES_TypeDef
	int m_iQty;		// How much of this ?
};

class CResourceQtyArray : public CGTypedArray<CResourceQty, CResourceQty&>
{
	// Similar to CUIDRefArray except it has m_iQty attached.
	// Define a list of index id's (not references) to resource objects.
	// (Not owned by the list)
public:
	CResourceQtyArray()
	{
	}
	CResourceQtyArray( LPCTSTR pszCmds )
	{
		s_LoadKeys( pszCmds );
	}

	bool operator == ( const CResourceQtyArray& array ) const;

	int  s_LoadKeys( LPCTSTR pszCmds );
	// HRESULT s_MethodObjs( LPCTSTR pszKey, CGVariant& vArgs, CGVariant& vValRet, CScriptConsole* pSrc );

	void v_GetKeys( CGVariant& vVal ) const;
	void v_GetNames( CGVariant& vVal ) const;

	int FindResourceID( CSphereUID rid ) const;
	int FindResourceType( RES_TYPE type ) const;

	int FindResourceMatch( CObjBase* pObj ) const;
	bool IsResourceMatchAll( CObjBase* pObj ) const;
};

class CResourceDef : public CScriptObj
{
public:
	LPCTSTR GetResourceName() const;
};

#define CResourceLinkPtr CResourceLink*
class CResourceLink : public CResourceDef
{

};

//***********************************************************

class CResourceRefArray : public CGRefArray<CResourceLink>
{
	// Define a list of pointer references to CResourceLink. (Not owned by the list)
	// An indexed list of CResourceLink s.
public:
	~CResourceRefArray()
	{
		RemoveAll();
	}

	int FindResourceType( RES_TYPE type ) const;
	int FindResourceID( CSphereUID rid ) const;
	int FindResourceName( RES_TYPE restype, LPCTSTR pszKey ) const;

	void v_Get( CGVariant& vVal ) const;
	bool v_Set( CGVariant& vVal, RES_TYPE restype );

	// HRESULT s_MethodObjs( LPCTSTR pszKey, CGVariant& vArgs, CGVariant& vValRet, CScriptConsole* pSrc );
	void s_WriteProps( CScript& s, LPCTSTR pszKey ) const;

private:
	CString GetResourceName( int iIndex ) const
	{
		// look up the name of the fragment given it's index.
		CResourceLinkPtr pResourceLink = ConstElementAt( iIndex );
		ASSERT(pResourceLink);
		return( pResourceLink->GetResourceName());
	}

public:
	CSCRIPT_CLASS_DEF1();
	enum M_TYPE_
	{
#define CUIDREFARRAYMETHOD(a,b,c,d) M_##a,
#include "..\spherelib\cuidrefarraymethods.tbl"
#undef CUIDREFARRAYMETHOD
		M_QTY,
	};
	static const CScriptMethod sm_Methods[M_QTY+1];
#ifdef USE_JSCRIPT
#define CUIDREFARRAYMETHOD(a,b,c,d) JSCRIPT_METHOD_DEF(a)
#include "..\spherelib\cuidrefarraymethods.tbl"
#undef CUIDREFARRAYMETHOD
#endif
};

#endif // _INC_CRESOURCEBASE_H
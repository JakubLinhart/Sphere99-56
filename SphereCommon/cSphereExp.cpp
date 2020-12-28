//
// CSphereExp.cpp
// Copyright 1996 - 2001 Menace Software (www.menasoft.com)
//

#include "stdafx.h"	// predef header.

static CSphereExpContext g_Exp( NULL, &g_Serv );	// default expression context.

CExpression* Exp_GetContext()
{
	// Accesses the globals,locals and the current context current 'this' object.
	// Diff context depending on current CSphereThread?
	CSphereThread* pTask = CSphereThread::GetCurrentThread();
	if ( pTask && pTask->m_pExecContext )
	{
		return pTask->m_pExecContext;
	}
	return &g_Exp;
}

//***************************************************************************
//	CSphereScriptContext

void CSphereScriptContext::OpenScript( const CScript* pScriptContext )
{
	// NOTE: These should be called stack based and therefore on the same thread.
	CloseScript();
	m_pScriptContext = pScriptContext;
	m_pPrvScriptContext = g_Cfg.SetScriptContext( pScriptContext );
}

void CSphereScriptContext::CloseScript()
{
	if ( m_pScriptContext )
	{
		m_pScriptContext = NULL;
		g_Cfg.SetScriptContext( m_pPrvScriptContext );
	}
}

//***************************************************************************

const CScriptPropX CSphereExpContext::sm_Functions[CSphereExpContext::F_QTY+1] =	// static
{
#define GLOBALMETHOD(a,b,c) CSCRIPT_PROPX_IMP(a,b,c)
#include "globalmethods.tbl"
#undef GLOBALMETHOD
	NULL,
};

CScriptPropArray CSphereExpContext::sm_FunctionsAll;	// static

CSphereExpContext::CSphereExpContext( CResourceObj* pBaseObj, CScriptConsole* pSrc )
	: CScriptExecContext(pBaseObj,pSrc)
{
	// NOTE: These should be called stack based and therefore on the same thread.
	CSphereThread* pThread = CSphereThread::GetCurrentThread();
	if ( pThread )
	{
		m_pPrvExecContext = pThread->SetExecContext( this );
	}
	else
	{
		m_pPrvExecContext = NULL;
	}
	m_iPrvTask = g_Serv.m_Profile.GetTaskCurrent();
	g_Serv.m_Profile.SwitchTask(PROFILE_Scripts);
}

CSphereExpContext::~CSphereExpContext()
{
	if ( m_iPrvTask >= 0 )
	{
		g_Serv.m_Profile.SwitchTask(m_iPrvTask);
	}
	CSphereThread* pThread = CSphereThread::GetCurrentThread();
	if ( pThread )
	{
		pThread->SetExecContext( m_pPrvExecContext );
	}
}

void CSphereExpContext::InitFunctions()	// static
{
	if ( sm_FunctionsAll.GetSize())
		return;

	CScriptExecContext::InitFunctions();
	sm_FunctionsAll.AddProps( CScriptExecContext::sm_FunctionsAll.GetData(), CScriptExecContext::sm_FunctionsAll.GetSize() );
	sm_FunctionsAll.AddProps( sm_Functions );
}

HRESULT CSphereExpContext::Function_Dispatch( LPCTSTR pszKey, CGVariant& vArgs, CGVariant& vValRet )
{
	// Evaluate an identifier.
	// Find the key in the defs collection
	// Skip to the end of the identifier name. ( + any args? )
	// The name can only be valid.

	// Access globals methods
	// Get a global var ref - A key name that just links to another object.

	int iProp = s_FindKeyInTable( pszKey, sm_Functions );
	if ( iProp < 0 )
	{
		HRESULT hRes = CScriptExecContext::Function_Dispatch(pszKey,vArgs,vValRet);
		if ( hRes != HRES_UNKNOWN_PROPERTY )
			return hRes;

		// Is it a global function/method? RES_Function
		CSphereUID ridFunc = g_Cfg.ResourceCheckIDType( RES_Function, pszKey );
		if ( ridFunc.IsValidRID())
		{
			CResourceLock sFunction( g_Cfg.ResourceGetDef(ridFunc));
			if ( ! sFunction.IsFileOpen())
				return( HRES_INVALID_HANDLE );
			// create a new sub-context with new args.
			CSphereExpArgs exec( GetBaseObject(), GetSrc(), vArgs );
			TRIGRET_TYPE iRet = exec.ExecuteScript( sFunction, TRIGRUN_SECTION_TRUE );
			vValRet = exec.m_vValRet;
			return( NO_ERROR );
		}

		// Check global vars and constants.
		if ( g_Cfg.m_Var.FindKeyVar(pszKey,vValRet))
			return NO_ERROR;
		if ( g_Cfg.m_Const.FindKeyVar(pszKey,vValRet))
			return NO_ERROR;

		return HRES_UNKNOWN_PROPERTY;
	}

	switch(iProp)
	{
	case F_AccountMgr:
		// Ref to the account manager. (for creating new accounts)
		vValRet.SetRef(&g_Accounts);
		break;
	case F_Account:
	case F_FindAccount:
		// lookup a specific account by name.
		vValRet.SetRef( g_Accounts.Account_FindNameCheck(vArgs));
		break;
	case F_Srv:
	case F_Serv:
		// "LASTNEWITEM" etc
		vValRet.SetRef(&g_Serv);
		break;
	case F_FindUID:
	// case F_UID:
		if ( vArgs.IsEmpty())
			return( HRES_BAD_ARG_QTY );
		vValRet.SetRef( g_Cfg.FindUID( vArgs.GetUID()));
		break;
	case F_Var:
		return g_Cfg.m_Var.s_MethodTags( vArgs, vValRet, m_pSrc );
	default:
		DEBUG_CHECK(0);
		return( HRES_INTERNAL_ERROR );
	}

	return( NO_ERROR );
}

//****************************************************************************
// -CSphereExpArgs

const CScriptPropX CSphereExpArgs::sm_Functions[CSphereExpArgs::F_QTY+1] =	// static
{
#define CSPHEREEXPFUNC(a,b,c) CSCRIPT_PROPX_IMP(a,b,c)
#include "csphereexpfunc.tbl"
#undef CSPHEREEXPFUNC
	NULL,
};

CScriptPropArray CSphereExpArgs::sm_FunctionsAll;	// static

CSphereExpArgs::CSphereExpArgs( CResourceObj* pBase, CScriptConsole* pSrc, LPCTSTR pszStr ) :
	CSphereExpContext(pBase,pSrc),
	m_s1(pszStr)
{
	// attempt to parse this.
	if ( Exp_IsSimpleNumberString(pszStr))
	{
		m_iN1 = Exp_GetComplex(pszStr);
	}
}

CSphereExpArgs::~CSphereExpArgs()
{
}

void CSphereExpArgs::InitFunctions()	// static
{
	if ( sm_FunctionsAll.GetSize())
		return;

	CSphereExpContext::InitFunctions();
	sm_FunctionsAll.AddProps( CSphereExpContext::sm_FunctionsAll.GetData(), CSphereExpContext::sm_FunctionsAll.GetSize() );
	sm_FunctionsAll.AddProps( sm_Functions );
}

HRESULT CSphereExpArgs::Function_Dispatch( LPCTSTR pszKey, CGVariant& vArgs, CGVariant& vValRet ) // virtual
{
	CScriptObj::s_FixExtendedProp( pszKey, "ArgV", vArgs );
	CScriptObj::s_FixExtendedProp( pszKey, "ArgChk", vArgs );
	CScriptObj::s_FixExtendedProp( pszKey, "ArgTxt", vArgs );

	F_TYPE_ iProp = (F_TYPE_) s_FindKeyInTable( pszKey, sm_Functions );
	if ( iProp < 0 )
	{
		return( CSphereExpContext::Function_Dispatch( pszKey, vArgs, vValRet ));
	}

	switch (iProp)
	{
	case F_ArgChk:
	case F_ArgTxt:
		switch ( vArgs.MakeArraySize())
		{
		case 0:
			return( HRES_BAD_ARG_QTY );
		case 1:
			{
				CGString sName;
				sName.Format( (iProp==F_ArgChk)?"ARGCHK_%d":"ARGTXT_%d", vArgs.GetInt());
				m_ArgArray.FindKeyVar( sName, vValRet );
			}
			break;
		default:
			return HRES_WRITE_FAULT;
		}
		break;
	case F_ArgO:
	case F_ArgO1:
		if ( m_pO1 == NULL )
			return HRES_INVALID_HANDLE;
		vValRet.SetRef( m_pO1 );
		break;
	case F_ArgN:
	case F_ArgN1:
		vValRet.SetInt( m_iN1 );
		break;
	case F_ArgN2:
		vValRet.SetInt( m_iN2 );
		break;
	case F_ArgN3:
		vValRet.SetInt( m_iN3 );
		break;
	case F_ArgPt: // "PT",
		vValRet.SetArrayFormat( "ix,iy,iz", m_iN1, m_iN2, m_iN3 );
		break;
	case F_ArgS:
	case F_ArgS1:
		if ( m_s1.IsEmpty())
		{
			vValRet = m_vVal;
		}
		else
		{
			vValRet = m_s1;
		}
		break;
	case F_ArgV:
		if ( vArgs.IsEmpty())
		{
			vValRet = m_vVal;
		}
		else
		{
			vValRet = m_vVal.GetArrayElement( vArgs.GetInt());
		}
		break;
	default:
		DEBUG_CHECK(0);
		return HRES_INTERNAL_ERROR;
	}

	return( NO_ERROR );
}

void CSphereExpArgs::AddCheck( int n, DWORD dwCheckVal )
{
	// Add local variables to this context.
	// Add an id tagged value ARGCHK#=val
	DEBUG_CHECK(dwCheckVal>100);
	CGString sName;
	sName.Format( "ARGCHK_%d", n );
	m_ArgArray.SetKeyInt( (const char*) sName, dwCheckVal );
	sName.Format( "ARGCHK_%d", dwCheckVal );
	m_ArgArray.SetKeyInt( (const char*) sName, n+1 );
}

void CSphereExpArgs::AddText( int id, const char* pszText )
{
	// Add local variables to this context.
	// Add an id tagged string ARGTXT_#=string
	CString sName;
	sName.Format( "ARGTXT_%d", id );
	m_ArgArray.SetKeyStr( (const char*) sName, pszText );
}

#if 0

//**********************************************

CSphereDefVars::CSphereDefVars() : CResourceObj(UID_INDEX_CLEAR)
{
	IncRefCount();	// static singleton
}
CSphereDefVars::~CSphereDefVars()
{
	StaticDestruct();	// static singleton
}

HRESULT CSphereDefVars::s_PropGet( LPCTSTR pszKey, CGVariant& vValRet, CScriptConsole* pSrc )
{
	// All objects have GetName() but it might mess up scripting here ?
	HRESULT hRes = s_PropGetTags( pszKey, vValRet);
	if ( hRes == NO_ERROR )
		return hRes;
	return( CResourceObj::s_PropGet( pszKey, vValRet, pSrc ));
}
HRESULT CSphereDefVars::s_PropSet( LPCTSTR pszKey, CGVariant& vVal )
{
	HRESULT hRes = s_PropSetTags( pszKey, vVal );
	if ( hRes == NO_ERROR )
		return NO_ERROR;
	return( CResourceObj::s_PropSet( pszKey, vVal ));
}
HRESULT CSphereDefVars::s_Method( LPCTSTR pszKey, CGVariant& vArgs, CGVariant& vValRet, CScriptConsole* pSrc )
{
	// Execute command from script
	// "Set/Get global (saved) variables",
	HRESULT hRes = s_MethodTags(pszKey,vArgs,vValRet,pSrc);
	if ( hRes == NO_ERROR )
		return NO_ERROR;
	return( CResourceObj::s_Method( pszKey, vArgs, vValRet, pSrc ));
}

#ifdef USE_JSCRIPT
#define GLOBALMETHOD(a,b,c)  JSCRIPT_METHOD_IMP(CSphereGlobalObject,a)
#include "globalmethods.tbl"
#undef GLOBALMETHOD
#endif

const CScriptMethod CSphereGlobalObject::sm_Methods[GLF_QTY+1] = 
{
#define GLOBALMETHOD(a,b,c)  CSCRIPT_METHOD_IMP(a,b,c)
#include "globalmethods.tbl"
#undef GLOBALMETHOD
		NULL,
};

CSCRIPT_CLASS_IMP1(SphereGlobalObject,NULL,sm_Methods,NULL,ScriptObj);

#endif

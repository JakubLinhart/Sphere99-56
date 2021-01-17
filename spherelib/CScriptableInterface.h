#pragma once

#define CSCRIPTPROP_RETNUL	0x0001
#define CSCRIPTPROP_RETREF	0x0002
#define CSCRIPTPROP_RETVAL	0x0004
#define CSCRIPTPROP_ARG1	0x0008
#define CSCRIPTPROP_ARG1S	0x0010
#define CSCRIPTPROP_ARG2	0x0020
#define CSCRIPTPROP_NARG1	0x0040
#define CSCRIPTPROP_NARG2	0x0080
#define CSCRIPTPROP_NARGS	0x0100
#define CSCRIPTPROP_UNUSED	0x0200
#define CSCRIPTPROP_DUPE	0x0400
#define CSCRIPTPROP_READO	0x0800
#define CSCRIPTPROP_WRITEO	0x1000

#define VARTYPE_BOOL		0
#define VARTYPE_CSTRING		1
#define VARTYPE_INT			2
#define VARTYPE_LPSTR		3
#define VARTYPE_LPCTSTR		4
#define VARTYPE_UID			5
#define VARTYPE_VOID		6
#define VARTYPE_WORD		7

#define DECLARE_LISTREC_REF(a)
#define DECLARE_LISTREC_REF2(a)
#define CSCRIPT_EXEC_DEF void Exec
#define CSCRIPT_CLASS_IMP0(a,b,c)
#define DECLARE_LISTREC_TYPE(a)
#define CSCRIPT_PROP_IMP(a,b,c)
#define CSCRIPT_PROPX_IMP(a,b,c)
#define CSCRIPT_METHOD_IMP(a,b,c)

#define CSCRIPT_CLASS_DEF1(...) \
	int s_FindMyPropKey(LPCTSTR pszKey); \
	int s_FindMyMethodKey(LPCTSTR pszKey); \
	static CScriptClass sm_ScriptClass;

#define CSCRIPT_CLASS_IMP1(CLASS_NAME,b,c,d,e) \
	int C##CLASS_NAME::s_FindMyPropKey(LPCTSTR pszKey) { throw "not implemented"; } \
	int C##CLASS_NAME::s_FindMyMethodKey(LPCTSTR pszKey) { throw "not implemented"; }

#define CSCRIPT_CLASS_DEF2(...) \
	int s_FindMyPropKey(LPCTSTR pszKey); \
	int s_FindMyMethodKey(LPCTSTR pszKey);

#define CSCRIPT_CLASS_IMP2(CLASS_NAME,b,c,d,e) \
	int C##CLASS_NAME::s_FindMyPropKey(LPCTSTR pszKey) { throw "not implemented"; } \
	int C##CLASS_NAME::s_FindMyMethodKey(LPCTSTR pszKey) { throw "not implemented"; }


class CScriptClass
{
public:
	void InitScriptClass() { throw "not implemented"; }
	void AddSubClass(CScriptClass* pSubClass) { throw "not implemented"; }
};

template <class TYPE>
class CScriptClassTemplate : public CScriptClass
{
public:
	virtual void InitScriptClass();
	bool IsInit() { throw "not implemented"; }
};



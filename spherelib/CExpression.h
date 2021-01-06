#pragma once

#include "catom.h"

#define _ISCSYM(ch) ( isalnum(ch) || (ch)=='_')	// __iscsym or __iscsymf

class CGVariant
{
public:
	CGVariant() { throw "not implemented"; }
	CGVariant(const UID_INDEX uid) { throw "not implemented"; }
	CGVariant(LPCTSTR pszValue) { throw "not implemented"; }
	CGVariant(VARTYPE type, void* rid) { throw "not implemented"; }

	void SetUID(UID_INDEX uid) { throw "not implemented"; }
	void SetRef(CScriptObj* val) { throw "not implemented"; }
	void SetBool(bool val) { throw "not implemented"; }
	void SetInt(int val) { throw "not implemented"; }
	void SetDWORD(DWORD val) { throw "not implemented"; }
	void SetStrFormat(LPCTSTR format, ...) { throw "not implemented"; }
	void SetVoid() { throw "not implemented"; }

	bool IsEmpty() const { throw "not implemented"; }
	bool IsNumeric() const { throw "not implemented"; }
	bool IsVoid() const { throw "not implemented"; }

	bool GetBool() const { throw "not implemented"; }
	int GetInt() const { throw "not implemented"; }
	DWORD GetDWORD() const { throw "not implemented"; }
	DWORD GetDWORDMask(DWORD flags1, DWORD flags2) const { throw "not implemented"; }
	LPCTSTR GetPSTR() const { throw "not implemented"; }
	CGString& GetStr() const { throw "not implemented"; }
	UID_INDEX GetUID() const { throw "not implemented"; }

	int CompareData(CGVariant& other) const { throw "not implemented"; }

	int MakeArraySize() const { throw "not implemented"; }
	CGString& GetArrayStr(int index) const { throw "not implemented"; }
	LPCTSTR GetArrayPSTR(int index) { throw "not implemented"; }
	int GetArrayInt(int index) const { throw "not implemented"; }
	void RemoveArrayElement(int index) { throw "not implemented"; }
	void SetArrayFormat(LPCTSTR format, ...) { throw "not implemented"; }
	void SetArrayElement(int index, LPCTSTR value) { throw "not implemented"; }
	CGVariant& GetArrayElement(int index) { throw "not implemented"; }

	CGVariant& operator=(const CGString& str) { throw "not implemented"; }
	operator LPCTSTR() { throw "not implemented"; }
	operator char*() { throw "not implemented"; }
	operator int() { throw "not implemented"; }
};

#define CVarDefPtr CVarDef*
class CVarDef : public CMemDynamic	// A variable from GRAYDEFS.SCP or other.
{
	// Similar to CScriptKey
private:
#define EXPRESSION_MAX_KEY_LEN SCRIPT_MAX_SECTION_LEN
	const CAtomRef m_aKey;	// the key for sorting/ etc.
public:
	int GetInt();
	DWORD GetDWORD();
	LPCTSTR GetKey() const
	{
		return(m_aKey.GetStr());
	}
	CVarDef(LPCTSTR pszKey) :
		m_aKey(pszKey)
	{
	}
	virtual LPCTSTR GetValStr() const = 0;
	virtual int GetValNum() const = 0;
	virtual CVarDef* CopySelf() const = 0;
};

struct CVarDefArray : public CGSortedArray<CVarDef*, const CVarDef&, CAtomRef>
{
public:
	CVarDefPtr FindKeyPtr(CAtomRef pName);
    // Sorted array
protected:
    int CompareKey(CAtomRef Key, const CVarDef& pVar) const;
    int Add(CVarDef* pVar);
public:
    void Copy(const CVarDefArray* pArray);
	int SetKeyVar(CAtomRef key, const CGVariant& val);
	int SetKeyStr(CAtomRef key, LPCTSTR value);
	UID_INDEX FindKeyVar(CAtomRef key) const;
	CGString FindKeyStr(CAtomRef key) const;

    CVarDefArray& operator = (const CVarDefArray& array)
    {
        Copy(&array);
        return(*this);
    }
};

#define Exp_GetComplex(str) Exp_GetContext()->GetComplex(str)
#define Exp_GetComplexRef(str) Exp_GetContext()->GetComplexRef(str)
#define Exp_GetValueRef(str) Exp_GetContext()->GetValueRef(str)
#define Exp_GetValue(str) Exp_GetContext()->GetValue(str)
#define Exp_GetIdentifierString(str1, str2) Exp_GetContext()->GetIdentifierString(str1, str2)
#define Exp_IsSimpleNumberString(str1) Exp_GetContext()->IsSimpleNumberString(str1)
#define Exp_ParseCmds(str1, pArgs, iCnt) Exp_GetContext()->ParseCmds(str1, pArgs, iCnt)

class CExpression
{
public:
	int GetComplex(LPCTSTR pStr) { throw "not implemented"; }
	int GetComplexRef(LPCTSTR pStr) { throw "not implemented"; }
	int GetValue(LPCTSTR pStr) { throw "not implemented"; }
	int GetValueRef(LPCTSTR pStr) { throw "not implemented"; }
	int GetIdentifierString(LPCTSTR pStr1, LPCTSTR pStr2) { throw "not implemented"; }
	bool IsSimpleNumberString(LPCTSTR pStr1) { throw "not implemented"; }
	int ParseCmds(LPCTSTR pszStr, int* pArgs, int iCnt) { throw "not implemented"; }
};

int Calc_GetRandVal(int iqty) { throw "not implemented"; }
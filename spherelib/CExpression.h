#pragma once

#include "catom.h"

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
    int CompareKey(CAtomRef Key, CVarDef* pVar) const;
    int Add(CVarDef* pVar);
public:
    void Copy(const CVarDefArray* pArray);
	int SetKeyVar(CAtomRef key, CGVariant& const val);
	int SetKeyStr(CAtomRef key, LPCTSTR value);
	UID_INDEX FindKeyVar(CAtomRef key) const;
	CGString FindKeyStr(CAtomRef key) const;

    CVarDefArray& operator = (const CVarDefArray& array)
    {
        Copy(&array);
        return(*this);
    }
};

class CExpression
{

} g_Exp;

extern int Calc_GetRandVal(int iqty);
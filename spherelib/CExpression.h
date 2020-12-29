#pragma once

#include "catom.h"

class CVarDefBase : public CMemDynamic	// A variable from GRAYDEFS.SCP or other.
{
	// Similar to CScriptKey
private:
#define EXPRESSION_MAX_KEY_LEN SCRIPT_MAX_SECTION_LEN
	const CAtomRef m_aKey;	// the key for sorting/ etc.
public:
	LPCTSTR GetKey() const
	{
		return(m_aKey.GetStr());
	}
	CVarDefBase(LPCTSTR pszKey) :
		m_aKey(pszKey)
	{
	}
	virtual LPCTSTR GetValStr() const = 0;
	virtual int GetValNum() const = 0;
	virtual CVarDefBase* CopySelf() const = 0;
};

struct CVarDefArray : public CGSortedArray<CVarDefBase*, const CVarDefBase&, CAtomRef>
{
    // Sorted array
protected:
    int CompareKey(CAtomRef Key, CVarDefBase* pVar) const;
    int Add(CVarDefBase* pVar);
public:
    void Copy(const CVarDefArray* pArray);

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
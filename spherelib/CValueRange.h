#pragma once
struct CValueRangeInt
{
	// Simple linearity
public:
	int m_iLo;
	int m_iHi;
public:
	void Init()
	{
		m_iLo = INT_MIN;
		m_iHi = INT_MAX;
	}
	int GetRange() const
	{
		return( static_cast<int>(m_iHi - m_iLo) );
	}
	int GetLinear( int iPercent ) const
	{	
		// ARGS: iPercent = 0-1000
		return( static_cast<int>(m_iLo) + IMULDIV( GetRange(), iPercent, 1000 ));
	}
	int GetRandom() const
	{	
		return( static_cast<int>(m_iLo) + Calc_GetRandVal( GetRange()));
	}
	int GetMin() const { return m_iLo; }
	int GetRandomLinear( int iPercent ) const;
	void v_Set(CGVariant& vVal);
	void v_Get(CGVariant& vVal);

public:
	CValueRangeInt()
	{
		Init();
	}
};

struct CValueRangeByte
{
	// Simple linearity
public:
	BYTE m_iLo;
	BYTE m_iHi;
public:
	void Init()
	{
		m_iLo = 0;
		m_iHi = 255;
	}
	int GetRange() const
	{
		return(static_cast<BYTE>(m_iHi - m_iLo));
	}
	int GetLinear(int iPercent) const
	{
		// ARGS: iPercent = 0-1000
		return(static_cast<BYTE>(m_iLo) + IMULDIV(GetRange(), iPercent, 1000));
	}
	int GetRandom() const
	{
		return(static_cast<BYTE>(m_iLo) + Calc_GetRandVal(GetRange()));
	}
	int GetRandomLinear(int iPercent) const;
	void v_Set(CGVariant& vVal);
	void v_Get(CGVariant& vVal);

public:
	CValueRangeByte()
	{
		Init();
	}
};

struct CValueCurveDef
{
	// Describe an arbitrary curve.
	// for a range from 0.0 to 100.0 (1000)
	// May be a list of probabilties from 0 skill to 100.0% skill.
public:
	CGTypedArray<int, int> m_aiValues;		// 0 to 100.0 skill levels
public:
	void Init()
	{
		m_aiValues.Empty();
	}
	int GetLinear(int iSkillPercent) const;
	int GetChancePercent(int iSkillPercent) const;
	int GetRandom() const;
	int GetRandomLinear(int iPercent) const;

	void v_Get(CGVariant& val) { throw "not implemented"; }
	void v_Set(CGVariant& val) { throw "not implemented"; }
};
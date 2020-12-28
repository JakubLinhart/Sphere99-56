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
	int GetRandomLinear( int iPercent ) const;
	void v_Set(CGVariant& vVal);
	void v_Get(CGVariant& vVal);

public:
	CValueRangeInt()
	{
		Init();
	}
};

#pragma once

struct CGRect			// Basic rectangle. (May not be on the map)
{
public:
	int m_left;		// West	 x=0
	int m_top;		// North y=0
	int m_right;	// East	( NON INCLUSIVE !)
	int m_bottom;	// South ( NON INCLUSIVE !)
	int m_map;

public:
	int Width() const { return(m_right - m_left); }
	int Height() const { return(m_bottom - m_top); }

	operator RECT& ()
	{
		return(*((RECT*)(&m_left)));
	}
	operator RECT* ()
	{
		return((::RECT*)(&m_left));
	}
	operator const RECT& () const
	{
		return(*((RECT*)(&m_left)));
	}
	operator const RECT* () const
	{
		return((::RECT*)(&m_left));
	}
	CGRect& operator = (RECT rect)
	{
		m_left = rect.left;
		m_top = rect.top;
		m_right = rect.right;
		m_bottom = rect.bottom;
		return(*this);
	}

	void SetRectEmpty()
	{
		m_left = m_top = 0;	// 0x7ffe
		m_right = m_bottom = 0;
		m_map = 0;
	}
	
	virtual void NormalizeRect()
	{
		if (m_bottom < m_top)
		{
			int wtmp = m_bottom;
			m_bottom = m_top;
			m_top = wtmp;
		}
		if (m_right < m_left)
		{
			int wtmp = m_right;
			m_right = m_left;
			m_left = wtmp;
		}
		if ((m_map < 0) || (m_map >= 256)) m_map = 0;
		if (!g_MapList.m_maps[m_map]) m_map = 0;
	}

	void NormalizeRectMax(int cx, int cy)
	{
		if (m_left < 0)
			m_left = 0;
		if (m_top < 0)
			m_top = 0;
		if (m_right > cx)
			m_right = cx;
		if (m_bottom > cy)
			m_bottom = cy;
	}
};

class CGRegion
{
public:
	CGRect m_rectUnion;	// The union rectangle.
	CGTypedArray<CGRect, const CGRect&> m_Rects;

	void EmptyRegion()
	{
		m_rectUnion.SetRectEmpty();
		m_Rects.Empty();
	}
};
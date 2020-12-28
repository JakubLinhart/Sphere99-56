#pragma once

struct CGRect			// Basic rectangle. (May not be on the map)
{
public:
	int left;		// West	 x=0
	int top;		// North y=0
	int right;	// East	( NON INCLUSIVE !)
	int bottom;	// South ( NON INCLUSIVE !)

public:
	int Width() const { return(right - left); }
	int Height() const { return(bottom - top); }

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
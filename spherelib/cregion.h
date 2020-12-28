#pragma once

struct CGRect			// Basic rectangle. (May not be on the map)
{
public:
	int m_left;		// West	 x=0
	int m_top;		// North y=0
	int m_right;	// East	( NON INCLUSIVE !)
	int m_bottom;	// South ( NON INCLUSIVE !)

public:
	int Width() const { return(m_right - m_left); }
	int Height() const { return(m_bottom - m_top); }
};
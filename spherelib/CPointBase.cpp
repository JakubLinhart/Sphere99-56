#include "cpointbase.h"

void CGPointBase::InitPoint()
{
	m_x = -1;	// invalid location.
	m_y = -1;
	m_z = 0;
	m_mapplane = 0;
}

int CGPointBase::GetDistZ(const CGPointBase& pt) const
{
	return(abs(m_z - pt.m_z));
}

void CGPointBase::Set(const CGPointBase& pt)
{
	m_x = pt.m_x;
	m_y = pt.m_y;
	m_z = pt.m_z;
	m_mapplane = pt.m_mapplane;
}

void CGPointBase::Set(const POINT pt)
{
	m_x = pt.x;
	m_y = pt.y;
	m_z = 0;
	m_mapplane = 0;
}

void CGPointBase::Set(const POINTS pt)
{
	m_x = pt.x;
	m_y = pt.y;
	m_z = 0;
	m_mapplane = 0;
}

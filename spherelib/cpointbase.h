#pragma once

#include "..\SphereSvr\stdafx.h"

struct CGPointBase	// Non initialized 3d point.
{
public:
	signed short m_x;	// equipped items dont need x,y
	signed short m_y;
	signed char m_z;	// This might be layer if equipped ? or equipped on corpse. Not used if in other container.
	BYTE m_mapplane;

public:
	void InitPoint();

	int GetDistZ(const CGPointBase& pt) const;

	void Set(const CGPointBase& pt);
	void Set(const POINT pt);
	void Set(const POINTS pt);
	void v_Set(CGVariant& vVal);
};
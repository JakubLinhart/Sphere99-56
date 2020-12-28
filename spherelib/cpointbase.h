#pragma once

#include "..\SphereSvr\stdafx.h"

enum DIR_TYPE	// Walking directions. m_dir
{
	DIR_INVALID = -1,

	DIR_N = 0,
	DIR_NE,
	DIR_E,
	DIR_SE,
	DIR_S,
	DIR_SW,
	DIR_W,
	DIR_NW,
	DIR_QTY,		// Also means "Center"

	DIR_ANIM_QTY = 5	// Seems we only need 5 pics for an anim, assume ALL bi-symetrical creatures
};

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
	int GetDist(const CGPointBase& pt) const; // Distance between points
	int GetDist3D(const CGPointBase& pt) const; // 3D Distance between points

	void Set(const CGPointBase& pt);
	void Set(const POINT pt);
	void Set(const POINTS pt);
	void v_Set(CGVariant& vVal);

	DIR_TYPE GetDir(const CGPointBase& pt, DIR_TYPE DirDefault = DIR_QTY) const; // Direction to point pt
};
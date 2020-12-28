// CMulTile.cpp
// Copyright Menace Software (www.menasoft.com).
//
// Deal with the resources in the MUL files.
//

#include "stdafx.h"
#include "cmultile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
#endif

#ifdef _WIN32

#pragma optimize( "agt", on )

/////////////////////////////////////////////////////////////////////////////
// -CMulSound

CMulSound::CMulSound( HASH_INDEX dwHashIndex ) :
	CMulBlockType( dwHashIndex )
{
	m_dwLength = 0;
}
CMulSound::CMulSound( SOUND_TYPE sound ) :
	CMulBlockType( VERDATA_MAKE_HASH( VERFILE_SOUND, sound ))
{
	m_dwLength = 0;
}

bool CMulSound::Load()
{
	// A sound resource from the SOUNDS.MUL file.
	CMulIndexRec Index;
	if ( ! g_MulInstall.ReadMulIndex( VERFILE_SOUNDIDX, VERFILE_SOUND, GetID(), Index ))
		return( false );
	m_dwLength = Index.GetBlockLength();
	LoadMulData( VERFILE_SOUND, Index );
	return( true );
}

void CMulSound::GetWaveFormat( void * pWaveFormat )
{
	// Get the format structure for playing this data.

	PCMWAVEFORMAT * pcmwf = (PCMWAVEFORMAT *) pWaveFormat;
	memset( pcmwf, 0, sizeof(PCMWAVEFORMAT));
	pcmwf->wf.wFormatTag         = WAVE_FORMAT_PCM;      
	pcmwf->wf.nChannels          = 2;
	pcmwf->wf.nSamplesPerSec     = 11025;
	pcmwf->wf.nBlockAlign        = (2*sizeof(WORD));
	pcmwf->wf.nAvgBytesPerSec    = (2*sizeof(WORD)*11025);
	pcmwf->wBitsPerSample        = 16;
}

/////////////////////////////////////////////////////////////////////////
// -CTileItemType

CTileItemType::CTileItemType( HASH_INDEX dwHashIndex ) :
	CMulImageType( dwHashIndex )
{
	assert( GetID() < ITEMID_MULTI );
}
CTileItemType::CTileItemType( ITEMID_TYPE id ) :
	CMulImageType( VERDATA_MAKE_HASH( VERFILE_ART, id+TERRAIN_QTY ))
{
	assert( GetID() < ITEMID_MULTI );
}

bool CTileItemType::Load()
{
	DWORD id = TERRAIN_QTY + GetID();
	CMulIndexRec Index;
	if ( ! g_MulInstall.ReadMulIndex( VERFILE_ARTIDX, VERFILE_ART, id, Index ))
		return( false );
	assert( Index.GetBlockLength() <= MAXTILESIZE );
	LoadMulData( VERFILE_ART, Index );
	return( true );
}

const CMulItemInfo * CTileItemType::GetInfo()
{
	if ( ! m_pInfo.IsValidNewObj())
	{
		m_pInfo = new CMulItemInfo( GetID());
	}
	return( m_pInfo );
}

CGRect CTileItemType::GetDrawRect( int sx, int sy )
{
	int iWidth;
	int iHeight;

	WORD * pData = GetLoadData();
	if ( ! pData ) 
	{
		iWidth  = 0;
		iHeight = 0;
	}
	else
	{
		iWidth  = pData[2];
		iHeight = pData[3];
		sy -= ( iHeight - TERRAIN_ART_WIDTH );	
		sx -= ( iWidth / 2 );	// center it.
	}

	CGRect rect;
	rect.left = sx;
	rect.top = sy;
	rect.right = sx + iWidth;
	rect.bottom = sy + iHeight;
	return( rect );
}

#ifdef SPHERE_CLIENT

int CTileItemType::GetAnimFrames()	// how many frames does this animate over ? (include self) (<=1 not animated.)
{
	// If this is an animated tile.
	// How many tiles does this animate over ?
	// The layer value is wrong most of the time !

	ASSERT( GetInfo()->m_flags & UFLAG4_ANIM );

	if ( m_pInfo->m_flags & UFLAG2_ZERO1 )
		return( m_pInfo->m_wUnk14 );

	//if ( m_pInfo->m_layer )	// This is sometimes wrong ?
	//	return( m_pInfo->m_layer );

	// We need to verify this value !
	// The MUL files are really goofy when it comes to animated items.

	CItemInst ItemNext;
	int idnext = GetID();
	for(;;)
	{
		// attempt to load the next frame.
		idnext++;
		ItemNext.AttachMul( (ITEMID_TYPE) idnext );

		const CMulItemInfo * pInfoNext = ItemNext.GetInfo();
		// if ( m_pInfo->m_layer == 0 )
		{
			// Must stop if the flag is not set.
			if ( ! ( pInfoNext->m_flags & UFLAG4_ANIM ))
			{
				// idnext++;	// inclusive
				break;
			}
		}

		if ( // pInfoNext->m_name[0] &&
			strcmp( m_pInfo->m_name, pInfoNext->m_name ))
		{
			// It has a name and it's not the same as the one we started with.
			break;
		}
	}

	// We know the value as best we can.
	// Now assign it the "correct" vlaue.
	m_pInfo->m_wUnk14 = idnext - GetID();
	m_pInfo->m_flags |= UFLAG2_ZERO1;

	return( m_pInfo->m_wUnk14 );
}

#endif

bool CTileItemType::DrawItemDirect( CSurfaceDC & SurfDC, int sx, int sy, bool fCenter )
{
	// This is strangely faster than the surface blit !
	// ARGS: sx,sy = middle, bottom start location.

	WORD * pData = GetLoadData();	
	if ( pData == NULL )
		return( false );

	assert( pData[0] != 0 );	// Art data size.
	assert( pData[1] != 0xFFFF );	// Art Trigger.

	int iWidth  = pData[2];
	int iHeight = pData[3];
	assert( iWidth && iHeight );

	// center it.
	if ( fCenter )
	{
		sy -= ( iHeight - TERRAIN_ART_WIDTH );	
		sx -= ( iWidth / 2 );	// center it.
	}

	if ( ! SurfDC.SetClipSrc( sx, sy, iWidth, iHeight )) 
		return false;

	int yy = SurfDC.m_rClipSrc.top;
	sy += yy;
	int iHeightDraw = SurfDC.m_rClipSrc.Height();
	yy += 4;

	WORD * pDst = (WORD*) SurfDC.m_Dest.GetLinePtr(sy);
	for ( ; iHeightDraw--; yy++ )
	{
		int index = 4+iHeight+pData[yy];
		int xx = sx;
		for(;;)
		{
			WORD usXOffset = pData[index++];
			WORD usXBits   = pData[index++];
			if ( ! usXBits )
				break;	// go to next row.

			assert( usXBits <= iWidth ); 
			xx += usXOffset;

			SurfDC.DrawXClip( xx, pDst, pData+index, usXBits );

			index += usXBits;
			xx += usXBits;
		}
		pDst = (WORD*) SurfDC.m_Dest.GetNextLine(pDst);
	}

	return( true );
}

#if 0

void CTileItemType::DrawItemSetup()
{
	// render the mul data to a temporary surface first.

	WORD * pData = GetLoadedData();	
	assert( pData );
	assert( pData[0] != 0 );	// Art data size.
	assert( pData[1] != 0xFFFF );	// Art Trigger.

	int iWidth  = pData[2];
	int iHeight = pData[3];

	m_Surf.CreateSurface( iWidth, iHeight );
	m_Surf.Erase();

	// center it.

	int iHeightCount = iHeight;

	for ( int y = 0; iHeightCount--; y++ )
	{
		int index = 4+iHeight+pData[4+y];
		int x = 0;
		for(;;)
		{
			WORD usXOffset = pData[index++];
			WORD usXBits   = pData[index++];
			if ( ! usXBits ) 
				break;	// go to next row.

			assert( usXBits <= iWidth ); 
			x += usXOffset;

			memcpy( m_Surf.m_Dest.GetPixPtr16( x, y ), pData+index, usXBits*2 );

			index += usXBits;
			x += usXBits;
		}
	}
}

bool CTileItemType::DrawItemPrepared( CSurfaceDC & SurfDC, int sx, int sy, DIR_TYPE dir )
{
	// ARGS: sx,sy = middle, bottom start location.
	ASSERT( dir != DIR_NW )

	if ( ! m_Surf.IsValid())
	{
		DrawSetup();
	}

	// center it.
	sy -= ( m_Surf.GetHeight() - TERRAIN_ART_WIDTH );
	sx -= ( m_Surf.GetWidth() / 2 );	// center it.

	// rotate it.

	return SurfDC.DrawTexture( (WORD*) m_Surf.GetLinePtr(0), 
		m_Surf.GetWidth(), m_Surf.GetHeight(), m_Surf.GetWidth(),
		rect.left+ishadowx, rect.m_bottom-ishadowy, 
		rect.m_right+ishadowx, rect.m_bottom-ishadowy, 
		rect.m_right, rect.m_bottom,
		rect.left, rect.m_bottom );

	// return SurfDC.Draw( sx, sy, m_Surf );

}

#endif

bool CTileItemType::DrawItem( CSurfaceDC & SurfDC, int sx, int sy, HUE_TYPE wHue, DIR_TYPE dir )
{
	// Draw a colorized item.
	// dir = flat items can be easily rotated.
	// UFLAG3_CLOTH items are already sphere scale. others must be converted to sphere scale first.

	if ( GetInfo()->m_flags & UFLAG3_CLOTH )
	{
		SurfDC.SetFilterCloth(true);
	}

	SurfDC.SetFilterHue( wHue );
	bool fRet;

	fRet = DrawItemDirect( SurfDC, sx, sy );

	SurfDC.SetFilterHue();
	SurfDC.SetFilterCloth(false);
	return( fRet );
}

//////////////////////////////////////////////////////////////////////
// -CTileTerrainType

CTileTerrainType::CTileTerrainType( HASH_INDEX dwHashIndex ) :
	CMulImageType( dwHashIndex )
{
	assert( GetBaseIndex() < TERRAIN_QTY );
}
CTileTerrainType::CTileTerrainType( TERRAIN_TYPE id ) :
	CMulImageType( VERDATA_MAKE_HASH( VERFILE_ART, id ))
{
	// Cache this in memory.
	assert( GetBaseIndex() < TERRAIN_QTY );
}

#define TERRAIN_ART_SIZE ( TERRAIN_ART_WIDTH*(TERRAIN_ART_HALF+1) )	// actual useful pixels

bool CTileTerrainType::Load()
{
	CMulIndexRec Index;
	if ( ! g_MulInstall.ReadMulIndex( VERFILE_ARTIDX, VERFILE_ART, GetID(), Index ))
		return( false );
	assert( Index.GetBlockLength() == 2048 );
	LoadMulData( VERFILE_ART, Index );
	return( true );
}

const CMulTerrainInfo * CTileTerrainType::GetInfo()
{
	if ( ! m_pInfo.IsValidNewObj())
	{
		m_pInfo = new CMulTerrainInfo( GetID());
	}
	return( m_pInfo);
}

void CTileTerrainType::DrawToTexture( WORD * pDst )
{
	// Convert the terrain diamond into a square texture.

	WORD * pData = GetLoadedData();
	ASSERT(pData);

	memset( pDst, 0, sizeof(WORD)*TERRAIN_ART_WIDTH );
	WORD * pDstCur = pDst + TERRAIN_ART_WIDTH;

	int iWidth = 2;
	bool rbDown = true;

	do
	{
		WORD * pDstRow = pDstCur;

		for ( int i=iWidth; --i; )
		{
			ASSERT( pDstCur >= pDst );
			ASSERT( pDstCur < pDst+TERRAIN_ART_SIZE );
			*pDstCur = *pData;
			pData++;	// WORD index.
			pDstCur -= TERRAIN_ART_WIDTH-1;
		}
		if ( rbDown && iWidth >= TERRAIN_ART_WIDTH )
		{
			pDstCur = pDstRow + 1;
			rbDown = false;
		}
		else if (rbDown)
		{
			pDstCur = pDstRow + TERRAIN_ART_WIDTH;
			iWidth += 2;
		}
		else
		{
			pDstCur = pDstRow + 1;
			iWidth -= 2;
		}
	} while (iWidth);
}

bool CTileTerrainType::DrawTerrain( CSurfaceDC & SurfDC, int sx, int sy, DIR_TYPE dirtop )
{
	// Draw from the Upper left corner. = top point.
	// dirtop = DIR_NW = default view.

	if ( GetID() == TERRAIN_HOLE ) 
		return( false );
	if ( ! SurfDC.SetClipSrc( sx-TERRAIN_ART_HALF, sy, TERRAIN_ART_WIDTH, TERRAIN_ART_WIDTH )) 
		return false;
	// The terrain block is a diamond shape.

	WORD * pData = GetLoadData();
	if ( pData == NULL )
		return( false );	// may be a valid case for this.

	int iWidth = 2;
	bool rbDown = true;

	do
	{
		SurfDC.DrawXYClip( sx, sy, pData, iWidth );
		pData += iWidth;	// WORD index.

		if ( rbDown && iWidth >= TERRAIN_ART_WIDTH )
		{
			rbDown = false;
		}
		else if (rbDown)
		{
			sx--;
			iWidth += 2;
		}
		else
		{
			sx++;
			iWidth -= 2;
		}
		sy++;
	} while (iWidth);

	return( true );
}

//////////////////////////////////////////////////////////////////////
// -CTileTextureType

CTileTextureType::CTileTextureType( HASH_INDEX dwHashIndex ) :
	CMulImageType( dwHashIndex )
{
	assert( GetBaseIndex() < TERRAIN_QTY );
	m_iWidth = 0;
}
CTileTextureType::CTileTextureType( TERRAIN_TYPE id ) :
	CMulImageType( VERDATA_MAKE_HASH( VERFILE_TEXMAPS, id ))
{
	// Cache this in video memory? 64x64 or 128x128
	assert( GetBaseIndex() < TERRAIN_QTY );
	m_iWidth = 0;
	// Load();	// Have to load them to know how big they are. or if they even exist.
}

bool CTileTextureType::LoadFromTerrain()
{
	// we can transform the terrain tile into a texture ?

	return false;

#ifdef SPHERE_CLIENT
	CTerrainInst terrain;
	terrain.AttachMul( GetID());
	ASSERT(terrain.IsValidRefObj());
	if ( ! terrain->LoadCheck())
		return( false );

	m_iWidth = TERRAIN_ART_WIDTH;
	Alloc( TERRAIN_ART_WIDTH * TERRAIN_ART_WIDTH * sizeof( COLOR_TYPE ));

	terrain->DrawToTexture( GetLoadData());
#endif
#ifdef SPHERE_MAKER
	CTileTerrainType terrain(GetID());
	if ( ! terrain.LoadCheck())
		return( false );

	m_iWidth = TERRAIN_ART_WIDTH;
	Alloc( TERRAIN_ART_WIDTH * TERRAIN_ART_WIDTH * sizeof( COLOR_TYPE ));

	terrain.DrawToTexture( GetLoadData());
#endif

	return( true );
}

bool CTileTextureType::Load()
{
	CMulIndexRec Index;
	if ( ! g_MulInstall.ReadMulIndex( VERFILE_TEXIDX, VERFILE_TEXMAPS, GetID(), Index ))
	{
		// we can transform the terrain tile into this ?
		return( LoadFromTerrain());
		//return( false );
	}

	assert( m_iWidth == 0 );
	if ( Index.GetBlockLength() == 64*64*2 )
		m_iWidth = 64;
	else if ( Index.GetBlockLength() == 128*128*2 )
		m_iWidth = 128;
	else
	{
		assert( 0 );
	}
	LoadMulData( VERFILE_TEXMAPS, Index );
	return( true );
}

#ifdef SPHERE_MAKER 

bool CTileTextureType::DrawSquare( CSurfaceDC & SurfDC, int sx, int sy )
{
	if ( ! SurfDC.SetClipSrc( sx, sy, m_iWidth, m_iWidth )) 
		return false;

	WORD * pData = GetLoadedData();	// We use bytes here !
	assert( pData );

	int iHeightDraw = SurfDC.m_rClipSrc.Height();
	int iWidthDraw = SurfDC.m_rClipSrc.Width();

	pData += ( SurfDC.m_rClipSrc.top * m_iWidth ) + SurfDC.m_rClipSrc.left;
	WORD * pDst = SurfDC.m_Dest.GetPixPtr16( sx + SurfDC.m_rClipSrc.left, sy + SurfDC.m_rClipSrc.top );

	while ( iHeightDraw-- )
	{
		SurfDC.Draw( pDst, pData, iWidthDraw );
		pData += m_iWidth;
		pDst = (WORD*) SurfDC.m_Dest.GetNextLine(pDst);
	}

	return( true );
}

#endif

////////////////////////////////////////
// Gumps 

CTileGumpType::CTileGumpType( HASH_INDEX dwHashIndex ) :
	CMulImageType( dwHashIndex )
{
	m_hMaskRegion = NULL;
	assert( GetBaseIndex() < GUMP_QTY );
}
CTileGumpType::CTileGumpType( GUMP_TYPE id ) : 
	CMulImageType( VERDATA_MAKE_HASH( VERFILE_GUMPART, id ))
{
	m_hMaskRegion = NULL;
	assert( GetBaseIndex() < GUMP_QTY );
	// Load();	// Have to load them to know how big they are..
}

CTileGumpType::~CTileGumpType()
{
	if ( m_hMaskRegion )
	{
		DeleteObject( m_hMaskRegion );
	}
}

bool CTileGumpType::Load()
{
	CMulIndexRec Index;
	if ( ! g_MulInstall.ReadMulIndex( VERFILE_GUMPIDX, VERFILE_GUMPART, GetID(), Index ))
		return( false );
	m_size.cy = Index.m_wVal3;
	m_size.cx = Index.m_wVal4;
	LoadMulData( VERFILE_GUMPART, Index );
	return( true );
}

bool CTileGumpType::DrawGump( CSurfaceDC & SurfDC, int sx, int sy, HUE_TYPE wHue )
{
	WORD * pData = GetLoadData();	// Must be loaded to know how big it is.
	if ( pData == NULL )
		return( false );

	if ( ! SurfDC.SetClipSrc( sx, sy, m_size.cx, m_size.cy )) 
		return false;
	SurfDC.SetFilterHue( wHue );

	int y = SurfDC.m_rClipSrc.top;
	sy += SurfDC.m_rClipSrc.top;
	int iHeightDraw = SurfDC.m_rClipSrc.bottom;
	ASSERT( iHeightDraw <= m_size.cy );

	WORD * pDst = (WORD*) SurfDC.m_Dest.GetLinePtr( sy );
	for ( ; y<iHeightDraw; y++ )
	{
		int index = ((DWORD*)pData)[y] * sizeof(COLOR_TYPE);
		// int index = *((DWORD*)&pData[ y * 2 ]) * sizeof(COLOR_TYPE);

#ifdef _DEBUG
		assert( IsValidOffset(pData+index));
#endif

		for ( int x=0; x<m_size.cx; )
		{
			WORD wColor = pData[index++];
			WORD wRun   = pData[index++];
			if ( ! wColor )		// act like clear ?
			{
				x += wRun;
				continue;
			}
			while ( wRun-- )
			{
				SurfDC.DrawXClip( sx+x, pDst, &wColor, 1 );
				x++;
			}
		}
		pDst = (WORD*) SurfDC.m_Dest.GetNextLine(pDst);
	}

	SurfDC.SetFilterHue();
	return( true );
}

//////////////////////////////////////////////////////////////////////
// -CTileAnimType

CTileAnimType::CTileAnimType( DWORD id ) : 
	CMulImageType( VERDATA_MAKE_HASH( VERFILE_ANIM, id ))
{
}

bool CTileAnimType::Load()
{
	CMulIndexRec Index;
	if ( ! g_MulInstall.ReadMulIndex( VERFILE_ANIMIDX, VERFILE_ANIM, GetID(), Index ))
		return( false );
	LoadMulData( VERFILE_ANIM, Index );
	return( true );
}

DWORD CTileAnimType::CvtCharAnimDirToIndex( CREID_TYPE id, ANIM_TYPE anim, DIR_TYPE dirRelative ) // static
{
	// For high detail, id = CREID_TYPE * 110 
	// 
	// always 5 anims per action. (DIR_ANIM_QTY)
	// 
	// 22 actions for high detail critters.
	//
	// 22000 - = Char C8 = Horse, 5 anims per action.
	// 13 actions for low detail
	// 
	// 35000 - = Char = Human
	// 35 actions for humans and equip
	//

	int index;
	int animqty;
	if ( id < CREID_HORSE1 )
	{
		// Monsters and high detail critters.
		// 22 actions per char.
		index = ( id * ANIM_QTY_MON * DIR_ANIM_QTY );
		animqty = ANIM_QTY_MON;
	}
	else if ( id < CREID_MAN )
	{
		// Animals and low detail critters.
		// 13 actions per char.
		index = 22000 + (( id - CREID_HORSE1 ) * ANIM_QTY_ANI * DIR_ANIM_QTY );
		animqty = ANIM_QTY_ANI;
	}
	else
	{
		// Human and equip
		// 35 actions per char.
		index = 35000 + (( id - CREID_MAN ) * ANIM_QTY_MAN * DIR_ANIM_QTY );
		animqty = ANIM_QTY_MAN;
	}

	// Factor in the animation. (assuming it exists, not all anims are supported)

	if ( anim < 0 || anim >= animqty )
	{
		anim = ANIM_WALK_UNARM;
	}

	index += ( anim * DIR_ANIM_QTY );

	// Convert the direction.
	assert( dirRelative >= 0 && dirRelative < DIR_QTY );

	static const int sm_DirOffsets[DIR_QTY] = 
	{
		3,	// DIR_N = flipped.
		2,	// flipped.
		1,	// flipped.
		0,	// DIR_SE
		1,
		2,
		3,
		4,
	};

	index += sm_DirOffsets[ dirRelative ];
	return( index );
}

void CTileAnimType::CvtIndexToCharAnim( DWORD index, CREID_TYPE & id, ANIM_TYPE & anim ) // static
{
	if ( index < 22000 )	// monster
	{
		id = (CREID_TYPE)( index / ( ANIM_QTY_MON * DIR_ANIM_QTY ));
		index -= id * ( ANIM_QTY_MON * DIR_ANIM_QTY );
	}
	else if ( index < 35000 )	// animal
	{
		index -= 22000;
		id = (CREID_TYPE)( index / ( ANIM_QTY_ANI * DIR_ANIM_QTY ));
		index -= id * ( ANIM_QTY_ANI * DIR_ANIM_QTY );
	}
	else	// human
	{
		index -= 35000;
		id = (CREID_TYPE)( index / ( ANIM_QTY_MAN * DIR_ANIM_QTY ));
		index -= id * ( ANIM_QTY_MAN * DIR_ANIM_QTY );
	}

	anim = (ANIM_TYPE) ( index / DIR_ANIM_QTY );
}

CGRect CTileAnimType::GetDrawRect( int sx, int sy, int iFrame, DIR_TYPE dirRelative )
{
	CGRect rect;
	if ( (DWORD) iFrame >= GetFrameCount())
	{
		rect.SetRectEmpty();
		return( rect );
	}
	CUOAnimFrame * pHead = GetFrameHead( iFrame );

	bool fFlipEW = ( dirRelative < DIR_SE );
	if ( fFlipEW )
	{
		rect.left = sx - ( pHead->m_Width - pHead->m_ImageCenterX );
	}
	else
	{
		rect.left = sx - pHead->m_ImageCenterX;
	}

	sy -= pHead->m_Height + pHead->m_ImageCenterY;	// top to bottom.
	sy += TERRAIN_ART_HALF;
	rect.top = sy;

	rect.right = rect.left + pHead->m_Width;
	rect.bottom = sy + pHead->m_Height;
	return( rect );
}

bool CTileAnimType::DrawFrame( CSurfaceDC & SurfDC, int sx, int sy, HUE_TYPE wHue, int iFrame, DIR_TYPE dirRelative )
{
	// sx = center.
	// AnimationGroup header
	// WORD[256] Palette

	DWORD dwFrameCount = GetFrameCount();	// check pData here.
	if ( (DWORD) iFrame >= dwFrameCount )
		return false;	

	// DWORD[FrameCount] FrameOffset 
	CUOAnimFrame * pHead = GetFrameHead( iFrame );

	// Seek from the end of Palette plus FrameOffset[FrameNum] bytes to find the start of Frame

	bool fFlipEW = ( dirRelative < DIR_SE );
	int ex = sx;
	int ey = sy;
	GetDrawFrameOffset( pHead, fFlipEW, ex, ey );

	if ( ! SurfDC.SetClipSrc( ex, ey, pHead->m_Width, pHead->m_Height )) 
		return false;

	SurfDC.SetFilterHue( wHue );
	if ( wHue & HUE_UNDERWEAR )
	{
		SurfDC.SetFilterCloth(true);
	}
	if ( wHue & HUE_TRANSLUCENT )
	{
		SurfDC.SetFilter( FILTER_TRANSPARENT );
	}

	BYTE * pFrame = (BYTE *)(pHead + 1);
	int PrevLineNum = 0xFF;

	WORD * pData = GetLoadedData();

	for(;;)
	{
		WORD wRowHeader = *((WORD*)(pFrame+0));
		WORD wRowOfs = *((WORD*)(pFrame+2));
		pFrame += 4;

		if ( wRowHeader == 0x7FFF || wRowOfs == 0x7FFF ) 
			break;

		WORD wRunLength = wRowHeader & 0xFFF;
		WORD wLineNum = wRowHeader >> 12;
		// WORD wUnknown = wRowOfs & 0x3F;

		if ( PrevLineNum != 0xFF && wLineNum != PrevLineNum ) 
		{
			ey++;
		}
		PrevLineNum = wLineNum;

		if ( ! SurfDC.IsInsideClipY( ey ))
		{
			pFrame += wRunLength;
			continue;
		}

		WORD * pDst = (WORD*) SurfDC.m_Dest.GetLinePtr( ey );

		if ( fFlipEW )
		{
			int X = sx - (((signed short) wRowOfs ) >> 6 ); 
			for ( int i=0; i<wRunLength; i++, X-- )
			{
				WORD wColor = pData[*pFrame++];	// get color from pallete
				SurfDC.DrawXClip( X, pDst, &wColor, 1 );
			}
		}
		else
		{
			int X = sx + (((signed short) wRowOfs ) >> 6 ); 
			for ( int i=0; i<wRunLength; i++, X++ )
			{
				WORD wColor = pData[*pFrame++];	// get color from pallete
				SurfDC.DrawXClip( X, pDst, &wColor, 1 );
			}
		}
	}

	SurfDC.SetFilterHue();
	SurfDC.SetFilterCloth(false);
	return( true );
}

//////////////////////////////////////////////////////////////////////
// -CTileLightType

CTileLightType::CTileLightType( HASH_INDEX dwHashIndex ) :
	CMulImageType( dwHashIndex )
{
	assert( GetBaseIndex() < LIGHT_QTY );
}
CTileLightType::CTileLightType( LIGHT_PATTERN light ) :
	CMulImageType( VERDATA_MAKE_HASH( VERFILE_LIGHT, light ))
{
	assert( GetBaseIndex() < LIGHT_QTY );
	// Load();	// size is stored here.
}

bool CTileLightType::Load()
{
	CMulIndexRec Index;
	if ( ! g_MulInstall.ReadMulIndex( VERFILE_LIGHTIDX, VERFILE_LIGHT, GetID(), Index ))
		return( false );
	m_size.cy = Index.m_wVal3;
	m_size.cx = Index.m_wVal4;
	// NOTE: These are just uncompressed rectangles. but somtimes has extra crap ?
	assert( Index.GetBlockLength() >= (DWORD) (m_size.cx*m_size.cy));

	LoadMulData( VERFILE_LIGHT, Index );
	return( true );
}

CGRect CTileLightType::GetDrawRect( int sx, int sy )
{
	CGRect rect;
	rect.left = sx;
	rect.right = sx + m_size.cx;
	rect.top = sy;
	rect.bottom = sy + m_size.cy;
	return( rect );
}

bool CTileLightType::DrawLight( CSurfaceDC & SurfDC, int sx, int sy )
{
	// This is just a simple x by y alpha pattern. (8 bits)
	assert( m_size.cx );
	assert( m_size.cy );

	if ( ! SurfDC.SetClipSrc( sx, sy, m_size.cx, m_size.cy )) 
		return false;

	BYTE * pData = (BYTE *) GetLoadedData();	// We use bytes here !
	assert( pData );

	int iHeightDraw = SurfDC.m_rClipSrc.Height();
	int iWidthDraw = SurfDC.m_rClipSrc.Width();

	pData += ( SurfDC.m_rClipSrc.top * m_size.cx ) + SurfDC.m_rClipSrc.left;
	WORD * pDst = SurfDC.m_Dest.GetPixPtr16( sx + SurfDC.m_rClipSrc.left, sy + SurfDC.m_rClipSrc.top );

	while ( iHeightDraw-- )
	{
		for ( int i=0; i<iWidthDraw; i++ )
		{
			WORD lightlevel = MAKEWORD( 0, pData[i] );
			SurfDC.Draw( pDst+i, &lightlevel, 1 );
		}
		pData += m_size.cx;
		pDst = (WORD*) SurfDC.m_Dest.GetNextLine(pDst);
	}

	return( true );
}

////////////////////////////////////////
// -CTileFont

bool CTileFontChar::Load()
{
	if ( ! m_lOffset ||
		! GetHeight() || 
		! GetWidth())	// Then it has no graphics.. Use 'space' instead..
		return( false );

	// Make up a fake index record for this char.
	CMulIndexRec Index;
	Index.SetupIndex( m_lOffset, GetHeight() * GetWidth() * sizeof(COLOR_TYPE) );

	assert( Index.GetBlockLength() <= FONT_MAX_SIZE*FONT_MAX_SIZE*sizeof(COLOR_TYPE) );

	LoadMulData( VERFILE_FONTS, Index );
	return( true );
}

bool CTileFontChar::DrawFontChar( CSurfaceDC & SurfDC, int sx, int sy )
{
	// Draw a single char on the surface.
	// x,y = bottom left

	BYTE iHeight = GetHeight();
	BYTE iWidth = GetWidth();
	sy -= iHeight;
	if ( ! SurfDC.SetClipSrc( sx, sy, iWidth, iHeight )) 
		return true;	// not normal return i know.

	// 0 = see thru color.
	WORD * pData = GetLoadData();
	if ( ! pData ) 
		return( false );

	for ( ; iHeight--; sy++ )
	{
		for ( int i=0; i<iWidth; )
		{
			if ( *pData )
			{
				int iRun = 1;
				while ( iRun+i < iWidth && pData[iRun] )
					iRun ++;
				SurfDC.DrawXYClip( sx+i, sy, pData, iRun );
				pData += iRun;
				i += iRun;
			}
			else
			{
				pData ++;
				i++;
			}
		}
	}
	return( true );
}

///////////////////////////////////

void CTileFont::Init()
{
	// Read in the whole font so we can use it.
	// Fonts are non-indexed but sequencial so just read from current pos.

	BYTE ucHeader;
	if ( g_MulInstall.m_File[VERFILE_FONTS].Read((void *)&ucHeader, 1 ) <= 0 )
		throw CGException(LOGL_CRIT, E_FAIL, "CTileFont.FontInit: Read (Header)");

	for ( int i=0; i<FONT_MAX_CHARS; i++ )
	{
		CUOFontImageHeader Hdr;
		if ( g_MulInstall.m_File[VERFILE_FONTS].Read( &Hdr, sizeof(Hdr)) <= 0 )
			throw CGException(LOGL_CRIT, E_FAIL, "CTileFont.FontInit: Read (FontHdr)");

		m_Char[i].InitFontChar( g_MulInstall.m_File[VERFILE_FONTS].GetPosition(), Hdr );

		// Skip data for now.
		if ( ! g_MulInstall.m_File[VERFILE_FONTS].Seek( Hdr.m_bRows * Hdr.m_bCols * sizeof(COLOR_TYPE), SEEK_CUR ))
		{
			throw CGException(LOGL_CRIT, E_FAIL, "CTileFont:Init: Seek");
		}

		if ( Hdr.m_bCols > m_size.cx )
			m_size.cx = Hdr.m_bCols;
		if ( Hdr.m_bRows > m_size.cy )
			m_size.cy = Hdr.m_bRows;
	}
}

int CTileFont::GetWidth( const TCHAR *pszText ) const
{
	ASSERT(pszText);
	int cx = 0;
	for ( int i = 0; pszText[i]; i++ )
	{
		TCHAR ch = pszText[i] - ' ';
		if ( ch < 0 ) 
			continue;
		assert( ch < FONT_MAX_CHARS );

		int iWidth = m_Char[ch].GetWidth();
		if ( ! iWidth )
		{
			iWidth = m_Char[0].GetWidth();	// Draw a space instead ?
		}

		cx += iWidth;
	}
	return( cx );
}

void CTileFont::DrawText( CSurfaceDC & SurfDC, int x, int y, HUE_TYPE wHue, const TCHAR *pszText )
{
	// x,y = bottom left of the text.

	ASSERT(pszText);

	SurfDC.SetFilterHue( wHue );

	for ( int i = 0; pszText[i]; i++ )
	{
		TCHAR ch = pszText[i] - ' ';
		if ( ch < 0 ) 
			continue;
		assert( ch < FONT_MAX_CHARS );
		if ( ! m_Char[ch].DrawFontChar( SurfDC, x, y ))
		{
			ch = 0;
			m_Char[0].DrawFontChar( SurfDC, x, y ); // Draw a space instead ?
		}

		x += m_Char[ch].GetWidth();
	}

	SurfDC.SetFilterHue();
}

void CTileFont::UnLoad()
{
	for ( int i=0; i<COUNTOF(m_Char); i++ )
	{
		m_Char[i].UnLoad();
	}
}

//*********************************************
// -CMulFonts

void CMulFonts::Load()
{
	// Fonts must stupidly be read in all at once.
	g_MulInstall.m_File[VERFILE_FONTS].SeekToBegin();
	for ( int f=0; f<COUNTOF(m_Font); f++ )
	{
		m_Font[f].Init();
	}
}

void CMulFonts::UnLoad()
{
	for ( int i=0; i<FONT_QTY; i++ )
	{
		m_Font[i].UnLoad();	// Fonts
	}
}

void CMulFonts::FontDrawUNICODE( CSurfaceDC & SurfDC, int x, int y, const WCHAR *pszText, HUE_TYPE wHue )
{
	// Draw the unicode font here.

#if 0

	HDC hDC = CreateCompatibleDC();

	// Map the surface to the DC. DrawTextW

	int iRet = TextOutW( hDC, x, y, pszText, wstrlen( pszText )); 	

#endif

}

//////////////////////////////////////////////////////////////////////
// - CMulHues

void CMulHues::Load( int iGamma )
{
	g_MulInstall.m_File[VERFILEX_HUES].SeekToBegin();

	DWORD dwHueGroupQty = g_MulInstall.m_File[VERFILEX_HUES].GetLength() / sizeof( CUOHueGroup );
	assert( iGamma < LIGHT_QTY );

	// Create temporary read buffer.
	CUOHueGroup * pHueGroups = new CUOHueGroup[ dwHueGroupQty ];
	ASSERT(pHueGroups);

	if ( g_MulInstall.m_File[VERFILEX_HUES].Read( (void *)pHueGroups, dwHueGroupQty * sizeof( CUOHueGroup )) <= 0 )
	{
		delete [] pHueGroups;
		throw CGException(LOGL_CRIT, E_FAIL, "CMulHues:Load Read" );
		return;
	}

	// Fix this silliness. get rid of all the extra crap stored with the hues.
	DWORD dwHueQty = UOHUE_BLOCK_QTY * dwHueGroupQty;
	m_Hues.SetSize(dwHueQty);

	int j = 0;
	for ( DWORD k=0; k<dwHueGroupQty; k++ )
	{
		for ( int n=0; n<UOHUE_BLOCK_QTY; n++ )
		{
			// Weird reverse order ?
#if 1
			memcpy( &(m_Hues.ElementAt(j)), pHueGroups[k].m_Entries[n].m_GammaRange, sizeof(CUOHueEntryBare));
#else
			for ( int i=0; i<LIGHT_QTY; i++ )
			{
				m_Hues[j].m_GammaRange[i] = pHueGroups[k].m_Entries[n].m_GammaRange[(LIGHT_QTY-1)-i];
			}
#endif
			j++;
		}
	}

	delete [] pHueGroups;

	// ??? Get any patched hues from the verdata.mul file.

#if 0
	CUOHueGroup hueGroup;
	CMulIndexRec dataIndex;
	if ( g_MulVerData.FindVerDataBlock( VERFILEX_HUES, group, dataIndex ) )
	{
		// Hues in the verdata file are seriously messed up
		// CUOHueGroup
		BYTE bOldData[sizeof(DWORD) + 8 * (sizeof(CUOHueEntry) + 64)];
		g_MulInstall.ReadMulData( VERFILE_VERDATA, dataIndex, &bOldData[0] );
		for ( int i = 0; i < 8; i++ )
		{
			memcpy(&(hueGroup.m_Entries[i]), &bOldData[sizeof(DWORD) + i * (sizeof(CUOHueEntry) + 64)], sizeof(CUOHueEntry));
		}
		memcpy(&hueGroup.m_Header, &bOldData[0], sizeof(DWORD));
	}

#endif
	// g_MulInstall.m_File[VERFILEX_HUES].Close();
}
#pragma optimize( "", on )
#endif


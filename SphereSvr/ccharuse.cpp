//
// CCharUse.cpp
// Copyright 1996 - 2001 Menace Software (www.menasoft.com)
//  CChar is either an NPC or a Player.
//

#include "stdafx.h"	// predef header.

static const WORD sm_Item_Blood[] =	// ??? not used
{
	0x1cf2, ITEMID_BLOOD1, ITEMID_BLOOD6,
};

bool CChar::Use_SpinWheel( CItem* pWheel, CItem* pHair )
{
	// Wheel is in use = type is wrong.
	if ( pWheel == NULL || pHair == NULL )
		return false;
	if ( ! pWheel->IsType(IT_SPINWHEEL))
		return false;
	if ( ! CanUse( pWheel, false ))
		return( false );

	pWheel->SetAnim( (ITEMID_TYPE)( pWheel->GetID() + 1 ), 2*TICKS_PER_SEC );

	// 1 pile of cotton yields spools of thread
	// 1 pile of wool yields ball(s) of yarn
	ITEMID_TYPE idTarg = ( pHair->IsType(IT_WOOL)) ? ITEMID_YARN1 : ITEMID_THREAD1;

	CItemDefPtr pItemDef = g_Cfg.FindItemDef(idTarg);
	if ( pItemDef == NULL || pItemDef->m_BaseResources.GetSize() <= 0 )
		return false;

	int iNeed = pItemDef->m_BaseResources[0].GetResQty();
	pHair->ConsumeAmount( iNeed );

	CItemPtr pItemNew = CItem::CreateScript(idTarg,this);
	Printf( "You create some %s.", (LPCTSTR) pItemNew->GetName());
	ItemBounce( pItemNew );
	return true;
}

bool CChar::Use_Loom( CItem* pLoom, CItem* pYarn )
{
	// Loom is in use = type is wrong.
	if ( pLoom == NULL )
		return false;
	if ( ! pLoom->IsType( IT_LOOM ))
		return false;

	if ( ! CanUse( pLoom, false ))
		return( true );

	static LPCTSTR const sm_Txt_LoomUse[] =
	{
		"You start a new piece of cloth.",
		"The piece of cloth needs a good deal more.",
		"The piece of cloth needs a little more.",
		"The piece of cloth is nearly finished.",
		"The piece of cloth is finished.",
	};

	// pLoom->SetAnim( (ITEMID_TYPE)( pLoom->GetID() + 1 ), 2*TICKS_PER_SEC );

	// Use more1 to record the type of resource last used on this object
	// Use more2 to record the number of resources used so far
	// Check what was used last.
	if ( pLoom->m_itLoom.m_ClothID != pYarn->GetDispID() &&
		pLoom->m_itLoom.m_ClothID )
	{
		// throw away what was on here before
		WriteString("You remove the previously uncompleted weave.");
		CItemPtr pItemCloth = CItem::CreateTemplate( pLoom->m_itLoom.m_ClothID, NULL, this );
		pItemCloth->SetAmount( pLoom->m_itLoom.m_ClothQty );
		pLoom->m_itLoom.m_ClothQty = 0;
		pLoom->m_itLoom.m_ClothID = ITEMID_NOTHING;
		ItemBounce( pItemCloth );
		return true;
	}

	pLoom->m_itLoom.m_ClothID = pYarn->GetDispID();

	CItemDefPtr pItemDef = g_Cfg.FindItemDef(ITEMID_CLOTH1);
	if ( pItemDef == NULL || pItemDef->m_BaseResources.GetSize() <= 0 )
		return false;

	int iNeed = pItemDef->m_BaseResources[0].GetResQty();
	int iUsed;
	int iHave = pLoom->m_itLoom.m_ClothQty;
	if ( iHave < iNeed )
	{
		iNeed -= iHave;
		iUsed = pYarn->ConsumeAmount( iNeed );
	}

	if ( iHave + iUsed < iNeed )
	{
		pLoom->m_itLoom.m_ClothQty = iHave + iUsed;
		WriteString( sm_Txt_LoomUse[ IMULDIV( pLoom->m_itLoom.m_ClothQty, COUNTOF( sm_Txt_LoomUse )-1, iNeed ) ] );
	}
	else
	{
		WriteString( sm_Txt_LoomUse[ COUNTOF( sm_Txt_LoomUse )-1 ] );
		pLoom->m_itLoom.m_ClothQty = 0;
		pLoom->m_itLoom.m_ClothID = ITEMID_NOTHING;
		ItemBounce( CItem::CreateScript(ITEMID_CLOTH1,this));
	}

	return true;
}

bool CChar::Use_MultiLockDown( CItem* pItemTarg )
{
	// Player must be standing in the house !
	ASSERT(pItemTarg);
	ASSERT(m_pArea);
	DEBUG_CHECK( m_pArea->IsMultiRegion() );

	if ( ! pItemTarg->IsMovableType())
		return( false );
	if ( pItemTarg->IsType(IT_KEY))	// Keys cannot be locked down.
		return( false );

	// I am in my house ?
	if ( ! pItemTarg->m_uidLink.IsValidObjUID())
	{
		// Are we allowed to do this on this server?
		if ( ! g_Cfg.m_fAllowKeyLockdown )
		{
			WriteString( "Locking items using this method is not allowed." );
			return true;
		}

		// If we are in a house then lock down the item.
		pItemTarg->m_uidLink = m_pArea->GetUIDIndex();
		WriteString( "The item has been locked down." );
		return( true );
	}

	if ( pItemTarg->m_uidLink == m_pArea->GetUIDIndex())
	{
		// Are we allowed to do this on this server?
		if ( ! g_Cfg.m_fAllowKeyLockdown )
		{
			WriteString( "Unlocking items using this method is not allowed." );
			return true;
		}

		// If we are in a house then lock down the item.
		pItemTarg->m_uidLink.InitUID();
		WriteString( "The item has been un-locked from the structure." );
		return( true );
	}

	return( false );
}

void CChar::Use_CarveCorpse( CItemCorpse* pCorpse )
{
	ASSERT(pCorpse);
	DEBUG_CHECK( pCorpse->IsType(IT_CORPSE));

	UpdateAnimate( ANIM_BOW );

	CCharPtr pChar = g_World.CharFind(pCorpse->m_uidLink);
	if ( pChar && pChar->IsStatFlag( STATF_Sleeping ))
	{
		// They are violently awakened.
		WriteString( "That is not a corpse!" );
		pChar->WriteString( "You are violently awakened" );
		pChar->OnTakeDamage( 1 + Calc_GetRandVal(10), this, DAMAGE_HIT_SLASH|DAMAGE_GENERAL );
		return;
	}

	CPointMap pnt = pCorpse->GetTopLevelObj()->GetTopPoint();

	bool fHumanCorpse = CCharDef::IsHumanID( pCorpse->GetCorpseType());
	if ( fHumanCorpse )
	{
		// If it's a player corpse, put the stuff in one of these and (mabey)
		// flag them a criminal

		if ( pChar && pChar->m_pPlayer.IsValidNewObj())
		{
			// Until the corse inherits NPC criminal status, only do
			// other players
			if ( CheckCorpseCrime( pCorpse, false, false ))
			{
				WriteString("Guards can now be called on you.");
			}
		}

		// Dump any stuff on corpse.
		pCorpse->ContentsDump( pnt );
	}

	CREID_TYPE CorpseID = (CREID_TYPE) pCorpse->m_itCorpse.m_BaseID;
	CCharDefPtr pCorpseDef = g_Cfg.FindCharDef( CorpseID );
	if ( pCorpseDef != NULL )
	{
		if ( pCorpseDef->m_wBloodHue != (HUE_TYPE)-1 )
		{
			CItemPtr pBlood = CItem::CreateBase( (ITEMID_TYPE) sm_Item_Blood[ Calc_GetRandVal( COUNTOF( sm_Item_Blood )) ] );
			ASSERT(pBlood);
			pBlood->SetHue( pCorpseDef->m_wBloodHue );
			pBlood->MoveToDecay( pCorpse->GetTopPoint(), 60*TICKS_PER_SEC );
		}
	}

	pCorpse->m_itCorpse.m_uidKiller = GetUID();	// by you.

	// based on corpse type.
	if ( pCorpseDef == NULL ||
		! pCorpse->m_itCorpse.m_timeDeath.IsTimeValid() )
	{
		WriteString( "You carve the corpse but find nothing usefull." );
		pCorpse->m_itCorpse.m_timeDeath.InitTime();
		return;
	}

	pCorpse->m_itCorpse.m_timeDeath.InitTime();	// been carved.

	LPCTSTR pszMsg = NULL;
	int iItems = 0;
	for ( int i=0; i < pCorpseDef->m_BaseResources.GetSize(); i++ )
	{
		int iQty = pCorpseDef->m_BaseResources[i].GetResQty();
		CSphereUID rid = pCorpseDef->m_BaseResources[i].GetResourceID();
		if ( rid.GetResType() != RES_ItemDef )
		{
			continue;
		}

		ITEMID_TYPE id = (ITEMID_TYPE) rid.GetResIndex();
		if ( id == ITEMID_NOTHING )
			break;

		CItemPtr pPart = CItem::CreateTemplate( id, NULL, this );
		ASSERT(pPart);

		LPCTSTR pszMsgNew = NULL;
		switch ( pPart->GetType())
		{
		case IT_FOOD:
		case IT_FOOD_RAW:	// meat
		case IT_MEAT_RAW:
			pszMsgNew = "You carve away some meat.";
			pPart->m_itFood.m_MeatType = CorpseID;
			break;
		case IT_HIDE:	// hides
		case IT_LEATHER:	// hides
			pszMsgNew = "You skin the corpse and get the hides.";
			pPart->m_itSkin.m_creid = CorpseID;
			break;
		case IT_FEATHER:	// feathers.
			pszMsgNew = "You pluck the bird and get some feathers.";
			pPart->m_itSkin.m_creid = CorpseID;
			break;
		case IT_FUR:	// fur
			pszMsgNew = "You skin the corpse and get some fur.";
			pPart->m_itSkin.m_creid = CorpseID;
			break;
		case IT_WOOL:	// wool
			pszMsgNew = "You skin the corpse and get some unspun wool.";
			pPart->m_itSkin.m_creid = CorpseID;
			break;
		case IT_BLOOD:
			if ( pCorpseDef->m_wBloodHue != (HUE_TYPE)-1 )
			{
				pPart->SetHue( pCorpseDef->m_wBloodHue );
			}
			pPart->m_itSkin.m_creid = CorpseID;
			pPart->SetAttr(ATTR_CAN_DECAY);	// not movable but decayable.
			break;
		}

		if ( pszMsg == NULL )
			pszMsg = pszMsgNew;
		if ( iQty > 1 )
			pPart->SetAmount( iQty );

		if ( !fHumanCorpse )
		{
			pCorpse->ContentAdd( pPart );
		}
		else
		{
			if ( pChar )
			{
				CGString sName;
				sName.Format( "%s of %s", (LPCTSTR) pPart->GetName(), (LPCTSTR) pChar->GetName());
				pPart->SetName( sName );
				pPart->m_uidLink = pChar->GetUID();
			}
			pPart->MoveToDecay( pnt, pPart->GetDecayTime() );
		}

		iItems++;
	}

	if ( ! iItems )
	{
		pszMsg = "You carve the corpse but find nothing usefull.";
	}
	if ( pszMsg != NULL )
	{
		WriteString( pszMsg );
	}

	if ( pCorpseDef->m_wBloodHue != (HUE_TYPE)-1 )
	{
		CItemPtr pBlood = CItem::CreateBase( ITEMID_BLOOD1 );
		ASSERT(pBlood);
		pBlood->SetHue( pCorpseDef->m_wBloodHue );
		pBlood->MoveToDecay( pnt, 3*60*TICKS_PER_SEC );
	}

	if ( fHumanCorpse )
	{
		pCorpse->DeleteThis();
	}
}

void CChar::Use_AdvanceGate( CItem* pItem )
{
	// Have we already used one ?
	ASSERT(pItem);
	DEBUG_CHECK( pItem->IsType(IT_ADVANCE_GATE));

	if ( ! IsClient())
		return;

	ASSERT( m_pPlayer.IsValidNewObj());

	if ( pItem->m_itAdvanceGate.m_Type )
	{
		CCharDefPtr pCharDef = g_Cfg.FindCharDef( (CREID_TYPE) RES_GET_INDEX( pItem->m_itAdvanceGate.m_Type ));
		if ( pCharDef == NULL )
		{
			WriteString( "The advance gate is not configured." );
			return;
		}
		if ( ! ReadScriptTrig( pCharDef, CCharDef::T_Create ))
		{
			WriteString( "The advance gate fails!" );
			return;
		}
	}

	RemoveFromView();
	Update();
	Effect( EFFECT_OBJ, ITEMID_FX_ADVANCE_EFFECT, this, 9, 30 );
}

void CChar::Use_MoonGate( CItem* pItem )
{
	ASSERT(pItem);

	bool fQuiet = pItem->m_itTelepad.m_fQuiet;
	CPointMap ptTeleport = pItem->m_itTelepad.m_pntMark;
	if ( pItem->IsType(IT_MOONGATE))
	{
		// RES_MoonGates
		// What gate are we at ?
		int iCount = g_Cfg.m_MoonGates.GetSize();
		if ( iCount == 0 )
		{
			Printf( "There are no moons in the sky" );
			return;
		}

		// Which gate am i ?
		int i=0;
		for ( ;; i++ )
		{
			if ( i>=iCount)
				return;
			int iDist = GetTopPoint().GetDist( g_Cfg.m_MoonGates[i] );
			if ( iDist <= SPHEREMAP_VIEW_SIZE )
				break;
		}

		// Set it's current destination based on the moon phases.
		int iPhaseJump = ( g_World.Moon_GetPhase( MOON_TRAMMEL ) - g_World.Moon_GetPhase( MOON_FELUCCA )) % iCount;
		// The % operator actually can produce negative numbers...Check that.
		if (iPhaseJump < 0)
			iPhaseJump += iCount;

		ptTeleport = g_Cfg.m_MoonGates[ (i+ iPhaseJump) % iCount ];
	}

	if ( ! m_pPlayer )
	{
		// NPCs and gates.
		ASSERT(m_pNPC.IsValidNewObj());
		if ( pItem->m_itTelepad.m_fPlayerOnly )
		{
			return;
		}
		if ( m_pNPC->m_Brain == NPCBRAIN_GUARD )
		{
			// Guards won't gate into unguarded areas.
			CRegionComplexPtr pArea = REF_CAST(CRegionComplex,ptTeleport.GetRegion(REGION_TYPE_MULTI|REGION_TYPE_AREA));
			if ( pArea == NULL || ! pArea->IsFlagGuarded())
				return;
		}
		if ( Noto_IsCriminal())
		{
			// wont teleport to guarded areas.
			CRegionComplexPtr pArea = REF_CAST(CRegionComplex,ptTeleport.GetRegion(REGION_TYPE_MULTI|REGION_TYPE_AREA));
			if ( pArea == NULL || pArea->IsFlagGuarded())
				return;
		}
	}

	// Teleport me. and take a step.
	Spell_Effect_Teleport( ptTeleport, true,
		(pItem->IsAttr(ATTR_DECAY))?true:false,
		(fQuiet) ? ITEMID_NOTHING : ITEMID_FX_TELE_VANISH );
}

bool CChar::Use_Kindling( CItem* pKindling )
{
	ASSERT(pKindling);
	if ( pKindling->IsItemInContainer())
	{
		WriteString( "You can't light the kindling in a container" );
		return false ;
	}

	if ( ! Skill_UseQuick( SKILL_CAMPING, Calc_GetRandVal(30)))
	{
		WriteString( "You fail to ignite the fire." );
		return false;
	}

	pKindling->SetID( ITEMID_CAMPFIRE );
	pKindling->SetAttr(ATTR_MOVE_NEVER | ATTR_CAN_DECAY);
	pKindling->SetTimeout( (4+pKindling->GetAmount())*60*TICKS_PER_SEC );
	pKindling->SetAmount(1);	// All kindling is set to one fire.
	pKindling->m_itLight.m_pattern = LIGHT_LARGE;
	pKindling->Update();
	pKindling->Sound( 0x226 );
	return( true );
}

bool CChar::Use_Cannon_Feed( CItem* pCannon, CItem* pFeed )
{
	if ( pCannon != NULL &&
		pCannon->IsType(IT_CANNON_MUZZLE) &&
		pFeed != NULL )
	{
		if ( ! CanUse( pCannon, false ))
			return( false );
		if ( ! CanUse( pFeed, true ))
			return( false );

		if ( pFeed->GetID() == ITEMID_REAG_SA )
		{
			if ( pCannon->m_itCannon.m_Load & 1 )
			{
				WriteString( "The cannon already has powder." );
				return( false );
			}
			pCannon->m_itCannon.m_Load |= 1;
			WriteString( "Powder loaded." );
			return( true );
		}

		if ( pFeed->IsType(IT_CANNON_BALL))
		{
			if ( pCannon->m_itCannon.m_Load & 2 )
			{
				WriteString( "The cannon already has shot." );
				return( false );
			}
			pCannon->m_itCannon.m_Load |= 2;
			WriteString( "Shot loaded." );
			return( true );
		}
	}

	WriteString( "Feed shot and powder into the cannon muzzle." );
	return( false );
}

bool CChar::Use_Train_Dummy( CItem* pItem, bool fSetup )
{
	// IT_TRAIN_DUMMY
	// Dummy animation timer prevents over dclicking.

	ASSERT(pItem);
	SKILL_TYPE skill = Fight_GetWeaponSkill();
	if ( skill == SKILL_ARCHERY ) // We do not allow archerytraining on dummys.
	{
		WriteString( "You cannot train archery on this." );
		return( false );
	}
	if ( Skill_GetBase(skill) > 300 )
	{
		WriteString( "You can learn alot from a dummy, but you've already learned it all." );
		return( false );
	}
	if ( ! pItem->IsTopLevel())
	{
	baddumy:
		WriteString( "You must be standing in front of or behind the dummy to use it." );
		return( false );
	}

	// Check location
	int dx = GetTopPoint().m_x - pItem->GetTopPoint().m_x;
	int dy = GetTopPoint().m_y - pItem->GetTopPoint().m_y;

	if ( pItem->GetDispID() == ITEMID_DUMMY1 )
	{
		if ( ! ( ! dx && ABS(dy) < 2 ))
			goto baddumy;
	}
	else
	{
		if ( ! ( ! dy && ABS(dx) < 2 ))
			goto baddumy;
	}

	if ( fSetup )
	{
		if ( Skill_GetActive() == NPCACT_TRAINING )
		{
			return true;
		}
		UpdateAnimate( ANIM_ATTACK_WEAPON );
		m_Act.m_TargPrv = m_uidWeapon;
		m_Act.m_Targ = pItem->GetUID();
		Skill_Start( NPCACT_TRAINING );
	}
	else
	{
		pItem->SetAnim( (ITEMID_TYPE) ( pItem->GetID() + 1 ), 3*TICKS_PER_SEC );
		pItem->Sound( 0x033 );
		Skill_UseQuick( skill, Calc_GetRandVal(40)) ;
	}
	return( true );
}

bool CChar::Use_Train_PickPocketDip( CItem* pItem, bool fSetup )
{
	// IT_TRAIN_PICKPOCKET
	// Train dummy.

	ASSERT(pItem);
	if ( Skill_GetBase(SKILL_STEALING) > 300 )
	{
		WriteString( "Try practicing on real people." );
		return( true );
	}
	if ( !pItem->IsTopLevel())
	{
	badpickpocket:
		WriteString( "You must be standing in front of or behind the dip to use it." );
		return( true );
	}

	int dx = GetTopPoint().m_x - pItem->GetTopPoint().m_x;
	int dy = GetTopPoint().m_y - pItem->GetTopPoint().m_y;

	bool fNS = ( pItem->GetDispID() == ITEMID_PICKPOCKET_NS ||
		pItem->GetDispID() == ITEMID_PICKPOCKET_NS2 );
	if ( fNS )
	{
		if ( ! ( ! dx && ABS(dy) < 2 ))
			goto badpickpocket;
	}
	else
	{
		if ( ! ( ! dy && ABS(dx) < 2 ))
			goto badpickpocket;
	}

	if ( fSetup )
	{
		if ( Skill_GetActive() == NPCACT_TRAINING )
		{
			return true;
		}
		m_Act.m_TargPrv = m_uidWeapon;
		m_Act.m_Targ = pItem->GetUID();
		Skill_Start( NPCACT_TRAINING );
	}
	else
	{
		if ( ! Skill_UseQuick( SKILL_STEALING, Calc_GetRandVal(40)))
		{
			pItem->Sound( 0x041 );
			pItem->SetAnim( fNS ? ITEMID_PICKPOCKET_NS_FX : ITEMID_PICKPOCKET_EW_FX, 3*TICKS_PER_SEC );
			UpdateAnimate( ANIM_ATTACK_WEAPON );
		}
		else
		{
			WriteString( "You feel confident in your deception." );
			// pItem->Sound( 0x033 );
		}
	}

	return( true );
}

bool CChar::Use_Train_ArcheryButte( CItem* pButte, bool fSetup )
{
	// IT_ARCHERY_BUTTE
	// If we are standing right next to the butte,
	// retrieve the arrows and bolts
	ASSERT(pButte);
	DEBUG_CHECK( pButte->IsType(IT_ARCHERY_BUTTE) );

	ITEMID_TYPE AmmoID;

	if ( GetDist( pButte ) < 2 )
	{
		if ( pButte->m_itArcheryButte.m_AmmoCount == 0 )
		{
			WriteString( "The target is empty.");
			return ( true );
		}

		AmmoID = pButte->m_itArcheryButte.m_AmmoType;
		CItemDefPtr pAmmoDef = g_Cfg.FindItemDef(AmmoID);
		if ( pAmmoDef != NULL )
		{
			CGString sMsg;
			sMsg.Format( "remove the %s%s from the target",
				(LPCTSTR) pAmmoDef->GetName(),
				(pButte->m_itArcheryButte.m_AmmoCount == 1 ) ? "" : "s" );
			Emote( sMsg );

			CItemPtr pRemovedAmmo =  CItem::CreateBase( AmmoID );
			ASSERT(pRemovedAmmo);
			pRemovedAmmo->SetAmount( pButte->m_itArcheryButte.m_AmmoCount );
			ItemBounce( pRemovedAmmo );
		}

		// Clear the target
		pButte->m_itArcheryButte.m_AmmoType = ITEMID_NOTHING;
		pButte->m_itArcheryButte.m_AmmoCount = 0;
		return ( true );
	}

	// We are not standing right next to the target...

	// We have to be using the archery skill on this
	SKILL_TYPE skill = Fight_GetWeaponSkill();
	if ( skill != SKILL_ARCHERY )
	{
		WriteString( "You can only train archery on this." );
		return ( true );
	}
	if ( Skill_GetBase(SKILL_ARCHERY)>300 )
	{
		WriteString( "You can't learn any more by using an archery butte." );
		return ( true );
	}

	// Make sure we have some ammo

	CItemPtr pWeapon = g_World.ItemFind(m_uidWeapon);
	ASSERT(pWeapon);
	CItemDefPtr pWeaponDef = pWeapon->Item_GetDef();
	AmmoID = (ITEMID_TYPE) pWeaponDef->m_ttWeaponBow.m_idAmmo.GetResIndex();

	// If there is a different ammo type on the butte currently,
	// tell us to remove the current type first.
	if ( (pButte->m_itArcheryButte.m_AmmoType != ITEMID_NOTHING) &&
		(pButte->m_itArcheryButte.m_AmmoType != AmmoID))
	{
		WriteString( "You need to remove what is on the target before you can train here." );
		return ( true );
	}

	// We need to be correctly aligned with the target before we can use it
	// For the south facing butte, we need to have the save x value and a y value of butte.y + 6
	// For the east facing butte, we need to have the same y value and an x value of butte.x + 6
	if ( ! pButte->IsTopLevel())
	{
	badalign:
		WriteString("You need to be correctly aligned with the butte in order to use it.");
		return ( true );
	}

	int targDistX = GetTopPoint().m_x - pButte->GetTopPoint().m_x;
	int targDistY = GetTopPoint().m_y - pButte->GetTopPoint().m_y;

	if (pButte->GetDispID() == ITEMID_ARCHERYBUTTE_S)
	{
		if ( ! ( targDistX == 0 && targDistY > 3 && targDistY < 7 ))
			goto badalign;
	}
	else
	{
		if ( ! ( targDistY == 0 && targDistX > 3 && targDistX < 7 ))
			goto badalign;
	}

	// Is anything in the way?
	if ( !CanSeeLOS(pButte))
	{
		WriteString("The target is currently blocked.");
		return ( true );
	}

	if ( fSetup )
	{
		if ( Skill_GetActive() == NPCACT_TRAINING )
		{
			return true;
		}
		UpdateAnimate( ANIM_ATTACK_WEAPON );
		m_Act.m_TargPrv = m_uidWeapon;
		m_Act.m_Targ = pButte->GetUID();
		Skill_Start( NPCACT_TRAINING );
		return true;
	}

	if ( m_pPlayer.IsValidNewObj() &&
		AmmoID &&
		ContentConsume( CSphereUID( RES_ItemDef, AmmoID )))
	{
		WriteString( "You have no ammunition" );
		return( true );
	}

	// OK...go ahead and fire at the target
	// Check the skill
	bool fSuccess = Skill_UseQuick( skill, Calc_GetRandVal(40));

	pButte->Effect( EFFECT_BOLT, (ITEMID_TYPE) pWeaponDef->m_ttWeaponBow.m_idAmmoX.GetResIndex(), this, 16, 0, false );
	pButte->Sound( 0x224 );

	static LPCTSTR const sm_Txt_ArcheryButte_Failure[] =
	{
		"shot is well off target.",
		"shot is wide of the mark.",
		"shot misses terribly.",
		"shot nearly misses the archery butte."
	};

	static LPCTSTR const sm_Txt_ArcheryButte_Success[] =
	{
		"hit the outer ring.",
		"hit the middle ring.",
		"hit the inner ring.",
		"hit the bullseye!."
	};

	if ( ! AmmoID )
		return( true );

	// Did we destroy the ammo?
	CItemDefPtr pAmmoDef = g_Cfg.FindItemDef(AmmoID);
	if (!fSuccess)
	{
		// Small chance of destroying the ammo
		if ( !Calc_GetRandVal(10))
		{
			CGString sMessage;
			sMessage.Format( "poor shot destroys the %s.", (LPCTSTR) pAmmoDef->GetName() );
			Emote( sMessage, NULL, true );
			return ( true );
		}

		Emote( sm_Txt_ArcheryButte_Failure[ Calc_GetRandVal(COUNTOF(sm_Txt_ArcheryButte_Failure)) ] );
	}
	else
	{
		// Very small chance of destroying another arrow
		if ( !Calc_GetRandVal(50) && pButte->m_itArcheryButte.m_AmmoCount )
		{
			CGString sMessage;
			sMessage.Format("shot splits another %s.", (LPCTSTR) pAmmoDef->GetName() );
			Emote( sMessage, NULL, true );
			return( true );
		}

		Emote( sm_Txt_ArcheryButte_Success[ Calc_GetRandVal(COUNTOF(sm_Txt_ArcheryButte_Success)) ] );
	}
	// Update the target
	pButte->m_itArcheryButte.m_AmmoType = AmmoID;
	pButte->m_itArcheryButte.m_AmmoCount++;
	return( true );
}

bool CChar::Use_Item_Web( CItem* pItemWeb )
{
	// IT_WEB
	// IT_EQ_STUCK
	// Try to break out of the web.
	// Or just try to damage it.

	// RETURN: true = held in place.
	//  false = walk thru it.
	ASSERT( pItemWeb );
	DEBUG_CHECK( pItemWeb->IsType(IT_WEB ));

	if ( GetDispID() == CREID_GIANT_SPIDER ||
		IsGM() ||
		! pItemWeb->IsTopLevel() ||
		IsStatFlag(STATF_DEAD|STATF_Insubstantial))
	{
		return( false );	// just walks thru it.
	}

	// Try to break it.
	int iStr = pItemWeb->m_itWeb.m_Hits_Cur;
	if ( iStr == 0 )
	{
		iStr = pItemWeb->m_itWeb.m_Hits_Cur = 60 + Calc_GetRandVal( 250 );
	}

	CItemPtr pFlag = LayerFind( LAYER_FLAG_Stuck );

	// Since broken webs become spider silk, we should get out of here now if we aren't in a web.
	if ( CanMove( pItemWeb ))
	{
		if (pFlag)
			pFlag->DeleteThis();
		return( false );
	}

	if ( pFlag )
	{
		// don't allow me to try to damage it too often.
		if ( pFlag->IsTimerSet())
			return( true );
	}

	int iDmg = pItemWeb->OnTakeDamage( m_StatStr, this );

	if ( iDmg != INT_MAX && GetTopPoint() != pItemWeb->GetTopPoint())
		return( false ); // at a distance ?

	// Stuck in it still.
	if ( pFlag == NULL )
	{
		if ( iDmg < 0 )
			return( false );

		// First time message.
		pFlag = CItem::CreateBase( ITEMID_WEB1_1 );
		ASSERT(pFlag);
		pFlag->SetType(IT_EQ_STUCK);
		pFlag->m_uidLink = pItemWeb->GetUID();
		LayerAdd( pFlag, LAYER_FLAG_Stuck );
	}
	else
	{
		if ( iDmg < 0 )
		{
			pFlag->DeleteThis();
			return( false );
		}
		Printf( "You are stuck on the %s", (LPCTSTR) pItemWeb->GetName());
	}

	pFlag->SetTimeout( TICKS_PER_SEC );	// Don't check it too often.
	return( true );
}

int CChar::Use_PlayMusic( CItem* pInstrument, int iDifficultyToPlay )
{
	// SKILL_ENTICEMENT, SKILL_MUSICIANSHIP,
	// ARGS:
	//	iDifficultyToPlay = 0-100
	// RETURN:
	//  0 = success
	//  -1 = too hard for u.
	//  -2 = can't play. no instrument.
	//

	if ( pInstrument == NULL )
	{
		pInstrument = ContentFind( CSphereUID(RES_TypeDef,IT_MUSICAL) );
		if ( pInstrument == NULL )
		{
			WriteString( "You have no musical instrument available" );
			return( -2 );
		}
	}

	bool fSuccess = Skill_UseQuick( SKILL_MUSICIANSHIP, iDifficultyToPlay );
	Sound( pInstrument->Use_Music( fSuccess ));
	if ( ! fSuccess )
	{
		WriteString( "You play poorly." );
		return( -1 );	// Impossible.
	}
	return( 0 );	// success
}

bool CChar::Use_Repair( CItem* pItemArmor )
{
	// Attempt to repair the item.
	// If it is repairable.

	if ( pItemArmor == NULL || ! pItemArmor->Armor_IsRepairable())
	{
		WriteString( "The item is not repairable" );
		return( false );
	}

	if ( pItemArmor->IsItemEquipped())
	{
		WriteString( "Can't repair an item being worn !" );
		return( false );
	}
	if ( ! CanUse( pItemArmor, true ))
	{
		WriteString( "Can't repair this where it is !" );
		return( false );
	}

	// Skip this, since OSI doesn't do an armslore check on repair
	//if ( ! Skill_UseQuick( SKILL_ARMSLORE, Calc_GetRandVal(30)))
	//{
		//WriteString( "You have trouble figuring out the item." );
		//return( false );
	//}

	if ( pItemArmor->m_itArmor.m_Hits_Cur >= pItemArmor->m_itArmor.m_Hits_Max )
	{
		WriteString( "The item is already in full repair." );
		return( false );
	}

	m_Act.m_pt = g_World.FindItemTypeNearby( GetTopPoint(), IT_ANVIL, 3 );
	if ( ! m_Act.m_pt.IsValidPoint())
	{
		WriteString( "You must be near an anvil to repair" );
		return( false );
	}

	CItemDefPtr pItemDef = pItemArmor->Item_GetDef();
	ASSERT(pItemDef);

	// Use up some raw materials to repair.
	int iTotalHits = pItemArmor->m_itArmor.m_Hits_Max;
	int iDamageHits = pItemArmor->m_itArmor.m_Hits_Max - pItemArmor->m_itArmor.m_Hits_Cur;
	int iDamagePercent = IMULDIV( 100, iDamageHits, iTotalHits );

	int iMissing = ResourceConsumePart( &(pItemDef->m_BaseResources), 1, iDamagePercent/2, true );
	if ( iMissing >= 0 )
	{
		// Need this to repair.
		CResourceDefPtr pCompDef = g_Cfg.ResourceGetDef( pItemDef->m_BaseResources.GetAt(iMissing).GetResourceID() );
		Printf( "You lack %s to repair this.", pCompDef ? (LPCTSTR) pCompDef->GetName() : "components" );
		return( false );
	}

	UpdateDir( m_Act.m_pt );
	UpdateAnimate( ANIM_ATTACK_WEAPON );

	// quarter the skill to make it.
	// + more damaged items should be harder to repair.
	// higher the percentage damage the closer to the skills to make it.
	//

	int iRes = pItemDef->m_SkillMake.FindResourceType( RES_Skill );
	if ( iRes < 0 ) // no skill ?
	{
		return( false );
	}

	CResourceQty RetMainSkill = pItemDef->m_SkillMake[iRes];

	int iSkillLevel = RetMainSkill.GetResQty() / 10;
	int iDifficulty = IMULDIV( iSkillLevel, iDamagePercent, 100 );
	if ( iDifficulty < iSkillLevel/4 )
	{
		iDifficulty = iSkillLevel/4;
	}

	LPCTSTR pszText;
	bool fSuccess = Skill_UseQuick( (SKILL_TYPE) RetMainSkill.GetResIndex(), iDifficulty );
	if ( fSuccess )
	{
		pItemArmor->m_itArmor.m_Hits_Cur = iTotalHits;
		pszText = "repair";
	}
	else
	{
		// Failure
		if ( ! Calc_GetRandVal(6))
		{
			pszText = "really damage";
			pItemArmor->m_itArmor.m_Hits_Max --;
			// pItemArmor->OnTakeDamage()
			pItemArmor->m_itArmor.m_Hits_Cur --;
		}
		else if ( ! Calc_GetRandVal(3))
		{
			pszText = "slightly damage";
			// pItemArmor->OnTakeDamage()
			pItemArmor->m_itArmor.m_Hits_Cur --;
		}
		else
		{
			pszText = "fail to repair";
		}

		// Some random amount.
		iDamagePercent = Calc_GetRandVal(iDamagePercent);
	}

	ResourceConsumePart( &(pItemDef->m_BaseResources), 1, iDamagePercent/2, false );

	if ( pItemArmor->m_itArmor.m_Hits_Cur <= 0 )
	{
		pszText = "destroy";
	}

	CGString sMsg;
	sMsg.Format( "%s the %s", (LPCTSTR) pszText, (LPCTSTR) pItemArmor->GetName());
	Emote( sMsg );

	if ( pItemArmor->m_itArmor.m_Hits_Cur <= 0 )
	{
		pItemArmor->DeleteThis();
	}

	return( fSuccess );
}

void CChar::Use_EatQty( CItem* pFood, int iQty )
{
	// low level eat
	ASSERT(pFood);
	if ( iQty <= 0 )
		return;

	if ( iQty > pFood->GetAmount())
		iQty = pFood->GetAmount();
	int iValue = pFood->Item_GetDef()->GetVolume(); // some food should have more value than other !
	if ( ! iValue )
		iValue = 1;

	CCharDefPtr pCharDef = Char_GetDef();
	ASSERT(pCharDef);

	int iSpace = pCharDef->m_MaxFood - m_StatFood;
	if ( iSpace <= 0 )	// no room
		return;
	int iValueTotal = iValue* iQty;
	if ( iValueTotal > iSpace && iQty > 1 )
	{
		iQty = iSpace / iValue;
		if ( iQty <= 0 )
			iQty = 1;
	}

	// Was the food poisoned ?
	// Did it have special (magical?) properties ?
	UpdateDir( pFood );

	// Poisoned ?
	switch ( pFood->GetType())
	{
	case IT_FRUIT:
	case IT_FOOD:
	case IT_FOOD_RAW:
	case IT_MEAT_RAW:
		if ( pFood->m_itFood.m_poison_skill )
		{
			// Spell_Effect_Poison( pItemFood->m_itFood.m_poison_skill, pItemFood->m_itFood.m_poison_skill/50, this ))
		}
		break;
	}

	pFood->ConsumeAmount( iQty );

	EatAnim( pFood->GetName(), iValue*iQty );
}

bool CChar::Use_Eat( CItem* pItemFood, int iQty )
{
	// What we can eat should depend on body type.
	// How much we can eat should depend on body size and current fullness.
	//
	// ??? monsters should be able to eat corpses / raw meat
	// IT_FOOD or IT_FOOD_RAW
	// NOTE: Some foods like apples are stackable !

	if ( ! CanMove( pItemFood ))
	{
		WriteString( "You can't move the item." );
		return false;
	}

	CCharDefPtr pCharDef = Char_GetDef();
	ASSERT(pCharDef);

	if ( pCharDef->m_MaxFood == 0 )
	{
		WriteString( "You are not capable of eating." );
		return false;
	}

	// Is this edible by me ?
	if ( ! Food_CanEat( pItemFood ))
	{
		WriteString( "You can't really eat this." );
		return( false );
	}

	if ( m_StatFood >= pCharDef->m_MaxFood )
	{
		WriteString( "You are simply too full to eat any more!" );
		return false;
	}

	Use_EatQty( pItemFood, iQty );

	LPCTSTR pMsg;
	int index = IMULDIV( m_StatFood, 6, pCharDef->m_MaxFood );

	switch ( index )
	{
	case 0:
		pMsg = "You eat the food, but are still extremely hungry.";
		break;
	case 1:
		pMsg = "After eating the food, you feel much less hungry.";
		break;
	case 2:
	case 3:
		pMsg = "You eat the food, and begin to feel more satiated.";
		break;
	case 4:
		pMsg = "You are nearly stuffed, but manage to eat the food.";
		break;
	case 5:
		pMsg = "You feel quite full after consuming the food.";
		break;
	case 6:
	default:
		pMsg = "You are stuffed!";
		break;
	}

	WriteString( pMsg );
	return( true );
}

bool CChar::Use_Drink( CItem* pItem )
{
	// IT_POTION:
	// IT_DRINK:
	// IT_PITCHER:
	// IT_WATER_WASH:
	// IT_BOOZE:

	if ( ! CanMove( pItem ))
	{
		WriteString( "You can't move the item." );
		return false;
	}

	static const SOUND_TYPE sm_DrinkSounds[] = { 0x030, 0x031 };
	Sound( sm_DrinkSounds[ Calc_GetRandVal( COUNTOF(sm_DrinkSounds)) ] );
	UpdateAnimate( ANIM_EAT );

	CItemDefPtr pItemDef = pItem->Item_GetDef();
	ITEMID_TYPE idbottle = (ITEMID_TYPE) RES_GET_INDEX( pItemDef->m_ttDrink.m_idEmpty );

	if ( pItem->IsType(IT_BOOZE))
	{
		// Beer wine and liquor. vary strength of effect. m_itBooze.m_EffectStr
		int iStrength = Calc_GetRandVal(300) + 10;

		// Create ITEMID_PITCHER if drink a pitcher.
		// GLASS or MUG or Bottle ?

		// Get you Drunk, but check to see if we already are drunk
		CItemPtr pDrunkLayer = LayerFind( LAYER_FLAG_Drunk );
		if ( pDrunkLayer == NULL )
		{
			CItemPtr pSpell = Spell_Equip_Create( SPELL_Liquor, LAYER_FLAG_Drunk, iStrength, 15*TICKS_PER_SEC, this, false );
			ASSERT(pSpell);
			pSpell->m_itSpell.m_spellcharges = 5+Calc_GetRandVal(5);	// how long to last.
		}
		else
		{
			// lengthen/strengthen the effect
			Spell_Equip_Remove( pDrunkLayer );
			pDrunkLayer->m_itSpell.m_spellcharges += 10;
			if ( pDrunkLayer->m_itSpell.m_spelllevel < 500 )
			{
				pDrunkLayer->m_itSpell.m_spelllevel += iStrength;
			}
			else
			{
				// dead drunk ?
			}
			Spell_Equip_Add( pDrunkLayer );
		}
	}

	if ( pItem->IsType(IT_POTION))
	{
		// Time limit on using potions.
		if ( LayerFind( LAYER_FLAG_PotionUsed ))
		{
			WriteString( "You can't drink another potion yet" );
			return false;
		}

		// Convey the effect of the potion.
		int iSkillQuality = pItem->m_itPotion.m_skillquality;

		OnSpellEffect( (SPELL_TYPE)RES_GET_INDEX(pItem->m_itPotion.m_Type), this, iSkillQuality, pItem );

		// Give me the marker that i've used a potion.
		Spell_Equip_Create( SPELL_NONE, LAYER_FLAG_PotionUsed, iSkillQuality, 15*TICKS_PER_SEC, this, false );
	}

	pItem->ConsumeAmount();

	// Create the empty bottle ?
	if ( idbottle != ITEMID_NOTHING )
	{
		ItemBounce( CItem::CreateScript( idbottle, this ));
	}
	return( true );
}

CCharPtr CChar::Use_Figurine( CItem* pItem, int iPaces )
{
	// NOTE: The figurine is NOT destroyed here.

	ASSERT(pItem);
	DEBUG_CHECK( pItem->IsType(IT_FIGURINE) || pItem->IsType(IT_EQ_HORSE));

	if ( pItem->m_uidLink.IsValidObjUID() &&
		pItem->m_uidLink.IsChar() &&
		pItem->m_uidLink != GetUID() &&
		! IsGM())
	{
		WriteString( "This figurine is not yours" );
		return( NULL );
	}

	CCharPtr pPet = g_World.CharFind(pItem->m_itFigurine.m_uidChar);
	if ( pPet != NULL && pPet->IsDisconnected())
	{
		// Pull the creature out of idle space.
		pPet->StatFlag_Clear( STATF_Ridden );
	}
	else
	{
		if ( pPet != NULL )
		{
			DEBUG_ERR(("Mount Bad UID to Pet 0%x" LOG_CR, pPet->GetUID()));
		}

		CREID_TYPE id = pItem->m_itFigurine.m_ID;
		if ( ! id )
		{
			// For figurines. convert to a creature.
			// Convert Icon ITEMID_TYPE back to CREID_TYPE
			// IT_EQ_HORSE
			// IT_FIGURINE

			CItemDefPtr pItemDef = pItem->Item_GetDef();
			ASSERT( pItemDef != NULL );
			id = (CREID_TYPE) pItemDef->m_ttFigurine.m_charid.GetResIndex();
			if ( ! CCharDef::IsValidDispID(id))
			{
				DEBUG_ERR(( "FIGURINE id 0%x, no creature" LOG_CR, pItem->GetDispID()));
				return NULL;
			}
		}

		// I guess we need to create a new one ? (support old versions.)
		pPet = CreateNPC( id );
		ASSERT(pPet);
		pPet->SetName( pItem->GetName());
		if ( pItem->GetHue())
		{
			pPet->m_prev_Hue = pItem->GetHue();
			pPet->SetHue( pItem->GetHue());
		}
	}

	pItem->m_itFigurine.m_uidChar.InitUID();

	if ( ! iPaces )
	{
		pPet->m_dirFace = m_dirFace;	// getting off ridden horse.
	}
	if ( ! pItem->IsAttr( ATTR_CURSED|ATTR_CURSED2 ))
	{
		pPet->NPC_PetSetOwner( this );
	}
	pPet->MoveNear( pItem->GetTopLevelObj()->GetTopPoint(), iPaces );
	pPet->Update();
	pPet->Skill_Start( SKILL_NONE );	// was NPCACT_RIDDEN

	if ( pItem->IsAttr( ATTR_MAGIC ))
	{
		pPet->UpdateAnimate( ANIM_CAST_DIR );
		pPet->SoundChar( CRESND_GETHIT );
	}
	else
	{
		pPet->SoundChar( CRESND_RAND1 );	// Horse winny
	}

	return pPet;
}

bool CChar::Use_Key( CItem* pKey, CItem* pItemTarg )
{
	ASSERT(pKey);
	ASSERT(pKey->IsType(IT_KEY));
	if ( pItemTarg == NULL )
	{
		WriteString( "Use a key on a locked item or another key to copy." );
		return false;
	}

	if ( pKey != pItemTarg && pItemTarg->IsType(IT_KEY) )
	{
		// We are trying to copy a key ?
		if ( ! CanUse( pItemTarg, true ))
		{
			WriteString( "You can't reach it." );
			return false;
		}

		if ( ! pKey->m_itKey.m_lockUID && ! pItemTarg->m_itKey.m_lockUID )
		{
			WriteString( "Both keys are blank." );
			return false;
		}
		if ( pItemTarg->m_itKey.m_lockUID && pKey->m_itKey.m_lockUID )
		{
			WriteString( "To copy keys get a blank key" );
			return false;
		}

		// Need tinkering tools ???

		if ( ! Skill_UseQuick( SKILL_TINKERING, 30+Calc_GetRandVal(40)))
		{
			WriteString( "You fail to copy the key." );
			return( false );
		}
		if ( pItemTarg->m_itKey.m_lockUID )
		{
			pKey->m_itKey.m_lockUID = pItemTarg->m_itKey.m_lockUID;
		}
		else
		{
			pItemTarg->m_itKey.m_lockUID = pKey->m_itKey.m_lockUID;
		}
		return( true );
	}

	if ( ! pKey->m_itKey.m_lockUID )
	{
		WriteString( "The key is a blank." );
		return false;
	}
	if ( pKey == pItemTarg )	// Rename the key.
	{
		// We may rename the key.
		if ( IsClient())
		{
			GetClient()->addPromptConsole( CLIMODE_PROMPT_NAME_KEY, "What would you like to name the key?" );
		}
		return false;
	}

	if ( ! CanUse( pItemTarg, false ))
	{
		WriteString( "You can't reach it." );
		return false;
	}

	if ( m_pArea->GetUIDIndex() == pKey->m_itKey.m_lockUID )
	{
		if ( Use_MultiLockDown( pItemTarg ))
		{
			return( true );
		}
	}

	if ( ! pItemTarg->m_itContainer.m_lockUID )	// or m_itContainer.m_lockUID
	{
		WriteString( "That does not have a lock.");
		return false;
	}
	if ( ! pKey->IsKeyLockFit( pItemTarg->m_itContainer.m_lockUID )) // or m_itKey
	{
		WriteString( "The key does not fit into that lock.");
		return false;
	}

	return( Use_KeyChange( pItemTarg ));
}

bool CChar::Use_KeyChange( CItem* pItemTarg )
{
	// Lock or unlock the item.
	switch ( pItemTarg->GetType() )
	{
	case IT_SIGN_GUMP:
		// We may rename the sign.
		if ( IsClient())
		{
			GetClient()->m_Targ.m_UID = pItemTarg->GetUID();
			GetClient()->addPromptConsole( CLIMODE_PROMPT_NAME_SIGN, "What should the sign say?" );
		}
		return true;
	case IT_CONTAINER:
		pItemTarg->SetType(IT_CONTAINER_LOCKED);
		WriteString( "You lock the container.");
		break;
	case IT_CONTAINER_LOCKED:
		pItemTarg->SetType(IT_CONTAINER);
		WriteString( "You unlock the container.");
		break;
	case IT_SHIP_HOLD:
		pItemTarg->SetType(IT_SHIP_HOLD_LOCK);
		WriteString( "You lock the hold.");
		break;
	case IT_SHIP_HOLD_LOCK:
		pItemTarg->SetType(IT_SHIP_HOLD);
		WriteString( "You unlock the hold.");
		break;
	case IT_DOOR_OPEN:
	case IT_DOOR:
		pItemTarg->SetType(IT_DOOR_LOCKED);
		WriteString( "You lock the door.");
		break;
	case IT_DOOR_LOCKED:
		pItemTarg->SetType(IT_DOOR);
		WriteString( "You unlock the door.");
		break;
	case IT_SHIP_TILLER:
		if ( IsClient())
		{
			GetClient()->m_Targ.m_UID = pItemTarg->GetUID();
			GetClient()->addPromptConsole( CLIMODE_PROMPT_NAME_SHIP, "What would you like to name the ship?" );
		}
		return true;
	case IT_SHIP_PLANK:
		pItemTarg->Ship_Plank( false );	// just close it.
		// Then fall thru and lock it.
	case IT_SHIP_SIDE:
		pItemTarg->SetType(IT_SHIP_SIDE_LOCKED);
		WriteString( "You lock the ship.");
		break;
	case IT_SHIP_SIDE_LOCKED:
		pItemTarg->SetType(IT_SHIP_SIDE);
		WriteString( "You unlock the ship.");
		break;
	default:
		WriteString( "That does not have a lock.");
		return false;
	}

	pItemTarg->Sound( 0x049 );
	return true;
}

bool CChar::Use_Seed( CItem* pSeed, CPointMap* pPoint )
{
	// Use the seed at the current point on the ground or some new point that i can touch.
	// IT_SEED from IT_FRUIT

	ASSERT(pSeed);
	CPointMap pt;
	if ( pPoint )
	{
		pt = *pPoint;
	}
	else
	{
		// NPC is using this .
		if ( ! pSeed->IsTopLevel())
			pt = GetTopPoint();
		else
			pt = pSeed->GetTopPoint();
	}

	if ( ! CanTouch(pt))
	{
		WriteString( "You can't reach this" );
		return( false );
	}

	// is there soil here ? IT_DIRT
	if ( ! IsGM() && ! g_World.IsItemTypeNear( pt, IT_DIRT ))
	{
		WriteString( "Try planting in soil" );
		return( false );
	}

	CItemDefPtr pItemDef = pSeed->Item_GetDef();
	ITEMID_TYPE idReset = (ITEMID_TYPE) RES_GET_INDEX( pItemDef->m_ttFruit.m_idReset );
	if ( idReset == 0 )
	{
		WriteString( "These seeds are no good" );
		return( false );
	}

	// Already a plant here ?
	CWorldSearch AreaItems( pt );
	for(;;)
	{
		CItemPtr pItem = AreaItems.GetNextItem();
		if (pItem == NULL)
			break;
		if ( pItem->IsType(IT_TREE) ||
			pItem->IsType(IT_FOLIAGE))
		{
			// already a tree here .
			WriteString( "Already a tree here" );
			return( false );
		}
		if ( pItem->IsType(IT_CROPS))
		{
			// already a plant here !
			pItem->DeleteThis();
		}
	}

	// plant it and consume the seed.

	CItemPtr pPlant = CItem::CreateScript( idReset, this );
	ASSERT( pPlant );

	pPlant->MoveTo(pt);
	if ( pPlant->IsType(IT_CROPS) || pPlant->IsType(IT_FOLIAGE))
	{
		pPlant->m_itCrop.m_ReapFruitID = pSeed->GetID();
		pPlant->Plant_CropReset();
	}
	else
	{
		pPlant->SetDecayTime(10*g_Cfg.m_iDecay_Item );
	}

	pSeed->ConsumeAmount();
	return( true );
}

bool CChar::Use_BedRoll( CItem* pItem )
{
	// IT_BEDROLL

	ASSERT(pItem);
	switch ( pItem->GetDispID())
	{
	case ITEMID_BEDROLL_C:
		if ( ! pItem->IsTopLevel())
		{
		putonground:
			WriteString( "Put this on the ground to open it." );
			return( true );
		}
		pItem->SetID( Calc_GetRandVal(2) ? ITEMID_BEDROLL_O_EW : ITEMID_BEDROLL_O_NS );
		pItem->Update();
		return true;
	case ITEMID_BEDROLL_C_NS:
		if ( ! pItem->IsTopLevel())
			goto putonground;
		pItem->SetID( ITEMID_BEDROLL_O_NS );
		pItem->Update();
		return true;
	case ITEMID_BEDROLL_C_EW:
		if ( ! pItem->IsTopLevel())
			goto putonground;
		pItem->SetID( ITEMID_BEDROLL_O_EW );
		pItem->Update();
		return true;
	case ITEMID_BEDROLL_O_EW:
	case ITEMID_BEDROLL_O_NS:
		pItem->SetID( ITEMID_BEDROLL_C );
		pItem->Update();
		return true;
	}

	return false;
}

bool CChar::Use_Item( CItem* pItem, bool fLink )
{
	// An NPC could use these as well.
	// don't check distance here.
	// Could be a switch or something.
	// ARGS:
	//   fLink = this is the result of a linked action.
	// RETURN:
	//   true = it activated.

	if ( pItem == NULL )
		return( false );

	bool fAction = true;

	switch ( pItem->GetType() )
	{
	case IT_ITEM_STONE:
		// Give them this item
		if ( pItem->m_itItemStone.m_wAmount == USHRT_MAX )
		{
			Printf( "It is dead." );
			return true;
		}
		if ( pItem->m_itItemStone.m_wRegenTime )
		{
			if ( pItem->IsTimerSet())
			{
				Printf( "It will regenerate in %d seconds", pItem->GetTimerDiff() / TICKS_PER_SEC );
				return true;
			}
			pItem->SetTimeout( pItem->m_itItemStone.m_wRegenTime * TICKS_PER_SEC );
		}
		ItemBounce( CItem::CreateTemplate( pItem->m_itItemStone.m_ItemID, GetPackSafe(), this ));
		if ( pItem->m_itItemStone.m_wAmount != 0 )
		{
			pItem->m_itItemStone.m_wAmount--;
			if ( pItem->m_itItemStone.m_wAmount == 0 )
			{
				pItem->m_itItemStone.m_wAmount = USHRT_MAX;
			}
		}
		return true;

	case IT_SEED:
		return Use_Seed( pItem, NULL );

	case IT_BEDROLL:
		return Use_BedRoll(pItem);

	case IT_BELLOWS:
		// This is supposed to make the nearby fire pit hotter.
		pItem->Sound( 0x02b );
		pItem->SetAnim( (ITEMID_TYPE)( pItem->GetID() + 1 ), 2*TICKS_PER_SEC );
		return true;

	case IT_KINDLING:
		return Use_Kindling( pItem );

	case IT_SPINWHEEL:
		// Just make them spin.
		if ( !fLink )
		{
			pItem->SetAnim( (ITEMID_TYPE)( pItem->GetID() + 1 ), 2*TICKS_PER_SEC );
			WriteString("Use wool or cotton on spinning wheels");
			return true;
		}
		else
			return false;

	case IT_TRAIN_DUMMY:	// Train dummy.
		if ( !fLink )
			return Use_Train_Dummy(pItem, true );
		else
			return false;
	case IT_TRAIN_PICKPOCKET:
		if ( !fLink )
			return Use_Train_PickPocketDip(pItem, true);
		else
			return false;
	case IT_ARCHERY_BUTTE:	// Archery Butte
		if ( !fLink )
			return Use_Train_ArcheryButte(pItem, true);
		else
			return false;

	case IT_LOOM:
		// pItem->SetAnim( (ITEMID_TYPE)( pItem->GetID() + 1 ), 2*TICKS_PER_SEC );
		if ( !fLink )
		{
			WriteString("Use thread or yarn on looms");
			return true;
		}
		else
			return false;

	case IT_BEE_HIVE:
		// Get honey from it.
		if ( !fLink )
		{
			ITEMID_TYPE id = ITEMID_NOTHING;
			if ( ! pItem->m_itBeeHive.m_honeycount )
			{
				WriteString( "The hive appears to be unproductive" );
			}
			else switch ( Calc_GetRandVal(3))
			{
			case 1:
				id = ITEMID_JAR_HONEY;
				break;
			case 2:
				id = ITEMID_BEE_WAX;
				break;
			}
			if ( id )
			{
				ItemBounce( CItem::CreateScript( id, this ));
				pItem->m_itBeeHive.m_honeycount --;
			}
			else
			{
				// ouch got stung. Poison bee sting ?
				WriteString( "Ouch!  Bee sting!" );
				OnTakeDamage( Calc_GetRandVal(5), this, DAMAGE_POISON|DAMAGE_GENERAL );
			}
			pItem->SetTimeout( 15*60*TICKS_PER_SEC );
			return true;
		}
		else
			return false;

	case IT_MUSICAL:
		if ( ! Skill_Wait(SKILL_MUSICIANSHIP))
		{
			m_Act.m_Targ = pItem->GetUID();
			Skill_Start( SKILL_MUSICIANSHIP );
		}
		break;

	case IT_CROPS:
	case IT_FOLIAGE:
		// Pick cotton/hay/etc...
		fAction = pItem->Plant_Use(this);
		break;

	case IT_FIGURINE:
		// Create the creature here.
		fAction = ( Use_Figurine( pItem, 0 ) != NULL );
		if ( fAction )
		{
			pItem->DeleteThis();
		}
		break;

	case IT_TRAP:
	case IT_TRAP_ACTIVE:
		// Explode or active the trap. (plus any linked traps.)
		if ( CanTouch( pItem->GetTopLevelObj()->GetTopPoint()))
		{
			OnTakeDamage( pItem->Use_Trap(), NULL, DAMAGE_HIT_BLUNT | DAMAGE_GENERAL );
		}
		else
		{
			pItem->Use_Trap();
		}
		break;

	case IT_SWITCH:
		// Switches can be linked to gates and doors and such.
		// Flip the switch graphic.
		pItem->SetSwitchState();
		break;

	case IT_PORT_LOCKED:	// Can only be trigered.
		if ( ! fLink && ! IsGM())
		{
			WriteString( "You can't move the gate." );
			return( true );
		}

	case IT_PORTCULIS:
		// Open a metal gate via a trigger of some sort.
		pItem->Use_Portculis();
		break;

	case IT_DOOR_LOCKED:
		if ( ! ContentFindKeyFor( pItem ))
		{
			WriteString( "This door is locked.");
			if ( ! pItem->IsTopLevel())
				return( false );
			if ( pItem->IsAttr(ATTR_MAGIC))
			{
				// Show it's magic face.
				ITEMID_TYPE id = ( GetDispID() & DOOR_NORTHSOUTH ) ? ITEMID_DOOR_MAGIC_SI_NS : ITEMID_DOOR_MAGIC_SI_EW;
				CItemPtr pFace = CItem::CreateBase( id );
				ASSERT(pFace);
				pFace->MoveToDecay( pItem->GetTopPoint(), 4*TICKS_PER_SEC );
			}
			if ( ! IsGM())
				return true;
		}

	case IT_DOOR_OPEN:
	case IT_DOOR:
		{
			bool fOpen = pItem->Use_Door( fLink );
			if ( fLink || ! fOpen ) // Don't link if we are just closing the door.
				return true;
		}
		break;

	case IT_SHIP_PLANK:
		// If on the ship close it. if not teleport to it.
		if ( m_pArea->IsFlag( REGION_FLAG_SHIP ) &&
			m_pArea->GetUIDIndex() == pItem->m_uidLink )
		{
			// Close it.
			// If on the ship close it. if not teleport to it.
			return( pItem->Ship_Plank( false ));
		}
		else if ( pItem->IsTopLevel())
		{
			// Move to it.
			CPointMap pntTarg = pItem->GetTopPoint();
			pntTarg.m_z ++;
			Spell_Effect_Teleport( pntTarg, true, false, ITEMID_NOTHING );
		}
		return true;

	case IT_SHIP_SIDE_LOCKED:
		if ( ! ContentFindKeyFor( pItem ))
		{
			WriteString( "That is locked" );
			return true;
		}

	case IT_SHIP_SIDE:
		// Open it if we have the key or are already on the ship
		pItem->Ship_Plank( true );
		break;

	case IT_GRAIN:
	case IT_GRASS:
	case IT_GARBAGE:
	case IT_FRUIT:
	case IT_FOOD:
	case IT_FOOD_RAW:
	case IT_MEAT_RAW:
		if ( !fLink )
			return Use_Eat( pItem );
		else
			return false;

	case IT_POTION:
	case IT_DRINK:
	case IT_PITCHER:
	case IT_WATER_WASH:
	case IT_BOOZE:
		if ( !fLink )
			return Use_Drink( pItem );
		else
			return false;

	case IT_LIGHT_OUT: // Can the light be lit ?
	case IT_LIGHT_LIT: // Can the light be doused ?
		fAction = pItem->Use_Light();
		break;

	case IT_CLOTHING:
	case IT_ARMOR:
	case IT_ARMOR_LEATHER:
	case IT_SHIELD:
	case IT_WEAPON_MACE_CROOK:
	case IT_WEAPON_MACE_PICK:
	case IT_WEAPON_MACE_SMITH:
	case IT_WEAPON_MACE_SHARP:
	case IT_WEAPON_SWORD:
	case IT_WEAPON_FENCE:
	case IT_WEAPON_BOW:
	case IT_WEAPON_AXE:
	case IT_WEAPON_XBOW:
	case IT_WEAPON_MACE_STAFF:
	case IT_JEWELRY:
	case IT_SHOVEL:
		if ( !fLink )
			return( ItemEquip( pItem ) == NO_ERROR );
		else
			return false;

	case IT_WEB:
		// try to get out of web.
		if ( !fLink )
			return( Use_Item_Web( pItem ));
		else
			return false;

	case IT_SPY_GLASS:
		if ( !fLink )
		{
			{
				// Spyglass will now tell you the moon phases...
				static LPCTSTR const sm_sPhases[8] =
				{
					"new moon",
					"waxing crescent",
					"first quarter",
					"waxing gibbous",
					"full moon",
					"waning gibbous",
					"third quarter",
					"waning crescent"
				};
				Printf( "Trammel is in the %s phase.", sm_sPhases[ g_World.Moon_GetPhase(MOON_TRAMMEL) ]);
				Printf( "Felucca is in the %s phase.", sm_sPhases[ g_World.Moon_GetPhase(MOON_FELUCCA) ]);
			}
			if ( m_pArea.IsValidRefObj() && m_pArea->IsFlag( REGION_FLAG_SHIP ))
			{
				ObjMessage( pItem->Use_SpyGlass(this), this );
			}
			return true;
		}
		else
			return false;

	case IT_SEXTANT:
		if ( !fLink )
		{
			CPointMap pt = GetTopPoint();
			if ( pt.m_x > pt.GetMulMap()->m_iSizeXWrap ) // in dungeons and t2a lands
			{
				ObjMessage( "I cannot tell where I'm at.", this );
			}
			else
			{
				CGString sTmp;
				sTmp.Format( "I am in %s, %s", (LPCTSTR) m_pArea->GetName(), (LPCTSTR) pItem->Use_Sextant(pt));
				ObjMessage( sTmp, this );
			}
			return true;
		}
		else
			return false;

	default:
		fAction = false;
	}

	// Try to follow the link as well. (if it has one)
	CItemPtr pLinkItem = g_World.ItemFind(pItem->m_uidLink);
	if ( pLinkItem != NULL && pLinkItem != pItem )
	{
		static CItemPtr sm_pItemFirst = NULL;	// watch out for loops.
		static int sm_iCount = 0;
		if ( ! fLink )
		{
			sm_pItemFirst = pItem;
			sm_iCount = 0;
		}
		else
		{
			if ( sm_pItemFirst == pItem )
				return true;	// kill the loop.
			if ( ++sm_iCount > 64 )
			{
				return( true );
			}
		}
		fAction |= Use_Item( pLinkItem, true );
	}

	return( fAction );
}

bool CChar::Use_Obj( CObjBase* pObj, bool fTestTouch )
{
	if ( pObj == NULL )
		return( false );
	if ( IsClient())
	{
		return GetClient()->Event_DoubleClick( pObj->GetUID(), false, fTestTouch );
	}
	else
	{
		return Use_Item( PTR_CAST(CItem,pObj), fTestTouch );
	}
}

HRESULT CChar::ItemEquipArmor( bool fForce )
{
	// equip ourselves as best as possible.

	CCharDefPtr pCharDef = Char_GetDef();
	ASSERT(pCharDef);
	if ( ! pCharDef->Can( CAN_C_EQUIP ))
		return( HRES_INVALID_HANDLE );

	int iBestScore[ LAYER_HORSE ];
	memset( iBestScore, 0, sizeof(iBestScore));
	CItemPtr pBestArmor[ LAYER_HORSE ];
	memset( pBestArmor, 0, sizeof(pBestArmor));

	if ( ! fForce )
	{
		// Block those layers that are already used.
		for ( int i=0; i<COUNTOF(iBestScore); i++ )
		{
			pBestArmor[i] = LayerFind((LAYER_TYPE)i);
			if ( pBestArmor[i] != NULL )
			{
				iBestScore[i] = INT_MAX;
			}
		}
	}

	CItemContainerPtr pPack = GetPack();
	ASSERT(pPack);
	CItemPtr pItem = pPack->GetHead();
	for ( ; pItem!=NULL; pItem = pItem->GetNext())
	{
		int iScore = pItem->Armor_GetDefense(true);
		if ( ! iScore )	// might not be armor.
			continue;

		// Can i even equip this ?
		LAYER_TYPE layer = CanEquipLayer( pItem, LAYER_QTY, NULL, true );
		if ( layer == LAYER_NONE )
			continue;

		if ( iScore > iBestScore[layer] )
		{
			iBestScore[layer] = iScore;
			pBestArmor[layer] = pItem;
		}
	}

	// now equip all the stuff we found.
	for ( int i=0; i<COUNTOF(iBestScore); i++ )
	{
		if ( pBestArmor[i] == NULL )
			continue;
		ItemEquip( pBestArmor[i], this );
	}

	return( NO_ERROR );
}

HRESULT CChar::ItemEquipWeapon( bool fForce )
{
	// find my best weapon and equip it.
	if ( ! fForce && m_uidWeapon.IsValidObjUID())	// we are ok.
		return( NO_ERROR );

	CCharDefPtr pCharDef = Char_GetDef();
	ASSERT(pCharDef);
	if ( ! pCharDef->Can( CAN_C_USEHANDS ))
		return( HRES_INVALID_HANDLE );

	// Go through all my weapons and come up with a score for it's usefulness.

	CItemPtr pBestWeapon;
	int iWeaponScoreMax = NPC_GetWeaponUseScore( NULL );	// just wrestling.

	CItemContainerPtr pPack = GetPack();
	ASSERT(pPack);
	CItemPtr pItem = pPack->GetHead();
	for ( ; pItem!=NULL; pItem = pItem->GetNext())
	{
		int iWeaponScore = NPC_GetWeaponUseScore( pItem );
		if ( iWeaponScore > iWeaponScoreMax )
		{
			iWeaponScoreMax = iWeaponScore;
			pBestWeapon = pItem;
		}
	}

	if ( pBestWeapon )
	{
		return ItemEquip(pBestWeapon);
	}

	return( NO_ERROR );
}


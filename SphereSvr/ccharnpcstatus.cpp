//
// CCharNPCStatus.cpp
// Copyright 1996 - 2001 Menace Software (www.menasoft.com)
//
// Test things to judge what an NPC might be thinking. (want to do)
// But take no actions here.
//

#include "stdafx.h"	// predef header.

CREID_TYPE CChar::NPC_GetAllyGroupType(CREID_TYPE idTest)	// static
{
	switch ( idTest )
	{
	case CREID_MAN:
	case CREID_WOMAN:
	case CREID_GHOSTMAN:
	case CREID_GHOSTWOMAN:
		return( CREID_MAN );
	case CREID_ETTIN:
	case CREID_ETTIN_AXE:
		return( CREID_ETTIN );
	case CREID_ORC_LORD:
	case CREID_ORC:
	case CREID_ORC_CLUB:
		return( CREID_ORC );
	case CREID_DAEMON:
	case CREID_DAEMON_SWORD:
		return( CREID_DAEMON );
	case CREID_DRAGON_GREY:
	case CREID_DRAGON_RED:
	case CREID_DRAKE_GREY:
	case CREID_DRAKE_RED:
		return( CREID_DRAGON_GREY );
	case CREID_LIZMAN:
	case CREID_LIZMAN_SPEAR:
	case CREID_LIZMAN_MACE:
		return( CREID_LIZMAN );
	case CREID_RATMAN:
	case CREID_RATMAN_CLUB:
	case CREID_RATMAN_SWORD:
		return( CREID_RATMAN );
	case CREID_SKELETON:
	case CREID_SKEL_AXE:
	case CREID_SKEL_SW_SH:
		return( CREID_SKELETON );
	case CREID_TROLL_SWORD:
	case CREID_TROLL:
	case CREID_TROLL_MACE:
		return( CREID_TROLL );
	case CREID_Tera_Warrior:
	case CREID_Tera_Drone:
	case CREID_Tera_Matriarch:
		return( CREID_Tera_Drone );
	case CREID_Ophid_Mage:
	case CREID_Ophid_Warrior:
	case CREID_Ophid_Queen:
		return( CREID_Ophid_Warrior );
	case CREID_HORSE1:
	case CREID_HORSE4:
	case CREID_HORSE2:
	case CREID_HORSE3:
	case CREID_HORSE_PACK:
		return( CREID_HORSE1 );
	case CREID_BrownBear:
	case CREID_GrizzlyBear:
	case CREID_PolarBear:
		return( CREID_BrownBear );
	case CREID_Cow_BW:
	case CREID_Cow2:
	case CREID_Bull_Brown:
	case CREID_Bull2:
		return( CREID_Bull_Brown );
	case CREID_Ostard_Desert:
	case CREID_Ostard_Frenz:
	case CREID_Ostard_Forest:
		return( CREID_Ostard_Forest );
	case CREID_Sheep:
	case CREID_Sheep_Sheered:
		return( CREID_Sheep );
	case CREID_Hart:
	case CREID_Deer:
		return( CREID_Deer );
	case CREID_Pig:
	case CREID_Boar:
		return( CREID_Pig );
	case CREID_Llama:
	case CREID_LLAMA_PACK:
		return( CREID_Llama );
	}
	return( idTest );
}

MOTIVE_LEVEL CChar::NPC_GetVendorMarkup( const CChar* pChar ) const
{
	// This vendor marks stuff up/down this percentage.
	// Base this on KARMA. Random is calculated at Restock time
	// When vendor sells to players this is the markup value.
	// RETURN:
	//  0-100 percent

	ASSERT(pChar);
	if ( IsStatFlag( STATF_Pet ))	// Not on a hired vendor.
		return( 0 );

	MOTIVE_LEVEL iHostility = NPC_GetHostilityLevelToward( pChar );
	if ( iHostility < 0 )
		iHostility = 0;
	iHostility += 15;
	if ( iHostility > 100 )
		iHostility = 100;

	CVarDefPtr pVar = m_TagDefs.FindKeyPtr("VENDORMARKUP");
	if ( pVar )
	{
		iHostility += pVar->GetInt();
	}

	return( iHostility );
}

int CChar::NPC_OnHearName( LPCTSTR pszText ) const
{
	// Did I just hear my name in this text ?
	// should be able to deal with "hi Dennis" in the future.
	// RETURN:
	//  index to skip past the name.

	LPCTSTR pszName = (LPCTSTR) GetName();

	int i = Str_FindWord( pszText, pszName );
	if ( i )
		return( i );

	if ( m_pNPC.IsValidNewObj() )
	{
		// My title ?
		if ( m_pNPC->m_Brain == NPCBRAIN_GUARD )
		{
			if ( ! _strnicmp( pszText, "GUARD ", 6 ))
				return 6;
		}
		else if ( NPC_IsVendor())
		{
			if ( ! _strnicmp( pszText, "VENDOR ", 7 ))
				return 7;
		}
	}

	CCharDefPtr pCharDef = Char_GetDef();
	ASSERT(pCharDef);

	// Named the chars type ? (must come first !)
	pszName = pCharDef->GetTradeName();
	for ( i=0; pszText[i] != '\0'; i++ )
	{
		if ( pszName[i] == '\0' )
		{
			// found name.
			while ( ISWHITESPACE( pszText[i] ))
				i ++;
			return( i );	// Char name found
		}
		if ( toupper( pszName[i] ) != toupper( pszText[i] ))	// not the name.
			break;
	}

	return( 0 );
}

bool CChar::NPC_CanSpeak() const
{
	// This just means that i understand speech !
	if ( ! m_pNPC )	// all players can speak.
		return( true );
	if ( m_pNPC->m_Speech.GetSize())	// this just means they can hear ?!
		return( true );
	// m_SpeechHue ?
	CCharDefPtr pCharDef = Char_GetDef();
	ASSERT(pCharDef);
	return( pCharDef->m_Speech.GetSize() );
}

bool CChar::NPC_FightMayCast() const
{
	// This NPC could cast spells if they wanted to ?
	// check mana and anti-magic
#define NPC_MAGERY_MIN_CAST 300	// Min magery to cast.

	int iSkillVal = Skill_GetBase(SKILL_MAGERY);
	if ( iSkillVal < NPC_MAGERY_MIN_CAST )
		return( false );
	ASSERT(m_pArea.IsValidRefObj());
	if ( m_pArea->IsFlag( REGION_ANTIMAGIC_DAMAGE | REGION_FLAG_SAFE ))	// can't cast here.
		return false;
	if ( m_StatMana < 5 )
		return( false );

	return( true );
}

bool CChar::NPC_IsSpawnedBy( const CItem* pItem ) const
{
	if ( ! IsStatFlag( STATF_Spawned ))	// shortcut - i'm not a spawned.
		return( false );
	if ( ! m_pNPC.IsValidNewObj())
		return( false );
	return( Memory_FindObjTypes( pItem, MEMORY_ISPAWNED ) != NULL );
}

bool CChar::NPC_IsOwnedBy( const CChar* pChar, bool fAllowGM ) const
{
	// Is pChar my master ?
	// BESERK will not listen to commands tho.
	// fAllowGM = consider GM's to be owners of all NPC's

	if ( pChar == NULL )
		return( false );
	if ( this == pChar )
		return( true );

	if ( fAllowGM && pChar->IsGM())
		return( pChar->GetPrivLevel() > GetPrivLevel());

	if ( ! IsStatFlag( STATF_Pet ) || m_pPlayer )	// shortcut - i'm not a pet.
		return( false );
	if ( ! m_pNPC )
		return( false );
	if ( m_pNPC->m_Brain == NPCBRAIN_BESERK )	// i cannot be commanded.
		return( false );

	return( Memory_FindObjTypes( pChar, MEMORY_IPET|MEMORY_FRIEND ) != NULL );
}

CCharPtr CChar::NPC_PetGetOwner() const
{
	// Assume i am a pet. Get my first (primary) owner. not just friends.
	// used for blame .

	if ( ! IsStatFlag( STATF_Pet ))
		return( NULL );

	CItemMemoryPtr pMemory = Memory_FindTypes( MEMORY_IPET );
	if ( pMemory == NULL )
	{
		// Mislink ?
		// StatFlag_Clear( STATF_Pet );
		DEBUG_CHECK(0);
		return( NULL );
	}
	CCharPtr pCharOwn = g_World.CharFind(pMemory->m_uidLink);
	if ( pCharOwn==this )
	{
		// This should never happen !
		DEBUG_CHECK(pCharOwn!=this);
	}
	return( pCharOwn);
}

int CChar::NPC_GetTrainMax( const CChar* pStudent, SKILL_TYPE Skill ) const
{
	// What is the max I can train to ?
	int iMax = IMULDIV( g_Cfg.m_iTrainSkillPercent, Skill_GetBase(Skill), 100 );
	if ( iMax > g_Cfg.m_iTrainSkillMax )
		return( g_Cfg.m_iTrainSkillMax );

	// Is this more that the student can take ?
	ASSERT(pStudent);
	int iMaxStudent = pStudent->Skill_GetMax( Skill );
	if ( iMax > iMaxStudent )
		return( iMaxStudent );

	return( iMax );
}

bool CChar::NPC_CheckWalkHere( const CPointMapBase& pt, const CRegionBasic* pArea, bool fIsCovered ) const
{
	// Does the NPC want to walk here ? step on this item ?

	if ( ! pt.IsCharValid())
		return false;

	ASSERT( m_pNPC.IsValidNewObj() );	// not an NPC

	if ( m_pArea.IsValidRefObj()) // most decisions are based on area.
	{
		if ( m_pNPC->m_Brain == NPCBRAIN_GUARD && ! IsStatFlag( STATF_War ))
		{
			// Guards will want to stay in guard range.
			if ( m_pArea->IsFlagGuarded() && ! pArea->IsFlagGuarded())
			{
				return( false );
			}
		}

		if ( m_pNPC->m_Brain == NPCBRAIN_UNDEAD )
		{
			// always avoid the light.
			bool fSafeThere = ( pt.GetSector()->IsDark() ||
				fIsCovered ||
				pArea->IsFlag(REGION_FLAG_UNDERGROUND));
			if ( ! fSafeThere )
			{
				// But was it safe here ?
				if ( GetTopPoint().GetSector()->IsDark() || IsStatFlag( STATF_InDoors ))
					return( false );
			}
		}

		if ( Noto_IsCriminal() && m_StatInt > 20 )	// I'm evil
		{
			if ( ! m_pArea->IsFlagGuarded() && pArea->IsFlagGuarded())		// too smart for this.
			{
				return( false );
			}
		}
	}

	CCharDefPtr pCharDef = Char_GetDef();
	ASSERT(pCharDef);

	// Is there a nasty object here that will hurt us ?
	CWorldSearch AreaItems( pt );
	for(;;)
	{
		CItemPtr pItem = AreaItems.GetNextItem();
		if ( pItem == NULL )
			break;

		int zdiff = pItem->GetTopZ() - pt.m_z;
		if ( ABS(zdiff) > 3 )
			continue;

		int iIntToAvoid = 10;	// how intelligent do i have to be to avoid this.
		switch ( pItem->GetType() )
		{
		case IT_SHRINE:
		case IT_DREAM_GATE:
		case IT_ADVANCE_GATE:
			// always avoid.
			return( false );

		case IT_WEB:
			if ( GetDispID() == CREID_GIANT_SPIDER )
				continue;
			iIntToAvoid = 80;
			goto try_avoid;
		case IT_FIRE: // fire object hurts us ?
			if ( pCharDef->Can(CAN_C_FIRE_IMMUNE))	// i like fire.
				continue;
			iIntToAvoid = 20;	// most creatures recognize fire as bad.
			goto try_avoid;
		case IT_SPELL:
			iIntToAvoid = 150;
			goto try_avoid;
		case IT_TRAP:
			iIntToAvoid = 150;
			goto try_avoid;
		case IT_TRAP_ACTIVE:
			iIntToAvoid = 50;
			goto try_avoid;
		case IT_MOONGATE:
		case IT_TELEPAD:
		try_avoid:
			if ( Calc_GetRandVal( m_StatInt) > Calc_GetRandVal( iIntToAvoid ))
				return( false );
			break;
		}
	}

	return( true );
}

CItemVendablePtr CChar::NPC_FindVendableItem( CItemVendable* pVendItem, CItemContainer* pContBuy, CItemContainer* pContStock ) // static
{
	// Does the NPC want to buy this item
	if ( pVendItem == NULL )
		return( NULL );

	if ( ! pVendItem->IsValidSaleItem( false ))
		return( NULL );

	ASSERT(pContBuy);
	CItemPtr pItemTest = pContBuy->ContentFind( CSphereUID(RES_ItemDef, pVendItem->GetID()));
	if ( pItemTest == NULL )
	{
		ASSERT(pContStock);
		pItemTest = pContStock->ContentFind( CSphereUID(RES_ItemDef, pVendItem->GetID()));
		if ( pItemTest == NULL )
			return( NULL );
	}

	CItemVendablePtr pItemSell = REF_CAST(CItemVendable,pItemTest);
	if ( pItemSell == NULL )
	{
		// This is odd. The vendor should not have had this Item !
		return( NULL );
	}

	if ( pVendItem->GetType() != pItemSell->GetType())	// sanity check.
		return( NULL );

	return( pItemSell );
}

////////////////////////////////////////////////////////////////////
// This stuff is still questionable.

MOTIVE_LEVEL CChar::NPC_WantThisItem( CItem* pItem )
{
	//  This should be the ULTIMATE place to check if the NPC wants this in any way.
	//  May not want to use it but rather just put it in my pack.
	//
	// NOTE:
	//  Don't check if i can see this or i can reach it.
	//  Don't check if i can carry this ?
	//
	//  Also take into consideration that some items such as:
	// ex. i want to eat fruit off a fruit tree.
	// ex. i want raw food that may be cooked etc.
	// ex. I want a corpse that i can cut up and eat the parts.
	// ex. I want a chest to loot.
	//
	// RETURN:
	//  0-100 percent = how bad do we want it ?

	// a container i would loot.

	if ( ! CanMove( pItem, false ))
	{
		// Some items like fruit trees, chests and corpses might still be useful.
		return( 0 );
	}

	CCharDefPtr pCharDef = Char_GetDef();
	ASSERT(pCharDef);

	int iRet = pCharDef->m_Desires.FindResourceMatch(pItem);
	if ( iRet >= 0 )
	{
		return( pCharDef->m_Desires[iRet].GetResQty() );
	}

	// i'm hungry and this is food ?
	int iFoodLevel = Food_GetLevelPercent();
	if ( Food_CanEat( pItem ) && iFoodLevel < 100 )
	{
		return( 100 - iFoodLevel );
	}

	// If i'm a vendor. is it something i would buy ?
	if ( NPC_IsVendor())
	{
		CItemVendablePtr pItemSell = NPC_FindVendableItem(
			PTR_CAST(CItemVendable,pItem),
			const_cast <CChar*>(this)->GetBank( LAYER_VENDOR_BUYS ),
			const_cast <CChar*>(this)->GetBank( LAYER_VENDOR_STOCK ));
		if ( pItemSell )
			return( pItemSell->GetVendorPrice(0) );
	}

#if 0
	// I can wear things and use equippable items.
	switch ( pItem->GetType() )
	{
	case IT_SHIELD:
	case IT_ARMOR:
	case IT_ARMOR_LEATHER:
	case IT_CLOTHING:
		break;
	case IT_COIN:
		if ( bWantGold )
			return true;
	}

	// If I'm smart and I want gold, I'll pick up valuables too (can be sold)
	if ( bWantGold &&
		m_StatInt > 50 &&
		( PTR_CAST(CItemVendable,pItem)))
	{
		return true;
	}

#endif

	// I guess I don't want it
	return 0;
}

int CChar::NPC_GetWeaponUseScore( CItem* pWeapon )
{
	// How good would i be at this weapon ?

	SKILL_TYPE skill;

	if ( pWeapon == NULL )
	{
		skill = SKILL_WRESTLING;
	}
	else
	{
		// Is it a weapon ?
		skill = pWeapon->Weapon_GetSkill();
		if ( skill == SKILL_WRESTLING )
			return( 0 );

		// I can't equip this anyhow.
		if ( CanEquipLayer( pWeapon, LAYER_QTY, NULL, true ) == LAYER_NONE )
			return( 0 );

#if 0
		// These are useless unless i have arrows bolts ?
		switch ( pWeapon->GetType())
		{
		case IT_WEAPON_BOW:
			break;
		case IT_WEAPON_XBOW:
			break;
		}
#endif
		// How much damage could i do with this ?
	}

	int iDmg = Fight_CalcDamage( pWeapon, skill );
	int iSkillLevel = Skill_GetAdjusted( skill );

	return( iSkillLevel + iDmg* 50 );
}

MOTIVE_LEVEL CChar::NPC_WantToUseThisItem( const CItem* pItem ) const
{
	// Does the NPC want to use the item now ?
	// This may be reviewing items i have in my pack.
	//
	// ex. armor is better than what i'm wearing ?
	// ex. str potion in battle.
	// ex. heal potion in battle.
	// ex. food when i'm hungry.

	return( 0 );
}

MOTIVE_LEVEL CChar::NPC_GetHostilityLevelToward( const CChar* pCharTarg ) const
{
	// What is my general hostility/Motivation level toward this creature ?
	//
	// based on:
	//  npc vs player, (evil npc's don't like players regurdless of align, xcept in town)
	//  karma (we are of different alignments)
	//  creature body type. (allie groups)
	//  hunger, (they could be food)
	//  memories of this creature.
	//
	// DO NOT consider:
	//   strength, he is far stronger or weaker than me.
	//	 health, i may be near death.
	//   location (guarded area), (xcept in the case that evil people like other evils in town)
	//   loot, etc.
	//
	// RETURN:
	//   100 = extreme hatred.
	//   0 = neutral.
	//   -100 = love them
	//

	if ( pCharTarg == NULL )
		return( 0 );

	// if it is a pet - register it the same as it's master.
	CCharPtr pCharOwn = pCharTarg->NPC_PetGetOwner();
	if ( pCharOwn != NULL && pCharOwn != this )
	{
		return( NPC_GetHostilityLevelToward( pCharOwn ));
	}

	ASSERT(m_pNPC.IsValidNewObj());

	int iKarma = Stat_Get(STAT_Karma);

	int iHostility = 0;

	if ( Noto_IsEvil() &&	// i am evil.
		! m_pArea->IsFlagGuarded() &&	// we are not in an evil town.
		pCharTarg->m_pPlayer.IsValidNewObj() )	// my target is a player.
	{
		// If i'm evil i give no benefit to players with bad karma.
		// I hate all players.
		// Unless i'm in a guarded area. then they are cool.
		iHostility = 51;
	}
	else if ( m_pNPC->m_Brain == NPCBRAIN_BESERK )	// i'm beserk.
	{
		// beserks just hate everyone all the time.
		iHostility = 100;
	}
	else if ( pCharTarg->m_pNPC.IsValidNewObj() &&	// my target is an NPC
		pCharTarg->m_pNPC->m_Brain != NPCBRAIN_BESERK &&	// ok to hate beserks.
		! g_Cfg.m_fMonsterFight )		// monsters are not supposed to fight other monsters !
	{
		iHostility = -50;
		goto domemorybase;	// set this low in case we are defending ourselves. but not attack for hunger.
	}
	else
	{
		// base hostillity on karma diff.

		int iKarmaTarg = pCharTarg->Stat_Get(STAT_Karma);
		int iKarmaDiff = iKarma - iKarmaTarg;

		if ( Noto_IsEvil())
		{
			// I'm evil.
			if ( iKarmaTarg > 0 )
			{
				iHostility += ( iKarmaTarg ) / 1024;
			}
		}
		else if ( iKarma > 300 )
		{
			// I'm good and my target is evil.
			if ( iKarmaTarg < -100 )
			{
				iHostility += ( -iKarmaTarg ) / 1024;
			}
		}
	}

	// Based on just creature type.

	if ( pCharTarg->m_pNPC.IsValidNewObj() )
	{
		// Human NPC's will attack humans .

		if ( GetDispID() == pCharTarg->GetDispID())
		{
			// I will never attack those of my own kind...even if starving
			iHostility -= 100;
		}
		else if ( NPC_GetAllyGroupType( GetDispID()) == NPC_GetAllyGroupType(pCharTarg->GetDispID()))
		{
			iHostility -= 50;
		}
		else if ( pCharTarg->m_pNPC->m_Brain == m_pNPC->m_Brain )	// My basic kind
		{
			// Won't attack other monsters. (unless very hungry)
			iHostility -= 30;
		}
	}
	else
	{
		// Not immediately hostile if looks the same as me.
		if ( ! IsHuman() && NPC_GetAllyGroupType( GetDispID()) == NPC_GetAllyGroupType(pCharTarg->GetDispID()))
		{
			iHostility -= 51;
		}
	}

#if 0
	// How hungry am I? Could this creature be food ?
	// some creatures are not appetising.
	{
		int iFoodLevel = Food_GetLevelPercent();
		if ( iFoodLevel < 50 && Food_CanEat( pCharTarg ) > iFoodLevel )
		{
			if ( iFoodLevel < 10 )
			{
				iHostility += 10;
			}
			if ( iFoodLevel < 1 )
			{
				iHostility += 15;
			}
		}
	}
#endif

domemorybase:
	// I have been attacked/angered by this creature before ?
	CItemMemoryPtr pMemory = Memory_FindObjTypes( pCharTarg, MEMORY_FIGHT|MEMORY_HARMEDBY|MEMORY_IRRITATEDBY|MEMORY_SAWCRIME|MEMORY_AGGREIVED );
	if ( pMemory )
	{
		iHostility += 50;
	}

	return( iHostility );
}

MOTIVE_LEVEL CChar::NPC_GetAttackContinueMotivation( CChar* pChar, MOTIVE_LEVEL iMotivation ) const
{
	// I have seen fit to attack them.
	// How much do i want to continue an existing fight ? cowardice ?
	// ARGS:
	//  iMotivation = My base motivation toward this creature.
	//
	// based on:
	// relative strength. levels.
	//
	// RETURN:
	// -101 = ? dead meat. (run away)
	//
	// 0 = I'm have no interest.
	// 50 = even match.
	// 100 = he's a push over.

	ASSERT( m_pNPC.IsValidNewObj() );

	if ( pChar->IsStatFlag( STATF_DEAD | STATF_INVUL | STATF_Stone ))
		return( -100 );

	if ( m_pNPC->m_Brain == NPCBRAIN_BESERK )
	{
		// Less interested the further away they are.
		return( iMotivation + 80 - GetDist( pChar ));
	}

	// Try to stay on one target.
	if ( Fight_IsActive() && m_Act.m_Targ == pChar->GetUID())
		iMotivation += 8;

	// Less interested the further away they are.
	iMotivation -= GetDist( pChar );

	// Undead are fearless.
	if ( m_pNPC->m_Brain == NPCBRAIN_UNDEAD	||
		m_pNPC->m_Brain == NPCBRAIN_GUARD ||
		m_pNPC->m_Brain == NPCBRAIN_CONJURED )
	{
		iMotivation += 90;
		return( iMotivation );
	}

	if ( ! g_Cfg.m_fMonsterFear )
	{
		return( iMotivation );
	}

	// I'm just plain stronger.
	iMotivation += m_StatStr - pChar->m_StatStr;

	// I'm healthy.
	int iTmp = GetHealthPercent() - pChar->GetHealthPercent();
	if ( iTmp < -50 )
		iMotivation -= 50;
	else if ( iTmp > 50 )
		iMotivation += 50;

	// I'm smart and therefore more cowardly. (if injured)
	iMotivation -= m_StatInt / 16;

	return( iMotivation );
}

MOTIVE_LEVEL CChar::NPC_GetAttackMotivation( CChar* pChar, MOTIVE_LEVEL iMotivation ) const
{
	// Some sort of monster.
	// Am I stronger than he is ? Should I continue fighting ?
	// Take into consideration AC, health, skills, etc..
	// RETURN:
	// <-1 = dead meat. (run away)
	// 0 = I'm have no interest.
	// 50 = even match.
	// 100 = he's a push over.

	ASSERT( m_pNPC.IsValidNewObj() );
	if ( m_StatHealth <= 0 )
		return( -1 );	// I'm dead.
	if ( pChar == NULL )
		return( 0 );
	ASSERT(pChar->m_pArea.IsValidRefObj());
	// Is the target interesting ?
	if ( pChar->m_pArea->IsFlag( REGION_FLAG_SAFE ))	// universal
		return( 0 );

	// If the area is guarded then think better of this.
	if ( pChar->m_pArea->IsFlagGuarded() && m_pNPC->m_Brain != NPCBRAIN_GUARD )		// too smart for this.
	{
		iMotivation -= m_StatInt / 20;
	}

	// Owned by or is one of my kind ?

	iMotivation += NPC_GetHostilityLevelToward( pChar );

	if ( iMotivation > 0 )
	{
		// Am i injured etc ?
		iMotivation = NPC_GetAttackContinueMotivation( pChar, iMotivation );
	}

#ifdef _DEBUG
	if ( g_Cfg.m_wDebugFlags & DEBUGF_MOTIVATION )
	{
		DEBUG_MSG(( "NPC_GetAttackMotivation '%s' for '%s' is %d" LOG_CR, (LPCTSTR) GetName(), (LPCTSTR) pChar->GetName(), iMotivation ));
	}
#endif
	return( iMotivation );
}


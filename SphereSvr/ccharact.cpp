//
// CCharAct.cpp
// Copyright 1996 - 2001 Menace Software (www.menasoft.com)
//

#include "stdafx.h"	// predef header.

int CChar::ContentConsume( CSphereUID rid, int iQty, bool fTest, DWORD dwArg )
{
	//if ( rid.GetResType() == RES_STAT )
	//{
		// Consumes MANA,STAM,HITS ?
	//}
	return( CContainer::ContentConsume( rid, iQty, fTest, dwArg ));
}

HRESULT CChar::TeleportToObj( int iType, const CGVariant& vArgs )
{
	// "GONAME", "GOTYPE", "GOCHAR"
	// 0 = object name
	// 1 = char
	// 2 = item type

	DWORD dwUID = m_Act.m_Targ &~ UID_F_ITEM;
	DWORD dwTotal = g_World.GetUIDCount();
	DWORD dwCount = dwTotal-1;

	int iArg;
	if ( iType )
	{
		if ( ! vArgs.IsEmpty() && iType == 1 )
			dwUID = 0;
		iArg = RES_GET_INDEX( vArgs.GetDWORD());
	}
	else
	{
		// _strupr( pszArgs );
	}

	while ( dwCount-- )
	{
		if ( ++dwUID >= dwTotal )
		{
			dwUID = 1;
		}

		CObjBasePtr pObj = STATIC_CAST(CObjBase,g_World.FindUIDObj(dwUID));
		if ( pObj == NULL )
			continue;

		switch ( iType )
		{
		case 0:
			{
				MATCH_TYPE match = Str_Match( vArgs.GetPSTR(), pObj->GetName());
				if ( match != MATCH_VALID )
					continue;
			}
			break;
		case 1:	// char
			{
				CCharPtr pChar = REF_CAST(CChar,pObj);
				if ( pChar == NULL )
					continue;
				if ( iArg-- > 0 )
					continue;
			}
			break;
		case 2:	// item type
			{
				CItemPtr pItem = REF_CAST(CItem,pObj);
				if ( pItem == NULL )
					continue;
				if ( ! pItem->IsType( (IT_TYPE) iArg ))
					continue;
			}
			break;
		case 3: // char id
			{
				CCharPtr pChar = REF_CAST(CChar,pObj);
				if ( pChar == NULL )
					continue;
				if ( pChar->GetID() != iArg )
					continue;
			}
			break;
		case 4:	// item id
			{
				CItemPtr pItem = REF_CAST(CItem,pObj);
				if ( pItem == NULL )
					continue;
				if ( pItem->GetID() != iArg )
					continue;
			}
			break;
		}

		CObjBasePtr pObjTop = pObj->GetTopLevelObj();
		ASSERT(pObjTop);
		if ( pObjTop->IsChar())
		{
			if ( ! CanDisturb( REF_CAST(CChar,pObjTop)))
				continue;
		}

		if ( pObjTop == this )
			continue;

		m_Act.m_Targ = pObj->GetUID();
		Spell_Effect_Teleport( pObjTop->GetTopPoint(), true, false );
		return( NO_ERROR );
	}
	return( HRES_BAD_ARGUMENTS );
}

HRESULT CChar::TeleportToCli( int iType, int iArgs )
{
	for ( CClientPtr pClient = g_Serv.GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
	{
		if ( ! iType )
		{
			if ( pClient->m_Socket.GetSocket() != iArgs )
				continue;
		}
		CCharPtr pChar = pClient->GetChar();
		if ( pChar == NULL )
			continue;
		if ( ! CanDisturb( pChar ))
			continue;
		if ( iType )
		{
			if ( iArgs-- )
				continue;
		}
		m_Act.m_Targ = pChar->GetUID();
		Spell_Effect_Teleport( pChar->GetTopPoint(), true, false );
		return( NO_ERROR );
	}
	return( HRES_BAD_ARGUMENTS );
}

void CChar::JailEffect()
{
	// Find an empty cell and put it in.
	// Now figure out what cell to put them in

	DEBUG_CHECK( IsPrivFlag(PRIV_JAILED));
	if ( ! m_TagDefs.FindKeyPtr("JAIL_RELEASEPOINT"))
	{
		// Set this if it does not exist.
		m_TagDefs.SetKeyVar("JAIL_RELEASEPOINT", GetTopPoint().v_Get());
	}

	int iCell = 0;
	TCHAR szJailName[ 128 ];
	if ( iCell )
	{
		sprintf( szJailName, "jail%d", iCell );
	}
	else
	{
		strcpy( szJailName, "jail" );
	}
	Spell_Effect_Teleport( g_Cfg.GetRegionPoint( szJailName ), true, false );
	WriteString( "You have been jailed" );
}

void CChar::Jail( CScriptConsole* pSrc, int iTimeMinutes )
{
	// Allow NPC's to be jailed for some reason.

	CAccountPtr pAccount = GetAccount();

	if ( iTimeMinutes > 0 )	// set the jailed flag.
	{
		if ( ! IsPrivFlag(PRIV_JAILED))
		{
			m_TagDefs.SetKeyVar("JAIL_RELEASEPOINT", GetTopPoint().v_Get());
		}
		SetPrivFlags( PRIV_JAILED );
		if ( pAccount )
		{
			CServTime timeRelease;
			timeRelease.InitTimeCurrent( iTimeMinutes*60*TICKS_PER_SEC);
			pAccount->m_TagDefs.SetKeyInt( "JAIL_RELEASETIME", timeRelease.GetTimeRaw());
		}
		JailEffect();
	}
	else	// forgive.
	{
		if ( ! IsPrivFlag(PRIV_JAILED))
		{
			return;
		}
		if ( pAccount )
		{
			pAccount->m_TagDefs.RemoveKey("JAIL_RELEASETIME");
		}
		ClearPrivFlags( PRIV_JAILED );
		WriteString( "You have been forgiven" );
		Spell_Effect_Teleport( CPointMap(m_TagDefs.FindKeyVar("JAIL_RELEASEPOINT")), true, false );
	}
}

void CChar::AddGoldToPack( int iAmount, CItemContainer* pPack )
{
	// A vendor is giving me gold. put it in my pack or other place.

	if ( pPack == NULL )
		pPack = GetPackSafe();

	while ( iAmount > 0 )
	{
		CItemPtr pGold = CItem::CreateScript( ITEMID_GOLD_C1, this );

		int iGoldStack = MIN( iAmount, USHRT_MAX );
		pGold->SetAmount( iGoldStack );

		Sound( pGold->GetDropSound( pPack ));
		pPack->ContentAdd( pGold );
		iAmount -= iGoldStack;
	}
}

bool CChar::LayerAdd( CItem* pItem, LAYER_TYPE layer )
{
	// add equipped items.
	// check for item already in that layer ?
	// OnTrigger for equip is done by ItemEquip()
	// NOTE: This could be part of the Load, so it may not truly be being "equipped" at this time.
	// NOTE: remove CanEquipLayer from here and just use ItemEquip for checking.!???
	if ( pItem == NULL )
		return false;
	if ( IsMyChild(pItem) &&
		pItem->GetEquipLayer() == layer )
	{
		// Already here.
		return true;
	}

	// NOTE: CanEquipLayer may bounce an item . If it stacks with this we are in trouble !
	// This takes care of any conflicting items in the slot !
	LAYER_TYPE layerAct = CanEquipLayer( pItem, layer, NULL, false );
	if ( layerAct == LAYER_NONE )
	{
		// we should not allow non-layered stuff to be put here ?
		// Put in pack instead ?
		if ( layer != LAYER_NONE && layer != LAYER_QTY && m_pPlayer )
		{
			DEBUG_MSG(( "ContentAdd %s '%s' can't equip item %s '%s'" LOG_CR, 
				(LPCTSTR) GetResourceName(), (LPCTSTR) GetName(),
				(LPCTSTR) pItem->GetResourceName(), (LPCTSTR) pItem->GetName()));
		}
#ifdef _DEBUG
		layerAct = CanEquipLayer( pItem, layer, NULL, false );
#endif
		ItemBounce( pItem );
		return false;
	}

	if ( layerAct == LAYER_SPECIAL )
	{
		if ( pItem->IsType( IT_EQ_TRADE_WINDOW ))
			layerAct = LAYER_NONE;
	}

	pItem->RemoveSelf(); // make sure all triggers fire. T_UnEquip
	CContainer::ContentAddPrivate( pItem );
	pItem->SetEquipLayer( layerAct );

	// update flags etc for having equipped this.
	switch ( layerAct )
	{
	case LAYER_HAND1:
	case LAYER_HAND2:
		// If weapon
		if ( pItem->IsTypeWeapon())
		{
			m_uidWeapon = pItem->GetUID();
			Fight_ResetWeaponSwingTimer();
		}
		else if ( pItem->IsTypeArmor())
		{
			// Shield of some sort.
			m_ArmorDisplay = CalcArmorDefense();
			StatFlag_Set( STATF_HasShield );
			UpdateStatsFlag();
		}
		break;
	case LAYER_SHOES:
	case LAYER_PANTS:
	case LAYER_SHIRT:
	case LAYER_HELM:		// 6
	case LAYER_GLOVES:	// 7
	case LAYER_COLLAR:	// 10 = gorget or necklace.
	case LAYER_HALF_APRON:
	case LAYER_CHEST:	// 13 = armor chest
	case LAYER_TUNIC:	// 17 = jester suit
	case LAYER_ARMS:		// 19 = armor
	case LAYER_CAPE:		// 20 = cape
	case LAYER_ROBE:		// 22 = robe over all.
	case LAYER_SKIRT:
	case LAYER_LEGS:
		// If armor or clothing = change in defense rating.
		m_ArmorDisplay = CalcArmorDefense();
		UpdateStatsFlag();
		break;

		// These effects are not magical. (make them spells !)

	case LAYER_FLAG_Criminal:
		StatFlag_Set( STATF_Criminal );
		return true;
	case LAYER_FLAG_SpiritSpeak:
		StatFlag_Set( STATF_SpiritSpeak );
		return true;
	case LAYER_FLAG_Stuck:
		StatFlag_Set( STATF_Immobile );
		break;
	}

	if ( layerAct != LAYER_DRAGGING )
	{
		switch ( pItem->GetType())
		{
		case IT_EQ_SCRIPT:	// pure script.
			break;
		case IT_EQ_SCRIPT_BOOK:
			ScriptBook_OnTick( PTR_CAST(CItemMessage,pItem), false );
			break;
		case IT_EQ_MEMORY_OBJ:
			Memory_UpdateFlags( PTR_CAST(CItemMemory,pItem));
			break;
		case IT_EQ_HORSE:
			StatFlag_Set(STATF_OnHorse);
			break;
		case IT_COMM_CRYSTAL:
			StatFlag_Set(STATF_COMM_CRYSTAL);
			break;
		}
	}

	pItem->Update();
	return true;
}

void CChar::UnEquipItem( CItem* pItem )
{
	// The item has already been unequipped !

	// remove equipped items effects
	LAYER_TYPE layer = pItem->GetEquipLayer();

	switch ( layer )
	{
	case LAYER_HAND1:
	case LAYER_HAND2:	// other hand = shield
		if ( pItem->IsTypeWeapon())
		{
			m_uidWeapon.InitUID();
			Fight_ResetWeaponSwingTimer();
		}
		else if ( pItem->IsTypeArmor())
		{
			// Shield
			m_ArmorDisplay = CalcArmorDefense();
			StatFlag_Clear( STATF_HasShield );
			UpdateStatsFlag();
		}
		break;
	case LAYER_SHOES:
	case LAYER_PANTS:
	case LAYER_SHIRT:
	case LAYER_HELM:		// 6
	case LAYER_GLOVES:	// 7
	case LAYER_COLLAR:	// 10 = gorget or necklace.
	case LAYER_CHEST:	// 13 = armor chest
	case LAYER_TUNIC:	// 17 = jester suit
	case LAYER_ARMS:		// 19 = armor
	case LAYER_CAPE:		// 20 = cape
	case LAYER_ROBE:		// 22 = robe over all.
	case LAYER_SKIRT:
	case LAYER_LEGS:
		m_ArmorDisplay = CalcArmorDefense();
		UpdateStatsFlag();
		break;

	case LAYER_FLAG_Criminal:
		StatFlag_Clear( STATF_Criminal );
		break;
	case LAYER_FLAG_SpiritSpeak:
		StatFlag_Clear( STATF_SpiritSpeak );
		break;
	case LAYER_FLAG_Stuck:
		StatFlag_Clear( STATF_Immobile );
		break;
	}
}

void CChar::OnRemoveOb( CGObListRec* pObRec )	// Override this = called when removed from list.
{
	// Unequip the item.
	// This may be a delete etc. It can not FAIL !
	CItemPtr pItem = STATIC_CAST(CItem,pObRec);
	ASSERT(pItem);
	DEBUG_CHECK( pItem->IsItemEquipped());

	LAYER_TYPE layer = pItem->GetEquipLayer();
	if ( layer != LAYER_DRAGGING && ! g_Serv.IsLoading())
	{
		CSphereExpContext exec(pItem, this);
		pItem->OnTrigger( CItemDef::T_UnEquip, exec);
	}

	CContainer::OnRemoveOb( pObRec );

	UnEquipItem( pItem );

	// Items with magic effects.
	if ( layer != LAYER_DRAGGING )
	{
		switch ( pItem->GetType())
		{
		case IT_COMM_CRYSTAL:
			if ( ContentFind( CSphereUID( RES_TypeDef,IT_COMM_CRYSTAL ), 0, 0 ) == NULL )
			{
				StatFlag_Clear(STATF_COMM_CRYSTAL);
			}
			break;
		case IT_EQ_HORSE:
			StatFlag_Clear(STATF_OnHorse);
			break;
		case IT_EQ_MEMORY_OBJ:
			// Clear the associated flags.
			Memory_UpdateClearTypes( REF_CAST(CItemMemory,pItem), 0xFFFF );
			break;
		}

		// If items are magical then remove effect here.
		Spell_Equip_Remove( pItem );
	}
}

void CChar::DropAll( CItemCorpse* pCorpse, WORD wItemAttr )
{
	// shrunk or died. (or sleeping)
	if ( IsStatFlag( STATF_Conjured ))
		return;	// drop nothing.

	CItemContainerPtr pPack = GetPack();
	if ( pPack != NULL )
	{
		if ( pCorpse == NULL )
		{
			pPack->ContentsDump( GetTopPoint(), wItemAttr ); // dump on ground
		}
		else
		{
			pPack->ContentsTransfer( pCorpse, true );
		}
	}

	// transfer equipped items to corpse or your pack (if newbie).
	UnEquipAllItems( pCorpse );
}

void CChar::UnEquipAllItems( CItemContainer* pDest )
{
	// We morphed, sleeping, died or became a GM.
	// Pets can be told to "Drop All"
	// drop item that is up in the air as well.

	if ( ! GetCount())
		return;
	CItemContainerPtr pPack;

	CItemPtr pItemNext;
	CItemPtr pItem=GetHead();
	for ( ; pItem!=NULL; pItem=pItemNext )
	{
		pItemNext = pItem->GetNext();
		LAYER_TYPE layer = pItem->GetEquipLayer();
		switch ( layer )
		{
		case LAYER_NONE:
			DEBUG_CHECK( pItem->IsType( IT_EQ_TRADE_WINDOW ));
			pItem->DeleteThis();	// Get rid of any trades.
			continue;
		case LAYER_FLAG_Poison:
		case LAYER_FLAG_Criminal:
		case LAYER_FLAG_Hallucination:
		case LAYER_FLAG_Potion:
		case LAYER_FLAG_Drunk:
		case LAYER_FLAG_Stuck:
		case LAYER_FLAG_PotionUsed:
			if ( IsStatFlag( STATF_DEAD ))
				pItem->DeleteThis();
			continue;
		case LAYER_PACK:
		case LAYER_HORSE:
			continue;
		case LAYER_HAIR:	// leave this.
		case LAYER_BEARD:
			// Copy hair and beard to corpse.
			if ( pDest == NULL )
				continue;
			if ( pDest->IsType(IT_CORPSE))
			{
				CItemPtr pDupe = CItem::CreateDupeItem( pItem );
				pDest->ContentAdd( pDupe );	// add content
				// Equip layer only matters on a corpse.
				pDupe->SetContainedLayer( layer );
			}
			continue;
		case LAYER_DRAGGING:
			layer = LAYER_NONE;
			break;
		default:
			// can't transfer this to corpse.
			if ( ! CItemDef::IsVisibleLayer( layer ))
				continue;
			break;
		}
		if ( pDest != NULL &&
			! pItem->IsAttr( ATTR_NEWBIE|ATTR_MOVE_NEVER|ATTR_CURSED2|ATTR_BLESSED2 ))
		{	// Move item to dest. (corpse ussually)
			pDest->ContentAdd( pItem );
			if ( pDest->IsType(IT_CORPSE))
			{
				// Equip layer only matters on a corpse.
				pItem->SetContainedLayer( layer );
			}
		}
		else
		{	// Move item to chars' pack.
			if ( pPack == NULL )
				pPack = GetPackSafe();
			pPack->ContentAdd( pItem );
		}
	}
}

void CChar::CancelAllTrades()
{
	// remove all trade windows. client logged out.
	for ( CItemPtr pItem=GetHead(); pItem!=NULL; )
	{
		CItemPtr pItemNext = pItem->GetNext();
		if ( pItem->IsType( IT_EQ_TRADE_WINDOW ))
		{
			pItem->DeleteThis();
		}
		pItem=pItemNext;
	}
}

void CChar::UpdateDrag( CItem* pItem, CObjBase* pCont, CPointMap* ppt )
{
	// Show the world that I am picking up or putting down this object.
	// NOTE: This makes people disapear.
	CUOCommand cmd;
	cmd.DragAnim.m_Cmd = XCMD_DragAnim;
	cmd.DragAnim.m_id = pItem->GetDispID();
	cmd.DragAnim.m_unk3 = 0;
	cmd.DragAnim.m_unk5 = 0;
	cmd.DragAnim.m_unk7 = 0;

	CPointMap ptThis = GetTopPoint();

	if ( pCont != NULL )
	{
		// I'm putting an object in a cont..
		CObjBasePtr pObjTop = pCont->GetTopLevelObj();
		if ( pObjTop == this )
			return;	// move stuff in my own pack.

		CPointMap ptTop = pObjTop->GetTopPoint();

		cmd.DragAnim.m_srcUID = GetUID();
		cmd.DragAnim.m_src_x = ptThis.m_x;
		cmd.DragAnim.m_src_y = ptThis.m_y;
		cmd.DragAnim.m_src_x = ptThis.m_z;
		cmd.DragAnim.m_dstUID = pObjTop->GetUID();
		cmd.DragAnim.m_dst_x = ptTop.m_x;
		cmd.DragAnim.m_dst_y = ptTop.m_y;
		cmd.DragAnim.m_dst_z = ptTop.m_z;
	}
	else if ( ppt != NULL )
	{
		// putting on ground.
		cmd.DragAnim.m_srcUID = GetUID();
		cmd.DragAnim.m_src_x = ptThis.m_x;
		cmd.DragAnim.m_src_y = ptThis.m_y;
		cmd.DragAnim.m_src_x = ptThis.m_z;
		cmd.DragAnim.m_dstUID = 0;
		cmd.DragAnim.m_dst_x = ppt->m_x;
		cmd.DragAnim.m_dst_y = ppt->m_y;
		cmd.DragAnim.m_dst_z = ppt->m_z;
	}
	else
	{
		// I'm getting an object from where ever it is.

		// ??? Note: this doesn't work for ground objects !
		CObjBasePtr pObjTop = pItem->GetTopLevelObj();
		if ( pObjTop == this )
			return;	// move stuff in my own pack.

		CPointMap ptTop = pObjTop->GetTopPoint();

		cmd.DragAnim.m_srcUID = (pObjTop==pItem) ? 0 : (DWORD) pObjTop->GetUID();
		cmd.DragAnim.m_src_x = ptTop.m_x;
		cmd.DragAnim.m_src_y = ptTop.m_y;
		cmd.DragAnim.m_src_z = ptTop.m_z;
		cmd.DragAnim.m_dstUID = 0; // GetUID();
		cmd.DragAnim.m_dst_x = ptThis.m_x;
		cmd.DragAnim.m_dst_y = ptThis.m_y;
		cmd.DragAnim.m_dst_x = ptThis.m_z;
	}

	UpdateCanSee( &cmd, sizeof(cmd.DragAnim), m_pClient );
}

void CChar::ObjMessage( LPCTSTR pMsg, const CObjBase* pSrc ) const
{
	if ( ! IsClient())
		return;
	GetClient()->addObjMessage( pMsg, pSrc );
}
void CChar::SetMessageColorType( int iMsgColorType )
{
	if ( ! IsClient())
		return;
	GetClient()->SetMessageColorType( iMsgColorType );
}
bool CChar::WriteString( LPCTSTR pMsg )	// Push a message back to the client if there is one.
{
	if ( ! IsClient())
		return false;
	return GetClient()->WriteString( pMsg );
}

void CChar::UpdateStatsFlag() const
{
	// Push status change to all who can see us.
	// For Weight, AC, Gold must update all
	// Just flag the stats to be updated later if possible.
	if ( ! IsClient())
		return;
	GetClient()->addUpdateStatsFlag();
}

bool CChar::UpdateAnimate( ANIM_TYPE action, bool fTranslate, bool fBackward, BYTE iFrameDelay )
{
	// NPC or character does a certain Animate
	// Translate the animation based on creature type.
	// ARGS:
	//   fBackward = make the anim go in reverse.
	//   iFrameDelay = in seconds (approx), 0=fastest, 1=slower

	if ( action < 0 )
		return false;

	if ( fBackward && iFrameDelay )	// backwards and delayed just dont work ! = invis
		iFrameDelay = 0;

	if ( fTranslate || IsStatFlag( STATF_OnHorse ))
	{
		// translate the action to one specific to the character.

		CCharDefPtr pCharDef = Char_GetDef();
		ASSERT(pCharDef);

		CItemPtr pWeapon = g_World.ItemFind(m_uidWeapon);
		if ( pWeapon != NULL && action == ANIM_ATTACK_WEAPON )
		{
			// action depends on weapon type (skill) and 2 Hand type.
			DEBUG_CHECK( pWeapon->IsItemEquipped());
			LAYER_TYPE layer = pWeapon->Item_GetDef()->GetEquipLayer();
			switch ( pWeapon->GetType() )
			{
			case IT_WEAPON_MACE_CROOK:
			case IT_WEAPON_MACE_PICK:
			case IT_WEAPON_MACE_SMITH:	// Can be used for smithing ?
			case IT_WEAPON_MACE_STAFF:
			case IT_WEAPON_MACE_SHARP:	// war axe can be used to cut/chop trees.
				action = ( layer == LAYER_HAND2 ) ?
					ANIM_ATTACK_2H_DOWN :
					ANIM_ATTACK_1H_DOWN;

			do_add_style:
				if ( Calc_GetRandVal( 2 ))
				{
					// add some style to the attacks.
					if ( layer == LAYER_HAND2 )
					{
						action = (ANIM_TYPE)( ANIM_ATTACK_2H_DOWN + Calc_GetRandVal(3));
					}
					else
					{
						action = (ANIM_TYPE)( ANIM_ATTACK_1H_WIDE + Calc_GetRandVal(3));
					}
				}
				break;
			case IT_WEAPON_SWORD:
			case IT_WEAPON_AXE:
				action = ( layer == LAYER_HAND2 ) ?
			ANIM_ATTACK_2H_WIDE :
				ANIM_ATTACK_1H_WIDE;
				goto do_add_style;
			case IT_WEAPON_FENCE:
				action = ( layer == LAYER_HAND2 ) ?
			ANIM_ATTACK_2H_JAB :
				ANIM_ATTACK_1H_JAB;
				goto do_add_style;
			case IT_WEAPON_BOW:
				action = ANIM_ATTACK_BOW;
				break;
			case IT_WEAPON_XBOW:
				action = ANIM_ATTACK_XBOW;
				break;
			}
		}

		if ( IsStatFlag( STATF_OnHorse ))	// on horse back.
		{
			// Horse back anims are dif.
			switch ( action )
			{
			case ANIM_WALK_UNARM:
			case ANIM_WALK_ARM:
				action = ANIM_HORSE_RIDE_SLOW;
				break;
			case ANIM_RUN_UNARM:
			case ANIM_RUN_ARMED:
				action = ANIM_HORSE_RIDE_FAST;
				break;
			case ANIM_STAND:
				action = ANIM_HORSE_STAND;
				break;
			case ANIM_FIDGET1:
			case ANIM_FIDGET_YAWN:
				action = ANIM_HORSE_SLAP;
				break;
			case ANIM_STAND_WAR_1H:
			case ANIM_STAND_WAR_2H:
				action = ANIM_HORSE_STAND;
				break;
			case ANIM_ATTACK_1H_WIDE:
			case ANIM_ATTACK_1H_JAB:
			case ANIM_ATTACK_1H_DOWN:
				action = ANIM_HORSE_ATTACK;
				break;
			case ANIM_ATTACK_2H_JAB:
			case ANIM_ATTACK_2H_WIDE:
			case ANIM_ATTACK_2H_DOWN:
				action = ANIM_HORSE_SLAP;
				break;
			case ANIM_WALK_WAR:
				action = ANIM_HORSE_RIDE_SLOW;
				break;
			case ANIM_CAST_DIR:
				action = ANIM_HORSE_ATTACK;
				break;
			case ANIM_CAST_AREA:
				action = ANIM_HORSE_ATTACK_BOW;
				break;
			case ANIM_ATTACK_BOW:
				action = ANIM_HORSE_ATTACK_BOW;
				break;
			case ANIM_ATTACK_XBOW:
				action = ANIM_HORSE_ATTACK_XBOW;
				break;
			case ANIM_GET_HIT:
				action = ANIM_HORSE_SLAP;
				break;
			case ANIM_BLOCK:
				action = ANIM_HORSE_SLAP;
				break;
			case ANIM_ATTACK_UNARM:
				action = ANIM_HORSE_ATTACK;
				break;
			case ANIM_BOW:
			case ANIM_SALUTE:
			case ANIM_EAT:
				action = ANIM_HORSE_ATTACK_XBOW;
				break;
			default:
				action = ANIM_HORSE_STAND;
				break;
			}
		}
		else if ( GetDispID() < CREID_MAN )
		{
			// Animals have certain anims. Monsters have others.

			if ( GetDispID() >= CREID_HORSE1 )
			{
				// All animals have all these anims thankfully
				switch ( action )
				{
				case ANIM_WALK_UNARM:
				case ANIM_WALK_ARM:
				case ANIM_WALK_WAR:
					action = ANIM_ANI_WALK;
					break;
				case ANIM_RUN_UNARM:
				case ANIM_RUN_ARMED:
					action = ANIM_ANI_RUN;
					break;
				case ANIM_STAND:
				case ANIM_STAND_WAR_1H:
				case ANIM_STAND_WAR_2H:
				default:
					action = ANIM_ANI_STAND;
					break;

				case ANIM_FIDGET1:
					action = ANIM_ANI_FIDGET1;
					break;
				case ANIM_FIDGET_YAWN:
					action = ANIM_ANI_FIDGET2;
					break;
				case ANIM_CAST_DIR:
					action = ANIM_ANI_ATTACK1;
					break;
				case ANIM_CAST_AREA:
					action = ANIM_ANI_EAT;
					break;
				case ANIM_GET_HIT:
					action = ANIM_ANI_GETHIT;
					break;

				case ANIM_ATTACK_1H_WIDE:
				case ANIM_ATTACK_1H_JAB:
				case ANIM_ATTACK_1H_DOWN:
				case ANIM_ATTACK_2H_DOWN:
				case ANIM_ATTACK_2H_JAB:
				case ANIM_ATTACK_2H_WIDE:
				case ANIM_ATTACK_BOW:
				case ANIM_ATTACK_XBOW:
				case ANIM_ATTACK_UNARM:
					switch ( Calc_GetRandVal(2))
					{
					case 0: action = ANIM_ANI_ATTACK1; break;
					case 1: action = ANIM_ANI_ATTACK2; break;
					}
					break;

				case ANIM_DIE_BACK:
					action = ANIM_ANI_DIE1;
					break;
				case ANIM_DIE_FORWARD:
					action = ANIM_ANI_DIE2;
					break;
				case ANIM_BLOCK:
				case ANIM_BOW:
				case ANIM_SALUTE:
					action = ANIM_ANI_SLEEP;
					break;
				case ANIM_EAT:
					action = ANIM_ANI_EAT;
					break;
				}

				while ( action != ANIM_WALK_UNARM && ! _ISSET( pCharDef->m_Anims, action))
				{
					// This anim is not supported. Try to use one that is.
					switch ( action )
					{
					case ANIM_ANI_SLEEP:	// All have this.
						action = ANIM_ANI_EAT;
						break;
					default:
						action = ANIM_WALK_UNARM;
						break;
					}
				}
			}
			else
			{
				// Monsters don't have all the anims.

				switch ( action )
				{
				case ANIM_CAST_DIR:
					action = ANIM_MON_Stomp;
					break;
				case ANIM_CAST_AREA:
					action = ANIM_MON_PILLAGE;
					break;
				case ANIM_DIE_BACK:
					action = ANIM_MON_DIE1;
					break;
				case ANIM_DIE_FORWARD:
					action = ANIM_MON_DIE2;
					break;
				case ANIM_GET_HIT:
					switch ( Calc_GetRandVal(3))
					{
					case 0: action = ANIM_MON_GETHIT; break;
					case 1: action = ANIM_MON_BlockRight; break;
					case 2: action = ANIM_MON_BlockLeft; break;
					}
					break;
				case ANIM_ATTACK_1H_WIDE:
				case ANIM_ATTACK_1H_JAB:
				case ANIM_ATTACK_1H_DOWN:
				case ANIM_ATTACK_2H_DOWN:
				case ANIM_ATTACK_2H_JAB:
				case ANIM_ATTACK_2H_WIDE:
				case ANIM_ATTACK_BOW:
				case ANIM_ATTACK_XBOW:
				case ANIM_ATTACK_UNARM:
					switch ( Calc_GetRandVal(3))
					{
					case 0: action = ANIM_MON_ATTACK1; break;
					case 1: action = ANIM_MON_ATTACK2; break;
					case 2: action = ANIM_MON_ATTACK3; break;
					}
					break;
				default:
					action = ANIM_WALK_UNARM;
					break;
				}
				// NOTE: Available actions depend HEAVILY on creature type !
				// ??? Monsters don't have all anims in common !
				// translate these !
				while ( action != ANIM_WALK_UNARM && ! _ISSET( pCharDef->m_Anims, action))
				{
					// This anim is not supported. Try to use one that is.
					switch ( action )
					{
					case ANIM_MON_ATTACK1:	// All have this.
						DEBUG_ERR(( "Anim 0%x This is wrong! Invalid SCP file data." LOG_CR, GetDispID()));
						action = ANIM_WALK_UNARM;
						break;

					case ANIM_MON_ATTACK2:	// Dolphins, Eagles don't have this.
					case ANIM_MON_ATTACK3:
						action = ANIM_MON_ATTACK1;	// ALL creatures have at least this attack.
						break;
					case ANIM_MON_Cast2:	// Trolls, Spiders, many others don't have this.
						action = ANIM_MON_BlockRight;	// Birds don't have this !
						break;
					case ANIM_MON_BlockRight:
						action = ANIM_MON_BlockLeft;
						break;
					case ANIM_MON_BlockLeft:
						action = ANIM_MON_GETHIT;
						break;
					case ANIM_MON_GETHIT:
						if ( _ISSET( pCharDef->m_Anims, ANIM_MON_Cast2))
							action = ANIM_MON_Cast2;
						else
							action = ANIM_WALK_UNARM;
						break;

					case ANIM_MON_Stomp:
						action = ANIM_MON_PILLAGE;
						break;
					case ANIM_MON_PILLAGE:
						action = ANIM_MON_ATTACK3;
						break;
					case ANIM_MON_AttackBow:
					case ANIM_MON_AttackXBow:
						action = ANIM_MON_ATTACK3;
						break;
					case ANIM_MON_AttackThrow:
						action = ANIM_MON_AttackXBow;
						break;

					default:
						DEBUG_ERR(( "Anim Unsupported 0%x for 0%x" LOG_CR, action, GetDispID()));
						action = ANIM_WALK_UNARM;
						break;
					}
				}
			}
		}
	}

	// NOTE: Some clients cannot see certain anims !

	WORD wRepeat = 1;

	CUOCommand cmd;
	cmd.CharAction.m_Cmd = XCMD_CharAction;
	cmd.CharAction.m_UID = GetUID();
	cmd.CharAction.m_action = action;
	cmd.CharAction.m_zero7 = 0;
	cmd.CharAction.m_dir = m_dirFace;
	cmd.CharAction.m_repeat = wRepeat;		// 1, repeat count. 0=forever.
	cmd.CharAction.m_backward = fBackward ? 1 : 0;	// 0, backwards (0/1)
	cmd.CharAction.m_repflag = ( wRepeat == 1 ) ? 0 : 1;	// 0=dont repeat. 1=repeat
	cmd.CharAction.m_framedelay = iFrameDelay;	// 1, 0=fastest.

	for ( CClientPtr pClient = g_Serv.GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
	{
		if ( ! pClient->CanSee( this ))
			continue;
		if ( action >= ANIM_QTY )	// 3d client ?
		{
			if ( ! pClient->m_fClientVer3d )
				continue;
		}
		pClient->xSendPkt( &cmd, sizeof(cmd.CharAction) );
	}
	return( true );
}

void CChar::UpdateMode( CClient* pExcludeClient, bool fFull )
{
	// If character status has been changed
	// (Polymorph, war mode or hide), resend him

	int iComplexity = 0;
	for ( CClientPtr pClient = g_Serv.GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
	{
		if ( pExcludeClient == pClient )
			continue;
		if ( ! pClient->CanSee( this ))
		{
			// In the case of "INVIS" used by GM's we must use this.
			if ( GetDist( pClient->GetChar()) <= SPHEREMAP_VIEW_SIZE )
			{
				pClient->addObjectRemove( this );
			}
			continue;
		}
		if ( pClient->IsPrivFlag( PRIV_DEBUG ))
			continue;
		if ( fFull )
		{
			pClient->addChar( this );
		}
		else
		{
			pClient->addCharMove( this );
		}

		// This really sucks but we have to do it !
		if ( ++iComplexity > 4*g_Cfg.m_iMaxCharComplexity )
			break;
	}
}

void CChar::UpdateMove( CPointMap pold, CClient* pExcludeClient, bool fFull )
{
	// Who now sees this char ?
	// Did they just see him move ?

	int iComplexity = 0;
	for ( CClientPtr pClient = g_Serv.GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
	{
		if ( pClient == pExcludeClient )
			continue;	// no need to see self move.
		if ( pClient == m_pClient && fFull )
		{
			// What do i now see ?
			pClient->addPlayerView( pold );
			continue;
		}
		CCharPtr pChar = pClient->GetChar();
		if ( pChar == NULL )
			continue;

		bool fCouldSee = ( pold.GetDist( pChar->GetTopPoint()) <= SPHEREMAP_VIEW_SIZE );

		if ( ! pClient->CanSee( this ))
		{	// can't see me now.
			if ( fCouldSee ) pClient->addObjectRemove( this );
		}
		else if ( fCouldSee )
		{	// They see me move.
			pClient->addCharMove( this );
		}
		else
		{	// first time this client has seen me.
			pClient->addChar( this );
		}

		// This really sucks but we have to do it !
		if ( ++iComplexity > 4*g_Cfg.m_iMaxCharComplexity )
			break;
	}
}

void CChar::UpdateDir( DIR_TYPE dir )
{
	if ( dir != m_dirFace && dir < DIR_QTY )
	{
		m_dirFace = dir;	// face victim.
		UpdateMove( GetTopPoint(), NULL, true );
	}
}

void CChar::UpdateDir( const CPointMap & pt )
{
	// Change in direction.
	UpdateDir( GetTopPoint().GetDir( pt ));
}

void CChar::UpdateDir( const CObjBaseTemplate* pObj )
{
	if ( pObj == NULL )
		return;
	CObjBasePtr pObjTop = pObj->GetTopLevelObj();
	if ( pObjTop == this )		// In our own pack.
		return;
	UpdateDir( pObjTop->GetTopPoint());
}

void CChar::Update( const CClient* pClientExclude ) // If character status has been changed (Polymorph), resend him
{
	// Or I changed looks.
	// I moved or somebody moved me  ?
	for ( CClientPtr pClient = g_Serv.GetClientHead(); pClient!=NULL; pClient = pClient->GetNext())
	{
		if ( pClient == pClientExclude )
			continue;
		if ( pClient == m_pClient )
		{
			pClient->addReSync();
		}
		else if ( pClient->CanSee( this ))
		{
			pClient->addChar( this );
		}
	}
}

SOUND_TYPE CChar::SoundChar( CRESND_TYPE type )
{
	SOUND_TYPE id;

	CCharDefPtr pCharDef = Char_GetDef();
	ASSERT(pCharDef);

	if ( GetDispID() == CREID_BLADES )
	{
		id = pCharDef->m_soundbase; // just one sound
	}
	else if ( GetDispID() >= CREID_MAN )
	{
		id = 0;

		static const SOUND_TYPE sm_Snd_Man_Die[] = { 0x15a, 0x15b, 0x15c, 0x15d };
		static const SOUND_TYPE sm_Snd_Man_Omf[] = { 0x154, 0x155, 0x156, 0x157, 0x158, 0x159 };
		static const SOUND_TYPE sm_Snd_Wom_Die[] = { 0x150, 0x151, 0x152, 0x153 };
		static const SOUND_TYPE sm_Snd_Wom_Omf[] = { 0x14b, 0x14c, 0x14d, 0x14e, 0x14f };

		if ( pCharDef->IsFemale())
		{
			switch ( type )
			{
			case CRESND_GETHIT:
				id = sm_Snd_Wom_Omf[ Calc_GetRandVal( COUNTOF(sm_Snd_Wom_Omf)) ];
				break;
			case CRESND_DIE:
				id = sm_Snd_Wom_Die[ Calc_GetRandVal( COUNTOF(sm_Snd_Wom_Die)) ];
				break;
			}
		}
		else
		{
			switch ( type )
			{
			case CRESND_GETHIT:
				id = sm_Snd_Man_Omf[ Calc_GetRandVal( COUNTOF(sm_Snd_Man_Omf)) ];
				break;
			case CRESND_DIE:
				id = sm_Snd_Man_Die[ Calc_GetRandVal( COUNTOF(sm_Snd_Man_Die)) ];
				break;
			}
		}
	}
	else
	{
		id = pCharDef->m_soundbase + type;
		switch ( pCharDef->m_soundbase )	// some creatures have no base sounds.
		{
		case 128: // old versions
		case 181:
		case 199:
			if ( type <= CRESND_RAND2 )
				id = 0;
			break;
		case 130: // ANIMALS_DEER3
		case 183: // ANIMALS_LLAMA3
		case 201: // ANIMALS_RABBIT3
			if ( type <= CRESND_RAND2 )
				id = 0;
			else
				id -= 2;
			break;
		}
	}

	if ( type == CRESND_HIT )
	{
		CItemPtr pWeapon = g_World.ItemFind(m_uidWeapon);
		if ( pWeapon != NULL )
		{
			DEBUG_CHECK( pWeapon->IsItemEquipped());
			// weapon type strike noise based on type of weapon and how hard hit.

			switch ( pWeapon->GetType() )
			{
			case IT_WEAPON_MACE_CROOK:
			case IT_WEAPON_MACE_PICK:
			case IT_WEAPON_MACE_SMITH:	// Can be used for smithing ?
			case IT_WEAPON_MACE_STAFF:
				// 0x233 = blunt01 (miss?)
				id = 0x233;
				break;
			case IT_WEAPON_MACE_SHARP:	// war axe can be used to cut/chop trees.
				// 0x232 = axe01 swing. (miss?)
				id = 0x232;
				break;
			case IT_WEAPON_SWORD:
			case IT_WEAPON_AXE:
				if ( pWeapon->Item_GetDef()->GetEquipLayer() == LAYER_HAND2 )
				{
					// 0x236 = hvyswrd1 = (heavy strike)
					// 0x237 = hvyswrd4 = (heavy strike)
					id = Calc_GetRandVal( 2 ) ? 0x236 : 0x237;
					break;
				}
			case IT_WEAPON_FENCE:
				// 0x23b = sword1
				// 0x23c = sword7
				id = Calc_GetRandVal( 2 ) ? 0x23b : 0x23c;
				break;
			case IT_WEAPON_BOW:
			case IT_WEAPON_XBOW:
				// 0x234 = xbow ( hit)
				id = 0x234;
				break;
			}
		}
		else if ( id == 0 )
		{
			static const SOUND_TYPE sm_Snd_Hit[] =
			{
				0x135, //= hit01 = (slap)
				0x137, //= hit03 = (hit sand)
				0x13b, //= hit07 = (hit slap)
			};
			id = sm_Snd_Hit[ Calc_GetRandVal( COUNTOF( sm_Snd_Hit )) ];
		}
	}

	if ( id <= 0 )
		return( 0 );
	Sound( id );
	return( id );
}

int CChar::ItemPickup( CItem* pItem, int amount )
{
	// Pickup off the ground or remove my own equipment. etc..
	// Check to see if i can really do this first.
	// This item is now "up in the air"
	// RETURN:
	//  amount we can pick up.
	//	-1 = we cannot pick this up.

	if ( amount < 0 ||
		pItem == NULL )
	{
		return -1;
	}

	if ( ! pItem->m_uidChanger.IsValidUID())
	{
		// Mark me as it's owner ?
		pItem->SetChangerSrc(this);
	}

	if ( IsMyChild(pItem) &&
		pItem->GetEquipLayer() == LAYER_DRAGGING )
	{
		// Silly , already dragging this.
		return( pItem->GetAmount());
	}

	// Check if something is already in the drag slot. if so drop it.
	CItemPtr pItemPrev = LayerFind( LAYER_DRAGGING );
	if ( pItemPrev != NULL )
	{
		ItemBounce( pItemPrev );
	}

	if ( ! CanTouch( pItem ) ||
		! CanMove( pItem, true ))
	{
		return -1;
	}

	CCharPtr pCharTop;	// the char that possesses this.
	if ( IsTakeCrime( pItem, &pCharTop ))
	{
		WriteString( "That is not yours. You will have to steal the item" );
		return -1;
	}

	CObjBasePtr pObjTop = pItem->GetTopLevelObj();
	CItemCorpsePtr pCorpseItem = REF_CAST(CItemCorpse,pObjTop);
	if ( pCorpseItem )
	{
		// Taking stuff off someones corpse can be a crime !
		if ( CheckCorpseCrime( pCorpseItem, true, false ))
		{
			WriteString( "Guards can now be called on you." );
		}
		// Are we hidden?
		if ( IsStatFlag(STATF_Hidden) )
		{
			// Chance of being revealed?
			Reveal();
		}
	}

	int iAmountMax = pItem->GetAmount();
	if ( iAmountMax <= 0 )
		return( -1 );
	if ( amount <= 0 )
		amount = 1;
	if ( amount > iAmountMax )
		amount = iAmountMax;
	int iItemWeight;
	if ( iAmountMax <= 1 )
	{
		iItemWeight = pItem->GetWeight();	// might be a container with stuff in it.
	}
	else
	{
		iItemWeight = ( pItem->Item_GetDef()->GetWeight()* amount );
	}

	// Is it too heavy to even drag ?
	bool fDrop = false;
	if ( GetWeightLoadPercent( GetTotalWeight() + iItemWeight ) > 300 )
	{
		WriteString("That is too heavy. You can't move that.");
		if ( pCharTop != this )
		{
			return( -1 );
		}
		fDrop = true;	// we can always drop it out of own pack !
	}

	CItemDef::T_TYPE_ trigger;
	if ( pCharTop != NULL )
	{
		trigger = pItem->IsItemEquipped() ? CItemDef::T_UnEquip : CItemDef::T_Pickup_Pack;
	}
	else
	{
		trigger = pItem->IsTopLevel() ? CItemDef::T_Pickup_Ground : CItemDef::T_Pickup_Pack;
	}
	if ( trigger != CItemDef::T_UnEquip )	// unequip is done later.
	{
		CSphereExpArgs exec( pItem, this, amount );
		if ( pItem->OnTrigger( trigger, exec ) == TRIGRET_RET_VAL )
			return( -1 );
	}

	if ( pItem->IsDisconnected())
	{
		// Seems the macro has deleted it !?
		return -1;
	}

	if ( pItem->Item_GetDef()->IsStackableType() && amount )
	{
		// Did we only pick up part of it ?
		// part or all of a pile. Only if pilable !
		if ( amount < iAmountMax )
		{
			// create left over item.
			pItem->UnStackSplit( amount, this );
		}
	}
	else
	{
		amount = iAmountMax;
	}

	if ( fDrop )
	{
		ItemDrop( pItem, GetTopPoint());
		return( -1 );
	}

	// do the dragging anim (to me) for everyone else to see.
	UpdateDrag( pItem );

	// Pick it up.
	pItem->SetDecayTime(-1);	// Kill any decay timer.

	LayerAdd( pItem, LAYER_DRAGGING );
	UnEquipItem( pItem );

	return( amount );
}

HRESULT CChar::ItemBounce( CItem* pItem )
{
	// We can't put this where we want to
	// So put in my pack if i can. else drop.
	// don't check where this came from !
	// NOTE:
	//  This item can absorb other items when it is bounced !
	// other items in packs may not be valid after this call !

	if ( pItem == NULL )
		return HRES_INVALID_HANDLE;

	CItemContainerPtr pPack = GetPackSafe();
	if ( pItem->GetParent() == pPack )
		return( NO_ERROR );

	LPCTSTR pszWhere = NULL;
	if ( CanCarry( pItem ))
	{
		// if we can carry it
		pszWhere = "in your pack";
		if ( pPack == NULL )	// cant create a pack while loading !
			goto dropit;	// this can happen at load time.
		pPack->ContentAdd( pItem ); // Add it to pack
		Sound( pItem->GetDropSound( pPack ));
	}
	else
	{
	dropit:
		if ( ! GetTopPoint().IsValidPoint())
		{
			// NPC is being created and has no valid point yet.
			if (pszWhere)
			{
				DEBUG_ERR(( "No pack to place loot item '%s' for NPC '%s'" LOG_CR, (LPCTSTR) pItem->GetResourceName(), (LPCTSTR) GetResourceName()));
			}
			else
			{
				DEBUG_ERR(( "Loot item %s too heavy for NPC %s" LOG_CR, (LPCTSTR) pItem->GetResourceName(), (LPCTSTR) GetResourceName()));
			}
			pItem->DeleteThis();
			return HRES_BAD_ARGUMENTS;
		}
		pszWhere = "at your feet. It is too heavy.";
		ItemDrop( pItem, GetTopPoint());
	}

	Printf( "You put the %s %s.", (LPCTSTR) pItem->GetName(), pszWhere );
	return( NO_ERROR );
}

HRESULT CChar::ItemDrop( CItem* pItem, const CPointMap & pt )
{
	// A char actively drops an item on the ground.
	if ( pItem == NULL || ! pItem->IsValidUID())
		return( HRES_INVALID_HANDLE );

	CItemDefPtr pItemDef = pItem->Item_GetDef();
	if (( g_Cfg.m_fFlipDroppedItems || pItemDef->Can(CAN_I_FLIP)) &&
		pItem->IsMovableType() &&
		! pItemDef->IsStackableType())
	{
		// Does this item have a flipped version.
		pItem->SetDispID( pItemDef->GetNextFlipID( pItem->GetDispID()));
	}

	if ( ! pItem->MoveToCheck( pt, this ))
		return(HRES_BAD_ARGUMENTS);

	return NO_ERROR;
}

HRESULT CChar::ItemEquip( CItem* pItem, CChar* pCharMsg )
{
	// Equip visible stuff. else throw into our pack.
	// Pay no attention to where this came from or if i can pick it up etc..
	// Bounce anything in the slot we want to go to. (if possible)
	// NOTE: This can be used from scripts as well to equip memories etc.
	// ASSUME this is ok for me to use. (movable etc)

	if ( pItem == NULL )
		return( HRES_INVALID_HANDLE );

	// In theory someone else could be dressing me ?
	if ( pCharMsg == NULL )
	{
		pCharMsg = this;
	}

	if ( IsMyChild(pItem))
	{
		if ( pItem->GetEquipLayer() != LAYER_DRAGGING )
		{
			// already equipped.
			return( NO_ERROR );
		}
	}

	// strong enough to equip this . etc ?
	// Move stuff already equipped out of the way.
	LAYER_TYPE layer = CanEquipLayer( pItem, LAYER_QTY, pCharMsg, false );
	if ( layer == LAYER_NONE )
	{
		ItemBounce( pItem );
		return( HRES_INVALID_HANDLE );
	}

	pItem->SetDecayTime(-1);	// Kill any decay timer.

	LayerAdd( pItem, layer );
	if ( ! pItem->IsItemEquipped())	// Equip failed ? (cursed?) Did it just go into pack ?
		return( HRES_INVALID_HANDLE );

	{
	CSphereExpContext exec(pItem, this);
	if ( pItem->OnTrigger( CItemDef::T_Equip, exec) == TRIGRET_RET_VAL )
	{
		return( HRES_INVALID_HANDLE );
	}
	}
	if ( ! pItem->IsItemEquipped())	// Equip failed ? (cursed?) Did it just go into pack ?
		return( HRES_INVALID_HANDLE );

	Spell_Equip_Add( pItem );	// if it has a magic effect.

	if ( CItemDef::IsVisibleLayer(layer))	// visible layer ?
	{
		Sound( 0x057 );
	}

	return( NO_ERROR );
}

#if 0
static int ResourceQtySortVal( void* pLeft, void* pRight )
{
	return( ((const CResourceQty*) pLeft)->GetResQty() - ((const CResourceQty*) pRight)->GetResQty());
}
#endif

void CChar::EatAnim( LPCTSTR pszName, int iQty )
{
	Stat_Set( STAT_Food, m_StatFood + iQty );

	static const SOUND_TYPE sm_EatSounds[] = { 0x03a, 0x03b, 0x03c };

	Sound( sm_EatSounds[ Calc_GetRandVal( COUNTOF(sm_EatSounds)) ] );
	UpdateAnimate( ANIM_EAT );

	CGString sEmoteMessage;
	sEmoteMessage.Format( "eat some %s", (LPCTSTR) pszName );
	Emote(sEmoteMessage);
}

bool CChar::Reveal( DWORD dwFlags )
{
	// Some outside influence may be revealing us.

	if ( ! IsStatFlag( dwFlags ))	// no effect.
		return( false );

	if (( dwFlags & STATF_Sleeping ) && IsStatFlag( STATF_Sleeping ))
	{
		// Try to wake me up.
		SleepWake();
	}
	bool fInvis = false;
	if (( dwFlags & STATF_Invisible ) && IsStatFlag( STATF_Invisible  ))
	{
		fInvis = true;
		SetHue( m_prev_Hue );
	}

	ASSERT( !( dwFlags & (STATF_Pet|STATF_Spawned|STATF_SaveParity|STATF_Ridden|STATF_OnHorse)));
	StatFlag_Clear( dwFlags );
	if ( IsStatFlag( STATF_Invisible | STATF_Hidden | STATF_Insubstantial | STATF_Sleeping ))
		return( false );

	if ( fInvis )
	{
		RemoveFromView();	// just the change in wHue requires this .
		Update();
	}
	else
	{
		UpdateMode( NULL, true );
	}

	WriteString( "You have been revealed" );

	if ( GetDispID() == CREID_CORPSER )
	{
		// Comes out from under the ground.
		UpdateAnimate( ANIM_MON_Stomp, false );
		Sound( 0x221 );
	}

	return( true );
}

void CChar::SpeakUTF8( LPCTSTR pszText, HUE_TYPE wHue, TALKMODE_TYPE mode, FONT_TYPE font, CLanguageID lang )
{
	// Ignore the font argument here !

	if ( IsStatFlag(STATF_Stone|STATF_Squelch))
		return;
	Reveal( STATF_Hidden|STATF_Sleeping );
	if ( mode == TALKMODE_YELL && GetPrivLevel() >= PLEVEL_Counsel )
	{	// Broadcast yell.
		mode = TALKMODE_BROADCAST;	// GM Broadcast (Done if a GM yells something)
	}
	CObjBase::SpeakUTF8( pszText, m_SpeechHue, mode, m_fonttype, lang );
}

void CChar::Speak( LPCTSTR pszText, HUE_TYPE wHue, TALKMODE_TYPE mode, FONT_TYPE font )
{
	// Speak to all clients in the area.
	// Ignore the font argument here !

	if ( IsStatFlag(STATF_Stone|STATF_Squelch))
		return;
	Reveal( STATF_Hidden|STATF_Sleeping );
	if ( mode == TALKMODE_YELL && GetPrivLevel() >= PLEVEL_Counsel )
	{	// Broadcast yell.
		mode = TALKMODE_BROADCAST;	// GM Broadcast (Done if a GM yells something)
	}
	CObjBase::Speak( pszText, m_SpeechHue, mode, m_fonttype );
}

CItemPtr CChar::Make_Figurine( CSphereUID uidOwner, ITEMID_TYPE id )
{
	// Make me into a figurine
	if ( IsDisconnected())	// we are already a figurine !
		return( NULL );
	if ( m_pPlayer.IsValidNewObj())
		return( NULL );

	CCharDefPtr pCharDef = Char_GetDef();
	ASSERT(pCharDef);

	// turn creature into a figurine.
	CItemPtr pItem = CItem::CreateScript( ( id == ITEMID_NOTHING ) ? pCharDef->m_trackID : id, this );
	ASSERT(pItem);

	pItem->SetType( IT_FIGURINE );
	pItem->SetName( GetName());
	pItem->SetHue( GetHue());
	pItem->m_itFigurine.m_ID = GetID();	// Base type of creature.
	pItem->m_itFigurine.m_uidChar = GetUID();
	pItem->m_uidLink = uidOwner;

	if ( IsStatFlag( STATF_Insubstantial ))
	{
		pItem->SetAttr(ATTR_INVIS);
	}

	SoundChar( CRESND_RAND1 );	// Horse winny
	m_Act.m_atRidden.m_FigurineUID = pItem->GetUID();
	StatFlag_Set( STATF_Ridden );
	Skill_Start( NPCACT_RIDDEN );
	SetDisconnected();

	return( pItem );
}

CItemPtr CChar::NPC_Shrink()
{
	// This will just kill conjured creatures.
	if ( IsStatFlag( STATF_Conjured ))
	{
		m_StatHealth = 0;
		return( NULL );
	}

	CItemPtr pItem = Make_Figurine( UID_INDEX_CLEAR, ITEMID_NOTHING );
	if ( pItem == NULL )
		return( NULL );

	pItem->SetAttr(ATTR_MAGIC);
	pItem->MoveToCheck( GetTopPoint());
	return( pItem );
}

CItemPtr CChar::Horse_GetMountItem() const
{
	// I am a horse.
	// Get my mount object. (attached to my rider)

	if ( ! IsStatFlag( STATF_Ridden ))
		return( NULL );

	DEBUG_CHECK( Skill_GetActive() == NPCACT_RIDDEN );
	DEBUG_CHECK( m_pNPC );
	DEBUG_CHECK( IsDisconnected());

	CItemPtr pItem = g_World.ItemFind( m_Act.m_atRidden.m_FigurineUID );
	if ( pItem == NULL ||
		( ! pItem->IsType( IT_FIGURINE ) && ! pItem->IsType( IT_EQ_HORSE )))
	{
		return( NULL );
	}

	DEBUG_CHECK( pItem->m_itFigurine.m_uidChar == GetUID());
	return( pItem );
}

CCharPtr CChar::Horse_GetMountChar() const
{
	// I am a horse.
	// Get my rider.

	CItemPtr pItem = Horse_GetMountItem();
	if ( pItem == NULL )
		return( NULL );
	return( REF_CAST(CChar,pItem->GetTopLevelObj()));
}

bool CChar::Horse_Mount( CCharPtr pHorse ) // Remove horse char and give player a horse item
{
	// Mount this ridable creature.
	// create IT_EQ_HORSE item on you.
	// RETURN:
	//  true = done mounting so take no more action.
	//  false = we can't mount this so do something else.
	//

	if ( ! CanTouch( pHorse ))
	{
		WriteString( "You can't reach the creature." );
		return( false );
	}

	// Get the m_MountID for the IT_EQ_HORSE
	CCharDefPtr pCharDef = pHorse->Char_GetDef();
	ASSERT(pCharDef);

static const WORD g_Item_Horse_Mounts[][2] = // extern
{	// Used with MOUNTID
	ITEMID_M_HORSE1,		CREID_HORSE1,
	ITEMID_M_HORSE2,		CREID_HORSE2,
	ITEMID_M_HORSE3,		CREID_HORSE3,
	ITEMID_M_HORSE4,		CREID_HORSE4,
	ITEMID_M_OSTARD_DES,	CREID_Ostard_Desert,	// t2A
	ITEMID_M_OSTARD_Frenz,	CREID_Ostard_Frenz,		// t2A
	ITEMID_M_OSTARD_For,	CREID_Ostard_Forest,	// t2A
	ITEMID_M_LLAMA,			CREID_Llama,			// t2A
	ITEMID_M_KIRIN,			CREID_KIRIN,			// lbr
	ITEMID_M_SEAHORSE,	CREID_SEAHORSE,
	ITEMID_M_DARK_STEED,	CREID_DARK_STEED,
	ITEMID_M_ETHEREAL_HORSE,	CREID_ETHEREAL_HORSE,
	ITEMID_M_NIGHTMARE,		CREID_NIGHTMARE,
	ITEMID_M_SILVER_STEED,	CREID_SILVER_STEED,
	ITEMID_M_BRITANNIAN_WARHORSE,	CREID_BRITANNIAN_WARHORSE,
	ITEMID_M_MAGECOUNCIL_WARHORSE,	CREID_MAGECOUNCIL_WARHORSE,
	ITEMID_M_MINAX_WARHORSE,	CREID_MINAX_WARHORSE,
	ITEMID_M_SHADOWLORD_WARHORSE,	CREID_SHADOWLORD_WARHORSE,
	ITEMID_M_ETHEREAL_LLAMA,	CREID_ETHEREAL_LLAMA,
	ITEMID_M_ETHEREAL_OSTARD,	CREID_ETHEREAL_OSTARD,
	ITEMID_M_UNICORN,		CREID_UNICORN,			// lbr
	ITEMID_M_RIDGEBACK1,	CREID_RIDGEBACK1,		// lbr
	ITEMID_M_RIDGEBACK2,	CREID_RIDGEBACK2,		// lbr
	ITEMID_M_BEETLE,		CREID_BEETLE,			// lbr
	ITEMID_M_SKELETAL_MOUNT, CREID_SKELETAL_MOUNT,	// lbr
	ITEMID_M_SWAMPDRAGON1,	CREID_SWAMP_DRAGON1,	// lbr
	ITEMID_M_SWAMPDRAGON2,	CREID_SWAMP_DRAGON2,	// lbr
	ITEMID_M_FIRE_STEED, CREID_FIRE_STEED,
	0,0,
};

	ITEMID_TYPE id = pCharDef->m_MountID;
	for ( int i=0; id == ITEMID_NOTHING; i++ )
	{
		if ( i>=COUNTOF(g_Item_Horse_Mounts))
		{
			return( false );
		}
		if ( pHorse->GetDispID() == g_Item_Horse_Mounts[i][1] )
		{
			id = (ITEMID_TYPE) g_Item_Horse_Mounts[i][0];
			break;
		}
	}

	if ( IsStatFlag( STATF_DEAD ) ||	// can't mount horse if dead!
		! IsHuman())	// only humans can ride horses.
	{
		WriteString( "You are not physically capable of riding a horse." );
		return( false );
	}
	if ( ! pHorse->NPC_IsOwnedBy( this ) || pHorse->m_pPlayer.IsValidNewObj())
	{
		WriteString( "You dont own that horse." );
		return( false );
	}

	Horse_UnMount();	// unmount if already on a horse.

	CItemPtr pItem = pHorse->Make_Figurine( GetUID(), id );
	if (pItem == NULL )
		return( false );

	pItem->SetType( IT_EQ_HORSE );
	pItem->SetTimeout( 10*TICKS_PER_SEC );	// give the horse a tick everyone once in a while.
	LayerAdd( pItem, LAYER_HORSE );	// equip the horse item

	return( true );
}

CCharPtr CChar::Horse_UnMount() // Get off a horse (Remove horse item and spawn new horse)
{
	if ( ! IsStatFlag( STATF_OnHorse ))
		return( NULL );

	CItemPtr pItem = LayerFind( LAYER_HORSE );
	if ( pItem == NULL )
	{
		StatFlag_Clear( STATF_OnHorse );	// flag got out of sync !
		return( NULL );
	}

	// What creature is the horse item ?
	CCharPtr pHorse = Use_Figurine( pItem, 0 );
	pItem->DeleteThis();
	return( pHorse );
}

void CChar::OnHearEquip( CChar* pCharSrc, TCHAR* szText )
{
	// Item in my inventory heard something ?

	ASSERT( IsStatFlag(STATF_COMM_CRYSTAL));

	CItemPtr pItemNext;
	CItemPtr pItem=GetHead();
	for ( ; pItem!=NULL; pItem=pItemNext )
	{
		pItemNext = pItem->GetNext();
		pItem->OnHear( szText, pCharSrc );
	}
}

bool CChar::OnTickEquip( CItem* pItem )
{
	// A timer expired for an item we are carrying.
	// Does it periodically do something ?
	// REUTRN:
	//  false = delete it.

	switch ( pItem->GetEquipLayer())
	{
	case LAYER_FLAG_Wool:
		// This will regen the sheep it was sheered from.
		// Sheared sheep regen wool on a new day.
		if ( GetID() != CREID_Sheep_Sheered )
			return false;

		// Is it a new day ? regen my wool.
		SetID( CREID_Sheep );
		return false;

	case LAYER_FLAG_ClientLinger:
		// remove me from other clients screens.
		DEBUG_CHECK( pItem->IsType( IT_EQ_CLIENT_LINGER ));
		SetDisconnected();
		return( false );

	case LAYER_SPECIAL:
		switch ( pItem->GetType())
		{
		case IT_EQ_SCRIPT:	// pure script.
		case IT_EQ_MESSAGE:
		case IT_EQ_DIALOG:	// take down the dialog?
			break;
		case IT_EQ_SCRIPT_BOOK:
			// Make the next action in the script if it is running.
			ScriptBook_OnTick( PTR_CAST(CItemMessage,pItem ), false );
			return( true );
		case IT_EQ_MEMORY_OBJ:
			return Memory_OnTick( PTR_CAST(CItemMemory,pItem ));
		default:
			DEBUG_CHECK( 0 );	// should be no other types here.
			break;
		}
		break;

	case LAYER_FLAG_Stuck:
		// Only allow me to try to damage the web so often
		// Non-magical. held by something.
		// IT_EQ_STUCK
		pItem->SetTimeout( -1 );
		return( true );

	case LAYER_HORSE:
		// Give my horse a tick. (It is still in the game !)
		// NOTE: What if my horse dies (poisoned?)
		{
			CCharPtr pHorse = g_World.CharFind( pItem->m_itFigurine.m_uidChar );
			if ( pHorse == NULL )
				return( false );
			pItem->SetTimeout( 10* TICKS_PER_SEC );
			return( pHorse->OnTick());
		}

	case LAYER_FLAG_Murders:
		// decay the murder count. IT_EQ_MURDER_COUNT
		DEBUG_CHECK( m_pPlayer );
		if ( ! m_pPlayer || m_pPlayer->m_wMurders <= 0 )
			return( false );
		if ( ! IsDisconnected())
		{
			m_pPlayer->m_wMurders--;
			if ( m_pPlayer->m_wMurders == 0 )
				return( false );
		}
		pItem->m_itEqMurderCount.m_Decay_Balance = g_Cfg.m_iMurderDecayTime;
		pItem->SetTimeout( g_Cfg.m_iMurderDecayTime );	// update it's decay time.
		return( true );
	}

	if ( pItem->IsType( IT_SPELL ))
	{
		return Spell_Equip_OnTick(pItem);
	}

	return( pItem->OnTick());
}

void CChar::SleepWake()
{
	if ( ! IsStatFlag( STATF_Sleeping ))
		return;
	CItemCorpsePtr pCorpse = FindMyCorpse();
	if ( pCorpse != NULL )
	{
		RaiseCorpse(pCorpse);
		StatFlag_Clear( STATF_Sleeping );
		Update();	// update light levels etc.
	}
	else
	{
		m_StatHealth = 0;	// Death
	}
}

CItemCorpsePtr CChar::SleepStart( bool fFrontFall )
{
	if ( IsStatFlag( STATF_DEAD | STATF_Sleeping | STATF_Polymorph ))
		return NULL;

	StatFlag_Set( STATF_Sleeping );

	CItemCorpsePtr pBody = MakeCorpse( fFrontFall );
	if ( ! pBody )
	{
		WriteString( "Can't sleep here" );
		StatFlag_Clear( STATF_Sleeping );
		return NULL;
	}

	SetID( m_prev_id );
	StatFlag_Clear( STATF_Hidden );

	Update();
	return pBody;
}

CItemCorpsePtr CChar::MakeCorpse( bool fFrontFall )
{
	// Creature corpse,
	// Do death anim
	// Drop loot.
	// NOTE:
	// some creatures (Elems) have no corpses.
	// IsStatFlag( STATF_DEAD ) might NOT be set. (sleeping)

	bool fLootable = ! IsStatFlag( STATF_Conjured ); // conjured creatures take all equip with them.
	CCharDefPtr pCharDef = Char_GetDef();
	bool fHasCorpse = ! pCharDef->Can(CAN_C_NOCORPSE);
	if ( fHasCorpse )
	{
		switch (GetDispID())
		{
		case CREID_WATER_ELEM:
		case CREID_AIR_ELEM:
		case CREID_FIRE_ELEM:
		case CREID_VORTEX:
		case CREID_BLADES:
			fHasCorpse = false;
			break;
		}
	}

	int iDecayTime = -1;	// never default.
	CItemPtr pItem;
	CItemCorpsePtr pCorpse;

	if ( fLootable && fHasCorpse )
	{
		Horse_UnMount(); // If i'm conjured then my horse goes with me.

		pItem = CItem::CreateScript( ITEMID_CORPSE, this );
		pCorpse = REF_CAST(CItemCorpse,pItem);
		if ( pCorpse == NULL )	// Weird internal error !
		{
			DEBUG_CHECK(pCorpse);
			pItem->DeleteThis();
			goto nocorpse;
		}

		CGString sName;
		sName.Format( "Body of %s", (LPCTSTR) GetName());
		pCorpse->SetName( sName );
		pCorpse->SetHue( GetHue());
		pCorpse->SetCorpseType( GetDispID());
		pCorpse->m_itCorpse.m_BaseID = GetID();	// id the corpse type here !
		pCorpse->m_itCorpse.m_facing_dir = m_dirFace;
		pCorpse->SetAttr(ATTR_INVIS);	// Don't display til ready.

		if ( IsStatFlag( STATF_DEAD ))
		{
			pCorpse->m_itCorpse.m_timeDeath.InitTimeCurrent();	// death time.
			pCorpse->m_itCorpse.m_uidKiller = m_Act.m_Targ;
			iDecayTime = (m_pPlayer.IsValidNewObj()) ?
			g_Cfg.m_iDecay_CorpsePlayer : g_Cfg.m_iDecay_CorpseNPC;
		}
		else	// Sleeping
		{
			pCorpse->m_itCorpse.m_timeDeath.InitTime();	// Not dead.
			pCorpse->m_itCorpse.m_uidKiller = GetUID();
			iDecayTime = -1;	// never
		}

		if ( IsRespawned())	// not being deleted.
		{
			pCorpse->m_uidLink = GetUID();
		}
	}
	else
	{
	nocorpse:
		// Some creatures can never sleep. (not corpse)
		if ( ! IsStatFlag( STATF_DEAD ))
			return( NULL );
		if ( IsHuman())
			return( NULL );	// conjured humans just disapear.

		pItem = CItem::CreateScript( ITEMID_FX_SPELL_FAIL, this );
		ASSERT(pItem);
		pItem->MoveToDecay( GetTopPoint(), 2*TICKS_PER_SEC );
	}

	// can fall forward.
	DIR_TYPE dir = m_dirFace;
	if ( fFrontFall )
	{
		dir = (DIR_TYPE) ( dir | 0x80 );
		if ( pCorpse )
		{
			pCorpse->m_itCorpse.m_facing_dir = dir;
		}
	}

	// Death anim. default is to fall backwards. lie face up.
	CUOCommand cmd;
	cmd.CharDeath.m_Cmd = XCMD_CharDeath;
	cmd.CharDeath.m_UID = GetUID();	// 1-4
	cmd.CharDeath.m_UIDCorpse = ( pCorpse == NULL ) ? 0 : (DWORD) pCorpse->GetUID(); // 9-12
	cmd.CharDeath.m_DeathFlight = IsStatFlag( STATF_Fly ) ? 1 : 0; 	// Die in flight ?
	cmd.CharDeath.m_Death2Anim = ( dir & 0x80 ) ? 1 : 0; // Fore/Back Death ?

	UpdateCanSee( &cmd, sizeof( cmd.CharDeath ), m_pClient );

	// Move non-newbie contents of the pack to corpse. (before it is displayed)
	if ( fLootable )
	{
		DropAll( pCorpse );
	}
	if ( pCorpse )
	{
		pCorpse->ClrAttr(ATTR_INVIS);	// make visible.
		pCorpse->MoveToDecay( GetTopPoint(), iDecayTime );
	}

	return( pCorpse );
}

bool CChar::RaiseCorpse( CItemCorpse* pCorpse )
{
	// We are creating a char from the current char and the corpse.
	// Move the items from the corpse back onto us.

	// If NPC is disconnected then reconnect them.
	// If the player is off line then don't allow this !!!

	ASSERT(pCorpse);
	if ( pCorpse->GetCount())
	{
		CItemContainerPtr pPack = GetPackSafe();
		CItemPtr pItemNext;
		for ( CItemPtr pItem = pCorpse->GetHead(); pItem!=NULL; pItem=pItemNext )
		{
			pItemNext = pItem->GetNext();
			if ( pItem->IsType( IT_HAIR ) ||
				pItem->IsType( IT_BEARD ) ||
				pItem->IsAttr( ATTR_MOVE_NEVER ))
				continue;	// Hair on corpse was copied!
			// Redress if equipped.
			if ( pItem->GetContainedLayer())
				ItemEquip( pItem );	// Equip the item.
			else
				pPack->ContentAdd( pItem );	// Toss into pack.
		}

		// Any items left just dump on the ground.
		pCorpse->ContentsDump( GetTopPoint());
	}

	if ( pCorpse->IsTopLevel() || pCorpse->IsItemInContainer())
	{
		// I should move to where my corpse is just in case.
		MoveToChar( pCorpse->GetTopLevelObj()->GetTopPoint());
	}

	// Corpse is now gone. 	// 0x80 = on face.
	SetID( pCorpse->m_itCorpse.m_BaseID );
	Update();
	UpdateDir( (DIR_TYPE)( pCorpse->m_itCorpse.m_facing_dir &~ 0x80 ));
	UpdateAnimate( ( pCorpse->m_itCorpse.m_facing_dir & 0x80 ) ? ANIM_DIE_FORWARD : ANIM_DIE_BACK, true, true, 2 );

	pCorpse->DeleteThis();

	return( true );
}

bool CChar::Death()
{
	// RETURN: false = delete

	if ( IsStatFlag( STATF_DEAD|STATF_INVUL ))
		return true;

	{
	CSphereExpContext exec( this, this );
	if ( OnTrigger( CCharDef::T_Death, exec) == TRIGRET_RET_VAL )
		return( true );
	}

	if ( IsClient())	// Prevents crashing ?
	{
		GetClient()->addPause();
	}

	// I am dead and we need to give credit for the kill to my attacker(s).
	TCHAR* pszKillStr = Str_GetTemp();
	int iKillStrLen = sprintf( pszKillStr, "%c'%s' was killed by ",
		(m_pPlayer.IsValidNewObj())?'P':'N', (LPCTSTR) GetName() );
	int iKillers = 0;

	// Look through my memories of who i was fighting. (make sure they knew they where fighting me)
	CItemPtr pItemNext;
	CItemPtr pItem=GetHead();
	for ( ; pItem; pItem=pItemNext )
	{
		pItemNext = pItem->GetNext();
		if ( ! pItem->IsMemoryTypes( MEMORY_HARMEDBY | MEMORY_AGGREIVED )) // i was harmed in some way but this person.
			continue;

		CItemMemoryPtr pMemory = REF_CAST(CItemMemory,pItem);
		ASSERT(pMemory);
		CCharPtr pKiller = g_World.CharFind( pMemory->m_uidLink );
		if ( pKiller != NULL )
		{
			// NOTE: pMemory->GetMotivation() is how much damage i did. (sort of)
			pKiller->Noto_Kill( this, false );

			iKillStrLen += sprintf( pszKillStr+iKillStrLen, "%s%c'%s'",
				iKillers ? ", " : "",
				(pKiller->m_pPlayer.IsValidNewObj())?'P':'N', pKiller->GetName() );
			iKillers ++;
		}

		Memory_ClearTypes( pMemory, 0xFFFF ^ MEMORY_FIGHT );
	}

	// record the kill event for posterity.

	iKillStrLen += sprintf( pszKillStr+iKillStrLen, ( iKillers ) ? "." LOG_CR : "accident." LOG_CR );
	g_Log.Event( LOG_GROUP_KILLS, (m_pPlayer.IsValidNewObj()) ? LOGL_EVENT : LOGL_TRACE, pszKillStr );

	if ( m_pParty )
	{
		m_pParty->SysMessageAll( pszKillStr );
	}

	NPC_PetClearOwners();	// Forgot who owns me. dismount my master if ridden.
	Reveal();
	SoundChar( CRESND_DIE );
	Spell_Effect_Dispel(100);		// Get rid of all spell effects.

	// Only players should loose fame upon death.
	if ( m_pPlayer.IsValidNewObj())
	{
		m_pPlayer->m_wDeaths++;
		Noto_Fame( -Stat_Get(STAT_Fame)/10 );
	}

	// create the corpse item.
	StatFlag_Set( STATF_DEAD );
	StatFlag_Clear( STATF_Stone | STATF_Freeze | STATF_Hidden | STATF_Sleeping );

	// This can return NULL
	CItemCorpsePtr pCorpse = MakeCorpse( Calc_GetRandVal(2));

	m_StatHealth = 0;	// on my way to death.

	if ( m_pPlayer.IsValidNewObj())
	{
		SetHue( HUE_DEFAULT );	// Get all pale.

		LPCTSTR pszGhostName;
		CCharDefPtr pCharDefPrev = g_Cfg.FindCharDef( m_prev_id );
		if ( pCharDefPrev )
		{
			pszGhostName = ( pCharDefPrev->IsFemale()) ? "c_GHOST_WOMAN" : "c_GHOST_MAN";
		}
		else
		{
			pszGhostName = "c_GHOST_MAN";
		}

		SetID( (CREID_TYPE) g_Cfg.ResourceGetIndexType( RES_CharDef, pszGhostName ));
		LayerAdd( CItem::CreateScript( ITEMID_DEATHSHROUD, this ), LAYER_QTY );
		Update();		// show everyone I am now a ghost.

		// Manifest the ghost War mode for ghosts.
		if ( ! IsStatFlag( STATF_War ))
		{
			StatFlag_Set( STATF_Insubstantial );
		}
	}

	// script could generate new loot ?
	{
	CSphereExpArgs exec( this, this, pCorpse );
	OnTrigger( CCharDef::T_DeathCorpse, exec );
	}

	if ( IsClient())
	{
		// Put up the death menu.
		CUOCommand cmd;
		cmd.DeathMenu.m_Cmd = XCMD_DeathMenu;
		cmd.DeathMenu.m_shift = 0;
		m_pClient->xSendPkt( &cmd, sizeof(cmd.DeathMenu));

		// We are now in "death menu mode"
		m_pClient->SetTargMode(CLIMODE_DEATH);
		m_pClient->m_Targ.m_pt = GetTopPoint();	// Insta res takes us back to where we died.
		return( true );
	}
	if ( m_pPlayer.IsValidNewObj())	// a logged out player.
	{
		SetDisconnected();	// Respawn the NPC later
		return( true );
	}

	// Some NPC.

	if ( IsRespawned())
	{
		SetDisconnected();	// Respawn the NPC later
		return true;
	}

	if ( pCorpse && pCorpse->m_uidLink == GetUID())
	{
		pCorpse->m_uidLink.InitUID();	// Makes no sense to link the corpse to something that is not going to be valid.
	}

	return false;	// must delete this now !
}

void CChar::Flip( LPCTSTR pCmd )
{
	UpdateDir( GetDirTurn( m_dirFace, 1 ));
}

bool CChar::OnFreezeCheck( int iDmg )
{
	// We are trying to move, Check why why are held in place.
	// Can we break free ?
	// RETURN: true = held in place.

	bool fGM = IsGM();

	DEBUG_CHECK( IsStatFlag( STATF_Stone | STATF_Freeze | STATF_Immobile ));

	CItemPtr pFlag = LayerFind( LAYER_FLAG_Stuck );
	if ( pFlag == NULL )	// stuck for some other reason i guess.
	{
		const char* pszMsg;
		if ( IsStatFlag( STATF_Sleeping ))
			pszMsg = "You are unconscious and can't move.";
		else if ( IsStatFlag( STATF_Immobile ))
			pszMsg = "You are stuck and can't move.";
		else // STATF_Stone | STATF_Freeze
			pszMsg = "You are frozen and can not move.";
		WriteString(pszMsg);	// some sort of magic ?
	}
	else
	{
		// IT_EQ_STUCK - STATF_Immobile
		CItemPtr pWeb = g_World.ItemFind(pFlag->m_uidLink);
		if ( pWeb == NULL ||
			! pWeb->IsTopLevel() ||
			pWeb->GetTopPoint() != GetTopPoint())
		{
			// Maybe we teleported away ?
			pFlag->DeleteThis();
			return( false );
		}

		// Only allow me to try to damage it once per sec.
		if ( ! pFlag->IsTimerSet())
		{
			return( Use_Obj( pWeb, false ));	// iDmg ?
		}
	}

	return( ! fGM);
}

CRegionPtr CChar::CheckMoveWalkDir( CPointMapBase& ptDst, DIR_TYPE dir, bool fCheckChars )
{
	// For both Players and NPC's
	// Walk towards this point as best we can.
	// NOTE:
	//   Affect stamina as if we WILL move ! (if we can)
	// RETURN:
	//  ptDst = the new z and location
	//  NULL = failed to walk here.

	if ( IsStatFlag( STATF_Freeze | STATF_Stone | STATF_Immobile ) && OnFreezeCheck(0))
	{
		// NPC's would call here.
		return( NULL );	// can't move.
	}

	if ( m_StatStam <= 0 && ! IsStatFlag( STATF_DEAD ))
	{
		WriteString( "You are too fatigued to move." );
		return( NULL );
	}

	int iWeightLoadPercent = GetWeightLoadPercent( GetTotalWeight());
	if ( iWeightLoadPercent > 200 )
	{
		WriteString( "You are too overloaded to move." );
		return( NULL );
	}

	ptDst.Move(dir);

	if ( m_zClimbHeight != SPHEREMAP_SIZE_Z && dir == m_dirClimb )
	{
		// I was climbing stairs it seems.
		ASSERT( m_zClimbHeight > SPHEREMAP_SIZE_MIN_Z );
		ptDst.m_z = m_zClimbHeight;
		// DEBUG_MSG(( "ClimbOff dir=%d,height=%d" LOG_CR, m_dirClimb, m_zClimbHeight ));
	}

	// ok to go here ? physical blocking objects ?
	CMulMapBlockState block;
	CRegionPtr pArea = CheckValidMove( ptDst, block );
	if ( pArea == NULL )
		return( NULL );

	if ( ! m_pPlayer )
	{
		// Does the NPC want to walk here ?
		if ( ! NPC_CheckWalkHere( ptDst, pArea, block.IsResultCovered()))
			return( NULL );
	}

	if ( IsStatFlag(STATF_OnHorse) &&
		block.IsResultCovered() &&
		g_Cfg.m_iMountHeight &&
		! pArea->IsFlag(REGION_FLAG_UNDERGROUND) &&	// but ok in dungeons
		! IsGM())
	{
		WriteString( "The ceiling is too low for you to be mounted!" );
		return( NULL );
	}

	// Bump into other creatures ?
	if ( ! IsStatFlag( STATF_DEAD | STATF_Sleeping | STATF_Insubstantial ) && fCheckChars )
	{
		CWorldSearch AreaChars( ptDst );
		AreaChars.SetFilterAllShow( true );	// show logged out chars.
		for(;;)
		{
			CCharPtr pChar = AreaChars.GetNextChar();
			if ( pChar == NULL )
				break;
			if ( pChar == this )
				continue;
			if ( pChar->GetTopZ() != ptDst.m_z )
				continue;
			if ( ! ptDst.IsSameMapPlane( pChar->GetTopMap()))
				continue;

			if ( ! m_pPlayer.IsValidNewObj())
				return( NULL ); // NPC's can't bump thru. unless mad ?

			if ( pChar->IsStatFlag( STATF_DEAD | STATF_Insubstantial ) ||
				pChar->IsDisconnected())
			{
				if ( CanDisturb(pChar) && pChar->IsStatFlag(STATF_SpiritSpeak))
				{
					WriteString( "You feel a tingling sensation" );
				}
				continue;
			}

			// How much stamina to push past ?
			int iStamReq = g_Cfg.Calc_WalkThroughChar( this, pChar );

			CGString sMsg;
			if ( pChar->IsStatFlag( STATF_Invisible ))
			{
				sMsg = "You push past something invisible";
				continue;
			}
			else if ( pChar->IsStatFlag( STATF_Hidden ))
			{
				// reveal hidden people ?
				sMsg.Format( "You stumble apon %s hidden.", (LPCTSTR) pChar->GetName());
				pChar->Reveal(STATF_Hidden);
			}
			else if ( pChar->IsStatFlag( STATF_Sleeping ))
			{
				sMsg.Format( "You step on the body of %s.", (LPCTSTR) pChar->GetName());
				// ??? damage.
			}
			else if ( iStamReq < 0 || m_StatStam < iStamReq + 1 )
			{
				sMsg.Format( "You are not strong enough to push %s out of the way.", (LPCTSTR) pChar->GetName());
				if ( ! IsGM())
				{
					WriteString( sMsg );
					return NULL;
				}
			}
			else
			{
				// ??? What is the true amount of stamina lost ?
				sMsg.Format( "You shove %s out of the way.", (LPCTSTR) pChar->GetName());
			}

			WriteString( sMsg );
			if ( IsGM())
			{
				// client deducts stam for us. make sure this is fixed.
				Stat_Change( STAT_Stam, 10 );
			}
			else
			{
				if ( iStamReq < 0 )		// this really should not happen.
					iStamReq = 0;
				Stat_Change( STAT_Stam, -iStamReq );
			}

			{
			CSphereExpContext exec( pChar, this );
			pChar->OnTrigger( CCharDef::T_PersonalSpace, exec);
			}
			break;
		}
	}

	// decrease stamina if running or overloaded.

	if ( ! IsGM() && ! IsStatFlag( STATF_DEAD ) )  // Don't decrease stamina if we are dead.  LOL
	{
		// We are overloaded. reduce our stamina faster.
		// Running acts like an increased load.
		int iStamReq = g_Cfg.Calc_DropStamWhileMoving( this, iWeightLoadPercent );
		if ( iStamReq )
		{
			// Lower avail stamina.
			Stat_Change( STAT_Stam, -iStamReq );
		}
	}

	m_zClimbHeight = SPHEREMAP_SIZE_Z;
	if ( block.m_Bottom.m_BlockFlags & CAN_I_CLIMB )
	{
		// Get Direction of the stairs.
		ITEMID_TYPE id = (ITEMID_TYPE)( block.m_Bottom.m_wTile - TERRAIN_QTY );
		CItemDefPtr pItemDef = g_Cfg.FindItemDef(id);
		if ( pItemDef && pItemDef->GetHeight() > 1 )
		{
			//DEBUG_CHECK( pItemDef->GetFlags() & UFLAG2_CLIMBABLE );
			m_dirClimb = CUOItemTypeRec::GetClimbableDir(pItemDef->GetFlags());
			// Get height at the top of the stairs.
			// standing position is half way up climbable items.
			m_zClimbHeight = ptDst.m_z + ( pItemDef->GetHeight() / 2 );	
			// Make sure I don't roll this over
			if ( m_zClimbHeight < SPHEREMAP_SIZE_MIN_Z || m_zClimbHeight > SPHEREMAP_SIZE_Z )
				m_zClimbHeight = SPHEREMAP_SIZE_Z;

			// DEBUG_MSG(( "ClimbOn dir=%d,height=%d,%d" LOG_CR, m_dirClimb, ptDst.m_z, m_zClimbHeight ));
		}
	}

	StatFlag_Mod( STATF_InDoors, block.IsResultCovered() || pArea->IsFlag(REGION_FLAG_UNDERGROUND));
	return( pArea );
}

void CChar::CheckRevealOnMove()
{
	// Are we going to reveal ourselves by moving ?
	if ( IsStatFlag( STATF_Invisible | STATF_Hidden | STATF_Sleeping ))
	{
		// Wake up if sleeping and this is possible.
		if ( IsStatFlag( STATF_Fly ) ||
			! IsStatFlag( STATF_Hidden ) ||
			IsStatFlag( STATF_Sleeping ) ||
			! Skill_UseQuick( SKILL_Stealth, Calc_GetRandVal( 105 )))
		{
			// check hiding again ? unless running ?
			Reveal();
		}
	}
}

bool CChar::CheckLocation( bool fStanding )
{
	// We are at this location
	// what will happen ?
	// RETURN: true = we teleported.

	if ( ! fStanding )
	{
		// Are we using a skill that is effected by motion ?
		switch ( Skill_GetActive())
		{
		case SKILL_MEDITATION:
		case SKILL_NECROMANCY:
		case SKILL_MAGERY:
			// Skill is broken if we move ?
			break;
		case SKILL_HIDING:	// this should become stealth ?
			break;
		case SKILL_ARCHERY:
			// If we moved and are wielding are in combat and are using a
			// crossbow/bow kind of weapon, then reset the weaponswingtimer.
			Fight_ResetWeaponSwingTimer();
			break;
		}

		// This could get REALLY EXPENSIVE !
		if ( m_pArea->OnRegionTrigger( this, CRegionType::T_Step ) == TRIGRET_RET_VAL )
			return( false );
	}

	CPointMap ptTop = GetTopPoint();
	CWorldSearch AreaItems( ptTop );
	for(;;)
	{
		CItemPtr pItem = AreaItems.GetNextItem();
		if ( pItem == NULL )
			break;

		int zdiff = pItem->GetTopZ() - GetTopZ();
		if ( ABS(zdiff) > 3 )
			continue;

		{
		CSphereExpArgs execArgs( pItem, this, (int) fStanding );
		if ( pItem->OnTrigger( CItemDef::T_Step, execArgs ) == TRIGRET_RET_VAL )
			continue;
		}

		switch ( pItem->GetType())
		{
		case IT_SHRINE:
			// Resurrect the ghost
			if ( fStanding )
				continue;
			OnSpellEffect( SPELL_Resurrection, this, 1000, pItem );
			return( false );
		case IT_WEB:
			if ( fStanding )
				continue;
			// Caught in a web.
			if ( Use_Item_Web( pItem ))
				return( true );
			continue;
		case IT_DREAM_GATE:
			// Move to another server by name
			if ( fStanding )
				continue;
			if ( IsClient())
			{
				CItemServerGate* pItemGate = REF_CAST(CItemServerGate,pItem);
				DEBUG_CHECK(pItemGate);
				if ( pItemGate == NULL )
					continue;
				CServerPtr pServ = pItemGate->GetServerRef();
				if ( pServ == NULL )
					continue;
				// Transfer our current char to the other server.
				return pServ->QueueCharToServer( this );
			}
			return( true );
			// case IT_CAMPFIRE:	// does nothing. standing on burning kindling shouldn't hurt us
		case IT_FIRE:
			// fire object hurts us ?
			// pItem->m_itSpell.m_spelllevel = 0-1000 = heat level.
			{
				int iSkillLevel = pItem->m_itSpell.m_spelllevel/2;
				iSkillLevel = iSkillLevel + Calc_GetRandVal(iSkillLevel);
				if ( IsStatFlag( STATF_Fly ))	// run through fire.
				{
					iSkillLevel /= 2;
				}
				OnTakeDamage( g_Cfg.GetSpellEffect( SPELL_Fire_Field, iSkillLevel ), NULL, DAMAGE_FIRE | DAMAGE_GENERAL );
			}
			Sound( 0x15f ); // Fire noise.
			return( false );
		case IT_SPELL:
			{
				SPELL_TYPE Spell = (SPELL_TYPE) RES_GET_INDEX(pItem->m_itSpell.m_spell);
				OnSpellEffect( Spell,
					g_World.CharFind(pItem->m_uidLink),
					pItem->m_itSpell.m_spelllevel, pItem );
				CSpellDefPtr pSpellDef = g_Cfg.GetSpellDef(Spell);
				if ( pSpellDef )
				{
					Sound( pSpellDef->m_sound);
				}
			}
			return( false );
		case IT_TRAP:
		case IT_TRAP_ACTIVE:
			// case IT_TRAP_INACTIVE: // reactive it?
			OnTakeDamage( pItem->Use_Trap(), NULL, DAMAGE_HIT_BLUNT | DAMAGE_GENERAL );
			return( false );
		case IT_SWITCH:
			if ( pItem->m_itSwitch.m_fStep )
			{
				Use_Item( pItem );
			}
			return( false );
		case IT_MOONGATE:
		case IT_TELEPAD:
			if ( fStanding )
				continue;
			Use_MoonGate( pItem );
			return( true );
		case IT_SHIP_PLANK:
			// a plank is a teleporter off the ship.
			if ( ! fStanding && ! IsStatFlag( STATF_Fly ))
			{
				// Find some place to go. (in direction of plank)
				if ( MoveToValidSpot( m_dirFace, SPHEREMAP_VIEW_SIZE, 1 ))
				{
					pItem->SetTimeout( 5*TICKS_PER_SEC );	// autoclose it behind us.
					return( true );
				}
			}
			continue;

		case IT_ADVANCE_GATE:
			// Upgrade the player to the skills of the selected NPC script.
			if ( fStanding )
				continue;
			Use_AdvanceGate( pItem );
			break;
		}
	}

	if ( fStanding )
		return( false );

	{
	CSphereExpContext exec( this, this );
	if ( OnTrigger( CCharDef::T_Step, exec ) == TRIGRET_RET_VAL )
		return true;
	}

	// Check the map teleporters in this CSector. (if any)
	CSectorPtr pSector = ptTop.GetSector();
	ASSERT(pSector);
	CTeleportPtr pTel = pSector->GetTeleport(ptTop);
	if ( pTel )
	{
		// Only do this for clients
		if ( IsClient())
		{
			Spell_Effect_Teleport( pTel->m_ptDst, true, false, ITEMID_NOTHING );
			return( true );
		}
	}
	return( false );
}

bool CChar::MoveToRegion( CRegionComplex* pNewArea, bool fAllowReject )
{
	// Moving to a new region. or logging out (not in any region)
	// pNewArea == NULL = we are logging out.
	// RETURN:
	//  false = do not allow in this area.

#ifdef _DEBUG
	if ( pNewArea )
	{
		DEBUG_CHECK( pNewArea->IsValid());
	}
	if ( m_pArea.IsValidRefObj() )
	{
		DEBUG_CHECK( m_pArea->IsValid());
	}
#endif

	if ( m_pArea == pNewArea ) // same as old area ?
		return true;

	if ( ! g_Serv.IsLoading() && fAllowReject )
	{
		if ( IsGM())
		{
			fAllowReject = false;
		}

		// Leaving region trigger. (may not be allowed to leave ?)
		if ( m_pArea )
		{
			if ( m_pArea->OnRegionTrigger( this, CRegionType::T_Exit ) == TRIGRET_RET_VAL )
			{
				if ( pNewArea && fAllowReject )
					return false;
			}
		}

		if ( IsClient() && pNewArea )
		{
			if ( pNewArea->IsFlag( REGION_FLAG_ANNOUNCE ) &&
				! pNewArea->IsPointInside( GetTopPoint()))	// new area.
			{
				CVarDefPtr pVar = pNewArea->m_TagDefs.FindKeyPtr("ANNOUNCEMENT");
				Printf( "You have entered %s", (pVar) ? (LPCTSTR) pVar->GetPSTR() :
					(LPCTSTR) pNewArea->GetName());
			}

			// Is it guarded ?
			else if ( m_pArea && ! IsStatFlag( STATF_DEAD ))
			{
				if ( pNewArea->IsFlagGuarded() != m_pArea->IsFlagGuarded())
				{
					if ( pNewArea->IsFlagGuarded() )
					{
						CVarDefPtr pVar = pNewArea->m_TagDefs.FindKeyPtr("GUARDOWNER");
						Printf( "You are now under the protection of %s guards",
							(pVar) ? (LPCTSTR)pVar->GetPSTR() : "the");
					}
					else
					{
						CVarDefPtr pVar = m_pArea->m_TagDefs.FindKeyPtr("GUARDOWNER");
						Printf( "You have left the protection of %s guards",
							(pVar) ? (LPCTSTR)pVar->GetPSTR() : "the" );
					}
				}
				if ( pNewArea->IsFlag(REGION_FLAG_NO_PVP) != m_pArea->IsFlag(REGION_FLAG_NO_PVP))
				{
					WriteString(( pNewArea->IsFlag(REGION_FLAG_NO_PVP)) ?
						"You are safe from other players here." :
						"You lose your safety from other players." );
				}
				if ( pNewArea->IsFlag(REGION_FLAG_SAFE) != m_pArea->IsFlag(REGION_FLAG_SAFE))
				{
					WriteString(( pNewArea->IsFlag(REGION_FLAG_SAFE)) ?
						"You have a feeling of complete safety" :
						"You lose your feeling of safety" );
				}
			}
		}

		// Entering region trigger.
		if ( pNewArea )
		{
			if ( pNewArea->OnRegionTrigger( this, CRegionType::T_Enter ) == TRIGRET_RET_VAL )
			{
				if ( m_pArea && fAllowReject )
					return false;
			}
		}
	}

	m_pArea = pNewArea;
	return( true );
}

bool CChar::MoveTo( CPointMap pt )
{
	// Same as MoveToChar but keep this for virtual support.
	// This could be us just taking a step or being teleported.
	// Low level: DOES NOT UPDATE DISPLAYS or container flags. (may be offline)
	// This does not check for gravity.
	//

	if ( ! pt.IsCharValid())
		return false;

	// Can this client exist in the map plane ? protect older clients.
	if ( m_pPlayer )
	{
		if ( IsClient())
		{
			CMulMapPtr pMap = pt.GetMulMap();
			ASSERT(pMap);
			if ( pMap->m_iResourceLevel > GetClient()->m_iClientResourceLevel )
			{
				return false;
			}
		}
		else if ( IsDisconnected())	// moving a logged out cchar !
		{
			// moving a logged out client !
			// We cannot put this char in non-disconnect state.
			pt.GetSector()->m_Chars_Disconnect.InsertHead( this );
			SetUnkPoint( pt );
			return true;
		}
	}

	// Did we step into a new region ?
	// Do enter region triggers here.
	CRegionComplexPtr pAreaNew = REF_CAST(CRegionComplex,pt.GetRegion( REGION_TYPE_MULTI|REGION_TYPE_AREA ));
	if ( ! MoveToRegion( pAreaNew, true ))
		return false;

	DEBUG_CHECK( m_pArea );

	bool fSectorChange = pt.GetSector()->MoveCharToSector( this );

	// Move this item to it's point in the world. (ground or top level)
	SetTopPoint( pt );

	if ( fSectorChange && ! g_Serv.IsLoading())	// there was a change in environment.
	{
		CSphereExpContext exec( this, this );
		OnTrigger( CCharDef::T_EnvironChange, exec);
	}

#ifdef _DEBUG
	int iRetWeird = CObjBase::IsWeird();
	if ( iRetWeird )
	{
		DEBUG_CHECK( ! iRetWeird );
	}
#endif

	return true;
}

bool CChar::MoveToValidSpot( DIR_TYPE dir, int iDist, int iDistStart )
{
	// Move from here to a valid spot.
	// ASSUME "here" is not a valid spot. (even if it really is)

	CPointMap pt = GetTopPoint();
	pt.MoveN( dir, iDistStart );
	pt.m_z += PLAYER_HEIGHT;

	CAN_TYPE wCanFlags = GetMoveCanFlags();	// CAN_C_SWIM
	for ( int i=0; i<iDist; i++ )
	{
		CMulMapBlockState block( wCanFlags );
		g_World.GetHeightPoint( pt, block, pt.GetRegion( REGION_TYPE_MULTI ));
		if ( ! block.IsResultBlocked())
		{
			// we can go here. (maybe)
			pt.m_z = block.GetResultZ();
			if ( Spell_Effect_Teleport( pt, true, true, ITEMID_NOTHING ))
				return( true );
		}
		pt.Move( dir );
	}
	return( false );
}

bool CChar::SetPrivLevel( CScriptConsole* pSrc, LPCTSTR pszFlags )
{
	// "PRIVSET"
	// Set this char to be a GM etc. (or take this away)
	// NOTE: They can be off-line at the time.

	ASSERT(pSrc);
	ASSERT(pszFlags);

	if ( pSrc->GetPrivLevel() < PLEVEL_Admin )	// Only an admin can do this.
		return( false );
	if ( pSrc->GetPrivLevel() < GetPrivLevel())	// priv is better than mine ?
		return( false );
	if ( pszFlags[0] == '\0' )
		return( false );
	if ( ! m_pPlayer )
		return false;

	CAccountPtr pAccount = GetAccount();
	ASSERT(pAccount);

	PLEVEL_TYPE PrivLevel = CAccount::GetPrivLevelText( pszFlags );

	// Remove Previous GM Robe
	ContentConsume( CSphereUID(RES_ItemDef,ITEMID_GM_ROBE), INT_MAX );

	if ( PrivLevel >= PLEVEL_Counsel )
	{
		// Give gm robe.
		CItemPtr pItem = CItem::CreateScript( ITEMID_GM_ROBE, this );
		ASSERT(pItem);
		pItem->SetAttr( ATTR_MOVE_NEVER | ATTR_NEWBIE | ATTR_MAGIC );
		pItem->m_itArmor.m_spelllevel = 1000;

		pAccount->SetPrivFlags( PRIV_GM_PAGE );
		if ( PrivLevel >= PLEVEL_GM )
		{
			pAccount->SetPrivFlags( PRIV_GM );
			pItem->SetHue( HUE_RED );
		}
		else
		{
			pItem->SetHue( HUE_BLUE_NAVY );
		}
		UnEquipAllItems();
		ItemEquip( pItem );
	}
	else
	{
		// Revoke GM status.
		pAccount->ClearPrivFlags( PRIV_GM_PAGE|PRIV_GM );
	}

	if ( pAccount->GetPrivLevel() < PLEVEL_Admin && PrivLevel < PLEVEL_Admin )	// can't demote admin this way.
	{
		pAccount->SetPrivLevel( PrivLevel );
	}

	Update();
	return true;
}

TRIGRET_TYPE CChar::OnTrigger( LPCTSTR pszTrigName, CScriptExecContext& exec )
{
	// Attach some trigger to the cchar. (PC or NPC)
	// RETURN: true = block further action.

	CCharDef::T_TYPE_ iAction;
	if ( ISINTRESOURCE(pszTrigName))
	{
		iAction = (CCharDef::T_TYPE_) GETINTRESOURCE(pszTrigName);
		pszTrigName = CCharDef::sm_Triggers[iAction].m_pszName;
	}
	else
	{
		iAction = (CCharDef::T_TYPE_) s_FindKeyInTable( pszTrigName, CCharDef::sm_Triggers );
	}

	// Based on the body type ?
	// RES_CharDef
	exec.SetBaseObject(this);
	CCharDefPtr pCharDef = Char_GetDef();
	ASSERT(pCharDef);
	TRIGRET_TYPE iRet = pCharDef->OnTriggerScript( exec, iAction, pszTrigName );
	if ( iRet != TRIGRET_RET_FALSE )
	{
		return( iRet );	// Block further action.
	}

	//
	// Go thru the event blocks for the NPC/PC to do events.
	//
	int i=0;	
	for (; i<m_Events.GetSize(); i++ )
	{
		// RES_Events
		CResourceTrigPtr pLink = REF_CAST(CResourceTriggered,m_Events[i]);
		if ( pLink == NULL )
			continue;
		if ( RES_GET_TYPE(pLink->GetUIDIndex()) != RES_Events && 
			RES_GET_TYPE(pLink->GetUIDIndex()) != RES_Profession )
			continue;
		iRet = pLink->OnTriggerScript( exec, iAction, pszTrigName );
		if ( iRet != TRIGRET_RET_FALSE && iRet != TRIGRET_RET_DEFAULT )
			return iRet;
	}

	if ( m_pNPC )	// newbie script may have both NULL !!
	{
		// RES_Events
		for ( i=0; i<pCharDef->m_Events.GetSize(); i++ )
		{
			CResourceTrigPtr pLink = REF_CAST(CResourceTriggered,pCharDef->m_Events[i]);
			if (pLink==NULL)
				continue;
			if ( RES_GET_TYPE(pLink->GetUIDIndex()) != RES_Events &&
				RES_GET_TYPE(pLink->GetUIDIndex()) != RES_Profession )
				continue;
			TRIGRET_TYPE iRet = pLink->OnTriggerScript( exec, iAction, pszTrigName );
			if ( iRet != TRIGRET_RET_FALSE && iRet != TRIGRET_RET_DEFAULT )
				return iRet;
		}
	}

	return( TRIGRET_RET_DEFAULT );
}

void CChar::OnTickFood()
{
	if ( IsStatFlag( STATF_Conjured ))
		return;	// No need for food.

	CCharDefPtr pCharDef = Char_GetDef();
	ASSERT(pCharDef);

	if ( pCharDef->m_MaxFood == 0 )	// No need for food.
	{
		// This may be money instead of food
		NPC_OnTickPetStatus(100);
		return;
	}

	if ( m_StatFood > 0 )
	{
		m_StatFood --;
	}
	else
	{
		m_StatFood ++;
	}

	int  nFoodLevel = Food_GetLevelPercent();

	NPC_OnTickPetStatus(nFoodLevel);

	if ( nFoodLevel < 40 )	// start looking for food at 40%
	{
		// Tell everyone we look hungry.

		Printf( "You are %s", (LPCTSTR) Food_GetLevelMessage( false, false ));

		if ( ! m_pPlayer )
		{
			NPC_OnTickFood( nFoodLevel );
		}
	}
}

bool CChar::OnTick()
{
	// Assume this is only called 1 time per sec.
	// Get a timer tick when our timer expires.
	// RETURN: false = delete this.

	int iTimeDiff = m_timeLastStatRegen.GetCacheAge();
	if ( iTimeDiff <= 0 )
	{
		// DEBUG_CHECK( iTimeDiff > 0 ); // ??? I have no idea why this happens. but it does
		return true;
	}

	if ( iTimeDiff >= TICKS_PER_SEC )	// don't bother with < 1 sec times.
	{
		// decay equipped items (spells)
		CItemPtr pItemNext;
		CItemPtr pItem=GetHead();
		for ( ; pItem!=NULL; pItem=pItemNext )
		{
			pItemNext = pItem->GetNext();

			if ( pItem->IsType(IT_EQ_MEMORY_OBJ))
			{
				// always check the validity of the memory objects. (even before expired)
				if ( g_World.ObjFind(pItem->m_uidLink) == NULL )
				{
					pItem->DeleteThis();
					continue;
				}
			}

			if ( ! pItem->IsTimerSet())
				continue;
			if ( ! pItem->IsTimerExpired())
				continue;
			if ( ! OnTickEquip( pItem ))
			{
				pItem->DeleteThis();
				if ( ! IsValidUID())
					return false;	// i've been deleted.
			}
		}

		// Players have a silly "always run" flag that gets stuck on.
		if ( IsClient())
		{
			int iTimeLastEvent = GetClient()->m_timeLastEvent.GetCacheAge();
			if ( iTimeLastEvent > TICKS_PER_SEC )
			{
				StatFlag_Clear(STATF_Fly); // we are not really running now are we.
			}
		}

		// NOTE: Summon flags can kill our hp here. check again.
		if ( m_StatHealth <= 0 )	// We can only die on our own tick.
		{
			return Death();
		}

		CRaceClassPtr pRace = Char_GetDef()->GetRace();
		ASSERT(pRace);

		for ( int i=STAT_Health; i<STAT_Fame; i++ )
		{
			int j = i-STAT_Health;
			m_StatRegen[j] += iTimeDiff;

			int iRate = pRace->GetRegenRate((STAT_TYPE)i);	// in TICKS_PER_SEC
			if ( i == STAT_Health )	// HitPoints regen rate is related to food and stam.
			{
				if ( m_pPlayer.IsValidNewObj())
				{
					if ( ! m_StatFood )
						continue; // iRate += iRate/2;	// much slower with no food.
					if ( IsStatFlag(STATF_Fly))
						continue;
				}

				// Fast metabolism bonus ?
				ASSERT(m_StatStam>=0);
				iRate += iRate / (1 + (m_StatStam/8));
			}

			if ( m_StatRegen[j] < iRate )
				continue;

			m_StatRegen[j] = 0;
			if ( i==STAT_Food )
			{
				OnTickFood();
			}
			else if ( m_Stat[i] != m_Stat[STAT_MaxHealth+i-STAT_Health] )
			{
				Stat_Change( (STAT_TYPE) i, 1 );
			}
		}

		m_timeLastStatRegen.InitTimeCurrent();
	}
	else
	{
		// Check this all the time.
		if ( m_StatHealth <= 0 )	// We can only die on our own tick.
		{
			return Death();
		}
	}

	if ( IsStatFlag( STATF_DEAD ))
		return true;
	if ( IsDisconnected())	// mounted horses can still get a tick.
	{
		goto bailout;
	}

	DEBUG_CHECK( IsTopLevel());	// not deleted or off line.

	if ( IsTimerExpired() && IsTimerSet())
	{
		// My turn to do some action.
		switch ( Skill_Done())
		{
		case -CSkillDef::T_Abort: Skill_Fail( true ); break;	// fail with message but no credit.
		case -CSkillDef::T_Fail: Skill_Fail( false ); break;
		case -CSkillDef::T_QTY: Skill_Cleanup(); break;
		}
		if ( ! m_pPlayer )
		{
			// What to do next ?
			ASSERT(m_pNPC);
			g_Serv.m_Profile.SwitchTask( PROFILE_NPCAI );
			NPC_OnTickAction();
			g_Serv.m_Profile.SwitchTask( PROFILE_Chars );
		}
	}
	else
	{
		// Are we ready to hit ?

		if (( iTimeDiff >= TICKS_PER_SEC ) &&
			( g_Cfg.m_wDebugFlags & DEBUGF_SKILL_TIME ))
		{
			TCHAR szTemp[ 128 ];
			int len = sprintf( szTemp, "%d", GetTimerAdjusted());
			if ( Fight_IsActive())
			{
				switch (m_Act.m_atFight.m_War_Swing_State)
				{
				case WAR_SWING_READY:		szTemp[len++] = '|'; break;
				case WAR_SWING_EQUIPPING:	szTemp[len++] = '-'; break;
				case WAR_SWING_SWINGING:	szTemp[len++] = '+'; break;
				default: szTemp[len++] = '?'; break;
				}
			}
			if ( IsStatFlag(STATF_Fly))
			{
				szTemp[len++] = 'F';
			}
			szTemp[len] = '\0';
			UpdateObjMessage( szTemp, szTemp, NULL, HUE_ORANGE, TALKMODE_NOSCROLL );
		}

		// Hit my current target. (if i'm ready)
		if ( IsStatFlag( STATF_War ))
		{
			if ( Fight_IsActive())
			{
				if ( m_Act.m_atFight.m_War_Swing_State == WAR_SWING_READY )
				{
					Fight_HitTry();
				}
			}
			else if ( Skill_GetActive() == SKILL_NONE )
			{
				Fight_AttackNext();
			}
		}
	}

bailout:

	if ( iTimeDiff >= TICKS_PER_SEC )
	{
		// Check location periodically for standing in fire fields, traps, etc.
		CheckLocation( true );
	}

	return( true );
}

HRESULT CChar::ScriptBook_Command( LPCTSTR pszCmd, bool fSystemCheck )
{
	// Launch a scriptbook command by this name on this CChar
	// Execute this command.
	// fSystemCheck = load new commands from the system.
	//   else just check commands we have in memory already.

	CSphereUID rid = g_Cfg.ResourceGetIDType( RES_Book, pszCmd );

	CItemPtr pItem=GetHead();
	for ( ; pItem!=NULL; pItem=pItem->GetNext())
	{
		if ( ! pItem->IsType(IT_EQ_SCRIPT_BOOK))
			continue;
		if ( rid == pItem->GetUIDIndex() || ! _stricmp( pszCmd, pItem->GetName()))
		{
			ScriptBook_OnTick( REF_CAST(CItemMessage,pItem), true );
			return NO_ERROR;
		}
	}

	if ( ! fSystemCheck )
		return( HRES_INVALID_HANDLE );
	if ( ! rid.IsValidRID())
		return( HRES_INVALID_HANDLE );

	// Assume there is a book id in RES_Book
	// Create the book for it.
	pItem = CItem::CreateScript(ITEMID_BOOK1,this);
	if ( pItem == NULL )
		return( HRES_INTERNAL_ERROR );
	CItemMessagePtr pBook = REF_CAST(CItemMessage,pItem );
	if ( pBook == NULL )
		return( HRES_INTERNAL_ERROR );
	
	pBook->SetType(IT_EQ_SCRIPT_BOOK);
	pBook->SetAttr(ATTR_CAN_DECAY);
	// Does the book id exist in scripts.
	pBook->m_itScriptBook.m_ResID = rid;
	if ( ! pBook->LoadSystemPages())
	{
		pBook->DeleteThis();
		return( HRES_INVALID_HANDLE );
	}

	LayerAdd( pBook, LAYER_SPECIAL );
	m_Act.m_Targ = pBook->GetUID();	// for last target stuff. (trigger stuff)

	ScriptBook_OnTick( pBook, true );	
	return( NO_ERROR );
}

void CChar::ScriptBook_OnTick( CItemMessage* pScriptItem, bool fForceStart )
{
	// IT_EQ_SCRIPT_BOOK
	// Take a tick for the current running script book or start it.

	if ( pScriptItem == NULL )
		return;

	// Default re-eval time.
	pScriptItem->SetTimeout( 5*60*TICKS_PER_SEC );

	// Did the script get superceded by something more important ?
	if ( m_pNPC )
	{
		if ( m_pNPC->m_Act_Motivation > pScriptItem->m_itScriptBook.m_iPriorityLevel )
			return;
	}

	// Is the script running ?
	int iPage = pScriptItem->m_itScriptBook.m_ExecPage;
	int iOffset = pScriptItem->m_itScriptBook.m_ExecOffset;

	if ( fForceStart )
	{
		if ( iPage )	// we should just wait for our tick !
			return;

		Skill_Cleanup();

		// Time to play. (continue from last)
		if ( pScriptItem->IsBookSystem())
		{
			// Need to load the book.
			if ( ! pScriptItem->LoadSystemPages())
				return;
		}
	}
	else
	{
		// just a tick.
		// ??? Auto starting scripts !!!
		if ( ! iPage )	// but it's not running
			return;
	}

	if ( m_pNPC )
	{
		Skill_Start( NPCACT_ScriptBook );
		m_pNPC->m_Act_Motivation = pScriptItem->m_itScriptBook.m_iPriorityLevel;
	}

	// Sample actions.
	// "GO=123,23,3"			// teleport directly here.
	// "TARG=123,23,3;ACT=goto"	// try to walk to this place.
	// "S=Hello, Welcome to my show!;T=30;M=W;M=W;M=E;M=E;S=Done;ANIM=Bow"

	LPCTSTR pszPage = pScriptItem->GetPageText( iPage );
	if ( pszPage == NULL )
	{
		// Done playing.
	play_stop:
		pScriptItem->m_itScriptBook.m_ExecPage = 0;
		pScriptItem->m_itScriptBook.m_ExecOffset = 0;

		if ( pScriptItem->IsBookSystem())
		{
			// unload the book.
			pScriptItem->UnLoadSystemPages();
		}
		if ( pScriptItem->IsAttr(ATTR_CAN_DECAY))
		{
			pScriptItem->DeleteThis();
		}
		else if ( pScriptItem->GetScriptTimeout() )
		{
			pScriptItem->SetTimeout( pScriptItem->GetScriptTimeout() );
		}
		return;
	}

	// STATF_Script_Play
	// Exec the script command
	TCHAR szTemp[ CSTRING_MAX_LEN ];
	TCHAR* pszVerb = szTemp;
	int len = 0;

restart_read:
	for(;;)
	{
		TCHAR ch = pszPage[ iOffset ];
		if ( ch )
		{
			iOffset++;
			if ( ch == '\n' || ch == '\r' || ch == '\t' )
				continue;	// ignore these.
			if ( ch == ';' )
			{
				break;	// end of command marker.
			}
			pszVerb[len++] = ch;
		}
		else
		{
			pszPage = pScriptItem->GetPageText( ++iPage );
			if ( pszPage == NULL || pszPage[0] == '\0' )
			{
				if ( ! len ) goto play_stop;
				break;
			}
			iOffset = 0;
		}
	}

	pszVerb[len] = '\0';
	pScriptItem->m_itScriptBook.m_ExecPage = iPage;
	pScriptItem->m_itScriptBook.m_ExecOffset = iOffset;

	// Execute the action.
	if ( len )
	{
		// Set the default time interval.
		if ( pszVerb[0] == 'T' && pszVerb[1] == '=' )
		{
			pszVerb += 2;
			pScriptItem->SetScriptTimeout( Exp_GetValue(pszVerb)); // store the last time interval here.
			len = 0;
			goto restart_read;
		}

		SERVMODE_TYPE iModePrv = g_Serv.m_iModeCode;
		g_Serv.m_iModeCode = SERVMODE_ScriptBook;	// book mode. (lower my priv level) never do account stuff here.

		// NOTE: We should do a priv check on all verbs here.
		if ( g_Cfg.CanUsePrivVerb( this, pszVerb, &g_Serv ))
		{
			CSphereExpContext exec( this, &g_Serv );
			HRESULT hRes = exec.ExecuteCommand(pszVerb);
			if ( IS_ERROR(hRes))
			{
				DEBUG_MSG(( "Bad Book Script verb '%s'" LOG_CR, pszVerb ));
				// should we stop ?
			}
		}
		g_Serv.m_iModeCode = iModePrv;
	}

	// When to check here again.
	pScriptItem->SetTimeout( pScriptItem->GetScriptTimeout());
}


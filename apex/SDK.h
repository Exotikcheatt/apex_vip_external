#pragma once
#include "driver.h"
#include "offsets.h"
#include "Utils.h"
#pragma warning(disable:4996)
typedef struct
{
	char szCN[0x100];
	char szEN[0x100];
	char szTexture[0x100];
}ITEM_NAME, * PITEM_NAME;

std::map<int, ITEM_NAME> g_ItemHash;

static int S_width = GetSystemMetrics(SM_CXSCREEN), S_height = GetSystemMetrics(SM_CYSCREEN);
static DWORD_PTR base = 0;
D3DMATRIX  vMatrix = D3DMATRIX();
DWORD_PTR MyLocalplayer = 0x0;
std::vector<DWORD_PTR> EntityList = std::vector<DWORD_PTR>();
std::vector<DWORD_PTR> EntityLoots = std::vector<DWORD_PTR>();

class Player {
private:

public:
	Player() {}
	Player(DWORD_PTR _ptr) {
		this->ptr = _ptr;
		unsigned long long pBoneArray = read<DWORD_PTR>(ptr + m_entity_bones);
		//DWORD_PTR latestPrimaryWeapons = read<DWORD_PTR>(ptr + 0x19ec) & 0xFFFF;
		//DWORD_PTR listPrimaryWeapons = read<DWORD_PTR>(base + m_cl_entitylist + (latestPrimaryWeapons * 0x20));
		//this->weaponNameIndex = read<int>(listPrimaryWeapons + 0x16EC);
		this->Team = read<INT>(ptr + m_entity_team_num);
		this->Health = read<INT>(ptr + m_player_health);
		this->ShieldHealth = read<INT>(ptr + m_shieldHealth);
		this->ShieldHealthMax = read<INT>(ptr + m_shieldHealthMax);
		this->HealthMax = read<INT>(ptr + m_player_max_health);
		this->origin = read<D3DXVECTOR3>(ptr + m_entity_origin); 
		this->IsDowned = read<INT>(ptr + m_player_bleedout_state);

		Head.x = read<float>(pBoneArray + 0xCC + (BoneId::head * 0x30)) + origin.x;
		Head.y = read<float>(pBoneArray + 0xDC + (BoneId::head * 0x30)) + origin.y;
		Head.z = read<float>(pBoneArray + 0xEC + (BoneId::head * 0x30)) + origin.z;

	}
	DWORD_PTR ptr = 0x0;
	INT Health = 0;
	INT HealthMax = 0;
	INT ShieldHealth = 0;
	INT ShieldHealthMax = 0;
	INT Team = 0;
	D3DXVECTOR3 origin = D3DXVECTOR3();
	D3DXVECTOR3 Head = D3DXVECTOR3();
	FLOAT Distance = 0;
	bool IsDowned = 0;
	int weaponNameIndex = 0;
	bool GetEntityBonePosition(uint32_t BoneId, D3DXVECTOR3& Out)
	{
		unsigned long long pBoneArray = read<DWORD_PTR>(ptr + m_entity_bones);

		Out.x = read<float>(pBoneArray + 0xCC + (BoneId * 0x30)) + origin.x;
		Out.y = read<float>(pBoneArray + 0xDC + (BoneId * 0x30)) + origin.y;
		Out.z = read<float>(pBoneArray + 0xEC + (BoneId * 0x30)) + origin.z;
		return true;
	}

	BOOL IsValid() {
		if ((this->origin.x != NULL) &&
			(this->Team != 0) &&
			(this->Health > NULL) &&
			(this->ptr != NULL)) return true;
		return false;
	}
};
class Loot {
private:

public:
	Loot() {}
	Loot(DWORD_PTR _ptr) {
		this->ptr = _ptr;
		this->origin = read<D3DXVECTOR3>(ptr + m_entity_origin);

	}
	DWORD_PTR ptr = 0x0;
	D3DXVECTOR3 origin = D3DXVECTOR3();
	DWORD nameid;
	FLOAT Distance = 0;
	int itemtpye = 0;

	BOOL IsValid() {
		if ((this->origin.x != NULL) &&
			(this->ptr != NULL)) return true;
		return false;
	}
};
class Manager {
private:
public:
	Player LocalPlayer = NULL;
	std::vector<Player> Players;
	std::vector<Loot> Loots;
	Player ClosestVisibleEnemy = NULL;
	Player ClosestCrosshairEnemy = NULL;
	INT Total = 0;

	Manager(int width, int height) {
		S_width = width;
		S_height = height;

		this->LocalPlayer = Player(MyLocalplayer);
		for (size_t i = 0; i < EntityList.size(); i++) {
			Player player(EntityList[i]);
			if (player.ptr != 0);// break;
			if (player.IsValid() && player.ptr != this->LocalPlayer.ptr) {
				player.Distance = Distance(this->LocalPlayer.origin,player.origin);
				this->Players.push_back(player);
			}
		}

		for (size_t i = 0; i < EntityLoots.size(); i++) {
			Loot loot(EntityLoots[i]);
			if (loot.ptr != 0);// break;
			if (loot.IsValid() && loot.ptr != this->LocalPlayer.ptr) {
				loot.Distance = Distance(this->LocalPlayer.origin, loot.origin);
				this->Loots.push_back(loot);
			}
		}

		float closestcrosshair = FLT_MAX;
		for (size_t i = 0; i < this->Players.size(); i++) {
			if (!this->LocalPlayer.IsDowned) {
				D3DXVECTOR3 out;
				if (WorldToScreen(this->Players[i].origin, out)) {
					float crossdis = GetCrossDistance(out.x, out.y, S_width / 2, S_height / 2);
					if ((crossdis < closestcrosshair)) {
						if (this->Players[i].Team != this->LocalPlayer.Team && this->Players[i].Distance < 125) {
							if (this->Players[i].IsDowned) {
								if (settings::aimknocked)
									ClosestCrosshairEnemy = this->Players[i];
							}
							else {
								ClosestCrosshairEnemy = this->Players[i];
							}
							closestcrosshair = crossdis;
						}
					}
				}
			}
		}
		float closestdis = FLT_MAX;
		for (int i = 0; i < this->Players.size(); i++) {
				if ((this->Players[i].Distance < closestdis)) {
					if (this->Players[i].Team != this->LocalPlayer.Team) {
						ClosestVisibleEnemy = this->Players[i];
						closestdis = this->Players[i].Distance;
					}
				}
		}

		for (int i = 0; i < this->Players.size(); i++) {
			Total = this->Players.size();

		}
	}

	bool WorldToScreen(D3DXVECTOR3& vIn, D3DXVECTOR3& vOut)
	{

		vOut.x = vMatrix.m[0][0] * vIn.x + vMatrix.m[0][1] * vIn.y + vMatrix.m[0][2] * vIn.z + vMatrix.m[0][3];
		vOut.y = vMatrix.m[1][0] * vIn.x + vMatrix.m[1][1] * vIn.y + vMatrix.m[1][2] * vIn.z + vMatrix.m[1][3];

		float w = vMatrix.m[3][0] * vIn.x + vMatrix.m[3][1] * vIn.y + vMatrix.m[3][2] * vIn.z + vMatrix.m[3][3];

		if (w < 0.65f)
			return false;

		float invw = 1.0f / w;

		vOut.x *= invw;
		vOut.y *= invw;

		float x = S_width / 2;
		float y = S_height / 2;

		x += 0.5 * vOut.x * S_width + 0.5;
		y -= 0.5 * vOut.y * S_height + 0.5;

		vOut.x = x;
		vOut.y = y;

		return true;
	}

	D3DXVECTOR3 CalcSoldierFuturePos(D3DXVECTOR3 InVec)
	{
		D3DXVECTOR3 NewPos, OutPos;
		if (WorldToScreen(InVec, NewPos)) {
			OutPos.x = NewPos.x;
			OutPos.y = NewPos.y;
			OutPos.z = NewPos.z;
		}
		else
		{
			OutPos.x = 0;
			OutPos.y = 0;
			OutPos.z = 0;
		}
		return OutPos;
	}

};

char* GetHelmetType(Player pPlayer)
{
	int HelmetId = 0;
	switch (HelmetId)
	{
		case (int)(HelmetID::HELMET_LV0) :
			return "Helmet LV0";
			break;
			case (int)(HelmetID::HELMET_LV1) :
				return "Helmet LV1";
				break;
				case (int)(HelmetID::HELMET_LV2) :
					return "Helmet LV2";
					break;
					case (int)(HelmetID::HELMET_LV3) :
						return "Helmet LV3";
						break;
						case (int)(HelmetID::HELMET_LV4) :
							return "Helmet LV4";
							break;
	}
	return "NULL";
}
char* GetArmorType(Player pPlayer)
{
	int ArmorId = 0;
	switch (ArmorId)
	{
		case (int)(ArmorID::BODY_ARMOR_LV0) :
			return "Armor LV0";
			break;
			case (int)(ArmorID::BODY_ARMOR_LV1) :
				return "Armor LV1";
				break;
				case (int)(ArmorID::BODY_ARMOR_LV2) :
					return "Armor LV2";
					break;
					case (int)(ArmorID::BODY_ARMOR_LV3) :
						return "Armor LV3";
						break;
						case (int)(ArmorID::BODY_ARMOR_LV4) :
							return "Armor LV4";
							break;
	}
	return "NULL";
}

char* GetGunName(Player pPlayer)
{
	int WeaponId = pPlayer.weaponNameIndex;
	switch (WeaponId)
	{
		case (int)(WeaponID::BARE_HANDS) :
			return "HANDS";
			break;
		case (int)WeaponID::HAVOC:
			return "HAVOC";
			break;
		case (int)WeaponID::LSTAR:
			return "LSTAR";
			break;
		case (int)WeaponID::KRABER:
			return "KRABER";
			break;
		case (int)WeaponID::MASTIFF:
			return "MASTIFF";
			break;
		case (int)WeaponID::DEVOTION:
			return "DEVOTION";
			break;
		case (int)WeaponID::SENTINEL:
			return "SENTINEL";
			break;
		case (int)WeaponID::CHARGE_RIFLE:
			return "CHARGE_RIFLE";
			break;
		case (int)WeaponID::LONGBOW:
			return "LONGBOW";
			break;
		case (int)WeaponID::TRIPLE_TAKE:
			return "TRIPLE_TAKE";
			break;
		case (int)WeaponID::WINGMAN:
			return "WINGMAN";
			break;
		case (int)WeaponID::SPITFIRE:
			return "SPITFIRE";
			break;
		case (int)WeaponID::PROWLER:
			return "PROWLER";
			break;
		case (int)WeaponID::HEMLOK:
			return "HEMLOK";
			break;
		case (int)WeaponID::FLATLINE:
			return "FLATLINE";
			break;
		case (int)WeaponID::RE45:
			return "RE45";
			break;
		case (int)WeaponID::P2020:
			return "P2020";
			break;
		case (int)WeaponID::R301:
			return "R301";
			break;
		case (int)WeaponID::R99:
			return "R99";
			break;
		case (int)WeaponID::ALTERNATOR:
			return "ALTERNATOR";
			break;
		case (int)WeaponID::G7_SCOUT:
			return "G7_SCOUT";
			break;
		case (int)WeaponID::MOZAMBIQUE:
			return "MOZAMBIQUE";
			break;
		case (int)WeaponID::PEACEKEEPER:
			return "PEACEKEEPER";
			break;
		case (int)WeaponID::EVA8_AUTO:
			return "EVA8_AUTO";
			break;

	}
	return "NULL";
}
char* GetGunType(Player pPlayer)
{
	int WeaponId = pPlayer.weaponNameIndex;
	switch (WeaponId)
	{
	case (int)WeaponID::BARE_HANDS:
		return "HANDS";
		break;
	case (int)WeaponID::KRABER:
	case (int)WeaponID::SENTINEL:
	case (int)WeaponID::LONGBOW:
	case (int)WeaponID::TRIPLE_TAKE:
	case (int)WeaponID::CHARGE_RIFLE:
		return "SNIPER";
		break;
	case (int)WeaponID::HEMLOK:
	case (int)WeaponID::FLATLINE:
	case (int)WeaponID::G7_SCOUT:
	case (int)WeaponID::R301:
	case (int)WeaponID::HAVOC:
		return "AR";
		break;
	case (int)WeaponID::DEVOTION:
	case (int)WeaponID::SPITFIRE:
	case (int)WeaponID::LSTAR:
		return "LMG";
		break;
	case (int)WeaponID::PROWLER:
	case (int)WeaponID::R99:
	case (int)WeaponID::ALTERNATOR:
		return "SMG";
		break;
	case (int)WeaponID::MASTIFF:
	case (int)WeaponID::EVA8_AUTO:
	case (int)WeaponID::PEACEKEEPER:
	case (int)WeaponID::MOZAMBIQUE:
		return "SHOTGUN";
		break;
	case (int)WeaponID::WINGMAN:
	case (int)WeaponID::RE45:
	case (int)WeaponID::P2020:
		return "PISTOL";
		break;

	}
	return "NULL";
}
bool IsPistol(Player pPlayer)
{
	int WeaponId = pPlayer.weaponNameIndex;
	if (WeaponId == NULL) return false;

	if (WeaponId == (int)WeaponID::RE45
		|| WeaponId == (int)WeaponID::P2020
		|| WeaponId == (int)WeaponID::WINGMAN)
		return true;
	else
		return false;
}
bool IsSniper(Player pPlayer)
{
	int WeaponId = pPlayer.weaponNameIndex;
	if (WeaponId == NULL) return false;

	if (WeaponId == (int)WeaponID::KRABER
		|| WeaponId == (int)WeaponID::SENTINEL
		|| WeaponId == (int)WeaponID::LONGBOW
		|| WeaponId == (int)WeaponID::TRIPLE_TAKE
		|| WeaponId == (int)WeaponID::CHARGE_RIFLE)
		return true;
	else
		return false;
}

bool IsLMG(Player pPlayer)
{
	int WeaponId = pPlayer.weaponNameIndex;
	if (WeaponId == NULL) return false;

	if (WeaponId == (int)WeaponID::DEVOTION || WeaponId == (int)WeaponID::SPITFIRE || WeaponId == (int)WeaponID::LSTAR)
		return true;
	else
		return false;
}
bool IsSmg(Player pPlayer)
{
	int WeaponId = pPlayer.weaponNameIndex;
	if (WeaponId == NULL) return false;

	if (WeaponId == (int)WeaponID::PROWLER
		|| WeaponId == (int)WeaponID::R99
		|| WeaponId == (int)WeaponID::ALTERNATOR)
		return true;
	else
		return false;
}
bool IsShotgun(Player pPlayer)
{
	int WeaponId = pPlayer.weaponNameIndex;
	if (WeaponId == NULL) return false;

	if (WeaponId == (int)WeaponID::MASTIFF
		|| WeaponId == (int)WeaponID::PEACEKEEPER
		|| WeaponId == (int)WeaponID::EVA8_AUTO
		|| WeaponId == (int)WeaponID::MOZAMBIQUE)
		return true;
	else
		return false;
}
bool IsAR(Player pPlayer)
{
	int WeaponId = pPlayer.weaponNameIndex;
	if (WeaponId == NULL) return false;

	if (WeaponId == (int)WeaponID::FLATLINE
		|| WeaponId == (int)WeaponID::G7_SCOUT
		|| WeaponId == (int)WeaponID::R301
		|| WeaponId == (int)WeaponID::HAVOC
		|| WeaponId == (int)WeaponID::HEMLOK)
		return true;
	else
		return false;
}
void AddItemName(int id, char* pCN, char* pEN, char* pTexture)
{
	ITEM_NAME in;
	lstrcpyA(in.szCN, pCN);
	lstrcpyA(in.szEN, pEN);
	lstrcpyA(in.szTexture, pTexture);
	g_ItemHash[id] = in;
}

void InitItemName()
{
	//SEProtectStartMutation();
	g_ItemHash.clear();
	AddItemName((int)ItemID::DeathBox, "克雷贝尔狙击枪", "DeathBox", "收");		//1
	//Weapons
	AddItemName((int)ItemID::KRABER_HEIRLOOM, "克雷贝尔狙击枪", "Kraber", "收");		//1
	AddItemName((int)ItemID::MASTIFF, "獒犬霰弹枪", "Mastiff", "藏");			//1
	AddItemName((int)ItemID::MASTIFF_GOLD, "獒犬霰弹枪", "Mastiff [GOLD]", "藏");			//1
	AddItemName((int)ItemID::LSTAR, "L-Star EMG", "L-Star", "A");
	AddItemName((int)ItemID::LSTAR_GOLD, "L-Star EMG", "L-Star [GOLD]", "A");
	AddItemName((int)ItemID::HAVOC, "哈沃克", "Havoc", "");
	AddItemName((int)ItemID::HAVOC_GOLD, "哈沃克", "Havoc Rifle [GOLD]", "");
	AddItemName((int)ItemID::DEVOTION, "专注轻机枪", "Devotion", "致");
	AddItemName((int)ItemID::DEVOTION_GOLD, "专注轻机枪", "Devotion [GOLD]", "致");
	AddItemName((int)ItemID::TRIPLE_TAKE, "三重式狙击枪", "Triple Take", "结");
	AddItemName((int)ItemID::TRIPLE_TAKE_GOLD, "黄金三重式狙击枪", "Triple Take [GOLD]", "结");
	AddItemName((int)ItemID::FLATLINE, "VK-47平行步枪", "Flatline", "");
	AddItemName((int)ItemID::FLATLINE_GOLD, "黄金VK-47平行步枪", "Flatline [GOLD]", "");
	AddItemName((int)ItemID::HEMLOK, "赫姆洛克突击步枪", "Hemlok", "闰");		//1
	AddItemName((int)ItemID::HEMLOK_GOLD, "赫姆洛克突击步枪", "Hemlok [GOLD]", "闰");		//1
	AddItemName((int)ItemID::G7_SCOUT, "G7侦查枪", "G7 Scout", "阳");
	AddItemName((int)ItemID::G7_SCOUT_GOLD, "黄金G7侦查枪", "G7 Scout [GOLD]", "阳");
	AddItemName((int)ItemID::ALTERNATOR, "转换者冲锋枪", "Alternator", "腾");
	AddItemName((int)ItemID::ALTERNATOR_GOLD, "黄金转换者冲锋枪", "Alternator [GOLD]", "腾");
	AddItemName((int)ItemID::R99, "R-99冲锋枪", "R-99", "律");
	AddItemName((int)ItemID::R99_GOLD, "R-99冲锋枪", "R-99 [GOLD]", "律");
	AddItemName((int)ItemID::PROWLER_HEIRLOOM, "潜袭冲锋枪", "Prowler", "霜");
	AddItemName((int)ItemID::LONGBOW, "长弓精确步枪", "Longbow", "雨");
	AddItemName((int)ItemID::LONGBOW_GOLD, "黄金长弓精确步枪", "Longbow [GOLD]", "雨");
	AddItemName((int)ItemID::CHARGE_RIFLE, "充能步枪", "Charge", "");
	AddItemName((int)ItemID::CHARGE_RIFLE_GOLD, "黄金充能步枪", "Charge [GOLD]", "");
	AddItemName((int)ItemID::SPITFIRE, "M600喷火轻机枪", "Spitfire", "调");
	AddItemName((int)ItemID::SPITFIRE_GOLD, "M600喷火轻机枪", "Spitfire [GOLD]", "调");
	AddItemName((int)ItemID::R301, "R-301卡宾枪", "R-301", "岁");
	AddItemName((int)ItemID::R301_GOLD, "R-301卡宾枪", "R-301 [GOLD]", "岁");
	AddItemName((int)ItemID::EVA8_AUTO, "EVA-8自动霰弹枪", "EVA-8 Auto", "云");	//1
	AddItemName((int)ItemID::EVA8_AUTO_GOLD, "EVA-8自动霰弹枪", "EVA-8 Auto [GOLD]", "云");	//1
	AddItemName((int)ItemID::PEACEKEEPER_HEIRLOOM, "和平捍卫者霰弹枪", "Peacekeeper", "冬");
	AddItemName((int)ItemID::MOZAMBIQUE, "莫桑比克霰弹枪", "Mozambique", "为");
	AddItemName((int)ItemID::MOZAMBIQUE_GOLD, "黄金莫桑比克霰弹枪", "Mozambique [GOLD]", "为");
	AddItemName((int)ItemID::WINGMAN, "辅助手枪", "Wingman", "露");
	AddItemName((int)ItemID::WINGMAN_GOLD, "黄金辅助手枪", "Wingman [GOLD]", "露");
	AddItemName((int)ItemID::P2020, "P2020手枪", "P2020", "吕");
	AddItemName((int)ItemID::P2020_GOLD, "黄金P2020手枪", "P2020 [GOLD]", "吕");
	AddItemName((int)ItemID::RE45, "RE-45自动手枪", "RE-45", "成");
	AddItemName((int)ItemID::RE45_GOLD, "黄金RE-45自动手枪", "RE-45 [GOLD]", "成");
	AddItemName((int)ItemID::Repeater, "RE-45自动手枪", "Repeater", "成");
	AddItemName((int)ItemID::Repeater_GOLD, "黄金RE-45自动手枪", "Repeater [GOLD]", "成");
	AddItemName((int)ItemID::VOLT, "黄金RE-45自动手枪", "Volt", "成");
	AddItemName((int)ItemID::VOLT_GOLD, "黄金RE-45自动手枪", "Volt [GOLD]", "成");
	AddItemName((int)ItemID::SENTINEL, "黄金RE-45自动手枪", "SENTINEL", "成");
	AddItemName((int)ItemID::SENTINEL_GOLD, "黄金RE-45自动手枪", "SENTINEL [GOLD]", "成");

	//Ammo
	AddItemName((int)ItemID::LIGHT_ROUNDS, "轻型弹药", "Light Ammo", "");
	AddItemName((int)ItemID::ENERGY_AMMO, "能量弹药", "Energy Ammo", "");
	AddItemName((int)ItemID::SHOTGUN_SHELLS, "霰弹弹药", "Shotgun Ammo", "");
	AddItemName((int)ItemID::HEAVY_ROUNDS, "重型弹药", "Heavy Ammo", "");
	AddItemName((int)ItemID::SNIPER_AMMO, "重型弹药", "Sniper Ammo", "");

	//Healing Items
	AddItemName((int)ItemID::ULTIMATE_ACCELERANT, "绝招加速剂", "Ultimate Accelerant", "");
	AddItemName((int)ItemID::PHOENIX_KIT, "凤凰治疗包", "Phoenix Kit", "金");
	AddItemName((int)ItemID::MED_KIT, "医疗箱", "Med Kit", "水");
	AddItemName((int)ItemID::SYRINGE, "注射器", "Syringe", "丽");
	AddItemName((int)ItemID::SHIELD_BATTERY, "大型护盾电池", "Shield Battery", "玉");
	AddItemName((int)ItemID::SHIELD_CELL, "小型护盾电池", "Shield Cell", "出");

	//Shield Items

	AddItemName((int)ItemID::HELMET_LV1, "头盔(lv1)", "Helmet Level 1", "生 lv1");
	AddItemName((int)ItemID::HELMET_LV2, "头盔(lv2)", "Helmet Level 2", "生 lv2");
	AddItemName((int)ItemID::HELMET_LV3, "头盔(lv3)", "Helmet Level 3", "生 lv3");
	AddItemName((int)ItemID::HELMET_LV4, "头盔(lv4)", "Helmet Level 4", "生 lv4");
	AddItemName((int)ItemID::BODY_ARMOR_LV1, "防弹衣(lv1)", "Body Armor Level 1", "冈 lv1");
	AddItemName((int)ItemID::BODY_ARMOR_LV2, "防弹衣(lv2)", "Body Armor Level 2", "冈 lv2");
	AddItemName((int)ItemID::BODY_ARMOR_LV3, "防弹衣(lv3)", "Body Armor Level 3", "冈 lv3");
	AddItemName((int)ItemID::BODY_ARMOR_LV4, "防弹衣(lv4)", "Body Armor Level 4", "冈 lv4");
	AddItemName((int)ItemID::EVO_SHIELD_LV1, "防弹衣(lv1)", "Evo Shield Level 1", "冈 lv1");
	AddItemName((int)ItemID::EVO_SHIELD_LV2, "防弹衣(lv2)", "Evo Shield Level 2", "冈 lv2");
	AddItemName((int)ItemID::EVO_SHIELD_LV3, "防弹衣(lv3)", "Evo Shield Level 3", "冈 lv3");
	AddItemName((int)ItemID::EVO_SHIELD_LV4, "防弹衣(lv4)", "Evo Shield Level 4", "冈 lv4");

	AddItemName((int)ItemID::KNOCKDOWN_SHIELD_LV1, "击倒护盾(lv1)", "Knockdown Shield Level 1", "号 lv1");
	AddItemName((int)ItemID::KNOCKDOWN_SHIELD_LV2, "击倒护盾(lv2)", "Knockdown Shield Level 2", "号 lv2");
	AddItemName((int)ItemID::KNOCKDOWN_SHIELD_LV3, "击倒护盾(lv3)", "Knockdown Shield Level 3", "号 lv3");
	AddItemName((int)ItemID::KNOCKDOWN_SHIELD_LV4, "击倒护盾(lv4)", "Knockdown Shield Level 4", "号 lv4");
	AddItemName((int)ItemID::BACKPACK_LV1, "背包(lv1)", "Backpack Level 1", "剑 lv1");
	AddItemName((int)ItemID::BACKPACK_LV2, "背包(lv2)", "Backpack Level 2", "剑 lv2");
	AddItemName((int)ItemID::BACKPACK_LV3, "背包(lv3)", "Backpack Level 3", "剑 lv3");
	AddItemName((int)ItemID::BACKPACK_LV4, "背包(lv4)", "Backpack Level 4", "剑 lv4");

	//Grenades
	AddItemName((int)ItemID::THERMITE_GRENADE, "铝热剂手雷", "Thermite Grenade", "");
	AddItemName((int)ItemID::FRAG_GRENADE, "破片手雷", "Frag Grenade", "巨");
	AddItemName((int)ItemID::ARC_STAR, "飞镖", "Arc Star", "阙");


	//Attachment Scopes
	AddItemName((int)ItemID::HCOG_CLASSIC, "单倍全息", "1x HCOG (Classic)", "盈");
	AddItemName((int)ItemID::HCOG_BRUISER, "2倍全息", "2x HCOG (Bruiser)", "往");
	AddItemName((int)ItemID::HOLO, "单倍幻影", "1x Holo", "昃");
	AddItemName((int)ItemID::VARIABLE_HOLO, "1-2倍幻影", "1x-2x Variable Holo", "天");
	AddItemName((int)ItemID::DIGITAL_THREAT, "1倍数字化镜", "1x Digital Threat", "月");
	AddItemName((int)ItemID::HCOG_RANGER, "3倍全息", "3x HCOG (Ranger)", "暑");
	AddItemName((int)ItemID::VARIABLE_AOG, "2-4倍瞄准镜", "2x-4x Variable AOG", "秋");
	AddItemName((int)ItemID::SNIPER, "6倍狙击瞄准镜", "6x Sniper", "张");
	AddItemName((int)ItemID::VARIABLE_SNIPER, "4-8倍瞄准镜", "4x-8x Variable Sniper", "来");
	AddItemName((int)ItemID::DIGITAL_SNIPER_THREAT, "4-10倍瞄准镜", "4x-10x Digital Sniper Threat", "寒");

	//Attachments
	AddItemName((int)ItemID::BARREL_STABILIZER_LV1, "枪管稳定器(lv1)", "Barrel Stabilizer Level 1", "宙 lv1");
	AddItemName((int)ItemID::BARREL_STABILIZER_LV2, "枪管稳定器(lv2)", "Barrel Stabilizer Level 2", "宙 lv2");
	AddItemName((int)ItemID::BARREL_STABILIZER_LV3, "枪管稳定器(lv3)", "Barrel Stabilizer Level 3", "宙 lv3");
	AddItemName((int)ItemID::BARREL_STABILIZER_LV4, "枪管稳定器(lv4)", "Barrel Stabilizer Level 4", "宙 lv4");
	AddItemName((int)ItemID::LIGHT_MAGAZINE_LV1, "加长式轻型弹匣lv1)", "Light Magazine Level 1", "日 lv1");
	AddItemName((int)ItemID::LIGHT_MAGAZINE_LV2, "加长式轻型弹匣(lv2)", "Light Magazine Level 2", "日 lv2");
	AddItemName((int)ItemID::LIGHT_MAGAZINE_LV3, "加长式轻型弹匣(lv3)", "Light Magazine Level 3", "日 lv3");
	AddItemName((int)ItemID::HEAVY_MAGAZINE_LV1, "加长式重型弹匣(lv1)", "Heavy Magazine Level 1", "荒 lv1");
	AddItemName((int)ItemID::HEAVY_MAGAZINE_LV2, "加长式重型弹匣(lv2)", "Heavy Magazine Level 2", "荒 lv2");
	AddItemName((int)ItemID::HEAVY_MAGAZINE_LV3, "加长式重型弹匣(lv3)", "Heavy Magazine Level 3", "荒 lv3");

	AddItemName((int)ItemID::ENERGY_MAGAZINE_LV1, "加长式能量弹匣(lv1)", "ENERGY Magazine Level 1", "荒 lv1");
	AddItemName((int)ItemID::ENERGY_MAGAZINE_LV2, "加长式能量弹匣(lv2)", "ENERGY Magazine Level 2", "荒 lv2");
	AddItemName((int)ItemID::ENERGY_MAGAZINE_LV3, "加长式能量弹匣(lv3)", "ENERGY Magazine Level 3", "荒 lv3");
	AddItemName((int)ItemID::SNIPER_MAGAZINE_LV1, "加长式能量弹匣(lv1)", "ENERGY Magazine Level 1", "荒 lv1");
	AddItemName((int)ItemID::SNIPER_MAGAZINE_LV2, "加长式能量弹匣(lv2)", "ENERGY Magazine Level 2", "荒 lv2");
	AddItemName((int)ItemID::SNIPER_MAGAZINE_LV3, "加长式能量弹匣(lv3)", "ENERGY Magazine Level 3", "荒 lv3");

	AddItemName((int)ItemID::SHOTGUN_BOLT_LV1, "霰弹枪枪栓(lv1)", "Shotgun Bolt Level 1", "宇 lv1");
	AddItemName((int)ItemID::SHOTGUN_BOLT_LV2, "霰弹枪枪栓(lv2)", "Shotgun Bolt Level 2", "宇 lv2");
	AddItemName((int)ItemID::SHOTGUN_BOLT_LV3, "霰弹枪枪栓(lv3)", "Shotgun Bolt Level 3", "宇 lv3");
	AddItemName((int)ItemID::STANDARD_STOCK_LV1, "标准枪托(lv1)", "Standard Stock Level 1", "辰 lv1");
	AddItemName((int)ItemID::STANDARD_STOCK_LV2, "标准枪托(lv2)", "Standard Stock Level 2", "辰 lv2");
	AddItemName((int)ItemID::STANDARD_STOCK_LV3, "标准枪托(lv3)", "Standard Stock Level 3", "辰 lv3");
	AddItemName((int)ItemID::SNIPER_STOCK_LV1, "狙击枪枪托(lv1)", "Sniper Stock Level 1", "洪 lv1");
	AddItemName((int)ItemID::SNIPER_STOCK_LV2, "狙击枪枪托(lv2)", "Sniper Stock Level 2", "洪 lv2");
	AddItemName((int)ItemID::SNIPER_STOCK_LV3, "狙击枪枪托(lv3)", "Sniper Stock Level 3", "洪 lv3");
	AddItemName((int)ItemID::TURBOCHARGER, "涡轮增压器", "Turbocharger", "黄");
	AddItemName((int)ItemID::SKULLPIERCER_RIFLING, "选择射击模式器", "Selectfire Receiver", "地");
	AddItemName((int)ItemID::HAMMERPOINT_ROUNDS, "精准束器", "Precision Choke", "宿");
	AddItemName((int)ItemID::DOUBLE_TAP_TRIGGER, "Disruptor Rounds", "Precision Choke", "宿");
	AddItemName((int)ItemID::HOPUP_187, "双射器", "doubletaptrigger", "宿");
	AddItemName((int)ItemID::QUICKDRAW_HOLSTER, "Hammerpoint Rounds", "Hammerpoint Rounds", "宿");
	AddItemName((int)ItemID::VAULT_KEY, "Hammerpoint Rounds", "Hammerpoint Rounds", "宿");
	AddItemName((int)ItemID::MOBILE_RESPAWN_BEACON, "Hammerpoint Rounds", "Hammerpoint Rounds", "宿");
	AddItemName((int)ItemID::ITEM_191, "Hammerpoint Rounds", "Hammerpoint Rounds", "宿");
	AddItemName((int)ItemID::TREASURE_PACK, "Hammerpoint Rounds", "Hammerpoint Rounds", "宿");

	//AddItemName(89, "穿心膛线", "Skullpiercer Rifling", "列");
	//玄 消焰器
	//SEProtectEnd();
}

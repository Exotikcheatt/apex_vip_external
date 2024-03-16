#include "Menu.h"
#include "Settings.h"
#include "SDK.h"
#include "safe/xor.hpp"
#include "safe/spoofcalls.h"
#include "safe/spoof_call.h"


inline HWND hwnd = NULL;
inline std::uint32_t oPID = 0;

c_menu_framework* menu_framework = new c_menu_framework;

void Crosshair()
{
	SPOOF_FUNC;
	switch (settings::crosshair)
	{
	case Renderer::CrosshairStyle::Dot:
		Renderer::GetInstance()->DrawCrosshair(Renderer::CrosshairStyle::Dot, S_width / 2, S_height / 2, settings::size, settings::stroke, D3DXCOLOR(0.f, 1.f, 0.f, 1.f));
		break;
	case Renderer::CrosshairStyle::Plus:
		Renderer::GetInstance()->DrawCrosshair(Renderer::CrosshairStyle::Plus, S_width / 2, S_height / 2, settings::size, settings::stroke, D3DXCOLOR(0.f, 1.f, 0.f, 1.f));
		break;
	case Renderer::CrosshairStyle::Cross:
		Renderer::GetInstance()->DrawCrosshair(Renderer::CrosshairStyle::Cross, S_width / 2, S_height / 2, settings::size, settings::stroke, D3DXCOLOR(0.f, 1.f, 0.f, 1.f));
		break;
	case Renderer::CrosshairStyle::Gap:
		Renderer::GetInstance()->DrawCrosshair(Renderer::CrosshairStyle::Gap, S_width / 2, S_height / 2, settings::size, settings::stroke, D3DXCOLOR(0.f, 1.f, 0.f, 1.f));
		break;
	case Renderer::CrosshairStyle::Diagonal:
		Renderer::GetInstance()->DrawCrosshair(Renderer::CrosshairStyle::Diagonal, S_width / 2, S_height / 2, settings::size, settings::stroke, D3DXCOLOR(0.f, 1.f, 0.f, 1.f));
		break;
	case Renderer::CrosshairStyle::HitMaker:
		Renderer::GetInstance()->DrawCrosshair(Renderer::CrosshairStyle::HitMaker, S_width / 2, S_height / 2, settings::size, settings::stroke, D3DXCOLOR(0.f, 1.f, 0.f, 1.f));
		break;
	default:
		break;
	}

}

void drawLoop(int width, int height)
{
	SPOOF_FUNC;
	menu_framework->do_menu_controls();
	if(settings::c_enable)
	Crosshair();

	Manager gm(width, height);
	DrawString("Players " + std::to_string((int)gm.Total), 18, 10 , 10, 1.f, 0.f, 0.f, 1.f);
	
	for (Player& player : gm.Players) {
		D3DXVECTOR3 footLocation = player.origin;
		D3DXVECTOR3 headLocation = player.Head;
		D3DXVECTOR3 oheadLocation;
		D3DXVECTOR3 ofootLocation;
		D3DXCOLOR PlayerBox = D3DXCOLOR();
		D3DXCOLOR PlayerLine = D3DXCOLOR();
		D3DXCOLOR PlayerFillBox = D3DXCOLOR();
		D3DXCOLOR PlayerKnocked = D3DXCOLOR();

		if (player.Team == gm.LocalPlayer.Team) {
			PlayerBox = settings::FriendBox;
			PlayerLine = settings::FriendLine;
			PlayerFillBox = settings::FriendFillbox;
			PlayerKnocked = settings::FriendKnocked;
		}
		else {
			if (player.Distance < 80) {

				PlayerBox = settings::Closed;
				PlayerLine = settings::Closed;
				PlayerFillBox = settings::Closed;

			}
			else {
				PlayerBox = settings::EnemyBOX;
				PlayerLine = settings::EnemyLine;
				PlayerFillBox = settings::EnemyFillbox;
				PlayerKnocked = settings::EnemyKnocked;
			}
		}

		if (player.Team != gm.LocalPlayer.Team) {
			
			if (gm.ClosestVisibleEnemy.Distance < 80)
			{
				DrawBox((S_width / 2) - 30, 170, 220, 50, 1, 0, 0, 0, 0.5, true);
				DrawString("Enemy Closed " + std::to_string((int)gm.ClosestVisibleEnemy.Distance) + "m", 20, S_width / 2, 180, 1.f, 0.f, 0.f, 1.f);
			}

		}
		if checkSetting(settings::teamKnocked, settings::enemyKnocked) {
			if (player.IsDowned)
			{
				PlayerBox = PlayerKnocked;
				PlayerLine = PlayerKnocked;
				PlayerFillBox = PlayerKnocked;
			}

		}

		if ((gm.WorldToScreen(footLocation, ofootLocation) && (gm.WorldToScreen(headLocation, oheadLocation)))) {
			float w2sHeight = Distance3D(ofootLocation, oheadLocation);
			float w2sWidth = w2sHeight;
			float heightoffset = Distance3D(ofootLocation, oheadLocation);
			float H = w2sHeight;
			float W = H / 2;
			float X = oheadLocation.x - W / 2;
			std::vector< Text > texts;
			if (player.Distance > settings::playersdis) continue;
			
			if checkSetting(settings::teamSnaplines, settings::enemySnaplines) {
				DrawLine(S_width / 2, 0, oheadLocation.x, oheadLocation.y, 1, pasteColorA(PlayerLine));
			}
			if checkSetting(settings::teamBoxes, settings::enemyBoxes) {
				if (settings::boxtype == 0)
				{
					if (!settings::esp_Outline)
						DrawBox(X, oheadLocation.y, W, H, 2.f, pasteColorA(PlayerBox), false);
					else if (settings::esp_Outline)
						Renderer::GetInstance()->DrawOutlineBox(X, oheadLocation.y, W, H, PlayerBox, 2.f);

				}
				else
				{
					if (!settings::esp_Outline)
						Renderer::GetInstance()->DrawCoalBox(X, oheadLocation.y, W, H, PlayerBox, 2.f);
					else if (settings::esp_Outline)
						Renderer::GetInstance()->DrawOutlineCoalBox(X, oheadLocation.y, W, H, PlayerBox, 2.f);

				}
			}
			if checkSetting(settings::teamFilledbox, settings::enemyFilledbox) {
				DrawBox(X, oheadLocation.y, W, H, 1, pasteColor(PlayerFillBox), 0.1, true);

			}
			if checkSetting(settings::teamHealth, settings::enemyHealth) {
				if(settings::healthbartype == 0)
					Renderer::GetInstance()->DrawHorBar(X, oheadLocation.y - 6, W, 4, (int)player.Health, (int)player.HealthMax, D3DXCOLOR(.48f, .98f, 0.f, 1.f), D3DXCOLOR(1.f, 0.f, 0.f, 1.f));
				else if(settings::healthbartype == 1)
					Renderer::GetInstance()->DrawVerBar(X - 6, oheadLocation.y, 4, H, (int)player.Health, (int)player.HealthMax, D3DXCOLOR(.48f, .98f, 0.f, 1.f), D3DXCOLOR(1.f, 0.f, 0.f, 1.f));
				else
					texts.push_back(Text("Health "+std::to_string((int)player.Health) + "%", 2, 14, D3DXCOLOR(.48f, .98f, 0.f, 1.f)));

			}
			if checkSetting(settings::teamShieldHealth, settings::enemyShieldHealth) {
				if (settings::shildbartype == 0) {
					if (player.ShieldHealth > 0)
						Renderer::GetInstance()->DrawHorBar(X, ofootLocation.y + 6, W, 4, player.ShieldHealth, player.ShieldHealthMax, D3DXCOLOR(1.f, 1.f, 1.f, 1.f), D3DXCOLOR(1.f, 0.f, 0.f, 1.f));
				}
				else if (settings::shildbartype == 1) {
					if (player.ShieldHealth > 0)
						Renderer::GetInstance()->DrawVerBar(X + W + 1, oheadLocation.y, 4, H, player.ShieldHealth, player.ShieldHealthMax,D3DXCOLOR(1.f, 1.f, 1.f, 1.f), D3DXCOLOR(1.f, 0.f, 0.f, 1.f));
				}
				else {
					if (player.ShieldHealth > 0)
						texts.push_back(Text("Shield " + std::to_string((int)player.ShieldHealth) + "%", 2, 14, D3DXCOLOR(1.f, 1.f, 1.f, 1.f)));
				}
			}
			if checkSetting(settings::teamDistance, settings::enemyDistance) {

				texts.push_back(Text(std::to_string((int)player.Distance) + "M", 1, 14, D3DXCOLOR(1.f, 1.f, 1.f, 1.f)));
			}
			if checkSetting(settings::teamWeaponName, settings::enemyWeaponName) {
				//if(settings::PlayerWeapontype ==0)
			//	texts.push_back(Text(GetGunName(player), 1, 14, D3DXCOLOR(0.f, 1.f, 0.f, 1.f)));
				//else
				//texts.push_back(Text(GetGunType(player), 1, 14, D3DXCOLOR(0.f, 1.f, 0.f, 1.f)));
			}
			
			int middle[3] = { ofootLocation.y,ofootLocation.y, ofootLocation.y };
			int Top[3] = { oheadLocation.y,oheadLocation.y, oheadLocation.y };
			for (auto text : texts)
			{
				int fontsize = text.size;
				float offset = (text.text.length() * fontsize) / 5;
				switch (text.side)
				{
				case 0:
					DrawString(text.text.c_str(), text.size, oheadLocation.x - offset, Top[1] - (heightoffset / 4) - fontsize, text.color.r, text.color.g, text.color.b);
					Top[1] -= fontsize;
					break;
				case 1:
					DrawString(text.text.c_str(), text.size, ofootLocation.x - offset, middle[1], text.color.r, text.color.g, text.color.b);
					middle[1] += fontsize;
					break;
				case 2:
					DrawString(text.text.c_str(), text.size, oheadLocation.x + (heightoffset / 4) + fontsize, Top[1] + fontsize, text.color.r, text.color.g, text.color.b);
					Top[1] += fontsize;
					break;
				}

			}
		}
	}
	
	for (Loot& loot : gm.Loots) {
		D3DXVECTOR3 footLocation = loot.origin;
		D3DXVECTOR3 headLocation = footLocation;
		D3DXVECTOR3 oheadLocation;
		D3DXVECTOR3 ofootLocation;
		headLocation.y += .5;
		if ((gm.WorldToScreen(footLocation, ofootLocation) && (gm.WorldToScreen(headLocation, oheadLocation)))) {
			float w2sHeight = Distance3D(ofootLocation, oheadLocation);
			float w2sWidth = w2sHeight;
			float heightoffset = Distance3D(ofootLocation, oheadLocation);
			float H = w2sHeight;
			float W = H / 2;
			float X = oheadLocation.x - W / 2;
			std::vector< Text > texts;


			int middle[3] = { ofootLocation.y,ofootLocation.y, ofootLocation.y };
			int Top[3] = { oheadLocation.y,oheadLocation.y, oheadLocation.y };
			for (auto text : texts)
			{
				int fontsize = text.size;
				float offset = (text.text.length() * fontsize) / 5;
				switch (text.side)
				{
				case 0:
					DrawString(text.text.c_str(), text.size, oheadLocation.x - offset, Top[1] - (heightoffset / 4) - fontsize, text.color.r, text.color.g, text.color.b);
					Top[1] -= fontsize;
					break;
				case 1:
					DrawString(text.text.c_str(), text.size, ofootLocation.x - offset, middle[1], text.color.r, text.color.g, text.color.b);
					middle[1] += fontsize;
					break;
				case 2:
					DrawString(text.text.c_str(), text.size, oheadLocation.x + (heightoffset / 4) + fontsize, Top[1] + fontsize, text.color.r, text.color.g, text.color.b);
					Top[1] += fontsize;
					break;
				}

			}

		}
	}

	if ((Inputs::keydown(VK_LBUTTON) || settings::autolock) && settings::aimbot)
	{

		D3DXVECTOR3 BONE = D3DXVECTOR3();
		std::vector<int> HitBoxesToScan{};
		switch (settings::aimbothitbox)
		{
		case 0:
			HitBoxesToScan.push_back(BoneId::head);
			break;
		case 1:
			HitBoxesToScan.push_back(BoneId::neck);
			break;
		case 2:
			HitBoxesToScan.push_back(BoneId::chest);
			break;
		default:
			break;
		}
		for (auto hitbox : HitBoxesToScan)
		{
			if (hitbox != -1)
			{
				if (gm.ClosestCrosshairEnemy.GetEntityBonePosition(hitbox, BONE)) {

					D3DXVECTOR3 Hitbox = gm.CalcSoldierFuturePos(D3DXVECTOR3(BONE.x, BONE.y, BONE.z));
					if (GetCrossDistance(Hitbox.x, Hitbox.y, S_width / 2, S_height / 2) <= settings::aimbotfov * 8)
					{
						auto roundPos = D3DXVECTOR2((float)round(Hitbox.x), (float)round(Hitbox.y));
						AimAtPosV2(S_width, S_height, roundPos.x, roundPos.y, settings::aimspeed, settings::aimlock, false);
						if (settings::aimbotdrawhitbox)
							DrawCircle(roundPos.x, roundPos.y, 3, 3, 1.f, 0.f, 0.f, 1.f, true);
						
					}
				}
			}
		}
	}
	if (settings::aimbotdrawfov)
		DrawCircle(S_width / 2, S_height / 2, settings::aimbotfov * 8, 1, 255, 255, 255, 255, false);

}

DWORD64 GetEntityById(int Ent, DWORD64 Base)
{
	SPOOF_FUNC;
	DWORD64 EntityList = Base + m_cl_entitylist; //updated
	DWORD64 BaseEntity = Read<DWORD64>(EntityList);
	if (!BaseEntity)
		return NULL;
	return  Read<DWORD64>(EntityList + (Ent << 5));
}


void setupPlayers()
{
	SPOOF_FUNC;
	int dis = 0;
	std::vector<DWORD_PTR> Players = std::vector<DWORD_PTR>();
	while (true)
	{
		DWORD_PTR  ViewRender = Read<DWORD_PTR>(base + m_view_render);
		DWORD_PTR View_matrix = Read<DWORD_PTR>(ViewRender + m_view_matrix);
		vMatrix = Read<D3DMATRIX>(View_matrix);
		MyLocalplayer = Read<DWORD_PTR>(base + m_LocalPlayer);
		D3DXVECTOR3 Localorigin = Read<D3DXVECTOR3>(MyLocalplayer + m_entity_origin);

		for (size_t i = 0; i < 64; i++)
		{
			DWORD64 Entity = GetEntityById(i, base);
			if (Entity == 0)
				continue;
			DWORD64 EntityHandle = Read<DWORD64>(Entity + OFFSET_NAME);
			std::string Identifier = Read<std::string>(EntityHandle);
			LPCSTR IdentifierC = Identifier.c_str();
			if (strcmp(IdentifierC, "player"))
			{
				D3DXVECTOR3 origin = Read<D3DXVECTOR3>(Entity + m_entity_origin);
				if (origin.x == NULL)
					continue;
				dis = Distance(Localorigin, origin);
				if (dis > 300)
					continue;
				Players.push_back(Entity);
			}
		}
		EntityList = Players;
		Players.clear();

	}
}

void OverlaySetup()
{
	SPOOF_FUNC;
	DirectOverlaySetup(drawLoop,FindWindow(NULL,"Apex Legends"));
}
int main()
{
	SPOOF_FUNC;

	if (mem::find_driver() == false)
	{
		std::cout << "Driver Not Loaded";
		Sleep(10000);
		exit(1);
	}

	system("CLS");
	std::cout << XorStr("\n[~] Initializing... ");

	Sleep(1000);

	printf(XorStr("\n[~] Waiting For r5apex.exe").c_str());

	mouse_interface();

	while (!hwnd)
	{
		hwnd = FindWindowA_Spoofed(0, XorStr("Apex Legends").c_str());
		Sleep(500);
	}

	oPID = mem::find_process("r5apex.exe");
	if (oPID == 0)
	{
		std::cout << "get Apex process fail\n";
	}

	base = mem::find_image();
	if (base == NULL)
	{
		std::cout << "get Apex adress fail\n";
	}

	SonyDriverHelper::api::Init();

	std::cout << (XorStr("Base Address = ").c_str()) << (void*)base << std::endl;

	InitItemName();
	ReadSettings();

	std::thread(setupPlayers).detach();
	std::thread(OverlaySetup).detach();
	getchar();
}
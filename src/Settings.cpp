#include "Settings.h"

void Settings::LoadSettings()
{
	constexpr auto path = L"Data/SKSE/Plugins/po3_LockpickShivSwapper.ini";

	CSimpleIniA ini;
	ini.SetUnicode();

	ini.LoadFile(path);

	ini::get_value(ini, enableDaggerSwap, "Dagger", "Enable", ";Switch out vanilla shiv for a dagger in your inventory");
	ini::get_value(ini, useHighestAvailableDagger, "Dagger", "UseHighestDamageDagger", ";If false, use the lowest damage dagger available");

	ini::get_value(ini, editor.toggleKey, "Editor", "ToggleKey", ";Keyboard scan codes : https://wiki.nexusmods.com/index.php/DirectX_Scancodes_And_How_To_Use_Them\n;Toggle positioning editor. Set this to 0 to disable it. Default key : LeftAlt");
	ini::get_value(ini, editor.toggleKey, "Editor", "AltActionKey", ";Default key : LeftShift");
	ini::get_value(ini, editor.boostLightingInEditor, "Editor", "BoostLighting", ";Increase light intensity in edit mode.");
	ini::get_value(ini, editor.moveAmount, "Editor", "MoveAmount", nullptr);
	ini::get_value(ini, editor.scaleAmount, "Editor", "ScaleAmount", nullptr);

	editor.assign_keybind(ini, KEY_BIND::kX_Up, "EditorKeyBinds", "MoveXUp", ";Hold alt action key to move in Y axis. Default key : Down");
	editor.assign_keybind(ini, KEY_BIND::kX_Down, "EditorKeyBinds", "MoveXDown", ";Hold alt action key to move in Y axis. ;Default key : Up");
	editor.assign_keybind(ini, KEY_BIND::kZ_Up, "EditorKeyBinds", "MoveZUp", ";Default key : Right");
	editor.assign_keybind(ini, KEY_BIND::kZ_Down, "EditorKeyBinds", "MoveZDown", ";Default key : Left");
	editor.assign_keybind(ini, KEY_BIND::kFlip, "EditorKeyBinds", "Flip", ";Flip model. Default key : Mouse Button 3");
	editor.assign_keybind(ini, KEY_BIND::kScaleUp, "EditorKeyBinds", "ScaleUp", ";Default key : ScrollWheel Up");
	editor.assign_keybind(ini, KEY_BIND::kScaleDown, "EditorKeyBinds", "ScaleDown", ";Default key : ScrollWheel Down");
	editor.assign_keybind(ini, KEY_BIND::kSavePreset, "EditorKeyBinds", "SavePreset", ";Save values to preset. Default key : Enter");
	editor.assign_keybind(ini, KEY_BIND::kReloadPreset, "EditorKeyBinds", "ReloadPreset", ";Reload values from preset. Hold alt action key to recalculate best position;Default key : R");

	(void)ini.SaveFile(path);
}

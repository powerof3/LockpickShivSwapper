#include "Settings.h"

void Settings::LoadSettings()
{
	constexpr auto path = L"Data/SKSE/Plugins/po3_LockpickShivSwapper.ini";

	CSimpleIniA ini;
	ini.SetUnicode();

	ini.LoadFile(path);

	ini::get_value(ini, enableDaggerSwap, "Dagger", "Enable", ";Switch out vanilla shiv for a dagger in your inventory");
	ini::get_value(ini, useHighestAvailableDagger, "Dagger", "UseHighestDamageDagger", ";If false, use the lowest damage dagger available");
	ini::get_value(ini, hideEquippedDagger, "Dagger", "HideEquippedDagger", ";Hide equipped dagger in hand when using it to lockpick");
	ini::get_value(ini, blacklistedKeywords, "Dagger", "BlacklistedKeywords", ";Don't show daggers with these keywords (eg. WeapMaterialGlass,WeapMaterialElven)", ",");

	ini::get_value(ini, editor.boostLightingInEditor, "TransformEditor", "BoostLighting", ";Edit model position,scale,rotation in-game\n\n;Increase light intensity in edit mode");
	ini::get_value(ini, editor.moveAmount, "TransformEditor", "MoveAmount", "; ");
	ini::get_value(ini, editor.scaleAmount, "TransformEditor", "ScaleAmount", nullptr);

	ini::get_value(ini, editor.toggleKey, "TransformEditorKeyBinds", "ToggleKey", ";DXScanCodes : https://www.creationkit.com/index.php?title=Input_Script\n\n;Toggle transform editor. Set this to 0 to disable it. Default key : T");
	ini::get_value(ini, editor.altActionKey, "TransformEditorKeyBinds", "AltActionKey", ";Default key : LeftShift");

	editor.assign_keybind(ini, KEY_BIND::kX_Up, "TransformEditorKeyBinds", "MoveXUp", ";Hold alt action key to move in Y axis. Default key : Down");
	editor.assign_keybind(ini, KEY_BIND::kX_Down, "TransformEditorKeyBinds", "MoveXDown", ";Hold alt action key to move in Y axis. ;Default key : Up");
	editor.assign_keybind(ini, KEY_BIND::kZ_Up, "TransformEditorKeyBinds", "MoveZUp", ";Default key : Right");
	editor.assign_keybind(ini, KEY_BIND::kZ_Down, "TransformEditorKeyBinds", "MoveZDown", ";Default key : Left");
	editor.assign_keybind(ini, KEY_BIND::kFlip, "TransformEditorKeyBinds", "Flip", ";Flip model. Default key : Mouse Button 3");
	editor.assign_keybind(ini, KEY_BIND::kScaleUp, "TransformEditorKeyBinds", "ScaleUp", ";Default key : ScrollWheel Up");
	editor.assign_keybind(ini, KEY_BIND::kScaleDown, "TransformEditorKeyBinds", "ScaleDown", ";Default key : ScrollWheel Down");
	editor.assign_keybind(ini, KEY_BIND::kSavePreset, "TransformEditorKeyBinds", "SavePreset", ";Save values to preset. Default key : Enter");
	editor.assign_keybind(ini, KEY_BIND::kReloadPreset, "TransformEditorKeyBinds", "ReloadPreset", ";Reload values from preset. Hold alt action key to recalculate best position. Default key : R");

	(void)ini.SaveFile(path);
}

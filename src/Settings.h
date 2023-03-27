#pragma once

class Settings : public ISingleton<Settings>
{
public:
	enum class KEY_BIND
	{
		kX_Up,         // Down
		kX_Down,       // Up
		kZ_Up,         // Right
		kZ_Down,       // Left
		kFlip,         // Mouse3
		kScaleUp,      // WheelUp
		kScaleDown,    // WheelDown
		kSavePreset,   // Enter
		kReloadPreset  // R
	};

	void LoadSettings();

	// members
	bool enableDaggerSwap{ true };
	bool useHighestAvailableDagger{ true };
	bool hideEquippedDagger{ true };

	struct
	{
		void assign_keybind(CSimpleIniA& a_ini, KEY_BIND a_keyBind, const char* a_section, const char* a_key, const char* a_comment)
		{
			const auto value = string::to_num<std::uint32_t>(a_ini.GetValue(a_section, a_key, std::to_string(keyBinds.value(a_keyBind)).c_str()));
			a_ini.SetValue(a_section, a_key, std::to_string(value).c_str(), a_comment);

			keyBinds.assign(a_keyBind, value);
		}

		std::uint32_t toggleKey{ RE::BSKeyboardDevice::Key::kT };
		std::uint32_t altActionKey{ RE::BSKeyboardDevice::Key::kLeftShift };

		BiMap<KEY_BIND, std::uint32_t> keyBinds{
			{ KEY_BIND::kX_Up, RE::BSKeyboardDevice::Key::kDown },
			{ KEY_BIND::kX_Down, RE::BSKeyboardDevice::Key::kUp },
			{ KEY_BIND::kZ_Up, RE::BSKeyboardDevice::Key::kRight },
			{ KEY_BIND::kZ_Down, RE::BSKeyboardDevice::Key::kLeft },
			{ KEY_BIND::kFlip, RE::BSWin32MouseDevice::Key::kButton3 + 256 },
			{ KEY_BIND::kScaleUp, RE::BSWin32MouseDevice::Key::kWheelUp + 256 },
			{ KEY_BIND::kScaleDown, RE::BSWin32MouseDevice::Key::kWheelDown + 256 },
			{ KEY_BIND::kSavePreset, RE::BSKeyboardDevice::Key::kEnter },
			{ KEY_BIND::kReloadPreset, RE::BSKeyboardDevice::Key::kR }
		};

		float moveAmount{ 0.1f };
		float scaleAmount{ 0.5f };

		bool boostLightingInEditor{ true };

	} editor;
};

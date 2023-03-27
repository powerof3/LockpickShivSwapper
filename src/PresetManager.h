#pragma once

struct ModelData
{
	std::string     presetPath;
	RE::NiTransform transform;
};

struct ModelOutput
{
	std::string              modelPath;
	std::optional<ModelData> modelData;
	std::string              nodeName;
	bool                     isDagger{ false };
};

class PresetManager : public ISingleton<PresetManager>
{
public:
	ModelOutput GetModel();

	void LoadPresets();

	void ReadPreset(RE::NiTransform& a_transform) const;
	void WritePreset(const RE::NiTransform& a_transform);
	void LogAction(std::string_view a_action) const;

	void Clear();

private:
	struct Dagger
	{
		static inline constexpr std::string_view defaultPreset{ R"(Data\ShivSwapper\DefaultPreset.ini)" };

	    static RE::TESObjectWEAP* GetRandom();
		static RE::TESObjectWEAP* GetEquipped();
		static RE::TESObjectWEAP* GetBest();

	private:
		static RE::TESObjectWEAP* IsDagger(RE::TESForm* a_form);
	};

	// members
	std::string presetPath;
	std::string truncPresetPath;
	std::string modelPath;

	bool isDagger{ false };

	StringMap<ModelData> daggersMap{};
	StringMap<ModelData> modelMap{};

	static inline constexpr std::string_view daggerEntry{ "DAGGER|" };
	static inline constexpr std::string_view modelEntry{ "MODEL|" };
};

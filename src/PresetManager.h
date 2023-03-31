#pragma once

#include "ModelData.h"

class PresetManager : public ISingleton<PresetManager>
{
public:
	Model::Output GetModel();

	void LoadPresets();
	void LoadConditions();

	void ReadPreset(RE::NiTransform& a_transform) const;
	void WritePreset(const RE::NiTransform& a_transform);
	void LogAction(std::string_view a_action) const;

	void ClearCurrentData();

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
	StringMap<DaggerData>  daggersMap{};
	std::vector<ModelData> modelVec{};

    std::string  presetPath;
	std::string  truncPresetPath;
	std::string  modelPath;
	std::int32_t modelVecIndex{ -1 };

	static inline constexpr std::string_view daggerEntry{ "DAGGER|" };
	static inline constexpr std::string_view modelEntry{ "MODEL|" };

	static inline srell::regex transformRegex{ R"(^([-+]?\d*\.\d+),([-+]?\d*\.\d+),([-+]?\d*\.\d+)$)" };
};

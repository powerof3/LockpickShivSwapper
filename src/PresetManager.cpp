#include "PresetManager.h"

#include "Settings.h"

RE::TESObjectWEAP* PresetManager::Dagger::IsDagger(RE::TESForm* a_form)
{
	const auto weap = a_form ? a_form->As<RE::TESObjectWEAP>() : nullptr;
	if (weap && weap->IsOneHandedDagger() && weap->GetPlayable()) {
		return weap;
	}
	return nullptr;
}

RE::TESObjectWEAP* PresetManager::Dagger::GetRandom()
{
	static StringSet tempDaggers;
	for (const auto& weapon : RE::TESDataHandler::GetSingleton()->GetFormArray<RE::TESObjectWEAP>()) {
		if (weapon->IsOneHandedDagger() && !weapon->IsBound() && !tempDaggers.contains(weapon->GetModel())) {
			tempDaggers.emplace(weapon->GetModel());
			return weapon;
		}
	}
	return nullptr;
}

RE::TESObjectWEAP* PresetManager::Dagger::GetEquipped()
{
	const auto player = RE::PlayerCharacter::GetSingleton();

	if (const auto rHand = player->GetEquippedObject(false)) {
		if (const auto rWeap = IsDagger(rHand)) {
			return rWeap;
		}
	}
	if (const auto lHand = player->GetEquippedObject(true)) {
		if (const auto lWeap = IsDagger(lHand)) {
			return lWeap;
		}
	}

	return nullptr;
}

RE::TESObjectWEAP* PresetManager::Dagger::GetBest()
{
	const bool useHighestDagger = Settings::GetSingleton()->useHighestAvailableDagger;

	std::pair<RE::TESObjectWEAP*, float> best{ nullptr, 0.0f };
	auto                                 highestDmg = useHighestDagger ? 0.0f : std::numeric_limits<float>::max();

	const auto player = RE::PlayerCharacter::GetSingleton();

	const auto inventory = player->GetInventory();
	for (auto& [item, data] : inventory) {
		auto& [count, entry] = data;
		if (const auto dagger = IsDagger(item); dagger && count > 0) {
			auto dmg = player->GetDamage(entry.get());
			if (useHighestDagger ? dmg > highestDmg : dmg < highestDmg) {
				highestDmg = dmg;
				best = { dagger, dmg };
			}
		}
	}

	return best.first;
}

ModelOutput PresetManager::GetModel()
{
	ModelOutput output;

	if (Settings::GetSingleton()->enableDaggerSwap) {
		auto dagger = Dagger::GetEquipped();
		if (dagger) {
			output.nodeName = fmt::format("{}  ({:08X})", "Weapon", dagger->GetFormID());
		}
		if (!dagger) {
			dagger = Dagger::GetBest();
		}
		if (dagger) {
			output.modelPath = dagger->GetModel();
			output.isDagger = true;
		}
	}

	if (modelPath = output.modelPath; !modelPath.empty()) {
		isDagger = output.isDagger;

		const auto& map = isDagger ? daggersMap : modelMap;
		if (const auto it = map.find(modelPath); it != map.end()) {
			output.modelData = it->second;
		}

		if (output.modelData) {
			presetPath = output.modelData->presetPath;
		} else if (isDagger) {
			presetPath = Dagger::defaultPreset;
		}
		truncPresetPath = presetPath.substr(5);
	}

	return output;
}

void PresetManager::LoadPresets()
{
	std::vector<std::string> configs;

	const std::filesystem::path            folderPath{ R"(Data\ShivSwapper)" };
	const std::filesystem::directory_entry directory{ folderPath };
	if (!directory.exists()) {
		std::filesystem::create_directory(folderPath);
	}
	const std::filesystem::directory_iterator directoryIt{ folderPath };

	for (const auto& entry : directoryIt) {
		if (entry.exists()) {
			if (const auto& path = entry.path(); !path.empty() && path.extension() == ".ini"sv) {
				configs.push_back(entry.path().string());
			}
		}
	}

	if (configs.empty()) {
		logger::info("No presets found within {}...", folderPath.string());
		return;
	}

	std::ranges::sort(configs);
	if (const auto it = std::ranges::find(configs, Dagger::defaultPreset); it != configs.end()) {
		std::rotate(configs.begin(), it, it + 1);
	}

	logger::info("{} configs found in {}", configs.size(), folderPath.string());

	for (auto& path : configs) {
		logger::info("\tINI : {}", path);

		CSimpleIniA ini;
		ini.SetUnicode();
		ini.SetAllowKeyOnly();

		if (const auto rc = ini.LoadFile(path.c_str()); rc < 0) {
			logger::error("	couldn't read INI");
			continue;
		}

		CSimpleIniA::TNamesDepend sections;
		ini.GetAllSections(sections);
		sections.sort(CSimpleIniA::Entry::LoadOrder());

		static constexpr srell::regex transformRegex{ R"(^([-+]?\d*\.\d+),([-+]?\d*\.\d+),([-+]?\d*\.\d+)$)" };

		for (auto& [section, comment, keyOrder] : sections) {
			RE::NiTransform transform{};

			if (const auto values = ini.GetSection(section); values && !values->empty()) {
				for (const auto& [key, value] : *values) {
					if (string::iequals(key.pItem, "Translation")) {
						if (srell::cmatch match; srell::regex_search(value, match, transformRegex)) {
							//match[0] gets the whole string
							transform.translate.x = string::to_num<float>(match[1].str());
							transform.translate.y = string::to_num<float>(match[2].str());
							transform.translate.z = string::to_num<float>(match[3].str());
						}
					} else if (string::iequals(key.pItem, "Rotation")) {
						if (srell::cmatch match; srell::regex_search(value, match, transformRegex)) {
							auto angleX = string::to_num<float>(match[1].str());
							auto angleY = string::to_num<float>(match[2].str());
							auto angleZ = string::to_num<float>(match[3].str());

							transform.rotate.SetEulerAnglesXYZ(RE::deg_to_rad(angleX), RE::deg_to_rad(angleY), RE::deg_to_rad(angleZ));
						}
					} else {
						transform.scale = string::to_num<float>(value);
					}
				}
				if (auto splitSection = string::split(section, "|"); splitSection[0] == "DAGGER") {
					daggersMap.insert_or_assign(splitSection[1], ModelData(path, transform));
				} else {
					daggersMap.insert_or_assign(splitSection[1], ModelData(path, transform));
				}
			}
		}
	}
}

void PresetManager::LogAction(std::string_view a_action) const
{
	RE::DebugNotification(a_action.data());
	RE::ConsoleLog::GetSingleton()->Print(fmt::format("[{}] [{}] : {}", truncPresetPath, modelPath, a_action).c_str());
}

void PresetManager::ReadPreset(RE::NiTransform& a_transform) const
{
	CSimpleIniA ini;
	ini.SetUnicode();

	ini.LoadFile(presetPath.c_str());

	std::string entry;
	if (isDagger) {
		entry = std::string(daggerEntry) + modelPath;
	} else {
		entry = std::string(modelEntry) + modelPath;
	}

	std::string translate = ini.GetValue(entry.c_str(), "Translation");
	if (srell::cmatch match; srell::regex_search(translate.c_str(), match, transformRegex)) {
		//match[0] gets the whole string
		a_transform.translate.x = string::to_num<float>(match[1].str());
		a_transform.translate.y = string::to_num<float>(match[2].str());
		a_transform.translate.z = string::to_num<float>(match[3].str());
	}

	std::string rotate = ini.GetValue(entry.c_str(), "Rotation");
	if (srell::cmatch match; srell::regex_search(rotate.c_str(), match, transformRegex)) {
		auto angleX = string::to_num<float>(match[1].str());
		auto angleY = string::to_num<float>(match[2].str());
		auto angleZ = string::to_num<float>(match[3].str());
		a_transform.rotate.SetEulerAnglesXYZ(RE::deg_to_rad(angleX), RE::deg_to_rad(angleY), RE::deg_to_rad(angleZ));
	}

	a_transform.scale = string::to_num<float>(ini.GetValue(entry.c_str(), "Scale"));
}

void PresetManager::WritePreset(const RE::NiTransform& a_transform)
{
	CSimpleIniA ini;
	ini.SetUnicode();

	ini.LoadFile(presetPath.c_str());

	std::string entry;

	if (isDagger) {
		entry = std::string(daggerEntry) + modelPath;
		daggersMap.insert_or_assign(modelPath, ModelData(presetPath, a_transform));
	} else {
		entry = std::string(modelEntry) + modelPath;
		modelMap.insert_or_assign(modelPath, ModelData(presetPath, a_transform));
	}

	const auto translate = fmt::format("{:.06f},{:.06f},{:.06f}", a_transform.translate.x, a_transform.translate.y, a_transform.translate.z);
	ini.SetValue(entry.c_str(), "Translation", translate.c_str(), nullptr);

	float x, y, z;
	a_transform.rotate.ToEulerAnglesXYZ(x, y, z);
	ini.SetValue(entry.c_str(), "Rotation", fmt::format("{:.02f},{:.02f},{:.02f}", RE::rad_to_deg(x), RE::rad_to_deg(y), RE::rad_to_deg(z)).c_str(), nullptr);

	ini.SetValue(entry.c_str(), "Scale", fmt::format("{:.01f}", a_transform.scale).c_str(), nullptr);

	(void)ini.SaveFile(presetPath.data());
}

void PresetManager::Clear()
{
	modelPath.clear();
	presetPath.clear();
	truncPresetPath.clear();

	isDagger = false;
}
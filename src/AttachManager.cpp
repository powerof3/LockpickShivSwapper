#include "AttachManager.h"
#include "ModelManager.h"
#include "Settings.h"

void AttachManager::LoadPresets()
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
	if (const auto it = std::ranges::find(configs, defaultPreset); it != configs.end()) {
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

		for (auto& [section, comment, keyOrder] : sections) {
			RE::NiTransform modelTransform{};

			if (const auto values = ini.GetSection(section); values && !values->empty()) {
				for (const auto& [key, value] : *values) {
					if (string::iequals(key.pItem, "Translation")) {
						if (srell::cmatch match; srell::regex_search(value, match, transformRegex)) {
							//match[0] gets the whole string
							modelTransform.translate.x = string::to_num<float>(match[1].str());
							modelTransform.translate.y = string::to_num<float>(match[2].str());
							modelTransform.translate.z = string::to_num<float>(match[3].str());
						}
					} else if (string::iequals(key.pItem, "Rotation")) {
						if (srell::cmatch match; srell::regex_search(value, match, transformRegex)) {
							auto angleX = string::to_num<float>(match[1].str());
							auto angleY = string::to_num<float>(match[2].str());
							auto angleZ = string::to_num<float>(match[3].str());

							modelTransform.rotate.SetEulerAnglesXYZ(RE::deg_to_rad(angleX), RE::deg_to_rad(angleY), RE::deg_to_rad(angleZ));
						}
					} else {
						modelTransform.scale = string::to_num<float>(value);
					}
				}
			}

			daggersTransforms.insert_or_assign(section, TransformData(path, modelTransform));
		}
	}
}

void AttachManager::ProcessButtonHeld(RE::INPUT_DEVICE a_device, std::uint32_t a_key, bool a_altKeyHeld) const
{
	static auto settings = Settings::GetSingleton();
	static auto moveAmount = settings->editor.moveAmount;

	switch (a_device) {
	case RE::INPUT_DEVICE::kMouse:
		a_key += SKSE::InputMap::kMacro_MouseButtonOffset;
		break;
	case RE::INPUT_DEVICE::kGamepad:
		a_key = SKSE::InputMap::GamepadMaskToKeycode(a_key);
		break;
	default:
		break;
	}

	if (const auto key_bind = settings->editor.keyBinds.key(a_key)) {
		switch (*key_bind) {
		case Settings::KEY_BIND::kX_Up:
			{
				if (a_altKeyHeld) {
					modelHolder->local.translate.y -= moveAmount;
				} else {
					modelHolder->local.translate.x += moveAmount;
				}
			}
			break;
		case Settings::KEY_BIND::kX_Down:
			{
				if (a_altKeyHeld) {
					modelHolder->local.translate.y += moveAmount;
				} else {
					modelHolder->local.translate.x -= moveAmount;
				}
			}
			break;
		case Settings::KEY_BIND::kZ_Up:
			modelHolder->local.translate.z += moveAmount;
			break;
		case Settings::KEY_BIND::kZ_Down:
			modelHolder->local.translate.z -= moveAmount;
			break;
		default:
			break;
		}
	}
}

void AttachManager::ProcessButtonDown(RE::INPUT_DEVICE a_device, std::uint32_t a_key, bool a_altKeyHeld)
{
	static auto settings = Settings::GetSingleton();
	static auto scaleAmount = settings->editor.scaleAmount;

	switch (a_device) {
	case RE::INPUT_DEVICE::kMouse:
		a_key += SKSE::InputMap::kMacro_MouseButtonOffset;
		break;
	case RE::INPUT_DEVICE::kGamepad:
		a_key = SKSE::InputMap::GamepadMaskToKeycode(a_key);
		break;
	default:
		break;
	}

	if (const auto key_bind = settings->editor.keyBinds.key(a_key)) {
		switch (*key_bind) {
		case Settings::KEY_BIND::kSavePreset:
			{
				WritePreset();
				RE::DebugNotification(fmt::format("[{}] : saved", modelPath).c_str());
			}
			break;
		case Settings::KEY_BIND::kReloadPreset:
			{
				if (a_altKeyHeld) {
					CalculateLockAlignment(false);
					RE::DebugNotification(fmt::format("[{}] : reloaded", modelPath).c_str());
				} else {
					ReadPreset();
					RE::DebugNotification(fmt::format("[{}] : reloaded from preset", modelPath).c_str());
				}
			}
			break;
		case Settings::KEY_BIND::kFlip:
			{
				float x, y, z;
				modelHolder->local.rotate.ToEulerAnglesXYZ(x, y, z);

				x = (x == 0.0f) ? -RE::NI_PI : 0.0f;
				z = (z == 0.0f) ? -RE::NI_PI : 0.0f;

				modelHolder->local.rotate.SetEulerAnglesXYZ(x, y, z);
			}
			break;
		case Settings::KEY_BIND::kScaleUp:
			modelHolder->local.scale += scaleAmount;
			break;
		case Settings::KEY_BIND::kScaleDown:
			modelHolder->local.scale -= scaleAmount;
			break;
		default:
			break;
		}
	}
}

void AttachManager::WritePreset()
{
	CSimpleIniA ini;
	ini.SetUnicode();

	ini.LoadFile(presetPath.data());

	const auto& transform = modelHolder->local;
	daggersTransforms.insert_or_assign(modelPath, TransformData(presetPath, transform));

	const auto translate = fmt::format("{:.06f},{:.06f},{:.06f}", transform.translate.x, transform.translate.y, transform.translate.z);
	ini.SetValue(modelPath.c_str(), "Translation", translate.c_str(), nullptr);

	float x, y, z;
	transform.rotate.ToEulerAnglesXYZ(x, y, z);
	ini.SetValue(modelPath.c_str(), "Rotation", fmt::format("{:.02f},{:.02f},{:.02f}", RE::rad_to_deg(x), RE::rad_to_deg(y), RE::rad_to_deg(z)).c_str(), nullptr);

	ini.SetValue(modelPath.c_str(), "Scale", fmt::format("{:.01f}", transform.scale).c_str(), nullptr);

	(void)ini.SaveFile(presetPath.data());
}

void AttachManager::ReadPreset() const
{
	CSimpleIniA ini;
	ini.SetUnicode();

	ini.LoadFile(presetPath.data());

	auto& transform = modelHolder->local;

	std::string translate = ini.GetValue(modelPath.c_str(), "Translation");
	if (srell::cmatch match; srell::regex_search(translate.c_str(), match, transformRegex)) {
		//match[0] gets the whole string
		transform.translate.x = string::to_num<float>(match[1].str());
		transform.translate.y = string::to_num<float>(match[2].str());
		transform.translate.z = string::to_num<float>(match[3].str());
	}

	std::string rotate = ini.GetValue(modelPath.c_str(), "Rotation");
	if (srell::cmatch match; srell::regex_search(rotate.c_str(), match, transformRegex)) {
		auto angleX = string::to_num<float>(match[1].str());
		auto angleY = string::to_num<float>(match[2].str());
		auto angleZ = string::to_num<float>(match[3].str());
		transform.rotate.SetEulerAnglesXYZ(RE::deg_to_rad(angleX), RE::deg_to_rad(angleY), RE::deg_to_rad(angleZ));
	}

	transform.scale = string::to_num<float>(ini.GetValue(modelPath.c_str(), "Scale"));
}

RE::TESObjectWEAP* AttachManager::GetRandomDagger()
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

RE::TESObjectWEAP* AttachManager::GetBestDagger()
{
	constexpr auto is_dagger = [](RE::TESForm* a_form) -> RE::TESObjectWEAP* {
		const auto weap = a_form ? a_form->As<RE::TESObjectWEAP>() : nullptr;
		if (weap && weap->IsOneHandedDagger() && weap->GetPlayable()) {
			return weap;
		}
		return nullptr;
	};

	const auto player = RE::PlayerCharacter::GetSingleton();
	if (const auto rHand = player->GetEquippedObject(false)) {
		if (const auto rWeap = is_dagger(rHand)) {
			return rWeap;
		}
	}
	if (const auto lHand = player->GetEquippedObject(true)) {
		if (const auto lWeap = is_dagger(lHand)) {
			return lWeap;
		}
	}

	std::map<RE::TESObjectWEAP*, float> map;
	auto                                highestDmg = 0.0f;

	const auto inventory = player->GetInventory();
	for (auto& [item, data] : inventory) {
		auto& [count, entry] = data;
		if (const auto dagger = is_dagger(item); dagger && count > 0) {
			auto dmg = player->GetDamage(entry.get());
			map.emplace(dagger, dmg);
			if (dmg > highestDmg) {
				highestDmg = dmg;
			}
		}
	}

	const auto it = std::ranges::find_if(map, [highestDmg](const auto& mo) {
		return numeric::essentially_equal(mo.second, highestDmg);
	});
	if (it != map.end()) {
		return it->first;
	}

	return nullptr;
}

void AttachManager::CalculateLockAlignment(bool a_serialize)
{
	modelHolder->local = ModelManager::GetSingleton()->CalculateBestLockAlignment();

	if (a_serialize) {
		WritePreset();
	}
}

auto AttachManager::AttachModel(const std::string& a_path, const TransformData& a_transformData, const RE::NiPointer<RE::NiNode>& a_shivNode) -> RE::BSResource::ErrorCode
{
	RE::NiPointer<RE::NiNode> loadedModel{};
	RE::BSResource::ErrorCode error = RE::BSModelDB::Demand(a_path.c_str(), loadedModel, RE::BSModelDB::DBTraits::ArgsType());

	if (error == RE::BSResource::ErrorCode::kNone) {
		logger::info("loading {}", a_path);

		modelPath = a_path;
		presetPath = a_transformData.presetPath;

		// Hide vanilla shiv geometry
		a_shivNode->CullGeometry(true);

		// Create holder node which will be attached to shivNode
		modelHolder = RE::NiPointer(RE::NiNode::Create(0));

		// transfer valid ninode/geometry from the loaded model to the modelHolder
		ModelManager::GetSingleton()->ProcessModel(loadedModel, modelHolder);

		if (a_transformData.transform.translate != RE::NiPoint3()) {
			modelHolder->local = a_transformData.transform;
		} else {
			CalculateLockAlignment(true);
		}

		a_shivNode->AttachChild(modelHolder.get());

		// doesn't actually do anything? Parent controller overrides all local animations
		// todo : see if existing sequences can be appending to the controller manager
		RE::NiTimeController::StartAnimations(modelHolder.get());
	}

	loadedModel.reset();

	return error;
}

void AttachManager::TryAttachModel(const RE::NiPointer<RE::NiNode>& a_shivNode)
{
	auto dagger = GetBestDagger();
	if (!dagger) {
		dagger = GetRandomDagger();
	}
	if (dagger) {
		if (auto model = dagger->GetModel(); !string::is_empty(model)) {
			if (const auto error = AttachModel(model, daggersTransforms[model], a_shivNode); error != RE::BSResource::ErrorCode::kNone) {
				logger::info("failed to load {} [{}]", model, stl::to_underlying(error));
			}
		}
	}
}

void AttachManager::ClearModels()
{
	ModelManager::GetSingleton()->Clear();

	modelPath.clear();
	presetPath.clear();
	modelHolder.reset();
}

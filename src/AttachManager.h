#pragma once

class AttachManager : public ISingleton<AttachManager>
{
public:
	void LoadPresets();

	void ProcessButtonHeld(RE::INPUT_DEVICE a_device, std::uint32_t a_key, bool a_altKeyHeld) const;
	void ProcessButtonDown(RE::INPUT_DEVICE a_device, std::uint32_t a_key, bool a_altKeyHeld);

	void TryAttachModel(const RE::NiPointer<RE::NiNode>& a_shivNode);
	void ClearModels();

private:
	struct TransformData
	{
		std::string     presetPath;
		RE::NiTransform transform;
	};

	static inline srell::regex     transformRegex{ R"(^([-+]?\d*\.\d+),([-+]?\d*\.\d+),([-+]?\d*\.\d+)$)" };
	static inline std::string_view defaultPreset{ R"(Data\ShivSwapper\DefaultPreset.ini)" };

	static RE::TESObjectWEAP* GetRandomDagger();
	static RE::TESObjectWEAP* GetBestDagger();

	void                      ReadPreset() const;
	void                      WritePreset();
	void                      CalculateLockAlignment(bool a_serialize);
	RE::BSResource::ErrorCode AttachModel(const std::string& a_path, const TransformData& a_transformData, const RE::NiPointer<RE::NiNode>& a_shivNode);

	// members
	std::string               modelPath;
	std::string               presetPath;
	RE::NiPointer<RE::NiNode> modelHolder;

	StringMap<TransformData> daggersTransforms;
};

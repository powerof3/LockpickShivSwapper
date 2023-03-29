#pragma once

namespace Model
{
	struct Data;
}

class ModelManager : public ISingleton<ModelManager>
{
public:
	void ProcessButtonHeld(RE::INPUT_DEVICE a_device, std::uint32_t a_key, bool a_altKeyHeld) const;
	void ProcessButtonDown(RE::INPUT_DEVICE a_device, std::uint32_t a_key, bool a_altKeyHeld);

	void TryAttachModel(const RE::NiPointer<RE::NiNode>& a_shivNode);
	void ClearModels();

private:
	// pre-calculated values for vanilla lockpick shiv
	struct ReferenceShiv
	{
		static inline constexpr RE::NiPoint3 translate{ 0.078537f, -128.888962f, 0.492452f };
		static inline constexpr RE::NiPoint3 startPoint{ 0.649953f, 6.23654f, 1.74555f };
	};

	void        ReadPreset() const;
	void        WritePreset() const;
	static void LogAction(std::string_view a_action);

	void         GetMinimalEnclosingSphere(const std::vector<RE::NiBound>& a_boundingSpheres);
	RE::NiPoint3 FindStartPoint(const RE::NiPoint3& a_offset);
	RE::NiPoint3 FindMidPoint();
	void         CalculateLockAlignment(bool a_serialize);

	void ProcessModel(const RE::NiNodePtr& loadedNode);
	bool AttachModel(const RE::NiNodePtr& a_shivNode, const RE::NiNodePtr& a_loadedModel, const std::optional<Model::Data>& a_modelData);

	void CullDagger(bool a_cull) const;

	// members
	RE::NiPointer<RE::NiNode> modelHolder{};
	RE::NiBound               minimalEnclosingSphere{};
	RE::NiTransform           bestLockAlignment{};

	bool        typeDagger{ false };
	std::string daggerNodeName{};
};

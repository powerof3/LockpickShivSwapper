#pragma once

class ModelManager : public ISingleton<ModelManager>
{
public:
	void            ProcessModel(const RE::NiPointer<RE::NiNode>& originalNode, const RE::NiPointer<RE::NiNode>& a_newNode);
	RE::NiTransform CalculateBestLockAlignment();
	void            Clear();

private:
	// pre-calculated values for vanilla lockpick shiv
	struct ReferenceShiv
	{
		static inline constexpr RE::NiPoint3 translate{ 0.078537f, -128.888962f, 0.492452f };
		static inline constexpr RE::NiPoint3 startPoint{ 0.649953f, 6.23654f, 1.74555f };
	};

	RE::NiBound         GetMinimalEnclosingSphere();
	static RE::NiPoint3 FindMidPoint(const RE::NiBound& a_minimalEnclosingSphere);
	static RE::NiPoint3 FindStartPoint(const RE::NiBound& a_minimalEnclosingSphere, const RE::NiPoint3& a_offset);

	// members
	std::vector<RE::NiBound> boundingSpheres{};
	RE::NiBound              minimalEnclosingSphere{};
};

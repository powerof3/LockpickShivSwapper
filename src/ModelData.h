#pragma once

namespace Model
{
	struct Data
	{
		std::string                    presetPath;
		std::optional<RE::NiTransform> transform;
	};

	struct ConditionalData : Data
	{
		std::vector<std::string>          rawConditions;
		std::unique_ptr<RE::TESCondition> conditions;
	};

	struct Output
	{
		std::string         modelPath;
		std::optional<Data> modelData;
		std::string         nodeName;
		std::int32_t        index{ -1 };  // for user with model vector
	};
}

using DaggerData = Model::Data;

struct ModelData
{
	std::string            path;
	Model::ConditionalData data;
};

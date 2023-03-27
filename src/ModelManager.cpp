#include "ModelManager.h"

#include "ModelManager.h"
#include "Settings.h"

void ModelManager::ReadPreset() const
{
	ModelManager::GetSingleton()->ReadPreset(modelHolder->local);
}

void ModelManager::WritePreset() const
{
	ModelManager::GetSingleton()->WritePreset(modelHolder->local);
}

void ModelManager::LogAction(std::string_view a_action)
{
	ModelManager::GetSingleton()->LogAction(a_action);
}

void ModelManager::ProcessButtonHeld(RE::INPUT_DEVICE a_device, std::uint32_t a_key, bool a_altKeyHeld) const
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

void ModelManager::ProcessButtonDown(RE::INPUT_DEVICE a_device, std::uint32_t a_key, bool a_altKeyHeld)
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
				LogAction("Saved tranforms to preset");
			}
			break;
		case Settings::KEY_BIND::kReloadPreset:
			{
				if (a_altKeyHeld) {
					CalculateLockAlignment(false);
					LogAction("Transforms recalculated");
				} else {
					ReadPreset();
					LogAction("Reloaded transforms from preset");
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

void ModelManager::GetMinimalEnclosingSphere(const std::vector<RE::NiBound>& a_boundingSpheres)
{
	// initialize the enclosing sphere to the first bound
	RE::NiBound enclosingSphere = a_boundingSpheres[0];

	// loop through the remaining bounds
	for (auto const& bound : a_boundingSpheres | std::views::drop(1)) {
		// check if the new bound is completely enclosed by the current enclosing sphere
		const auto distance = bound.center.GetDistance(enclosingSphere.center);
		if (distance + bound.radius <= enclosingSphere.radius) {
			continue;  // the new bound is already enclosed, no need to update the enclosing sphere
		}

		// the new bound is not completely enclosed, calculate the minimal enclosing sphere
		const float        newRadius = (distance + bound.radius + enclosingSphere.radius) / 2.0f;
		const RE::NiPoint3 newCenter = enclosingSphere.center + (bound.center - enclosingSphere.center) * (newRadius - enclosingSphere.radius) / distance;

		enclosingSphere.center = newCenter;
		enclosingSphere.radius = newRadius;
	}

	minimalEnclosingSphere = enclosingSphere;
}

RE::NiPoint3 ModelManager::FindStartPoint(const RE::NiPoint3& a_offset)
{
	auto [center, radius] = minimalEnclosingSphere;
	center += a_offset;

	// Bisect the sphere along the y-axis
	const RE::NiPoint3 startPoint = { 0.0f, radius, 0.0f };

	return { center + startPoint };
}

RE::NiPoint3 ModelManager::FindMidPoint()
{
	auto [center, radius] = minimalEnclosingSphere;
	center.y = ReferenceShiv::translate.y - radius;

	constexpr float targetY = ReferenceShiv::startPoint.y * 2.5f;
	const float     startPointY = FindStartPoint(center).y;

	// Adjust midpoint to match vanilla shiv start so the blade tip is aligned with lock, more or less
	if (startPointY < targetY) {
		center.y += targetY - startPointY;
	} else if (startPointY > targetY) {
		center.y += startPointY + targetY;
	}

	// Scale down the center point's z-coordinate if it is too large
	// Only happens with one dagger
	if (center.z > 10.0f) {
		center.z /= 10.0f;
	}

	return center;
}

void ModelManager::CalculateLockAlignment(bool a_serialize)
{
	if (bestLockAlignment.translate == RE::NiPoint3()) {
		minimalEnclosingSphere.radius *= 10.0f;
		minimalEnclosingSphere.center *= 10.0f;

		bestLockAlignment.translate = FindMidPoint();
		bestLockAlignment.scale = 10.0f;
	}

	modelHolder->local = bestLockAlignment;

	if (a_serialize) {
		WritePreset();
	}
}

void ModelManager::TryAttachModel(const RE::NiPointer<RE::NiNode>& a_shivNode)
{
	const auto [modelPath, nodeName, modelData, isDagger] = ModelManager::GetSingleton()->GetModel();

	if (!modelPath.empty()) {
		typeDagger = isDagger;
		daggerNodeName = nodeName;

		RE::NiNodePtr                               loadedModel;
		constexpr RE::BSModelDB::DBTraits::ArgsType args{};

		if (const auto error = Demand(modelPath.c_str(), loadedModel, args); error == RE::BSResource::ErrorCode::kNone) {
			logger::info("loading {}", modelPath);
			if (AttachModel(a_shivNode, loadedModel, modelData) && !daggerNodeName.empty()) {
				CullDagger(true);
			}
			loadedModel.reset();
		} else {
			logger::info("failed to load {} [{}]", modelPath, stl::to_underlying(error));
		}
	}
}

bool ModelManager::AttachModel(const RE::NiNodePtr& a_shivNode, const RE::NiNodePtr& a_loadedModel, const std::optional<ModelData>& a_modelData)
{
	// Hide vanilla shiv geometry
	a_shivNode->CullGeometry(true);

	// transfer valid ninode/geometry from the loaded model to the modelHolder
	ProcessModel(a_loadedModel);

	if (modelHolder) {
		if (a_modelData && a_modelData->transform.translate != RE::NiPoint3()) {
			modelHolder->local = a_modelData->transform;
		} else {
			CalculateLockAlignment(true);
		}

		RE::AttachNode(a_shivNode, modelHolder);

		// doesn't actually do anything? Parent controller overrides all local animations
		RE::NiTimeController::StartAnimations(modelHolder.get());

		return true;

	} else {
		// something went wrong, unhide vanilla shiv geometry
		a_shivNode->CullGeometry(false);
		return false;
	}
}

void ModelManager::CullDagger(bool a_cull) const
{
	if (!Settings::GetSingleton()->hideEquippedDagger) {
		return;
	}

	// bethesda hates left handed players
	// no stored left dagger in BipedAnim[kOneHandedDagger]
    if (const auto current3D = RE::PlayerCharacter::GetSingleton()->GetCurrent3D()) {
		if (const auto node = current3D->GetObjectByName(daggerNodeName)) {
			node->SetAppCulled(a_cull);
		}
	}
}

void ModelManager::ProcessModel(const RE::NiNodePtr& loadedNode)
{
	std::vector<RE::NiPointer<RE::NiAVObject>> objects{};
	std::vector<RE::NiBound>                   boundingSpheres{};

	objects.reserve(loadedNode->children.size());

	RE::BSVisit::TraverseScenegraphGeometries(loadedNode.get(), [&](RE::BSGeometry* a_geo) -> RE::BSVisit::BSVisitControl {
		// Ignore scabbards, blood geometry
		if (a_geo->GetAppCulled() || typeDagger && a_geo->name.contains("Scb")) {
			return RE::BSVisit::BSVisitControl::kContinue;
		}

		// apply parent node transform, if any
		// hopefully nothing is nested
		if (a_geo->parent) {
			a_geo->local.translate += a_geo->parent->local.translate;
		}

		// Get visible bound
		auto bound = a_geo->modelBound;
		bound.center += a_geo->local.translate;
		bound.center *= a_geo->local.scale;
		bound.radius *= a_geo->local.scale;
		boundingSpheres.emplace_back(bound);

		objects.emplace_back(a_geo->Clone());

		return RE::BSVisit::BSVisitControl::kContinue;
	});

	if (objects.empty()) {
		return;
	}

	modelHolder = RE::NiPointer(RE::NiNode::Create(static_cast<std::uint16_t>(objects.size())));

	for (const auto& object : objects) {
		RE::AttachNode(modelHolder, object);
	}

	// for calculating lock alignment later
    GetMinimalEnclosingSphere(boundingSpheres);

	objects.clear();
}

void ModelManager::ClearModels()
{
	ModelManager::GetSingleton()->Clear();

	RE::DetachNode(modelHolder);

	if (!daggerNodeName.empty()) {
		CullDagger(false);
		daggerNodeName.clear();
	}

	typeDagger = false;
}

#include "ModelManager.h"

void ModelManager::ProcessModel(const RE::NiPointer<RE::NiNode>& originalNode, const RE::NiPointer<RE::NiNode>& a_newNode)
{
	a_newNode->name = "NewShiv";

	std::vector<RE::NiPointer<RE::NiAVObject>> objects;
	objects.reserve(originalNode->children.size());

	for (auto& object : originalNode->children) {
		if (object && !object->GetAppCulled()) {
			objects.emplace_back(object);
		}
	}

	for (auto& object : objects) {
		RE::BSVisit::TraverseScenegraphGeometries(object.get(), [&](RE::BSGeometry* a_geo) -> RE::BSVisit::BSVisitControl {
		    // Hide scabbards
			if (a_geo->name.contains("Scb")) {
				a_geo->SetAppCulled(true);
				return RE::BSVisit::BSVisitControl::kContinue;
			}
			// Get visible bound
			if (!a_geo->GetAppCulled()) {
				auto bound = a_geo->modelBound;
				bound.center += a_geo->local.translate;
				bound.center *= a_geo->local.scale;
				bound.radius *= a_geo->local.scale;
				boundingSpheres.emplace_back(bound);
			}
			return RE::BSVisit::BSVisitControl::kContinue;
		});
	}

	for (auto& object : objects) {
		a_newNode->AttachChild(object.get());
	}
	objects.clear();
}

void ModelManager::Clear()
{
	boundingSpheres.clear();
	minimalEnclosingSphere = RE::NiBound();
}

RE::NiTransform ModelManager::CalculateBestLockAlignment()
{
	RE::NiTransform transform{};

	if (minimalEnclosingSphere.radius == 0.0f) {
		minimalEnclosingSphere = GetMinimalEnclosingSphere();

		minimalEnclosingSphere.radius *= 10.0f;
		minimalEnclosingSphere.center *= 10.0f;
	}

	transform.translate = FindMidPoint(minimalEnclosingSphere);
	transform.scale = 10.0f;

	return transform;
}

RE::NiBound ModelManager::GetMinimalEnclosingSphere()
{
	// initialize the enclosing sphere to the first bound
	RE::NiBound enclosingSphere = boundingSpheres[0];

	// loop through the remaining bounds
	for (auto const& bound : boundingSpheres | std::views::drop(1)) {
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

	return enclosingSphere;
}

RE::NiPoint3 ModelManager::FindStartPoint(const RE::NiBound& a_minimalEnclosingSphere, const RE::NiPoint3& a_offset)
{
	auto [center, radius] = a_minimalEnclosingSphere;
	center += a_offset;

	// Bisect the sphere along the y-axis
	const RE::NiPoint3 startPoint = { 0.0f, radius, 0.0f };

	return { center + startPoint };
}

RE::NiPoint3 ModelManager::FindMidPoint(const RE::NiBound& a_minimalEnclosingSphere)
{
	auto [center, radius] = a_minimalEnclosingSphere;
	center.y = ReferenceShiv::translate.y - radius;

	constexpr float targetY = ReferenceShiv::startPoint.y * 2.5f;
	const float     startPointY = FindStartPoint(a_minimalEnclosingSphere, center).y;

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

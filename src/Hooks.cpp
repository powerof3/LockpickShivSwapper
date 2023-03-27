#include "Hooks.h"
#include "AttachManager.h"
#include "Settings.h"

namespace Hooks
{
	static bool          isAltKeyHeld{ false };
	static std::uint32_t numTimesTogglePressed{ 0 };

	// Init
	struct Init3DElements
	{
		static void thunk(RE::LockpickingMenu* a_this)
		{
			func(a_this);

			if (!a_this->init3DElements) {
				const auto lockModelHandle = static_cast<RE::BSResource::ModelHandle*>(a_this->lockDBHandle);
				const auto lockModel = lockModelHandle ? lockModelHandle->data : RE::NiPointer<RE::NiNode>();

				if (const auto shivNode = RE::NiPointer(lockModel ? lockModel->GetObjectByName("Shiv")->AsNode() : nullptr)) {
					AttachManager::GetSingleton()->TryAttachModel(shivNode);
				}

				a_this->init3DElements = true;
			}
		}
		static inline REL::Relocation<decltype(thunk)> func;
	};

	struct ProcessButton
	{
		static bool thunk(RE::LockpickingMenu* a_this, RE::ButtonEvent* a_event)
		{
			if (!func(a_this, a_event)) {
				const auto settings = Settings::GetSingleton();

				const auto key = a_event->GetIDCode();
				const auto device = a_event->GetDevice();

				if (a_event->IsHeld()) {
					if (key == settings->editor.altActionKey) {
						isAltKeyHeld = true;
					}
					if (numTimesTogglePressed == 1) {
						AttachManager::GetSingleton()->ProcessButtonHeld(device, key, isAltKeyHeld);
					}
				} else if (a_event->IsDown()) {
					if (key == settings->editor.toggleKey) {
						numTimesTogglePressed++;
						switch (numTimesTogglePressed) {
						case 1:
							BoostLights(true);
							break;
						case 2:
							{
								BoostLights(false);
								numTimesTogglePressed = 0;
							}
							break;
						default:
							break;
						}
					}
					if (numTimesTogglePressed == 1) {
						AttachManager::GetSingleton()->ProcessButtonDown(device, key, isAltKeyHeld);
					}
				} else if (a_event->IsUp()) {
					if (key == settings->editor.altActionKey) {
						isAltKeyHeld = false;
					}
				}
				return false;
			}
			return true;
		}
		static inline REL::Relocation<decltype(thunk)> func;

		static inline constexpr std::size_t index{ 1 };
		static inline constexpr std::size_t size{ 0x1 };

	private:
		static void BoostLights(bool a_boost)
		{
			if (!Settings::GetSingleton()->editor.boostLightingInEditor) {
				return;
			}

			if (const auto shadowSceneNode = RE::UI3DSceneManager::GetSingleton()->shadowSceneNode) {
				for (const auto& light : shadowSceneNode->activeLights) {
					if (light && light->light) {
						if (a_boost) {
							light->light->fade += 2.0f;
						} else {
							light->light->fade -= 2.0f;
						}
					}
				}
			}
		}
	};

	// Clear
	struct ClearMenuObject
	{
		static void thunk(RE::UI3DSceneManager* a_this, RE::NiNode* a_object)
		{
			isAltKeyHeld = false;
			numTimesTogglePressed = 0;

			AttachManager::GetSingleton()->ClearModels();

			func(a_this, a_object);
		}
		static inline REL::Relocation<decltype(thunk)> func;
	};

	void Install()
	{
		// nop game's { a_this->init3DElements = true; }
		REL::Relocation<std::uintptr_t> init3DElements{ RELOCATION_ID(51081, 51960), OFFSET(0x4F4, 0x4B2) };
		REL::safe_write(init3DElements.address(), REL::NOP7, sizeof(REL::NOP7));

		REL::Relocation<std::uintptr_t> advance_movie{ RELOCATION_ID(51071, 51950), 0xE5 };
		stl::write_thunk_call<Init3DElements>(advance_movie.address());

		stl::write_vfunc<RE::LockpickingMenu, ProcessButton>();

		REL::Relocation<std::uintptr_t> cleanup_menu{ RELOCATION_ID(51082, 51961), 0x77 };
		stl::write_thunk_call<ClearMenuObject>(cleanup_menu.address());
	}
}

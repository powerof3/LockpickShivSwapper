#pragma once

namespace RE
{
	using NiNodePtr = NiPointer<NiNode>;

	// https://github.com/Exit-9B/AmmoEnchanting/blob/main/src/RE/BSResource.h
	namespace BSResource
	{
		struct ModelHandle : ID
		{
			// members
			std::int32_t      unk0C;
			void*             unk10;
			std::int64_t      unk18;
			void*             unk20;
			NiPointer<NiNode> data;
		};
		static_assert(sizeof(ModelHandle) == 0x30);
	}

	template <class T>
	void AttachNode(const NiNodePtr& a_root, const NiPointer<T>& a_obj)
	{
		if (TaskQueueInterface::ShouldUseTaskQueue()) {
			TaskQueueInterface::GetSingleton()->QueueNodeAttach(a_obj.get(), a_root.get());
		} else {
			a_root->AttachChild(a_obj.get());
		}
	}

	inline void DetachNode(NiNodePtr& a_obj)
	{
		if (a_obj) {
			if (TaskQueueInterface::ShouldUseTaskQueue()) {
				TaskQueueInterface::GetSingleton()->QueueNodeDetach(a_obj.get());
			} else {
				if (a_obj->parent) {
					a_obj->parent->AttachChild(a_obj.get());
				}
			}
		}
		a_obj.reset();
	}
}

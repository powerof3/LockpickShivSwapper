#pragma once

namespace RE
{
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
}

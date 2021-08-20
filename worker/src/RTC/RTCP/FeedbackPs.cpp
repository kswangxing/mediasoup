#define MS_CLASS "RTC::RTCP::FeedbackPs"
// #define MS_LOG_DEV_LEVEL 3

#include "RTC/RTCP/FeedbackPs.hpp"
#include "Logger.hpp"
#include "RTC/RTCP/FeedbackItem.hpp"
#include "RTC/RTCP/FeedbackPsFir.hpp"
#include "RTC/RTCP/FeedbackPsLei.hpp"
#include "RTC/RTCP/FeedbackPsRpsi.hpp"
#include "RTC/RTCP/FeedbackPsSli.hpp"
#include "RTC/RTCP/FeedbackPsTst.hpp"
#include "RTC/RTCP/FeedbackPsVbcm.hpp"

namespace RTC
{
	namespace RTCP
	{
		/* Class methods. */

		template<typename Item>
		ObjectPool<FeedbackPsItemsPacket<Item>> FeedbackPsItemsPacket<Item>::Pool(1000);

		template<typename Item>
		ObjectPool<Item> FeedbackPsItemsPacket<Item>::ItemPool(1000);

		template<typename Item>
		FeedbackPsItemsPacket<Item>* FeedbackPsItemsPacket<Item>::Parse(const uint8_t* data, size_t len)
		{
			MS_TRACE();

			if (len < sizeof(CommonHeader) + sizeof(FeedbackPacket::Header))
			{
				MS_WARN_TAG(rtcp, "not enough space for Feedback packet, discarded");

				return nullptr;
			}

			// NOLINTNEXTLINE(llvm-qualified-auto)
			auto* commonHeader = const_cast<CommonHeader*>(reinterpret_cast<const CommonHeader*>(data));

			FeedbackPsItemsPacket<Item>* packet = Pool.New(commonHeader);

			size_t offset = sizeof(CommonHeader) + sizeof(FeedbackPacket::Header);

			while (len > offset)
			{
				auto item = FeedbackItem::Parse<Item>(data + offset, len - offset);

				if (item.has_value())
				{
					if (!item->IsCorrect())
					{
						break;
					}

					packet->AddItem(ItemPool.New(item.value()));

					offset += item->GetSize();
				}
				else
				{
					break;
				}
			}

			return packet;
		}

		template<typename Item>
		void FeedbackPsItemsPacket<Item>::Release(FeedbackPsItemsPacket<Item>* pipfb)
		{
			Pool.Delete(pipfb);
		}

		/* Instance methods. */

		template<typename Item>
		size_t FeedbackPsItemsPacket<Item>::Serialize(uint8_t* buffer)
		{
			MS_TRACE();

			size_t offset = FeedbackPacket::Serialize(buffer);

			for (auto* item : this->items)
			{
				offset += item->Serialize(buffer + offset);
			}

			return offset;
		}

		template<typename Item>
		void FeedbackPsItemsPacket<Item>::Dump() const
		{
			MS_TRACE();

			MS_DUMP("<%s>", FeedbackPsPacket::MessageType2String(Item::messageType).c_str());
			FeedbackPsPacket::Dump();
			for (auto* item : this->items)
			{
				item->Dump();
			}
			MS_DUMP("</%s>", FeedbackPsPacket::MessageType2String(Item::messageType).c_str());
		}

		// explicit instantiation to have all FeedbackRtpPacket definitions in this file.
		template class FeedbackPsItemsPacket<FeedbackPsFirItem>;
		template class FeedbackPsItemsPacket<FeedbackPsSliItem>;
		template class FeedbackPsItemsPacket<FeedbackPsRpsiItem>;
		template class FeedbackPsItemsPacket<FeedbackPsTstrItem>;
		template class FeedbackPsItemsPacket<FeedbackPsTstnItem>;
		template class FeedbackPsItemsPacket<FeedbackPsVbcmItem>;
		template class FeedbackPsItemsPacket<FeedbackPsLeiItem>;
	} // namespace RTCP
} // namespace RTC

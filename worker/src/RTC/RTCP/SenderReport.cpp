#define MS_CLASS "RTC::RTCP::SenderReport"
// #define MS_LOG_DEV_LEVEL 3

#include "RTC/RTCP/SenderReport.hpp"
#include "Logger.hpp"
#include <cstring>
#include "ObjectPool.hpp"

namespace RTC
{
	namespace RTCP
	{
		/* Class methods. */

		ObjectPool<SenderReport> SRPool(1000);

		SenderReport* SenderReport::Parse(const uint8_t* data, size_t len)
		{
			MS_TRACE();

			// Get the header.
			auto* header = const_cast<Header*>(reinterpret_cast<const Header*>(data));

			// Packet size must be >= header size.
			if (len < sizeof(Header))
			{
				MS_WARN_TAG(rtcp, "not enough space for sender report, packet discarded");

				return nullptr;
			}

			return SRPool.New(header);
		}

		void SenderReport::Release(SenderReport* sr)
		{
			SRPool.Delete(sr);
		}

		/* Instance methods. */

		void SenderReport::Dump() const
		{
			MS_TRACE();

			MS_DUMP("<SenderReport>");
			MS_DUMP("  ssrc         : %" PRIu32, GetSsrc());
			MS_DUMP("  ntp sec      : %" PRIu32, GetNtpSec());
			MS_DUMP("  ntp frac     : %" PRIu32, GetNtpFrac());
			MS_DUMP("  rtp ts       : %" PRIu32, GetRtpTs());
			MS_DUMP("  packet count : %" PRIu32, GetPacketCount());
			MS_DUMP("  octet count  : %" PRIu32, GetOctetCount());
			MS_DUMP("</SenderReport>");
		}

		size_t SenderReport::Serialize(uint8_t* buffer)
		{
			MS_TRACE();

			// Copy the header.
			std::memcpy(buffer, this->header, sizeof(Header));

			return sizeof(Header);
		}

		/* Class methods. */

		ObjectPool<SenderReportPacket> SRPPool(1000);

		SenderReportPacket* SenderReportPacket::Parse(const uint8_t* data, size_t len)
		{
			MS_TRACE();

			// Get the header.
			auto* header = const_cast<CommonHeader*>(reinterpret_cast<const CommonHeader*>(data));

			SenderReportPacket* packet = SRPPool.New(header);

			size_t offset = sizeof(Packet::CommonHeader);

			SenderReport* report = SenderReport::Parse(data + offset, len - offset);

			if (report)
				packet->AddReport(report);

			return packet;
		}

		void SenderReportPacket::Release(SenderReportPacket* srp)
		{
			SRPPool.Delete(srp);
		}

		/* Instance methods. */

		size_t SenderReportPacket::Serialize(uint8_t* buffer)
		{
			MS_TRACE();

			MS_ASSERT(this->reports.size() == 1, "invalid number of sender reports");

			size_t offset = Packet::Serialize(buffer);

			// Serialize reports.
			for (auto* report : this->reports)
			{
				offset += report->Serialize(buffer + offset);
			}

			return offset;
		}

		void SenderReportPacket::Dump() const
		{
			MS_TRACE();

			MS_DUMP("<SenderReportPacket>");
			for (auto* report : this->reports)
			{
				report->Dump();
			}
			MS_DUMP("</SenderReportPacket>");
		}
	} // namespace RTCP
} // namespace RTC

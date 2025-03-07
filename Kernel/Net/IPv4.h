#pragma once

#include <AK/AKString.h>
#include <AK/Assertions.h>
#include <AK/IPv4Address.h>
#include <AK/NetworkOrdered.h>
#include <AK/Types.h>

enum class IPv4Protocol : u16 {
    ICMP = 1,
    TCP = 6,
    UDP = 17,
};

NetworkOrdered<u16> internet_checksum(const void*, size_t);

class [[gnu::packed]] IPv4Packet
{
public:
    u8 version() const { return (m_version_and_ihl >> 4) & 0xf; }
    void set_version(u8 version) { m_version_and_ihl = (m_version_and_ihl & 0x0f) | (version << 4); }

    u8 internet_header_length() const { return m_version_and_ihl & 0xf; }
    void set_internet_header_length(u8 ihl) { m_version_and_ihl = (m_version_and_ihl & 0xf0) | (ihl & 0x0f); }

    u16 length() const { return m_length; }
    void set_length(u16 length) { m_length = length; }

    u16 ident() const { return m_ident; }
    void set_ident(u16 ident) { m_ident = ident; }

    u8 ttl() const { return m_ttl; }
    void set_ttl(u8 ttl) { m_ttl = ttl; }

    u8 protocol() const { return m_protocol; }
    void set_protocol(u8 protocol) { m_protocol = protocol; }

    u16 checksum() const { return m_checksum; }
    void set_checksum(u16 checksum) { m_checksum = checksum; }

    const IPv4Address& source() const { return m_source; }
    void set_source(const IPv4Address& address) { m_source = address; }

    const IPv4Address& destination() const { return m_destination; }
    void set_destination(const IPv4Address& address) { m_destination = address; }

    void* payload() { return this + 1; }
    const void* payload() const { return this + 1; }

    u16 payload_size() const { return m_length - sizeof(IPv4Packet); }

    NetworkOrdered<u16> compute_checksum() const
    {
        ASSERT(!m_checksum);
        return internet_checksum(this, sizeof(IPv4Packet));
    }

private:
    u8 m_version_and_ihl { 0 };
    u8 m_dscp_and_ecn { 0 };
    NetworkOrdered<u16> m_length;
    NetworkOrdered<u16> m_ident;
    NetworkOrdered<u16> m_flags_and_fragment;
    u8 m_ttl { 0 };
    NetworkOrdered<u8> m_protocol;
    NetworkOrdered<u16> m_checksum;
    IPv4Address m_source;
    IPv4Address m_destination;
};

static_assert(sizeof(IPv4Packet) == 20);

inline NetworkOrdered<u16> internet_checksum(const void* ptr, size_t count)
{
    u32 checksum = 0;
    auto* w = (const u16*)ptr;
    while (count > 1) {
        checksum += convert_between_host_and_network(*w++);
        if (checksum & 0x80000000)
            checksum = (checksum & 0xffff) | (checksum >> 16);
        count -= 2;
    }
    while (checksum >> 16)
        checksum = (checksum & 0xffff) + (checksum >> 16);
    return ~checksum & 0xffff;
}

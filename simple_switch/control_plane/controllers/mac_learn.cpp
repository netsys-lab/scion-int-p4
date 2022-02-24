#include "mac_learn.h"
#include "bitstring.h"

#include <p4/v1/p4data.pb.h>
#include <p4/v1/p4runtime.pb.h>
#include <p4/v1/p4runtime.grpc.pb.h>
#include <p4/config/v1/p4info.pb.h>

#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

using p4::config::v1::P4Info;


// Constants
constexpr uint32_t NUM_SWITCH_PORTS = 8;

// The IDs of actions and tables are set by @id annotations in the P4 source.
constexpr uint32_t ACTION_NONE = 0x01000001;
constexpr uint32_t ACTION_FORWARD = 0x01000003;
constexpr uint32_t TABLE_LEARN = 0x02000001;
constexpr uint32_t TABLE_FORWARD = 0x02000002;

// The ID of the MAC learn digest is read from the P4Info message.
static const char* DIGEST_MAC_LEARN_NAME = "macLearnMsg_t";


// Forward declarations
static std::unique_ptr<p4::v1::Entity> buildLearnTableEntry(MacAddr mac);
static std::unique_ptr<p4::v1::Entity> buildForwardTableEntry(MacAddr mac, Port outPort);
static std::unique_ptr<p4::v1::Entity> buildFloodMcastGrpEntity(uint32_t id, Port exclude);
static std::unique_ptr<p4::v1::Entity> buildDigestEntity(uint32_t digestId);


/////////////////////
// MacLearningCtrl //
////////////////////

MacLearningCtrl::MacLearningCtrl(SwitchConnection& con, const p4::config::v1::P4Info &p4Info)
{
    for (const auto& digest : p4Info.digests())
    {
        if (digest.preamble().name() == DIGEST_MAC_LEARN_NAME)
        {
            macLearnDigestId = digest.preamble().id();
            break;
        }
    }
    if (!macLearnDigestId)
        throw std::runtime_error(
            std::string("P4Info does not contain a digest of the name ")
            + DIGEST_MAC_LEARN_NAME);
}


void MacLearningCtrl::handleArbitrationUpdate(
    SwitchConnection &con, const p4::v1::MasterArbitrationUpdate& arbUpdate)
{
    if (!arbUpdate.status().code())
    {
        createFloodMulticastGroup(con);
        installStaticTableEntries(con);
        configDigestMessages(con);
    }
}

bool MacLearningCtrl::handleDigest(SwitchConnection &con, const p4::v1::DigestList& digestList)
{
    if (digestList.digest_id() == macLearnDigestId)
    {
        auto writeRequest = con.createWriteRequest();
        for (const auto& digest : digestList.data())
        {
            if (!digest.has_struct_() || digest.struct_().members_size() != 2)
                throw std::runtime_error("Invalid digest format");
            auto macLearnMsg = digest.struct_();

            // Extract source MAC
            auto srcMac = fromBitstring<MAC_ADDR_BYTES, MacAddr>(macLearnMsg.members(0).bitstring());

            // Extract ingress port
            auto ingressPort = fromBitstring<PORT_BYTES, Port>(macLearnMsg.members(1).bitstring());

            std::cout << "Learned: MAC 0x" << std::hex << std::setw(12) << std::setfill('0');
            std::cout << srcMac << " is behind port " << ingressPort << std::endl;

            // Update learn and forward table
            writeRequest.addUpdate(p4::v1::Update::INSERT, buildLearnTableEntry(srcMac));
            writeRequest.addUpdate(p4::v1::Update::INSERT, buildForwardTableEntry(srcMac,
                ingressPort));
        }

        // Send updates to dataplane
        con.sendWriteRequest(writeRequest);

        // Acknowledge digests
        con.ackDigestList(digestList.digest_id(), digestList.list_id());

        return true;
    }
    return false;
}

/// \brief Create multicast groups for flooding packets.
/// \details Flooding is achieved by creating a multicast group for every port. The flooding
/// multicast group of port n contains all other ports with the exception of the CPU port. Thereby,
/// setting the multicast group of a packet entering the switch on port n to n floods the packet
/// to all other ports.
bool MacLearningCtrl::createFloodMulticastGroup(SwitchConnection &con)
{
    auto request = con.createWriteRequest();
    for (uint32_t i = 0; i < NUM_SWITCH_PORTS; ++i)
        request.addUpdate(p4::v1::Update::INSERT, buildFloodMcastGrpEntity(i + 1, i));
    return con.sendWriteRequest(request);
}

/// \brief Install table entries that are known a priori and should not be learned.
bool MacLearningCtrl::installStaticTableEntries(SwitchConnection &con)
{
    // Create entry for broadcast address
    auto request = con.createWriteRequest();
    request.addUpdate(p4::v1::Update::INSERT, buildLearnTableEntry(0xFFFFFFFFFFFF));
    return con.sendWriteRequest(request);
}

/// \brief Configure the transmission of digests.
bool MacLearningCtrl::configDigestMessages(SwitchConnection &con)
{
    auto request = con.createWriteRequest();
    request.addUpdate(p4::v1::Update::INSERT, buildDigestEntity(macLearnDigestId));
    return con.sendWriteRequest(request);
}


///////////////////////
// Utility Functions //
///////////////////////

/// \brief Build a configuration message describing an entry in the learn table set to no_action.
/// \param[in] mac Source MAC address to match.
static std::unique_ptr<p4::v1::Entity> buildLearnTableEntry(MacAddr mac)
{
    auto entity = std::make_unique<p4::v1::Entity>();

    auto entry = entity->mutable_table_entry();
    entry->set_table_id(TABLE_LEARN);

    // Match rule
    auto match = entry->add_match();
    match->set_field_id(1);
    auto exactMatch = match->mutable_exact();
    toBitstring<MAC_ADDR_BYTES, MacAddr>(mac, *exactMatch->mutable_value());

    // Action
    auto action = entry->mutable_action()->mutable_action();
    action->set_action_id(ACTION_NONE);

    return entity;
}

/// \brief Build a configuration message describing an entry in the forward table.
/// \param[in] mac Destination MAC address to match.
/// \param[in] outPort Port matching packets get forwarded to.
static std::unique_ptr<p4::v1::Entity> buildForwardTableEntry(MacAddr mac, Port outPort)
{
    auto entity = std::make_unique<p4::v1::Entity>();

    auto entry = entity->mutable_table_entry();
    entry->set_table_id(TABLE_FORWARD);

    // Match rule
    auto match = entry->add_match();
    match->set_field_id(1);
    auto exactMatch = match->mutable_exact();
    toBitstring<MAC_ADDR_BYTES, MacAddr>(mac, *exactMatch->mutable_value());

    // Action
    auto action = entry->mutable_action()->mutable_action();
    action->set_action_id(ACTION_FORWARD);
    auto param = action->add_params();
    param->set_param_id(1);
    toBitstring<PORT_BYTES>(outPort, *param->mutable_value());

    return entity;
}

/// \brief Build a configuration message for a multicast group encompassing all switch port but one.
/// \param[in] id Multicast group ID. Must be larger than zero.
/// \param[in] exclude Port excluded from the group.
static std::unique_ptr<p4::v1::Entity> buildFloodMcastGrpEntity(uint32_t id, Port exclude)
{
    auto entity = std::make_unique<p4::v1::Entity>();

    auto entry = entity->mutable_packet_replication_engine_entry();
    auto multicastGroup = entry->mutable_multicast_group_entry();

    multicastGroup->set_multicast_group_id(id);
    uint32_t instance = 0;
    for (Port i = 0; i < NUM_SWITCH_PORTS; ++i)
    {
        if (i != exclude)
        {
            auto replica = multicastGroup->add_replicas();
            replica->set_egress_port(i);
            replica->set_instance(instance++);
        }
    }

    return entity;
}

/// \brief Build a configuration message configuring the digest extern.
/// \param[in] digestId ID of the digest extern to configure.
static std::unique_ptr<p4::v1::Entity> buildDigestEntity(uint32_t digestId)
{
    auto entity = std::make_unique<p4::v1::Entity>();

    auto entry = entity->mutable_digest_entry();
    entry->set_digest_id(digestId);
    auto config = entry->mutable_config();
    // Send digests as soon as possible, not in batches.
    config->set_max_timeout_ns(0);
    config->set_max_list_size(1);
    config->set_ack_timeout_ns(1000 * 1000);

    return entity;
}

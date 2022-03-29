#include "int.h"
#include "report/report.pb.h"

#include <p4/v1/p4data.pb.h>
#include <p4/v1/p4runtime.pb.h>
#include <p4/v1/p4runtime.grpc.pb.h>
#include <p4/config/v1/p4info.pb.h>

#include <boost/array.hpp>
#include <boost/coroutine2/all.hpp>

#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <bitset>
#include <fstream>
#include <vector>
#include <ctime>
#include <thread>
#include <limits>


#define CPU_PORT 128

using p4::config::v1::P4Info;

typedef boost::coroutines2::coroutine<std::chrono::milliseconds>  coroByteCnt_t;

// Constants
constexpr uint32_t NUM_SWITCH_PORTS = 8;

// The IDs of actions and tables are set by @id annotations in the P4 source.
constexpr uint32_t ACTION_INSERT_INT = 0x01002001;
constexpr uint32_t ACTION_CLONE_INT = 0x01002002;
constexpr uint32_t ACTION_INSERT_NODE_ID = 0x01002003;
constexpr uint32_t ACTION_INSERT_TX_UTIL = 0x01002004;
constexpr uint32_t ACTION_INSERT_AS_ADDR = 0x01002005;
constexpr uint32_t TABLE_SCION_INT = 0x02002001;
constexpr uint32_t TABLE_INT_NODE_ID = 0x02002002;
constexpr uint32_t TABLE_INT_TX_UTIL = 0x02002003;
constexpr uint32_t TABLE_INT_AS_ADDR = 0x02002004;

// The ID of the tc byte counter is read from the P4Info message.
static const char* COUNTER_TX_BYTE_NAME = "txCounter";

// Forward declarations
static void readIntTable(std::string& intTablePath, std::vector<uint64_t>& asList, std::vector<uint16_t>& bitmaskIntList, std::vector<uint16_t>& bitmaskScionList);
static void splitIpAddress(std::string& address, std::string& ipAddress, uint16_t& port);
static void splitScionAddress(std::string& address, uint16_t& isdAddress, uint64_t& asAddress);
static std::unique_ptr<p4::v1::Entity> buildScionIntTableEntry(isdAddr isd, asAddr as, uint16_t bitmaskInt, uint16_t bitmaskScion, uint32_t defAction);
static std::unique_ptr<p4::v1::Entity> buildSciAsAddrTableEntry(asAddr as);
static std::unique_ptr<p4::v1::Entity> buildIntNodeIdTableEntry(nodeID_t nodeID);
static std::unique_ptr<p4::v1::Entity> buildIntTxUtilTableEntry(Port port, LinkUtil txCount);
static std::unique_ptr<p4::v1::Entity> buildCloneSessionEntry(uint32_t sessionId);
static uint64_t takeUint64(const char* payload, uint32_t pos);
static uint32_t takeUint32(const char* payload, uint32_t pos);
static uint16_t takeUint16(const char* payload, uint32_t pos);
static uint8_t takeUint8(const char* payload, uint32_t pos);


IntController::IntController(SwitchConnection& con, const p4::config::v1::P4Info &p4Info_,
    std::string hostASStr, uint32_t nodeId, std::string intTablePath, std::string kafkaAddress, 
    std::string tcpAddress)
    : p4Info(p4Info_)
    , counterTxId(0)
    , nodeID(nodeId)
    , kafkaProd(kafkaAddress)
{
    // Get counter IDs by their names
    for (const auto& counter : p4Info.counters())
    {
        if (counter.preamble().name() == COUNTER_TX_BYTE_NAME)
        {
            counterTxId = counter.preamble().id();
            break;
        }
    }
            
    // Check if all counters were defined
    if (!counterTxId)
        throw std::runtime_error(
            std::string("P4Info does not contain a counter of the name ")
            + COUNTER_TX_BYTE_NAME);

    //Get address of AS and ISD the switch belongs to
    splitScionAddress(hostASStr, hostISD, hostAS);
    std::string hostASNamePart;
    std::stringstream hostASStream(hostASStr);
    std::getline(hostASStream, hostASNamePart, '-');
    hostISD = std::stoi("0x" + hostASNamePart, nullptr, 16);
    // Get address of AS the switch belongs to
    hostAS = 0;
    while (std::getline(hostASStream, hostASNamePart, ':'))
    {
        hostAS = (hostAS << 16) + std::stoull("0x" + hostASNamePart, nullptr, 16);
    }
    
    // Create tcpSocket
    if (tcpAddress.length() > 0)
    {
        std::string ipAddr;
        uint16_t port;
        splitIpAddress(tcpAddress, ipAddr, port);
        
        auto addr = boost::asio::ip::make_address(ipAddr);
        tcp::endpoint ep(addr, port);
        tcpSocket.createClient(ep);
    }
            
    // Read table from given file
    readIntTable(intTablePath, asList, bitmaskIntList, bitmaskScionList);
    
    // Initialize txCount memory
    txCountList = std::vector<uint32_t>(512, 0);
}

void IntController::handleArbitrationUpdate(
    SwitchConnection &con, const p4::v1::MasterArbitrationUpdate& arbUpdate)
{
    if (!arbUpdate.status().code())
    {
        installStaticTableEntries(con);
        configCloneSession(con);
    }
}

bool IntController::handlePacketIn(SwitchConnection& con, const p4::v1::PacketIn& packetIn)
{
    auto payload = packetIn.payload().c_str();
    auto payloadLen = packetIn.payload().length();
    uint32_t pos = 0;
    
    // Check whether the int_cpu header (8 byte length) can be existent
    if (payloadLen < 8) {
        std::cout << "ERROR: Received INT stack with invalid length!" << std::endl;
        return false;
    }
    // Get header length from int_cpu header
    auto hdrLen = takeUint64(payload, pos);
    pos += 8;
    
    // Check if header with this lentgh can be existent in payload
    if (payloadLen - pos < hdrLen) {
        std::cout << "ERROR: Ill-formated data received. Cannot be read." << std::endl;
        return false;
    }
    
    // Create Protobuf for key
    telemetry::report::FlowKey flowKey;
    // Set flow_id to flow ID read from SCION common header
    uint32_t flowKeyMsg = takeUint32(payload, pos);
    flowKey.set_flow_id(flowKeyMsg % (1 << 20));
    std::string kafkaKey;
    if (!flowKey.SerializeToString(&kafkaKey)) {
        std::cout << "Failed to serialize FlowKey with protobuf!" << std::endl;
        return false;
    }
    
    // Get destintaion ISD and AS address from int_cpu header at the front of the payload
    auto asAddr = takeUint64(payload, (uint32_t) pos + 12);
    
    // Get the index of AS in the input table and read the according bitmask
    auto bitmaskInt = takeUint16(payload, (uint32_t) (pos + hdrLen - 8));
    auto bitmaskScion = takeUint16(payload, (uint32_t) (pos + hdrLen - 4));
    
    // Check, if payload's length is a multiple of the lengths of the defined INT fields
    auto intStackSize = takeUint8(payload, (uint32_t) (pos + hdrLen - 12 - 3)) - 3;
    auto intHopSize = takeUint8(payload, (uint32_t) (pos + hdrLen - 10)) % (1 << 5);
    if ((intStackSize % intHopSize) != 0) {
        std::cout << "ERROR: Received INT stack with invalid length!" << std::endl;
        return false;
    }
    
    // Create Kafka report basics
    telemetry::report::Report report;
    
    // Move pos forward to skip the headers
    pos += hdrLen;
    
    // Get INT data from INT stack
    for (int i = 0; i < intStackSize / intHopSize; i++) {
        // Create hop in kafka report
        auto newHop = report.add_hops();
        auto metadata = newHop->mutable_metadata();
        
        // Check for all possible INT data fields and write them into metadata
        if (bitmaskInt & (1 << 15)) // NODE_ID
        {
            uint32_t readNodeID = takeUint32(payload, pos);
            pos += 4;
            newHop->set_node_id(readNodeID);
        }
        if (bitmaskInt & (1 << 14)) // INT_L1_IF_ID
        {
            for (int j = 0; j < 4; j++)
                (*metadata)[telemetry::report::INTERFACE_LEVEL1].push_back(*(payload + pos + j));
            pos += 4;
        }
        if (bitmaskInt & (1 << 13)) // INT_HOP_LATENCY
        {
            for (int j = 0; j < 4; j++)
                (*metadata)[telemetry::report::HOP_LATENCY].push_back(*(payload + pos + j));
            pos += 4;
        }
        if (bitmaskInt & (1 << 12)) // INT_QUEUE
        {
            for (int j = 0; j < 4; j++)
                (*metadata)[telemetry::report::QUEUE_OCCUPANCY].push_back(*(payload + pos + j));
            pos += 4;
        }
        if (bitmaskInt & (1 << 11)) // INT_IG_TIME
        {
            auto ingressTimestamp = takeUint64(payload, pos) * 1000;
            pos += 8;
            //std::cout << "  Hop " << i << " - Ingress timestamp: " << std::dec << ingressTimestamp << std::endl;
            for (int i = 7; i >= 0; i--)
            {
                (*metadata)[telemetry::report::INGRESS_TIMESTAMP].push_back(*(reinterpret_cast<char*>(&ingressTimestamp) + i));
            }
        }
        if (bitmaskInt & (1 << 10)) // INT_EG_TIME
        {
            auto egressTimestamp = takeUint64(payload, pos) * 1000;
            pos += 8;
            //std::cout << "  Hop " << i << " - Egress timestamp: " << std::dec << egressTimestamp << std::endl;
            for (int i = 7; i >= 0; i--)
            {
                (*metadata)[telemetry::report::EGRESS_TIMESTAMP].push_back(*(reinterpret_cast<char*>(&egressTimestamp) + i));
            }
        }
        if (bitmaskInt & (1 << 9)) // INT_L2_IF_ID
        {
            for (int j = 0; j < 8; j++)
                (*metadata)[telemetry::report::INTERFACE_LEVEL2].push_back(*(payload + pos + j));
            pos += 8;
        }
        if (bitmaskInt & (1 << 8)) // INT_EG_IF_UTIL
        {
            for (int j = 0; j < 4; j++)
                (*metadata)[telemetry::report::EGRESS_TX_UTILIZATION].push_back(*(payload + pos + j));
            pos += 4;
        }
        if (bitmaskInt & (1 << 7)) // INT_BUFFER_INFOS
        {
            for (int j = 0; j < 4; j++)
                (*metadata)[telemetry::report::BUFFER_OCCUPANCY].push_back(*(payload + pos + j));
            pos += 4;
        }
        if (bitmaskScion & 1) // AS_ADDR
        {
            uint64_t readAS = takeUint64(payload, pos);
            pos += 8;
            newHop->set_asn(readAS);
        }
    }
    
    // Add packet type and header to Kafka report
    report.set_packet_type(telemetry::report::Report_PacketType_SCION);
    report.set_truncated_packet((payload + 8), hdrLen);
    
    // Serialize Kafka report
    std::string strReport;
    if (!report.SerializeToString(&strReport)) {
        std::cout << "Failed to serialize Report with protobuf!" << std::endl;
        return false;
    }
    
    // Send report to Kafka topic
    std::stringstream topic_name;
    topic_name << "AS" << std::hex << ((asAddr >> 32) % (1 << 16))
                << "_" << std::hex << ((asAddr >> 16) % (1 << 16))
                << "_" << std::hex << (asAddr % (1 << 16))
                << "-" << std::hex << nodeID;
    
    if (!kafkaProd.send(topic_name.str(), kafkaKey, strReport))
        std::cout << "ERROR: Failed to send message to Kafka topic" << std::endl;
    
    // Send protobuf message over tcp port
    tcpSocket.send(strReport);
    return true;
}

/// \brief Install table entries that are known a priori and should not be learned.
bool IntController::installStaticTableEntries(SwitchConnection &con)
{
    // Create entry for broadcast address
    auto request = con.createWriteRequest();
    
    // Create entries for Scion INT table
    std::cout << "Node serves as sink for AS " << std::hex << hostAS << " of ISD " << hostISD << std::endl;
    request.addUpdate(p4::v1::Update::INSERT, buildScionIntTableEntry(
        hostISD,
        hostAS, 
        0, 0,
        ACTION_CLONE_INT
    ));
    con.sendWriteRequest(request);
    for (int i = 0; i < asList.size(); i++)
    {
        request = con.createWriteRequest();
        std::cout << "Write INT-Bitmask " << std::hex << bitmaskIntList[i] << " for AS " << (asList[i] >> 48) << "-" << (asList[i] & 0xffffffffffff) << std::endl;
        std::cout << "Write SCION-specific Bitmask " << std::hex << bitmaskScionList[i] << " for AS " << (asList[i] >> 48) << "-" << (asList[i] & 0xffffffffffff) << std::endl;
        if (!(asList[i] >> 48 == hostISD && (asList[i] & 0xffffffffffff) == hostAS))
            request.addUpdate(p4::v1::Update::INSERT, buildScionIntTableEntry(
                (asList[i] >> 48),
                (asList[i] & 0xffffffffffff),
                bitmaskIntList[i],
                bitmaskScionList[i],
                ACTION_INSERT_INT
            ));
        con.sendWriteRequest(request);
    }
    
    // Create entries for node ID and AS address
    request = con.createWriteRequest();
    request.addUpdate(p4::v1::Update::INSERT, buildIntNodeIdTableEntry(nodeID));
    request.addUpdate(p4::v1::Update::INSERT, buildSciAsAddrTableEntry(hostAS));
    
    for (int i = 0; i < 512; i++)
    {
        request.addUpdate(p4::v1::Update::INSERT, buildIntTxUtilTableEntry(i, 0));
    }
    
    return con.sendWriteRequest(request);
}

/// \brief Configure the cloning of messages to CPU.
bool IntController::configCloneSession(SwitchConnection &con)
{
    auto request = con.createWriteRequest();
    request.addUpdate(p4::v1::Update::INSERT, buildCloneSessionEntry(1));
    return con.sendWriteRequest(request);
}

static void readIntTable(std::string& intTablePath,
                  std::vector<uint64_t>& asList,
                  std::vector<uint16_t>& bitmaskIntList,
                  std::vector<uint16_t>& bitmaskScionList)
{
    // Get table with bitmasks from file
    std::ifstream intTable;
    
    intTable.open(intTablePath, std::ios::in);
    if (!intTable.is_open())
        throw std::runtime_error(
            std::string("ERROR: Failed to open int_table.txt"));
    
    // Write configuration defined in the file into lists
    std::string line;
    while (std::getline(intTable, line))
    {
        // Check that line is no comment
        if (line[0] != '#')
        {
            std::istringstream lineStr(line);
            std::string asName;
            uint16_t bitmaskInt;
            uint16_t bitmaskScion;
            lineStr >> asName >> std::hex >> bitmaskInt >> bitmaskScion;
            uint16_t isdAddr = 0;
            uint64_t asAddr = 0;
            splitScionAddress(asName, isdAddr, asAddr);
            asAddr = ((uint64_t)isdAddr << 48) + asAddr;
            asList.push_back(asAddr);
            bitmaskIntList.push_back(bitmaskInt);
            bitmaskScionList.push_back(bitmaskScion);
        }
    }
   
    intTable.close();
}

/// \brief Split a string "ip.address:port" into address and port.
/// \param[in] address The original address string.
/// \param[out] ipAddress The IP address as string.
/// \param[out] port The port as uint16.
static void splitIpAddress(std::string& address, std::string& ipAddress, uint16_t& port)
{
    if (address.length() > 0)
    {
        std::stringstream addrStr(address);
        std::string portStr;
        std::getline(addrStr, ipAddress, ':');
        std::getline(addrStr, portStr, ':');
        port = std::stoi(portStr);
    }
}

/// \brief Split a string "isdAddress-AS:Address" into ISD address and AS address.
/// \param[in] address The original SCION address string.
/// \param[out] isdAddress The ISD address as uint16.
/// \param[out] asAddress The AS address as uint16.
static void splitScionAddress(std::string& address, uint16_t& isdAddress, uint64_t& asAddress)
{
    std::string addressNamePart;
    std::stringstream addressStream(address);
    std::getline(addressStream, addressNamePart, '-');
    isdAddress = std::stoi("0x" + addressNamePart, nullptr, 16);
    // Get address of AS the switch belongs to
    asAddress = 0;
    while (std::getline(addressStream, addressNamePart, ':'))
    {
        asAddress = (asAddress << 16) + std::stoull("0x" + addressNamePart, nullptr, 16);
    }
}

/// \brief Build a configuration message describing an entry in the Scion INT table to insert an INT header.
/// \param[in] isd Destination ISD of the INT flow to be defined.
/// \param[in] as Destination AS of the INT flow to be defined.
/// \param[in] bitmaskInt INT bitmask of the INT flow to be defined.
/// \param[in] bitmaskScion Domain specific bitmask for SCION of the INT flow to be defined.
/// \param[in] defAction Defines, whether INT headers have to be inserted or deleted.
static std::unique_ptr<p4::v1::Entity> buildScionIntTableEntry(isdAddr isd, asAddr as, uint16_t bitmaskInt, uint16_t bitmaskScion, uint32_t defAction)
{
    auto entity = std::make_unique<p4::v1::Entity>();

    auto entry = entity->mutable_table_entry();
    entry->set_table_id(TABLE_SCION_INT);

    // Match rules
    // Set match field 1 (ISD address)
    auto matchIsd = entry->add_match();
    matchIsd->set_field_id(1);
    auto exactMatchIsd = matchIsd->mutable_exact();
    toBitstring<ISD_BYTES, isdAddr>(isd, *exactMatchIsd->mutable_value());
    
    // Set match field 2 (AS address)
    auto matchAs = entry->add_match();
    matchAs->set_field_id(2);
    auto exactMatchAs = matchAs->mutable_exact();
    toBitstring<AS_BYTES, asAddr>(as, *exactMatchAs->mutable_value());
    
    if (defAction == ACTION_INSERT_INT)
    {
        // Action
        auto action = entry->mutable_action()->mutable_action();
        action->set_action_id(ACTION_INSERT_INT);
        auto param = action->add_params();
        param->set_param_id(1);
        toBitstring<sizeof(uint16_t)>(bitmaskInt, *param->mutable_value());
        param = action->add_params();
        param->set_param_id(2);
        toBitstring<sizeof(uint16_t)>(bitmaskScion, *param->mutable_value());
    } else if (defAction == ACTION_CLONE_INT) {
        // Action
        auto action = entry->mutable_action()->mutable_action();
        action->set_action_id(ACTION_CLONE_INT);
    }

    return entity;
}

/// \brief Build a configuration message describing an entry in the int node ID table set to insert_int_node_id.
/// \param[in] nodeID Node ID that should be inserted in INT stack.
static std::unique_ptr<p4::v1::Entity> buildIntNodeIdTableEntry(nodeID_t nodeID)
{
    auto entity = std::make_unique<p4::v1::Entity>();

    auto entry = entity->mutable_table_entry();
    entry->set_table_id(TABLE_INT_NODE_ID);

    // Match rule
    auto match = entry->add_match();
    match->set_field_id(1);
    auto exactMatch = match->mutable_exact();
    toBitstring<FLAG_BYTES, flag>(1, *exactMatch->mutable_value());

    // Action
    auto action = entry->mutable_action()->mutable_action();
    action->set_action_id(ACTION_INSERT_NODE_ID);
    auto param = action->add_params();
    param->set_param_id(1);
    toBitstring<NODE_ID_BYTES>(nodeID, *param->mutable_value());

    return entity;
}

/// \brief Build a configuration message describing an entry in the int AS address table set to insert_sci_as_addr.
/// \param[in] as AS address that should be inserted in INT stack.
static std::unique_ptr<p4::v1::Entity> buildSciAsAddrTableEntry(asAddr as)
{
    auto entity = std::make_unique<p4::v1::Entity>();

    auto entry = entity->mutable_table_entry();
    entry->set_table_id(TABLE_INT_AS_ADDR);

    // Match rule
    auto match = entry->add_match();
    match->set_field_id(1);
    auto exactMatch = match->mutable_exact();
    toBitstring<FLAG_BYTES, flag>(1, *exactMatch->mutable_value());

    // Action
    auto action = entry->mutable_action()->mutable_action();
    action->set_action_id(ACTION_INSERT_AS_ADDR);
    auto param = action->add_params();
    param->set_param_id(1);
    toBitstring<8, asAddr>(as, *param->mutable_value());

    return entity;
}

/// \brief Build a configuration message describing an entry in the int tx link utilization table set to insert_sci_as_addr.
/// \param[in] port Egress port of the message
/// \param[in] txCount Tx byte count of the corresponding port.
static std::unique_ptr<p4::v1::Entity> buildIntTxUtilTableEntry(Port port, LinkUtil txCount)
{
    auto entity = std::make_unique<p4::v1::Entity>();

    auto entry = entity->mutable_table_entry();
    entry->set_table_id(TABLE_INT_TX_UTIL);

    // Match rule 1: The flag that enables tx link utilization has to be 1.
    auto matchFlag = entry->add_match();
    matchFlag->set_field_id(1);
    auto exactMatchFlag = matchFlag->mutable_exact();
    toBitstring<FLAG_BYTES, flag>(1, *exactMatchFlag->mutable_value());
    // Match rule 2: The egress port has to match (Every port has an own tx counter).
    auto matchPort = entry->add_match();
    matchPort->set_field_id(2);
    auto exactMatchPort = matchPort->mutable_exact();
    toBitstring<PORT_BYTES, Port>(port, *exactMatchPort->mutable_value());

    // Action
    auto action = entry->mutable_action()->mutable_action();
    action->set_action_id(ACTION_INSERT_TX_UTIL);
    auto param = action->add_params();
    param->set_param_id(1);
    toBitstring<LINK_UTIL_BYTES, LinkUtil>(txCount, *param->mutable_value());

    return entity;
}

/// \brief Build a configuration message for a clone session entry cloning the message to the CPU-port.
/// \param[in] id session ID. Must be larger than zero.
static std::unique_ptr<p4::v1::Entity> buildCloneSessionEntry(uint32_t id)
{
    auto entity = std::make_unique<p4::v1::Entity>();

    auto entry = entity->mutable_packet_replication_engine_entry();
    auto cloneSession = entry->mutable_clone_session_entry();

    cloneSession->set_session_id(id);
    
    auto replica = cloneSession->add_replicas();
    replica->set_egress_port(CPU_PORT);
    replica->set_instance(1);
    
    cloneSession->set_class_of_service(0);
    cloneSession->set_packet_length_bytes(0);

    return entity;
}

static uint64_t takeUint64(const char* payload, uint32_t pos)
{
        // Data is transferred in big-endian format, but saved in little-endian format
        uint64_t number;
        for (int i = 0; i < 8; i ++)
        {
            *(reinterpret_cast<char*>(&number) + i) = *(payload + pos + 7 - i);
        }
        return number;
}

static uint32_t takeUint32(const char* payload, uint32_t pos)
{
        // Data is transferred in big-endian format, but saved in little-endian format
        uint32_t number;
        for (int i = 0; i < 4; i ++)
        {
            *(reinterpret_cast<char*>(&number) + i) = *(payload + pos + 3 - i);
        }
        return number;
}

static uint16_t takeUint16(const char* payload, uint32_t pos)
{
        // Data is transferred in big-endian format, but saved in little-endian format
        uint16_t number;
        for (int i = 0; i < 2; i ++)
        {
            *(reinterpret_cast<char*>(&number) + i) = *(payload + pos + 1 - i);
        }
        return number;
}

static uint8_t takeUint8(const char* payload, uint32_t pos)
{
        // Data is transferred in big-endian format, but saved in little-endian format
        uint8_t number;
        *(reinterpret_cast<char*>(&number)) = *(payload + pos);
        return number;
}

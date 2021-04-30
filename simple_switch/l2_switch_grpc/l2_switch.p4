// Learning L2 switch for v1model architecture as implemented by the BMv2 simple switch target.
// Simple Switch Documentation:
// https://github.com/p4lang/behavioral-model/blob/main/docs/simple_switch.md
// v1model header:
// https://github.com/p4lang/p4c/blob/master/p4include/v1model.p4

#include <core.p4>
#include <v1model.p4>


/////////////
// Headers //
/////////////

typedef bit<9> ingressPort_t;
typedef bit<9> egressPort_t;
typedef bit<48> macAddr_t;

header ethernet_h {
    macAddr_t dstAddr;
    macAddr_t srcAddr;
    bit<16> type;
}

struct headers_t {
    ethernet_h ethernet;
}

struct metadata_t {
}


struct macLearnMsg_t
{
    macAddr_t srcAddr;
    ingressPort_t ingressPort;
};


////////////
// Parser //
////////////

parser EthernetParser(
    packet_in packet,
    out headers_t hdr,
    inout metadata_t meta,
    inout standard_metadata_t std_meta)
{
    state start {
        transition parse_ethernet;
    }

    state parse_ethernet {
        packet.extract(hdr.ethernet);
        transition accept;
    }
}


///////////////////////////
// Checksum Verification //
///////////////////////////

control MyVerifyChecksum(inout headers_t hdr, inout metadata_t meta)
{
    apply { }
}


////////////////////////
// Ingress Processing //
////////////////////////

control MyIngress(
    inout headers_t hdr,
    inout metadata_t meta,
    inout standard_metadata_t std_meta)
{
    @id(0x01000001)
    @brief("Do nothing.")
    action no_action() {
    }

    @id(0x01000002)
    @brief("Learn the source MAC address by sending it to the controller.")
    action learn_source() {
        macLearnMsg_t msg;
        msg.srcAddr = hdr.ethernet.srcAddr;
        msg.ingressPort = std_meta.ingress_port;
        // Send a message to the control plane. bmv2 ignores the first parameter.
        digest(0x17000001, msg);
    }

    @id(0x01000003)
    @brief("Forward the packet to the given port.")
    action forward(egressPort_t port) {
        std_meta.egress_spec = port;
    }

    @id(0x01000004)
    @brief("Replicate the packet on all but the ingress port.")
    action flood() {
        std_meta.mcast_grp = (bit<16>)std_meta.ingress_port;
    }

    @id(0x02000001)
    @brief("Contains the source MAC addresses learned so far.")
    table learn_table {
        key = {
            hdr.ethernet.srcAddr: exact;
        }
        actions = {
            no_action;
            learn_source;
        }
        default_action = learn_source();
        size = 1024;
    }

    @id(0x02000002)
    @brief("Maps destination MAC address to egress port.")
    table forward_table {
        key = {
            hdr.ethernet.dstAddr: exact;
        }
        actions = {
            forward;
            flood;
        }
        default_action = flood();
        size = 1024;
    }

    apply {
        if (hdr.ethernet.isValid()) {
            learn_table.apply();
            forward_table.apply();
        }
    }
}


///////////////////////
// Egress Processing //
///////////////////////

control MyEgress(
    inout headers_t hdr,
    inout metadata_t meta,
    inout standard_metadata_t std_meta)
{
    apply { }
}


//////////////
// Checksum //
//////////////

control MyComputeChecksum(inout headers_t hdr, inout metadata_t meta)
{
    apply { }
}


//////////////
// Deparser //
//////////////

control MyDeparser(packet_out packet, in headers_t hdr)
{
    apply {
        packet.emit(hdr.ethernet);
    }
}


////////////
// Switch //
////////////

@pkginfo(name="learning_ethernet_switch")
@pkginfo(version="0.1")
V1Switch(
    EthernetParser(),
    MyVerifyChecksum(),
    MyIngress(),
    MyEgress(),
    MyComputeChecksum(),
    MyDeparser()
) main;

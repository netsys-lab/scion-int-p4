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

#include "headers/common.p4"

struct headers_t {
    ethernet_h ethernet;
}

struct metadata_t {
}

#include "parser/ethernetParser.p4"
#include "include/l2_switch.p4"


////////////
// Parser //
////////////

parser EthernetSwitchParser(
    packet_in packet,
    out headers_t hdr,
    inout metadata_t meta,
    inout standard_metadata_t std_meta)
{
    EthernetParser() ethernetParser;
    state start {
        transition parse_ethernet;
    }

    state parse_ethernet {
        ethernetParser.apply(packet, hdr.ethernet);
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
    L2Switch() l2switch;
    apply {
        l2switch.apply(hdr, meta, std_meta);
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
    EthernetSwitchParser(),
    MyVerifyChecksum(),
    MyIngress(),
    MyEgress(),
    MyComputeChecksum(),
    MyDeparser()
) main;

#define DISABLE_IPV6

// INT Values
#define NUM_INTER_HOPS 10
#define SPACE_FOR_HOPS 2080 //NUM_INTER_HOPS*416
#define UDP_PORT 12345

#if !defined(NUM_INTER_HOPS)
#error "A maximum number of intermediate hops has to be defined as NUM_INTER_HOPS!"
#endif

#if NUM_INTER_HOPS > 255
#error "The maximum number of intermediate hops NUM_INTER_HOPS has to be smaller than 256."
#endif

#if defined DISABLE_IPV4 && defined DISABLE_IPV6
#error "Disabling both IPv4 and IPv6 support is not supported"
#endif

#if !defined(DISABLE_IPV4) && !defined(DISABLE_IPV6)
#error "Enabling both IPv4 and IPv6 support is not supported"
#endif

#include <core.p4>
#include <v1model.p4>

#include "headers/common.p4"
#include "headers/scion.p4"
#include "headers/int.p4"


/////////////
// Headers //
/////////////

typedef bit<9> ingressPort_t;
typedef bit<9> egressPort_t;

struct headers_t {
    ethernet_h		ethernet;
#ifndef DISABLE_IPV4
	ipv4_h			ipv4;
#endif /* DISABLE_IPV4 */
#ifndef DISABLE_IPV6
	ipv6_h			ipv6;
#endif /* DISABLE_IPV6 */
	udp_h			udp;
	int_cpu_h       int_cpu;                            // Saves length of headers for truncated_packet field in Kafka msg
	scion_common_h		scion_common;
	scion_addr_common_h	scion_addr_common;
	scion_addr_host_32_h	scion_addr_dst_host_32;
	scion_addr_host_32_h	scion_addr_dst_host_32_2;
	scion_addr_host_32_h	scion_addr_dst_host_32_3;
	scion_addr_host_128_h	scion_addr_dst_host_128;
	scion_addr_host_32_h	scion_addr_src_host_32;
	scion_addr_host_32_h	scion_addr_src_host_32_2;
	scion_addr_host_32_h	scion_addr_src_host_32_3;
	scion_addr_host_128_h	scion_addr_src_host_128;
	scion_path_meta_h	scion_path_meta;
	scion_info_field_h	scion_info_field_0;
	scion_info_field_h	scion_info_field_1;
	scion_info_field_h	scion_info_field_2;
	scion_hop_field_h	scion_hop_fields;
	udp_h           udp_scion;
	int_shim_h      int_shim;
	int_md_h        int_md;
	int_stack_t     int_stack;
	int_chksum_compl_h  int_chksum_compl;
	payload_h       payload;
}

// hopCount is used in parser to extract int_stacks from variable number of intermediate hops
struct metadata_t {
    bit<2>  intState;
    bit<32> intStackLen;
    @field_list(1)
    bit<64> cpuHdrLen;
    bit<16> addLen;
    // Remember the set bitmask fields
    bit<1>  intNodeID;
    bit<1>  intL1IfID;
    bit<1>  intHopLatency;
    bit<1>  intQueue;
    bit<1>  intIngressTime;
    bit<1>  intEgressTime;
    bit<1>  intL2IfID;
    bit<1>  intEgIfUtil;
    bit<1>  intBufferInfos;
    bit<1>  intChksumCompl;
    bit<1>  sciAsAddr;
}


struct macLearnMsg_t
{
    macAddr_t srcAddr;
    ingressPort_t ingressPort;
};

#include "include/parser.p4"
#include "include/l2_switch.p4"
#include "include/int_switch.p4"


///////////////////////////
// Checksum Verification //
///////////////////////////

control MyVerifyChecksum(inout headers_t hdr, inout metadata_t meta)
{
    apply {
#ifndef DISABLE_IPV4

		verify_checksum(hdr.ipv4.isValid(), {
	        hdr.ipv4.version,
			hdr.ipv4.ihl,
			hdr.ipv4.diffserv,
			hdr.ipv4.totalLen,
			hdr.ipv4.identification,
			hdr.ipv4.flags,
			hdr.ipv4.fragOffset,
			hdr.ipv4.ttl,
			hdr.ipv4.protocol,
			hdr.ipv4.srcAddr,
			hdr.ipv4.dstAddr}, hdr.ipv4.hdrChecksum, HashAlgorithm.csum16);

		// Since headers behind UPD header are counted as payload, they have to be listed here...
		verify_checksum_with_payload(hdr.udp.isValid() && hdr.udp.checksum != 0, {
		    hdr.ipv4.srcAddr,
			hdr.ipv4.dstAddr,
			8w0,
			hdr.ipv4.protocol,
			hdr.udp.len,
			hdr.udp.srcPort,
			hdr.udp.dstPort,
			hdr.udp.len,
			hdr.scion_common,
			hdr.scion_addr_common,
			hdr.scion_addr_dst_host_32,
			hdr.scion_addr_dst_host_32_2,
			hdr.scion_addr_dst_host_32_3,
			hdr.scion_addr_dst_host_128,
			hdr.scion_addr_src_host_32,
			hdr.scion_addr_src_host_32_2,
			hdr.scion_addr_src_host_32_3,
			hdr.scion_addr_src_host_128,
			hdr.scion_path_meta,
			hdr.scion_info_field_0,
			hdr.scion_info_field_1,
			hdr.scion_info_field_2,
			hdr.scion_hop_fields,
			hdr.udp_scion,
			hdr.int_shim,
			hdr.int_md,
			hdr.int_stack.pre_int_stack,
			hdr.int_stack.nodeID,
			hdr.int_stack.l1InterfaceInID,
			hdr.int_stack.l1InterfaceEgID,
			hdr.int_stack.hopLatency,
			hdr.int_stack.queueID,
			hdr.int_stack.queueOccu,
			hdr.int_stack.ingressTime,
			hdr.int_stack.egressTime,
			hdr.int_stack.l2InterfaceInID,
			hdr.int_stack.l2InterfaceEgID,
			hdr.int_stack.egressIFUtilization,
			hdr.int_stack.bufferID,
			hdr.int_stack.bufferOccu,
			hdr.int_stack.sciAsAddr,
			hdr.int_chksum_compl.checksumComplement,
			hdr.payload}, hdr.udp.checksum, HashAlgorithm.csum16);

#endif /* DISABLE_IPV4 */
#ifndef DISABLE_IPV6

		// Update UDP checksum, as this also includes the data we need to take into account the fields we (possibly) changed in the SCION header
		verify_checksum(hdr.ipv6.isValid() && hdr.scion_path_meta.isValid(), {
		    hdr.ipv6.srcAddr,
			hdr.ipv6.dstAddr,
			hdr.udp.srcPort,
			hdr.udp.dstPort,
			hdr.scion_path_meta.currInf, hdr.scion_path_meta.currHF, hdr.scion_path_meta.rsv, hdr.scion_path_meta.seg0Len, hdr.scion_path_meta.seg1Len, hdr.scion_path_meta.seg2Len,
			hdr.scion_info_field_0.segId,
			hdr.scion_info_field_1.segId,
			hdr.scion_info_field_2.segId,
			ig_md.udp_checksum_tmp
	    }, ig_md.udp_checksum_tmp, HashAlgorithm.csum16);

		// Assume it is a one-hop path
		verify_checksum(hdr.ipv6.isValid() && !hdr.scion_path_meta.isValid(), {
		    hdr.ipv6.srcAddr,
			hdr.ipv6.dstAddr,
			hdr.udp.srcPort,
			hdr.udp.dstPort,
			hdr.scion_info_field_0.segId,
			ig_md.udp_checksum_tmp
	    }, ig_md.udp_checksum_tmp, HashAlgorithm.csum16);

#endif /* DISABLE_IPV6 */

    }
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
    INTSwitchIngress() intswitch;
    
    apply {
        l2switch.apply(hdr, meta, std_meta);
        intswitch.apply(hdr, meta, std_meta);
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
    INTSwitchEgress() intswitch;
    
    apply {
        intswitch.apply(hdr, meta, std_meta);
    }
}


//////////////
// Checksum //
//////////////

control MyComputeChecksum(inout headers_t hdr, inout metadata_t meta)
{
    apply {

#ifndef DISABLE_IPV4

	    update_checksum(hdr.ipv4.isValid(), {
	        hdr.ipv4.version,
			hdr.ipv4.ihl,
			hdr.ipv4.diffserv,
			hdr.ipv4.totalLen,
			hdr.ipv4.identification,
			hdr.ipv4.flags,
			hdr.ipv4.fragOffset,
			hdr.ipv4.ttl,
			hdr.ipv4.protocol,
			hdr.ipv4.srcAddr,
			hdr.ipv4.dstAddr}, hdr.ipv4.hdrChecksum, HashAlgorithm.csum16);

		update_checksum_with_payload(hdr.udp.isValid() && hdr.udp.checksum != 0, {
		    hdr.ipv4.srcAddr,
			hdr.ipv4.dstAddr,
			8w0,
			hdr.ipv4.protocol,
			hdr.udp.len,
			hdr.udp.srcPort,
			hdr.udp.dstPort,
			hdr.udp.len,
			hdr.scion_common,
			hdr.scion_addr_common,
			hdr.scion_addr_dst_host_32,
			hdr.scion_addr_dst_host_32_2,
			hdr.scion_addr_dst_host_32_3,
			hdr.scion_addr_dst_host_128,
			hdr.scion_addr_src_host_32,
			hdr.scion_addr_src_host_32_2,
			hdr.scion_addr_src_host_32_3,
			hdr.scion_addr_src_host_128,
			hdr.scion_path_meta,
			hdr.scion_info_field_0,
			hdr.scion_info_field_1,
			hdr.scion_info_field_2,
			hdr.scion_hop_fields,
			hdr.udp_scion,
			hdr.int_shim,
			hdr.int_md,
			hdr.int_stack.pre_int_stack,
			hdr.int_stack.nodeID,
			hdr.int_stack.l1InterfaceInID,
			hdr.int_stack.l1InterfaceEgID,
			hdr.int_stack.hopLatency,
			hdr.int_stack.queueID,
			hdr.int_stack.queueOccu,
			hdr.int_stack.ingressTime,
			hdr.int_stack.egressTime,
			hdr.int_stack.l2InterfaceInID,
			hdr.int_stack.l2InterfaceEgID,
			hdr.int_stack.egressIFUtilization,
			hdr.int_stack.bufferID,
			hdr.int_stack.bufferOccu,
			hdr.int_stack.sciAsAddr,
			hdr.int_chksum_compl.checksumComplement,
			hdr.payload}, hdr.udp.checksum, HashAlgorithm.csum16);

#endif /* DISABLE_IPV4 */
#ifndef DISABLE_IPV6

	    update_checksum(hdr.ipv6.isValid() && hdr.scion_path_meta.isValid(), {
		    hdr.ipv6.srcAddr,
		    hdr.ipv6.dstAddr,
		    hdr.udp.srcPort,
		    hdr.udp.dstPort,
		    hdr.scion_path_meta.currInf, hdr.scion_path_meta.currHF, hdr.scion_path_meta.rsv, hdr.scion_path_meta.seg0Len, hdr.scion_path_meta.seg1Len, hdr.scion_path_meta.seg2Len,
		    hdr.scion_info_field_0.segId,
		    hdr.scion_info_field_1.segId,
		    hdr.scion_info_field_2.segId}, ig_md.udp_checksum_tmp, HashAlgorithm.csum16);

#endif /* DISABLE_IPV6 */
    }
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

@pkginfo(name="int_switch")
@pkginfo(version="0.1")
V1Switch(
    EthernetParser(),
    MyVerifyChecksum(),
    MyIngress(),
    MyEgress(),
    MyComputeChecksum(),
    MyDeparser()
) main;

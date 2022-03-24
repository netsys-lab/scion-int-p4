#ifndef __include_int_switch__
#define __include_int_switch__

#include "../headers/common.p4"
#include "../headers/scion.p4"
#include "../headers/int.p4"


// Register to count tx per port
counter(512, CounterType.bytes) txCounter;


////////////////////////
// Ingress Processing //
////////////////////////

control INTSwitchIngress(
    inout headers_t hdr,
    inout metadata_t meta,
    inout standard_metadata_t std_meta)
{
    // Action for INT-source
    @id(0x01002001)
    @brief("Insert INT header.")
    action insert_int(instruction_bitmap_t instructionBits, domain_bitmap_t domainBits) {
        // Set headers valid
        hdr.int_shim.setValid();
        hdr.int_md.setValid();
        // INT shim header - some general information about following INT header and lost udp port
        hdr.int_shim.type = Type.MD;
        hdr.int_shim.npt = 0x01;
        hdr.int_shim.r = 0x00;
        hdr.int_shim.length = 0x03;
        hdr.int_shim.udpPort = hdr.udp_scion.dstPort;
        // Set udp port to signal following INT shim header
        hdr.udp_scion.dstPort = UDP_PORT;
        // INT-MD header - specifies handling of INT data
        hdr.int_md.version = 0x02;
        hdr.int_md.discard = 0x00;
        hdr.int_md.exceededHopCount = 0x00;
        hdr.int_md.mtuExceeded = 0x00;
        hdr.int_md.reserved = 0x00;
        hdr.int_md.hopML = 0x00;
        hdr.int_md.remainingHopCount = NUM_INTER_HOPS;
        hdr.int_md.instructionBitmap = instructionBits;
        if (domainBits == 0x0000)
        {
            hdr.int_md.domainID = 0x0000;
        } else {
            hdr.int_md.domainID = 0x0001;  // Using domain ID 1 for SCION
        }
        hdr.int_md.domainInstructions = domainBits;
        hdr.int_md.domainFlags = 0x0000;
        meta.addLen = 0x10;
        // Add INT-Stack
            // Add node ID
        if (hdr.int_md.instructionBitmap & 0x8000 == 0x8000)
            meta.intNodeID = 1;
            // Add level1 interface IDs
        if (hdr.int_md.instructionBitmap & 0x4000 == 0x4000)
        {
	        meta.intL1IfID = 1;
	    }
            // Add hop latency
        if (hdr.int_md.instructionBitmap & 0x2000 == 0x2000)
        {
	        meta.intHopLatency = 1;
	    }
            // Add queue ID and occupancy
        if (hdr.int_md.instructionBitmap & 0x1000 == 0x1000)
        {
	        meta.intQueue = 1;
        }
            // Add ingress timestamp
        if (hdr.int_md.instructionBitmap & 0x0800 == 0x0800)
        {
            meta.intIngressTime = 1;
	    }
	        // Add egress timestamp
        if (hdr.int_md.instructionBitmap & 0x0400 == 0x0400)
        {
            meta.intEgressTime = 1;
	    }
	        // Add level2 interface IDs
        if (hdr.int_md.instructionBitmap & 0x0200 == 0x0200)
        {
	        meta.intL2IfID = 1;
	    }
	        // Add egress interface Tx utilization
        if (hdr.int_md.instructionBitmap & 0x0100 == 0x0100)
        {
	        meta.intEgIfUtil = 1;
	    }
	        // Add buffer ID and occupancy
        if (hdr.int_md.instructionBitmap & 0x0080 == 0x0080)
        {
	        meta.intBufferInfos = 1;
	    }
	        // Add checksum complement
        if (hdr.int_md.instructionBitmap & 0x0040 == 0x0040)
        {
	        meta.intChksumCompl = 1;
        }
            // Add Scion AS address of this node
        if (hdr.int_md.domainInstructions & 0x0001 == 0x0001)
        {
            meta.sciAsAddr = 1;
        }
        // Mark insertion of INT so that egress control does not delete INT
        meta.intState = 1;
    }
    
    @id(0x01002002)
    @brief("Clone packet to delete INT header.")
    action clone_int() {
        meta.intState = 0;
        // Clone from ingress to egress processing and use clone session defined in controller.cpp
        clone_preserving_field_list(CloneType.I2E, 1, 1);
    }

    // Table searches for UDP over SCION packages to insert INT header
    @id(0x02002001)
    @brief("Checks for SCION UDP messages.")
    table scion_int {
        key = {
            hdr.scion_addr_common.dstISD: exact;
            hdr.scion_addr_common.dstAS: exact;
        }
        actions = {
            insert_int;
            clone_int;
            NoAction;
        }
        default_action = NoAction();
    }

    apply {
	    meta.intState = 2;
	    meta.addLen = 0;
        if (hdr.ethernet.isValid()) {
            if (hdr.udp_scion.isValid()) {
                scion_int.apply();
            }
        }
    }
}


///////////////////////
// Egress Processing //
///////////////////////

control INTSwitchEgress(
    inout headers_t hdr,
    inout metadata_t meta,
    inout standard_metadata_t std_meta)
{
    action update_tx_counter() {
        txCounter.count((bit<32>)std_meta.egress_port);
    }

    @id(0x01002003)
    @brief("Add nodeID to INT stack")
    action insert_int_node_id(bit<32> nodeID) {
        meta.addLen = meta.addLen + 0x04;
        hdr.int_stack.nodeID.setValid();
        hdr.int_stack.nodeID.nodeID = nodeID;
    }
    
    // Add ingress timestamp to INT stack
    action insert_int_ig_timestamp() {
        meta.addLen = meta.addLen + 0x08;
        hdr.int_stack.ingressTime.setValid();
        hdr.int_stack.ingressTime.ingressTime = (bit<64>)std_meta.ingress_global_timestamp;
    }
    
    // Add egress timestamp to INT stack
    action insert_int_eg_timestamp() {
        meta.addLen = meta.addLen + 0x08;
        hdr.int_stack.egressTime.setValid();
        hdr.int_stack.egressTime.egressTime = (bit<64>)std_meta.egress_global_timestamp;
    }
    
    // Add egress link tx utilization to INT stack
    @id(0x01002004)
    @brief("Insert Tx link utilization into INT stack.")
    action insert_int_eg_if_util(bit<32> txUtil) {
        meta.addLen = meta.addLen + 0x04;
        hdr.int_stack.egressIFUtilization.setValid();
        hdr.int_stack.egressIFUtilization.egressIFUtil = txUtil;
    }
    
    @id(0x01002005)
    @brief("Add Scion AS addr to INT stack")
    action insert_sci_as_addr(bit<64> asAddr) {
        meta.addLen = meta.addLen + 0x08;
        hdr.int_stack.sciAsAddr.setValid();
        hdr.int_stack.sciAsAddr.asAddr = asAddr;
    }
    
    // Update length-fields of underlying headers
    action int_refresh_length() {
        hdr.int_md.hopML = hdr.int_md.hopML + (bit<5>)((meta.addLen - 0x10) / 4);
        hdr.int_shim.length = hdr.int_shim.length + (bit<8>)((meta.addLen-0x10) / 4);
        hdr.udp_scion.len = hdr.udp_scion.len + meta.addLen;
        hdr.scion_common.payloadLen = hdr.scion_common.payloadLen + meta.addLen;
        hdr.udp.len = hdr.udp.len + meta.addLen;
        
    #ifndef DISABLE_IPV4
	    
	    hdr.ipv4.totalLen = hdr.ipv4.totalLen + meta.addLen;

    #endif /* DISABLE_IPV4 */
    #ifndef DISABLE_IPV6
	    
	    hdr.ipv6.payloadLen = hdr.ipv6.payloadLen + meta.addLen;

    #endif /* DISABLE_IPV6 */
        
        std_meta.packet_length = std_meta.packet_length + (bit<32>)meta.addLen;
    }
    
    @brief("Delete INT header.")
    action delete_int() {
        // Reset UDP destination port
        hdr.udp_scion.dstPort = hdr.int_shim.udpPort;
        // Update length-fields
        bit<16> subLen = 0x10 + (bit<16>)hdr.int_md.hopML * 4 * (NUM_INTER_HOPS + 1 - (bit<16>)hdr.int_md.remainingHopCount);
        hdr.udp_scion.len = hdr.udp_scion.len - subLen;
        hdr.scion_common.payloadLen = hdr.scion_common.payloadLen - subLen;
        hdr.udp.len = hdr.udp.len - subLen;

#ifndef DISABLE_IPV4

	    hdr.ipv4.totalLen = hdr.ipv4.totalLen - subLen;

#endif /* DISABLE_IPV4 */
#ifndef DISABLE_IPV6

	    hdr.ipv6.payloadLen = hdr.ipv6.payloadLen - subLen;

#endif /* DISABLE_IPV6 */

        std_meta.packet_length = std_meta.packet_length - (bit<32>)subLen;
        // Set headers for int invalid
        hdr.int_shim.setInvalid();
        hdr.int_md.setInvalid();
        hdr.int_stack.pre_int_stack.setInvalid();
    }
    
    @brief("Keep INT header for cloned packet.")
    action keep_int_cpu() {
        // Set int_cpu valid to store length of SCION and INT headers
        hdr.int_cpu.setValid();
        hdr.int_cpu.cpuHdrLen = meta.cpuHdrLen;
        // Set payload and all headers before SCION invalid to minimize packet size
	    hdr.ethernet.setInvalid();
	    
#ifndef DISABLE_IPV4

	    hdr.ipv4.setInvalid();
	    
#endif /* DISABLE_IPV4 */
#ifndef DISABLE_IPV6

	    hdr.ipv6.setInvalid();
	    
#endif /* DISABLE_IPV6 */

	    hdr.udp.setInvalid();
	    hdr.payload.setInvalid();
    }
    
    @id(0x02002002)
    @brief("Check if node ID has to be added to INT stack.")
    table int_node_id_table {
        key = {
            meta.intNodeID: exact;
        }
        actions = {
            insert_int_node_id;
            NoAction;
        }
        default_action = NoAction();
    }
    
    // Table checks if ingress timestamp has to be added to INT stack
    table int_ig_timestamp_table {
        key = {
            meta.intIngressTime: exact;
        }
        actions = {
            insert_int_ig_timestamp;
            NoAction;
        }
        default_action = NoAction();
        const entries = {
            1 : insert_int_ig_timestamp();
        }
    }
    
    // Table checks if egress timestamp has to be added to INT stack
    table int_eg_timestamp_table {
        key = {
            meta.intEgressTime: exact;
        }
        actions = {
            insert_int_eg_timestamp;
            NoAction;
        }
        default_action = NoAction();
        const entries = {
            1 : insert_int_eg_timestamp();
        }
    }
    
    // Table checks if egress interface tx utilization has to be added to INT stack
    @id(0x02002003)
    @brief("Check if Tx link utilization has to be added to INT stack.")
    table int_eg_if_util_table {
        key = {
            meta.intEgIfUtil: exact;
            std_meta.egress_port: exact;
        }
        actions = {
            insert_int_eg_if_util;
            NoAction;
        }
        default_action = NoAction();
    }
    
    @id(0x02002004)
    @brief("Check if Scion AS address has to be added to INT stack.")
    table sci_as_addr_table {
        key = {
            meta.sciAsAddr: exact;
        }
        actions = {
            insert_sci_as_addr;
            NoAction;
        }
        default_action = NoAction();
    }
    
    // Table checks if the length fields of underlying headers need to be refreshed
    table scion_int_length {
        key = {
            meta.intState: exact;
        }
        actions = {
            int_refresh_length;
            NoAction;
        }
        default_action = NoAction();
        const entries = {
            1: int_refresh_length();
        }
    }

    // Table lookup to check whether to delete INT or to delete everything else, if packet is forwarded to CPU
    @brief("Checks for SCION UDP messages.")
    table handle_clone_instances {
        key = {
            // Instance_type is 1 for cloned packet (defined in cloneSessionEntry)
            std_meta.instance_type: exact;
        }
        actions = {
            delete_int;
            keep_int_cpu;
        }
        default_action = delete_int();
        const entries = {
            (1) : keep_int_cpu();
        }
    }

    apply {
        if (hdr.ethernet.isValid()) {
            // If INT was inserted then insert stack and update lengths
            if (hdr.udp_scion.isValid() && meta.intState == 1) {
        	    int_node_id_table.apply();
        	    int_ig_timestamp_table.apply();
        	    int_eg_timestamp_table.apply();
        	    int_eg_if_util_table.apply();
        	    sci_as_addr_table.apply();
        	    scion_int_length.apply();
        	}
        	// If packet already had an INT header delete it
        	if (hdr.udp_scion.isValid() && meta.intState == 0) {
                if (hdr.int_shim.isValid()) {
                    handle_clone_instances.apply();
                // Drop cloned packet if it did not contain an INT stack
                } else if (std_meta.instance_type == 1) {
                    mark_to_drop(std_meta);
                }
            } else if (std_meta.instance_type == 1) {
                mark_to_drop(std_meta);
            }
        }
        
        update_tx_counter();
    }
}

#endif

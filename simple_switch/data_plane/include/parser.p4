#ifndef __include_parser__
#define __include_parser__

// Edited from SIDN/p4-scion repository

/* 
 * Copyright (c) 2021, SIDN Labs
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * 
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * 
 * * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include "../headers/common.p4"
#include "../headers/scion.p4"
#include "../headers/int.p4"

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
		packet.extract(hdr.ethernet);
		transition select(hdr.ethernet.etherType) {
#ifndef DISABLE_IPV4
			EtherType.IPV4: ipv4;
#endif /* DISABLE_IPV4 */
#ifndef DISABLE_IPV6
			EtherType.IPV6: ipv6;
#endif /* DISABLE_IPV6 */
    		default: accept;
		}
	}

#ifndef DISABLE_IPV4
	state ipv4 {
		packet.extract(hdr.ipv4);
		transition select(hdr.ipv4.protocol) {
    		Proto.UDP: udp;
    		default: accept;
		}
	}
#endif /* DISABLE_IPV4 */

#ifndef DISABLE_IPV6
	state ipv6 {
		packet.extract(hdr.ipv6);
		transition select(hdr.ipv6.nextHdr) {
    		Proto.UDP: udp;
    		default: accept;
		}
	}
#endif /* DISABLE_IPV6 */

    state udp {
        packet.extract(hdr.udp);
        transition select(hdr.udp.dstPort) {
            50000: scion;
            default: accept;
        }
    }

	state scion {
		packet.extract(hdr.scion_common);
		
		meta.cpuHdrLen = 12;
		
		packet.extract(hdr.scion_addr_common);
		
		meta.cpuHdrLen = meta.cpuHdrLen + 16;
		

		transition select(hdr.scion_common.dl, hdr.scion_common.sl) {
			(0, 0): addr_dst_src_host_32_32;
			(0, 1): addr_dst_src_host_32_64;
			(0, 2): addr_dst_src_host_32_96;
			(0, 3): addr_dst_src_host_32_128;
			(1, 0): addr_dst_src_host_64_32;
			(1, 1): addr_dst_src_host_64_64;
			(1, 2): addr_dst_src_host_64_96;
			(1, 3): addr_dst_src_host_64_128;
			(2, 0): addr_dst_src_host_96_32;
			(2, 1): addr_dst_src_host_96_64;
			(2, 2): addr_dst_src_host_96_96;
			(2, 3): addr_dst_src_host_96_128;
			(3, 0): addr_dst_src_host_128_32;
			(3, 1): addr_dst_src_host_128_64;
			(3, 2): addr_dst_src_host_128_96;
			(3, 3): addr_dst_src_host_128_128;
			default: accept;
		}
	}

	state addr_dst_src_host_32_32 {
		packet.extract(hdr.scion_addr_dst_host_32);
		packet.extract(hdr.scion_addr_src_host_32);
		
		meta.cpuHdrLen = meta.cpuHdrLen + 8;

		transition path;
	}

	state addr_dst_src_host_32_64 {
		packet.extract(hdr.scion_addr_dst_host_32);
		packet.extract(hdr.scion_addr_src_host_32);
		packet.extract(hdr.scion_addr_src_host_32_2);
		
		meta.cpuHdrLen = meta.cpuHdrLen + 12;

		transition path;
	}

	state addr_dst_src_host_32_96 {
		packet.extract(hdr.scion_addr_dst_host_32);
		packet.extract(hdr.scion_addr_src_host_32);
		packet.extract(hdr.scion_addr_src_host_32_2);
		packet.extract(hdr.scion_addr_src_host_32_3);
		
		meta.cpuHdrLen = meta.cpuHdrLen + 16;

		transition path;
	}

	state addr_dst_src_host_32_128 {
		packet.extract(hdr.scion_addr_dst_host_32);
		packet.extract(hdr.scion_addr_src_host_128);
		
		meta.cpuHdrLen = meta.cpuHdrLen + 20;

		transition path;
	}

	state addr_dst_src_host_64_32 {
		packet.extract(hdr.scion_addr_dst_host_32);
		packet.extract(hdr.scion_addr_dst_host_32_2);
		packet.extract(hdr.scion_addr_src_host_32);
		
		meta.cpuHdrLen = meta.cpuHdrLen + 12;

		transition path;
	}

	state addr_dst_src_host_64_64 {
		packet.extract(hdr.scion_addr_dst_host_32);
		packet.extract(hdr.scion_addr_dst_host_32_2);
		packet.extract(hdr.scion_addr_src_host_32);
		packet.extract(hdr.scion_addr_src_host_32_2);
		
		meta.cpuHdrLen = meta.cpuHdrLen + 16;

		transition path;
	}

	state addr_dst_src_host_64_96 {
		packet.extract(hdr.scion_addr_dst_host_32);
		packet.extract(hdr.scion_addr_dst_host_32_2);
		packet.extract(hdr.scion_addr_src_host_32);
		packet.extract(hdr.scion_addr_src_host_32_2);
		packet.extract(hdr.scion_addr_src_host_32_3);
		
		meta.cpuHdrLen = meta.cpuHdrLen + 20;

		transition path;
	}

	state addr_dst_src_host_64_128 {
		packet.extract(hdr.scion_addr_dst_host_32);
		packet.extract(hdr.scion_addr_dst_host_32_2);
		packet.extract(hdr.scion_addr_src_host_128);
		
		meta.cpuHdrLen = meta.cpuHdrLen + 24;

		transition path;
	}

	state addr_dst_src_host_96_32 {
		packet.extract(hdr.scion_addr_dst_host_32);
		packet.extract(hdr.scion_addr_dst_host_32_2);
		packet.extract(hdr.scion_addr_dst_host_32_3);
		packet.extract(hdr.scion_addr_src_host_32);
		
		meta.cpuHdrLen = meta.cpuHdrLen + 16;

		transition path;
	}

	state addr_dst_src_host_96_64 {
		packet.extract(hdr.scion_addr_dst_host_32);
		packet.extract(hdr.scion_addr_dst_host_32_2);
		packet.extract(hdr.scion_addr_dst_host_32_3);
		packet.extract(hdr.scion_addr_src_host_32);
		packet.extract(hdr.scion_addr_src_host_32_2);
		
		meta.cpuHdrLen = meta.cpuHdrLen + 20;

		transition path;
	}

	state addr_dst_src_host_96_96 {
		packet.extract(hdr.scion_addr_dst_host_32);
		packet.extract(hdr.scion_addr_dst_host_32_2);
		packet.extract(hdr.scion_addr_dst_host_32_3);
		packet.extract(hdr.scion_addr_src_host_32);
		packet.extract(hdr.scion_addr_src_host_32_2);
		packet.extract(hdr.scion_addr_src_host_32_3);
		
		meta.cpuHdrLen = meta.cpuHdrLen + 24;
	
		transition path;
	}

	state addr_dst_src_host_96_128 {
		packet.extract(hdr.scion_addr_dst_host_32);
		packet.extract(hdr.scion_addr_dst_host_32_2);
		packet.extract(hdr.scion_addr_dst_host_32_3);
		packet.extract(hdr.scion_addr_src_host_128);
		
		meta.cpuHdrLen = meta.cpuHdrLen + 28;

		transition path;
	}

	state addr_dst_src_host_128_32 {
		packet.extract(hdr.scion_addr_dst_host_128);
		packet.extract(hdr.scion_addr_src_host_32);
		
		meta.cpuHdrLen = meta.cpuHdrLen + 20;

		transition path;
	}

	state addr_dst_src_host_128_64 {
		packet.extract(hdr.scion_addr_dst_host_128);
		packet.extract(hdr.scion_addr_src_host_32);
		packet.extract(hdr.scion_addr_src_host_32_2);
		
		meta.cpuHdrLen = meta.cpuHdrLen + 24;

		transition path;
	}

	state addr_dst_src_host_128_96 {
		packet.extract(hdr.scion_addr_dst_host_128);
		packet.extract(hdr.scion_addr_src_host_32);
		packet.extract(hdr.scion_addr_src_host_32_2);
		packet.extract(hdr.scion_addr_src_host_32_3);
		
		meta.cpuHdrLen = meta.cpuHdrLen + 28;

		transition path;
	}

	state addr_dst_src_host_128_128 {
		packet.extract(hdr.scion_addr_dst_host_128);
		packet.extract(hdr.scion_addr_src_host_128);
		
		meta.cpuHdrLen = meta.cpuHdrLen + 32;
		transition path;
	}

	state path {
		transition select(hdr.scion_common.pathType) {
			PathType.SCION: path_scion;
			PathType.ONEHOP: path_onehop;
			// Other path types are not supported
		}
	}

	state path_onehop {
		packet.extract(hdr.scion_info_field_0);
		packet.extract(hdr.scion_hop_fields, 192);
		
		meta.cpuHdrLen = meta.cpuHdrLen + 32;
	
		transition select(hdr.scion_common.nextHdr) {
		    NextHdr.UDP_SCION: udp_scion_state;
		    default: accept;
		}
	}

	state path_scion {
		packet.extract(hdr.scion_path_meta);
		
		meta.cpuHdrLen = meta.cpuHdrLen + 4;
		
		// We assume there is at least one info field present
		transition select(hdr.scion_path_meta.seg1Len, hdr.scion_path_meta.seg2Len) {
			(0, 0): info_field_0;
			(_, 0): info_field_1;
			default: info_field_2;
		}		
	}

	state info_field_0 {
		packet.extract(hdr.scion_info_field_0);
		
		meta.cpuHdrLen = meta.cpuHdrLen + 8;

		transition hop_fields;
	}

	state info_field_1 {
		packet.extract(hdr.scion_info_field_0);
		packet.extract(hdr.scion_info_field_1);
		
		meta.cpuHdrLen = meta.cpuHdrLen + 16;

		transition hop_fields;
	}

	state info_field_2 {
		packet.extract(hdr.scion_info_field_0);
		packet.extract(hdr.scion_info_field_1);
		packet.extract(hdr.scion_info_field_2);
		
		meta.cpuHdrLen = meta.cpuHdrLen + 24;

		transition hop_fields;
	}

	state hop_fields {
	    bit<32> hopLen = ((bit<32>)hdr.scion_path_meta.seg2Len + (bit<32>)hdr.scion_path_meta.seg1Len + (bit<32>)hdr.scion_path_meta.seg0Len) * 96;
	    packet.extract(hdr.scion_hop_fields, hopLen);
	    
	    meta.cpuHdrLen = meta.cpuHdrLen + ((bit<64>)hopLen / 8);
		
		transition select(hdr.scion_common.nextHdr) {
		    NextHdr.UDP_SCION: udp_scion_state;
		    default: accept;
		}
	}
	
	state udp_scion_state {
	    packet.extract(hdr.udp_scion);
	    
	    meta.cpuHdrLen = meta.cpuHdrLen + 8;
	    
	    transition select(hdr.udp_scion.dstPort) {
	        UDP_PORT: int_shim;
	        default: accept;
	    }
	}
	
	state int_shim {
	    packet.extract(hdr.int_shim);
	    
	    meta.cpuHdrLen = meta.cpuHdrLen + 4;
	    
	    transition select(hdr.int_shim.type) {
	        Type.MD: int_md;
	        default: accept;
	    }
	}
	
	state int_md {
	    packet.extract(hdr.int_md);
	    
	    meta.cpuHdrLen = meta.cpuHdrLen + 12;
	    
	    transition select(hdr.int_md.remainingHopCount) {
	        NUM_INTER_HOPS + 1: accept;
	        default: int_stack;
	    }
	}
	
	state int_stack {
	    meta.intStackLen = (bit<32>)hdr.int_md.hopML * 32 * (NUM_INTER_HOPS - (bit<32>)hdr.int_md.remainingHopCount + 1);
	    packet.extract(hdr.int_stack.pre_int_stack, meta.intStackLen);

	    meta.intNodeID = 0;
	    meta.intL1IfID = 0;
	    meta.intHopLatency = 0;
	    meta.intQueue = 0;
	    meta.intIngressTime = 0;
	    meta.intEgressTime = 0;
	    meta.intL2IfID = 0;
	    meta.intEgIfUtil = 0;
	    meta.intBufferInfos = 0;
	    meta.intChksumCompl = 0;
	    meta.sciAsAddr = 0;
	    transition get_payload;
	}
	
	state get_payload {
	    bit<32> payloadLen = (bit<32>)hdr.scion_common.payloadLen * 8 - 192 - meta.intStackLen;
	    packet.extract(hdr.payload, payloadLen - (payloadLen % 8));

        transition accept;
	}
}

#endif

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

// Parser for ethernet header
parser EthernetParser(
    packet_in packet,
    out ethernet_h ethernet)
{
    state start {
		packet.extract(ethernet);
		transition accept;
	}
}

// Parser for ipv4
#ifndef DISABLE_IPV4
parser IPv4Parser(
    packet_in packet,
    out ipv4_h ipv4)
{
    state start {
        packet.extract(ipv4);
        transition accept;
    }
}
#endif /* DISABLE_IPV4 */

// Parser for ipv6
#ifndef DISABLE_IPV6
parser IPv6Parser(
    packet_in packet,
    out ipv6_h ipv6)
{
    state start {
        packet.extract(ipv6);
        transition accept;
    }
}
#endif /* DISABLE_IPV6 */

// Parser for udp header
parser UDPParser(
    packet_in packet,
    out udp_h udp)
{
    state start {
		packet.extract(udp);
		transition accept;
	}
}

// Parser for SCION common header
parser ScionCommonParser(
    packet_in packet,
    out scion_common_h scion_common)
{
    state start {
		packet.extract(scion_common);
		transition accept;
	}
}

// Parser for SCION address header
parser ScionAdressParser(
    packet_in packet,
    in scion_common_h scion_common,
    out scion_addr_common_h scion_addr_common,
	out scion_addr_host_32_h scion_addr_dst_host_32,
	out scion_addr_host_32_h scion_addr_dst_host_32_2,
	out scion_addr_host_32_h scion_addr_dst_host_32_3,
	out scion_addr_host_128_h scion_addr_dst_host_128,
	out scion_addr_host_32_h scion_addr_src_host_32,
	out scion_addr_host_32_h scion_addr_src_host_32_2,
	out scion_addr_host_32_h scion_addr_src_host_32_3,
	out scion_addr_host_128_h scion_addr_src_host_128,
	inout metadata_t meta)
{
    state start {
        packet.extract(scion_addr_common);
		
		meta.cpuHdrLen = meta.cpuHdrLen + 16;
		

		transition select(scion_common.dl, scion_common.sl) {
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
		packet.extract(scion_addr_dst_host_32);
		packet.extract(scion_addr_src_host_32);
		
		meta.cpuHdrLen = meta.cpuHdrLen + 8;

		transition accept;
	}

	state addr_dst_src_host_32_64 {
		packet.extract(scion_addr_dst_host_32);
		packet.extract(scion_addr_src_host_32);
		packet.extract(scion_addr_src_host_32_2);
		
		meta.cpuHdrLen = meta.cpuHdrLen + 12;

		transition accept;
	}

	state addr_dst_src_host_32_96 {
		packet.extract(scion_addr_dst_host_32);
		packet.extract(scion_addr_src_host_32);
		packet.extract(scion_addr_src_host_32_2);
		packet.extract(scion_addr_src_host_32_3);
		
		meta.cpuHdrLen = meta.cpuHdrLen + 16;

		transition accept;
	}

	state addr_dst_src_host_32_128 {
		packet.extract(scion_addr_dst_host_32);
		packet.extract(scion_addr_src_host_128);
		
		meta.cpuHdrLen = meta.cpuHdrLen + 20;

		transition accept;
	}

	state addr_dst_src_host_64_32 {
		packet.extract(scion_addr_dst_host_32);
		packet.extract(scion_addr_dst_host_32_2);
		packet.extract(scion_addr_src_host_32);
		
		meta.cpuHdrLen = meta.cpuHdrLen + 12;

		transition accept;
	}

	state addr_dst_src_host_64_64 {
		packet.extract(scion_addr_dst_host_32);
		packet.extract(scion_addr_dst_host_32_2);
		packet.extract(scion_addr_src_host_32);
		packet.extract(scion_addr_src_host_32_2);
		
		meta.cpuHdrLen = meta.cpuHdrLen + 16;

		transition accept;
	}

	state addr_dst_src_host_64_96 {
		packet.extract(scion_addr_dst_host_32);
		packet.extract(scion_addr_dst_host_32_2);
		packet.extract(scion_addr_src_host_32);
		packet.extract(scion_addr_src_host_32_2);
		packet.extract(scion_addr_src_host_32_3);
		
		meta.cpuHdrLen = meta.cpuHdrLen + 20;

		transition accept;
	}

	state addr_dst_src_host_64_128 {
		packet.extract(scion_addr_dst_host_32);
		packet.extract(scion_addr_dst_host_32_2);
		packet.extract(scion_addr_src_host_128);
		
		meta.cpuHdrLen = meta.cpuHdrLen + 24;

		transition accept;
	}

	state addr_dst_src_host_96_32 {
		packet.extract(scion_addr_dst_host_32);
		packet.extract(scion_addr_dst_host_32_2);
		packet.extract(scion_addr_dst_host_32_3);
		packet.extract(scion_addr_src_host_32);
		
		meta.cpuHdrLen = meta.cpuHdrLen + 16;

		transition accept;
	}

	state addr_dst_src_host_96_64 {
		packet.extract(scion_addr_dst_host_32);
		packet.extract(scion_addr_dst_host_32_2);
		packet.extract(scion_addr_dst_host_32_3);
		packet.extract(scion_addr_src_host_32);
		packet.extract(scion_addr_src_host_32_2);
		
		meta.cpuHdrLen = meta.cpuHdrLen + 20;

		transition accept;
	}

	state addr_dst_src_host_96_96 {
		packet.extract(scion_addr_dst_host_32);
		packet.extract(scion_addr_dst_host_32_2);
		packet.extract(scion_addr_dst_host_32_3);
		packet.extract(scion_addr_src_host_32);
		packet.extract(scion_addr_src_host_32_2);
		packet.extract(scion_addr_src_host_32_3);
		
		meta.cpuHdrLen = meta.cpuHdrLen + 24;
	
		transition accept;
	}

	state addr_dst_src_host_96_128 {
		packet.extract(scion_addr_dst_host_32);
		packet.extract(scion_addr_dst_host_32_2);
		packet.extract(scion_addr_dst_host_32_3);
		packet.extract(scion_addr_src_host_128);
		
		meta.cpuHdrLen = meta.cpuHdrLen + 28;

		transition accept;
	}

	state addr_dst_src_host_128_32 {
		packet.extract(scion_addr_dst_host_128);
		packet.extract(scion_addr_src_host_32);
		
		meta.cpuHdrLen = meta.cpuHdrLen + 20;

		transition accept;
	}

	state addr_dst_src_host_128_64 {
		packet.extract(scion_addr_dst_host_128);
		packet.extract(scion_addr_src_host_32);
		packet.extract(scion_addr_src_host_32_2);
		
		meta.cpuHdrLen = meta.cpuHdrLen + 24;

		transition accept;
	}

	state addr_dst_src_host_128_96 {
		packet.extract(scion_addr_dst_host_128);
		packet.extract(scion_addr_src_host_32);
		packet.extract(scion_addr_src_host_32_2);
		packet.extract(scion_addr_src_host_32_3);
		
		meta.cpuHdrLen = meta.cpuHdrLen + 28;

		transition accept;
	}

	state addr_dst_src_host_128_128 {
		packet.extract(scion_addr_dst_host_128);
		packet.extract(scion_addr_src_host_128);
		
		meta.cpuHdrLen = meta.cpuHdrLen + 32;
		transition accept;
	}
}

// Parser for SCION path header
parser ScionPathParser(
    packet_in packet,
    in scion_common_h scion_common,
    out scion_path_meta_h scion_path_meta,
	out scion_info_field_h scion_info_field_0,
	out scion_info_field_h scion_info_field_1,
	out scion_info_field_h scion_info_field_2,
	out scion_hop_field_h scion_hop_fields,
	inout metadata_t meta)
{
    state start {
        transition select(scion_common.pathType) {
			PathType.SCION: path_scion;
			PathType.ONEHOP: path_onehop;
			// Other path types are not supported
		}
	}

	state path_onehop {
		packet.extract(scion_info_field_0);
		packet.extract(scion_hop_fields, 192);
		
		meta.cpuHdrLen = meta.cpuHdrLen + 32;
	
		transition accept;
	}

	state path_scion {
		packet.extract(scion_path_meta);
		
		meta.cpuHdrLen = meta.cpuHdrLen + 4;
		
		// We assume there is at least one info field present
		transition select(scion_path_meta.seg1Len, scion_path_meta.seg2Len) {
			(0, 0): info_field_0;
			(_, 0): info_field_1;
			default: info_field_2;
		}		
	}

	state info_field_0 {
		packet.extract(scion_info_field_0);
		
		meta.cpuHdrLen = meta.cpuHdrLen + 8;

		transition hop_fields;
	}

	state info_field_1 {
		packet.extract(scion_info_field_0);
		packet.extract(scion_info_field_1);
		
		meta.cpuHdrLen = meta.cpuHdrLen + 16;

		transition hop_fields;
	}

	state info_field_2 {
		packet.extract(scion_info_field_0);
		packet.extract(scion_info_field_1);
		packet.extract(scion_info_field_2);
		
		meta.cpuHdrLen = meta.cpuHdrLen + 24;

		transition hop_fields;
	}

	state hop_fields {
	    bit<32> hopLen = ((bit<32>)scion_path_meta.seg2Len + (bit<32>)scion_path_meta.seg1Len + (bit<32>)scion_path_meta.seg0Len) * 96;
	    packet.extract(scion_hop_fields, hopLen);
	    
	    meta.cpuHdrLen = meta.cpuHdrLen + ((bit<64>)hopLen / 8);
		
		transition accept;
	}
}

// Parser for INT Shim and MD header + Stack
parser IntParser(
    packet_in packet,
    out int_shim_h int_shim,
    out int_md_h int_md,
	out int_stack_t int_stack,
    inout metadata_t meta)
{
    state start {
		packet.extract(int_shim);

	    meta.cpuHdrLen = meta.cpuHdrLen + 4;
	    
	    transition select(int_shim.type) {
	        Type.MD: int_md_state;
	        default: accept;
	    }
	}
	
	state int_md_state {
	    packet.extract(int_md);
	    
	    meta.cpuHdrLen = meta.cpuHdrLen + 12;
	    
	    transition select(int_md.remainingHopCount) {
	        NUM_INTER_HOPS + 1: accept;
	        default: int_stack_state;
	    }
	}
	
	state int_stack_state {
	    meta.intStackLen = (bit<32>)int_md.hopML * 32 * (NUM_INTER_HOPS - (bit<32>)int_md.remainingHopCount + 1);
	    packet.extract(int_stack.pre_int_stack, meta.intStackLen);

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
	    transition accept;
	}
}

#endif

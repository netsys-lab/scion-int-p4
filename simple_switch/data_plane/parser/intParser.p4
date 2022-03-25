#ifndef __parser_intParser__
#define __parser_intParser__


#include "../headers/common.p4"
#include "../headers/scion.p4"
#include "../headers/int.p4"

////////////
// Parser //
////////////

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
	    
	    transition select(int_shim.length) {
	        0x03: accept;
	        default: int_stack_state;
	    }
	}
	
	state int_stack_state {
	    meta.intStackLen = ((bit<32>)int_shim.length - 0x03) * 32;
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

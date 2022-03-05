#ifndef __include_int__
#define __include_int__

enum bit<4> Type {
  MD = 0x01,
  DESTINATION = 0x02,
  MX = 0x03
}

// INT shim header over UDP is used
header int_shim_h {
    bit<4>    type;
    bit<2>    npt;
    bit<2>    r;
    bit<8>    length;
    bit<16>   udpPort;
}

typedef bit<16> instruction_bitmap_t;
typedef bit<16> domain_bitmap_t;

// Definition of INT-MD header
header int_md_h {
    bit<4>                  version;
    bit<1>                  discard;
    bit<1>                  exceededHopCount;
    bit<1>                  mtuExceeded;
    bit<12>                 reserved;
    bit<5>                  hopML;
    bit<8>                  remainingHopCount;
    instruction_bitmap_t    instructionBitmap;
    bit<16>                 domainID;
    domain_bitmap_t         domainInstructions;
    bit<16>                 domainFlags;
}

// Define the INT stack from the header elements defined before used as header stacks
header pre_int_stack_h {
    varbit<SPACE_FOR_HOPS> int_field;
}

header node_id_h {
    bit<32>     nodeID;
}
header l1_interface_in_id_h {
    bit<16>     l1InterfaceInID;
}
header l1_interface_eg_id_h {
    bit<16>     l1InterfaceEgID;
}
header hop_latency_h {
    bit<32>     hopLatency;
}
header queue_id_h {
    bit<8>      queueID;
}
header queue_occu_h {
    bit<24>     queueOccu;
}
header ingress_time_h {
    bit<64>     ingressTime;
}
header egress_time_h {
    bit<64>     egressTime;
}
header l2_interface_in_id_h {
    bit<32>     l2InterfaceInID;
}
header l2_interface_eg_id_h {
    bit<32>     l2InterfaceEgID;
}
header egress_if_util_h {
    bit<32>     egressIFUtil;
}
header buffer_id_h {
    bit<8>      bufferID;
}
header buffer_occu_h {
    bit<24>     bufferOccu;
}
header scion_as_addr_h {
    bit<64>     asAddr;
}

header int_cpu_h {
    bit<64> cpuHdrLen;
}

// Define the INT stack from the header elements defined before used as header stacks
struct int_stack_t {
    pre_int_stack_h         pre_int_stack;
    node_id_h               nodeID;
    l1_interface_in_id_h    l1InterfaceInID;
    l1_interface_eg_id_h    l1InterfaceEgID;
    hop_latency_h           hopLatency;
    queue_id_h              queueID;
    queue_occu_h            queueOccu;
    ingress_time_h          ingressTime;
    egress_time_h           egressTime;
    l2_interface_in_id_h    l2InterfaceInID;
    l2_interface_eg_id_h    l2InterfaceEgID;
    egress_if_util_h        egressIFUtilization;
    buffer_id_h             bufferID;
    buffer_occu_h           bufferOccu;
    scion_as_addr_h         sciAsAddr;
}

// Checksum complement has to be after domain specific data
header int_chksum_compl_h {
    bit<32>     checksumComplement;
}

header payload_h {
    varbit<65040> payload;  // Max scion payload length - UDP header - known INT header, the round to next multiple of 8
}

#endif

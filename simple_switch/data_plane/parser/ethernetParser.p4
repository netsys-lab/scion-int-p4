#ifndef __parser_ethernetParser__
#define __parser_ethernetParser__

#include "../headers/common.p4"

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

#endif

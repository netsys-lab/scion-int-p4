
control L2Switch(
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
        std_meta.mcast_grp = (bit<16>)(std_meta.ingress_port + 1);
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

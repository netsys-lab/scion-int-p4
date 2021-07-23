package main

import (
	"encoding/binary"
	"math/rand"
	"time"

	"github.com/lschulz/p4-examples/telemetry/kafka/report"
)

const (
	InterfaceLevel1  = 1
	HopLatency       = 2
	QueueOccupancy   = 3
	IngressTimestamp = 4
	EgressTimestamp  = 5
	InterfaceLevel2  = 6
	TxUtilization    = 7
	BufferOccupancy  = 8
	QueueDropReason  = 15
)

// Generate a random report.
func GenerateReport(path []uint64, nodeId uint32) *report.Report {
	var (
		hops                []*report.Hop
		hopLatency          [4]byte
		ingressTimestamp    [8]byte
		egressTimestamp     [8]byte
		egressTxUtilization [4]byte
	)

	hops = make([]*report.Hop, 0, len(path))

	for _, asn := range path {
		t0 := uint64(time.Now().Nanosecond())
		t1 := t0 + 10_000 + 1000*uint64(rand.Int63n(100))
		binary.BigEndian.PutUint32(hopLatency[:], uint32(1000+rand.Int31n(101)-50))
		binary.BigEndian.PutUint64(ingressTimestamp[:], t0)
		binary.BigEndian.PutUint64(egressTimestamp[:], t1)
		binary.BigEndian.PutUint32(egressTxUtilization[:], uint32(1000+rand.Int31n(101)-50))

		node := report.Hop{
			Asn:    asn,
			NodeId: nodeId,
			Metadata: map[uint32][]byte{
				uint32(report.MetadataType_HOP_LATENCY):           hopLatency[:],
				uint32(report.MetadataType_INGRESS_TIMESTAMP):     ingressTimestamp[:],
				uint32(report.MetadataType_EGRESS_TIMESTAMP):      egressTimestamp[:],
				uint32(report.MetadataType_EGRESS_TX_UTILIZATION): egressTxUtilization[:],
			},
		}

		hops = append(hops, &node)
	}

	return &report.Report{
		Hops:            hops,
		PacketType:      report.Report_SCION,
		TruncatedPacket: make([]byte, 64),
	}
}

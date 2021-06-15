package report

import (
	"encoding/binary"
	"math/rand"
	"time"
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
func GenerateReport(path []uint64, nodeId uint32) *Report {
	var (
		hops                []*Node
		hopLatency          [4]byte
		ingressTimestamp    [8]byte
		egressTimestamp     [8]byte
		egressTxUtilization [4]byte
	)

	hops = make([]*Node, 0, len(path))

	for _, asn := range path {
		t0 := uint64(time.Now().Nanosecond())
		t1 := t0 + 10_000 + 1000*uint64(rand.Int63n(100))
		binary.BigEndian.PutUint32(hopLatency[:], uint32(1000+rand.Int31n(101)-50))
		binary.BigEndian.PutUint64(ingressTimestamp[:], t0)
		binary.BigEndian.PutUint64(egressTimestamp[:], t1)
		binary.BigEndian.PutUint32(egressTxUtilization[:], uint32(1000+rand.Int31n(101)-50))

		node := Node{
			Asn:    asn,
			NodeId: nodeId,
			Metadata: map[uint32][]byte{
				HopLatency:       hopLatency[:],
				IngressTimestamp: ingressTimestamp[:],
				EgressTimestamp:  egressTimestamp[:],
				TxUtilization:    egressTxUtilization[:],
			},
		}

		hops = append(hops, &node)
	}

	report := Report{
		Hops:            hops,
		PacketType:      Report_SCION,
		TruncatedPacket: make([]byte, 64),
	}

	return &report
}

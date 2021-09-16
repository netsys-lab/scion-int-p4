package main

import (
	"context"
	"encoding/binary"
	"flag"
	"fmt"
	"os"
	"runtime/pprof"
	"sync"
	"time"

	"github.com/confluentinc/confluent-kafka-go/kafka"
	"github.com/lschulz/p4-examples/telemetry/kafka/report"
	"google.golang.org/protobuf/proto"
)

// Interface for an INT node.
type Node interface {
	// Append this node's metadata to the report.
	AppendMetadata(report *report.Report)
}

// A node that always reports the same metadata values.
type StaticNode struct {
	asn     uint64
	id      uint32
	latency [4]byte
	txUtil  [4]byte
}

func CreateStaticNode(asn uint64, id uint32, latency uint32, txUtil uint32) StaticNode {
	node := StaticNode{
		asn: asn,
		id:  id,
	}
	binary.BigEndian.PutUint32(node.latency[:], latency)
	binary.BigEndian.PutUint32(node.txUtil[:], txUtil)
	return node
}

func (node *StaticNode) AppendMetadata(ingressIf uint16, egressIf uint16, rep *report.Report) {
	var iface [4]byte

	binary.BigEndian.PutUint16(iface[:2], ingressIf)
	binary.BigEndian.PutUint16(iface[2:], egressIf)

	hop := report.Hop{
		Asn:    node.asn,
		NodeId: node.id,
		Metadata: map[uint32][]byte{
			uint32(report.MetadataType_INTERFACE_LEVEL1):      iface[:],
			uint32(report.MetadataType_HOP_LATENCY):           node.latency[:],
			uint32(report.MetadataType_EGRESS_TX_UTILIZATION): node.txUtil[:],
		},
	}
	rep.Hops = append(rep.Hops, &hop)
}

// A (unidirectional) flow from one INT node to another.
type Flow struct {
	// Flow key for the generated reports
	key *report.FlowKey
	// Function adding metadata to the generated reports
	generator func(rep *report.Report)
	// Number of packet to generate
	pktCount int
	// Kafka producer client the reports a are sent through
	prod *kafka.Producer
	// Kafka partition to report to
	partition kafka.TopicPartition
}

// Start generating reports in a separate goroutine.
// wg is incremented before the goroutine starts and decremented once it has exited.
func (flow *Flow) start(wg *sync.WaitGroup) {
	wg.Add(1)
	go func() {
		defer wg.Done()

		// Prepare flow key and report
		wireKey, err := proto.Marshal(flow.key)
		if err != nil {
			panic(err)
		}
		rep := report.Report{
			PacketType: report.Report_None,
		}

		// Generate reports
		for i := 0; i < flow.pktCount; i++ {
			rep.Hops = rep.Hops[:0]
			flow.generator(&rep)

			wireRep, err := proto.Marshal(&rep)
			if err != nil {
				panic(err)
			}

			flow.prod.Produce(&kafka.Message{
				TopicPartition: flow.partition,
				Key:            wireKey,
				Value:          wireRep,
			}, nil)
		}
	}()
}

// Create topics with tha names given in topics.
// Errors because of preexisting topics are ignored.
func createTopics(admin *kafka.AdminClient, topics []string) {
	var options kafka.CreateTopicsAdminOption
	topicSpecs := make([]kafka.TopicSpecification, len(topics))
	for i, topic := range topics {
		topicSpecs[i].Topic = topic
		topicSpecs[i].NumPartitions = 1
		topicSpecs[i].ReplicationFactor = 1
	}

	results, err := admin.CreateTopics(context.Background(), topicSpecs, options)
	if err != nil {
		panic(err)
	}
	for _, result := range results {
		code := result.Error.Code()
		if code != kafka.ErrNoError && code != kafka.ErrTopicAlreadyExists {
			panic(result)
		}
	}
}

func main() {
	var (
		server      string
		pktsPerFlow int
		cpuprofile  string
		wg          sync.WaitGroup
	)

	flag.StringVar(&server, "bootstrap-server", "localhost:9092", "Server to connect to.")
	flag.IntVar(&pktsPerFlow, "pkts-per-flow", 1_000_000, "Reports generated per flow.")
	flag.StringVar(&cpuprofile, "cpuprofile", "", "Write a CPU profile to this file.")
	flag.Parse()

	if cpuprofile != "" {
		file, err := os.Create(cpuprofile)
		if err != nil {
			panic(err)
		}
		pprof.StartCPUProfile(file)
		defer pprof.StopCPUProfile()
	}

	// Open connection to Kafka broker
	prod, err := kafka.NewProducer(&kafka.ConfigMap{
		"bootstrap.servers":   server,
		"go.batch.producer":   false,
		"go.delivery.reports": false,
	})
	if err != nil {
		panic(err)
	}
	defer prod.Close()
	admin, err := kafka.NewAdminClientFromProducer(prod)
	if err != nil {
		panic(err)
	}
	defer admin.Close()

	// Prepare topology
	node1 := CreateStaticNode(0xff00_0000_0001, 1, 10000, 500)
	node2 := CreateStaticNode(0xff00_0000_0002, 1, 10000, 500)
	node3 := CreateStaticNode(0xff00_0000_0003, 1, 10000, 500)
	node4 := CreateStaticNode(0xff00_0000_0004, 1, 10000, 500)

	topics := []string{"ASff00_0_1-1", "ASff00_0_4-1"}
	createTopics(admin, topics)

	flows := []Flow{
		{
			key: &report.FlowKey{DstAs: 0xff00_0000_0004, SrcAs: 0xff00_0000_0001, FlowId: 0x01},
			generator: func(rep *report.Report) {
				node1.AppendMetadata(0, 1, rep)
				node2.AppendMetadata(1, 2, rep)
				node4.AppendMetadata(1, 0, rep)
			},
			pktCount:  pktsPerFlow,
			prod:      prod,
			partition: kafka.TopicPartition{Topic: &topics[0], Partition: kafka.PartitionAny},
		},
		{
			key: &report.FlowKey{DstAs: 0xff00_0000_0004, SrcAs: 0xff00_0000_0001, FlowId: 0x02},
			generator: func(rep *report.Report) {
				node1.AppendMetadata(0, 2, rep)
				node3.AppendMetadata(1, 2, rep)
				node4.AppendMetadata(2, 0, rep)
			},
			pktCount:  pktsPerFlow,
			prod:      prod,
			partition: kafka.TopicPartition{Topic: &topics[0], Partition: kafka.PartitionAny},
		},
		{
			key: &report.FlowKey{DstAs: 0xff00_0000_0001, SrcAs: 0xff00_0000_0004, FlowId: 0x01},
			generator: func(rep *report.Report) {
				node4.AppendMetadata(0, 1, rep)
				node2.AppendMetadata(2, 1, rep)
				node1.AppendMetadata(1, 0, rep)
			},
			pktCount:  pktsPerFlow,
			prod:      prod,
			partition: kafka.TopicPartition{Topic: &topics[1], Partition: kafka.PartitionAny},
		},
	}

	// Generate reports
	t0 := time.Now()
	for i := range flows {
		flows[i].start(&wg)
	}
	wg.Wait()
	remaining := prod.Flush(10_000)
	t1 := time.Now()

	elapsed := t1.Sub(t0).Seconds()
	totalPkts := (len(flows) * pktsPerFlow) - remaining
	fmt.Printf("Sent %v reports in %v seconds.\n", totalPkts, elapsed)
	fmt.Printf("Rate: %v reports/second\n", float64(totalPkts)/elapsed)
}

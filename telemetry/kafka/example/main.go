package main

import (
	"flag"
	"fmt"
	"math/rand"

	"github.com/confluentinc/confluent-kafka-go/kafka"
	"github.com/lschulz/p4-examples/telemetry/kafka/report"
	"google.golang.org/protobuf/proto"
)

func main() {
	var (
		numEvents       int
		numFlows        int
		bootstrapServer string
	)

	flag.IntVar(&numEvents, "produce", 0,
		"Produce the given number of events.")
	flag.IntVar(&numFlows, "flows", 1,
		"For producers: The number of flows for which to generate events.")
	flag.StringVar(&bootstrapServer, "bootstrap-server", "localhost:9092",
		"Server to connect to.")
	flag.Parse()

	if numEvents == 0 {
		runConsumer(bootstrapServer)
	} else {
		runProducer(numEvents, numFlows, bootstrapServer)
	}

}

func runConsumer(server string) {
	// Open connection to Kafka broker
	c, err := kafka.NewConsumer(&kafka.ConfigMap{
		"bootstrap.servers": server,
		"group.id":          "ConsumerGroup1",
		"auto.offset.reset": "latest",
	})
	if err != nil {
		panic(err)
	}
	defer c.Close()

	// Subscribe to reports from all ASes
	c.SubscribeTopics([]string{"ASff00_0_1-1", "ASff00_0_4-1"}, nil)

	// Read reports and print them
	for {
		msg, err := c.ReadMessage(-1)
		if err == nil {
			fmt.Printf("Message on %s:\n", msg.TopicPartition)
			err := printReport(msg.Key, msg.Value)
			if err != nil {
				fmt.Printf("Error: %v\n", err)
			}
		} else {
			fmt.Printf("Consumer error: %v (%v)\n", err, msg)
		}
	}
}

func printReport(rawKey []byte, rawValue []byte) error {
	var (
		err   error
		key   report.FlowKey
		value report.Report
	)

	err = proto.Unmarshal(rawKey, &key)
	if err != nil {
		return err
	}

	err = proto.Unmarshal(rawValue, &value)
	if err != nil {
		return err
	}

	fmt.Printf("Flow key: %s\n", key.String())
	fmt.Printf("Report: %s\n", value.String())

	return nil
}

func runProducer(numEvents int, numFlows int, server string) {
	// Open connection to Kafka broker
	p, err := kafka.NewProducer(&kafka.ConfigMap{
		"bootstrap.servers": server,
	})
	if err != nil {
		panic(err)
	}
	defer p.Close()

	// Monitor asynchronous delivery reports
	go func() {
		for e := range p.Events() {
			switch ev := e.(type) {
			case *kafka.Message:
				if ev.TopicPartition.Error != nil {
					fmt.Printf("Delivery failed: %v\n", ev.TopicPartition)
				} else {
					fmt.Printf("Message delivered to %v\n", ev.TopicPartition)
				}
			}
		}
	}()

	// Paths
	var topics = [...]string{
		"ASff00_0_1-1",
		"ASff00_0_4-1",
	}
	var paths = [...][3]uint64{
		{0xff00_0000_0001, 0xff00_0000_0002, 0xff00_0000_0004},
		{0xff00_0000_0004, 0xff00_0000_0003, 0xff00_0000_0001},
	}

	// Generate random flow keys
	var flows = make([]report.FlowKey, numFlows)
	for i := range flows {
		flows[i].FlowId = uint32(rand.Int31n(1 << 20))
	}

	// Send random reports
	for i := 0; i < numEvents; i++ {
		x := rand.Intn(len(topics))
		topic := &topics[x]
		path := paths[x][:]
		flow := &flows[rand.Int31n(int32(numFlows))]

		key, err := proto.Marshal(flow)
		if err != nil {
			panic(err)
		}

		report, err := proto.Marshal(GenerateReport(path, 1))
		if err != nil {
			panic(err)
		}

		p.Produce(&kafka.Message{
			TopicPartition: kafka.TopicPartition{Topic: topic, Partition: kafka.PartitionAny},
			Key:            key,
			Value:          report,
		}, nil)
	}

	p.Flush(1000)
}

module github.com/lschulz/p4-examples/telemetry/kafka/example

go 1.16

replace github.com/lschulz/p4-examples/telemetry/kafka/report => ../report

require (
	github.com/confluentinc/confluent-kafka-go v1.7.0
	github.com/lschulz/p4-examples/telemetry/kafka/report v0.0.0-00010101000000-000000000000
	google.golang.org/protobuf v1.27.1
)

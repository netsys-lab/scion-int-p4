SCION-INT Telemetry Report Test
===============================

SCION-INT telemetry reports are stored as protobuf messages in a Kafka cluster. The message format
is defined in [report/report.proto](report/report.proto). In Kafka, reports are keyed on the flow
that has generated them. Flows are identified by [FlowKey](report/report.proto) messages.

Kafka events are assigned to topics. We name topics after the INT Sink that generates the events.
For example, an INT Sink in AS 1-ff00:0:1 would be named "ASff00_0_1-1", where the last "1" is an
identifier distinguishing different INT Sinks in the same AS.

Useful Links:
- https://developers.google.com/protocol-buffers
- https://www.confluent.de/learn/kafka-tutorial/


Running the example program
---------------------------
The example program in this directory implements a simple telemetry event producer and consumer.
To run it you need Go and a Kafka server.

To install Kafka, follow the instructions at https://kafka.apache.org/quickstart .
Then run the following command in the folder where you extracted Kafka:
```bash
### Run a Kafka broker (see https://kafka.apache.org/quickstart):
bin/zookeeper-server-start.sh config/zookeeper.properties
bin/kafka-server-start.sh config/server.properties

### Create topics
bin/kafka-topics.sh --bootstrap-server localhost:9092 --topic ASff00_0_1-1 --create
bin/kafka-topics.sh --bootstrap-server localhost:9092 --topic ASff00_0_4-1 --create
```

Build the example:
```bash
./compile_protobuf.sh # if report.proto was changed
go build -o kafka-test
```
If you need to recompile the protocol definition, make sure you have the protobuf compiler installed
and the Go plugin is in your PATH, e.g. by running
```bash
# See https://developers.google.com/protocol-buffers/docs/reference/go-generated
go install google.golang.org/protobuf/cmd/protoc-gen-go@latest
export PATH=$PATH:$HOME/go/bin
```

Run a consumer with:
```bash
./kafka-test
```

Generate random reports from a second terminal:
```bash
./kafka-test --produce 10 --flows 2
```

Kafka stores its temporary data base in `/tmp`. You can reset it by deleting the following folders:
```bash
rm -rf /tmp/kafka-logs /tmp/zookeeper
```

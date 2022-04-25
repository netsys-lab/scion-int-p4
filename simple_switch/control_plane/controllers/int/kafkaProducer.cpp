#include "kafkaProducer.h"
#include <cppkafka/cppkafka.h>


// Constructor for Kafka Producer
kafkaProducer::kafkaProducer(const std::string& kafkaAddress) : kafkaConfig({{ "bootstrap.servers", kafkaAddress }}), kafkaProd(kafkaConfig)
{
}

bool kafkaProducer::send(const std::string& topic_name,
                    const std::string& key, 
                    const std::string& report)
{
    kafkaProd.produce(cppkafka::MessageBuilder(topic_name).key(key).payload(report));
    
    return true;
}

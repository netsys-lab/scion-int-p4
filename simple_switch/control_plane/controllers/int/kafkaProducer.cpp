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
    // If first call, the create kafkaProducer
    /*if (kafkaProd == NULL) {
        kafkaProd = new cppkafka::Producer(kafkaConfig);
    }*/
    // Send kafka message
    kafkaProd.produce(cppkafka::MessageBuilder(topic_name).key(key).payload(report));
    
    return true;
}

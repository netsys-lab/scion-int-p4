#pragma once

#include <cppkafka/cppkafka.h>

// Class that holds a Kafka producer
class kafkaProducer
{
    public:
        kafkaProducer(const std::string& kafkaAddress);
        bool send(const std::string& topic_name, const std::string& key, const std::string& report);
    private:
        cppkafka::Configuration kafkaConfig;
        cppkafka::Producer kafkaProd;
};

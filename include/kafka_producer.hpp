#pragma once

#include <kafka/KafkaProducer.h>

class KafkaProducer
{
public:
    static KafkaProducer &instance()
    {
        static KafkaProducer instance;
        return instance;
    }

    void init(const std::string &brokers)
    {
        kafka::Properties props({{"bootstrap.servers", brokers}});
        props.put("acks", "all");
        props.put("retries", "3");
        props.put("enable.idempotence", "true");
        producer = std::make_unique<kafka::clients::producer::KafkaProducer>(props);
        if (!producer)
        {
            throw std::runtime_error("Failed to create Kafka producer");
        }
        std::cerr << "[Kafka] Producer connected to : " << brokers << std::endl;
    }

    void push(const std::string &topic, const std::string &val, const std::string &key = "")
    {

        kafka::clients::producer::ProducerRecord record(kafka::Topic(topic), kafka::Key(key.c_str(), key.size()), kafka::Value(val.c_str(), val.size()));
        auto deliveryCb = [](const kafka::clients::producer::RecordMetadata &metadata, const kafka::Error &error)
        {
            if (error)
                std::cerr << "[Kafka] Failed to deliver message: " << error.message() << std::endl;
            else
                std::cerr << "[Kafka] Message delivered: " << metadata.toString() << std::endl;
        };
        producer->send(record, deliveryCb, kafka::clients::producer::KafkaProducer::SendOption::ToCopyRecordValue);
    };

    KafkaProducer(const KafkaProducer &) = delete;
    KafkaProducer &operator=(const KafkaProducer &) = delete;

private:
    KafkaProducer() = default;
    ~KafkaProducer() = default;
    std::unique_ptr<kafka::clients::producer::KafkaProducer> producer;
};
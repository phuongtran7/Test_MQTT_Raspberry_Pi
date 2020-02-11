#include "mqtt/async_client.h"
#include "fmt/format.h"
#include "flatbuffers/flexbuffers.h"

class MQTT_Subscriber : public virtual mqtt::callback,
	public virtual mqtt::iaction_listener
{
private:
	std::string address_;
	std::string topic_;
	int qos_;
	mqtt::async_client client_;
	mqtt::connect_options connOpts;

private:
	void on_failure(const mqtt::token& tok) override {
		fmt::print("Connection attempt failed. Will try to reconnect.\n");
		client_.reconnect();
	}

	void on_success(const mqtt::token& tok) override {
		fmt::print("Succeed.\n");
	}

	void connected(const std::string& cause) override {
		fmt::print("Connection success. Will subscribte to {}.\n", topic_);
		client_.subscribe(topic_, qos_, nullptr, *this);
	}

	void connection_lost(const std::string& cause) override {
		fmt::print("Connection lost: {}.\nReconnecting...\n", cause);
		client_.reconnect();
	}

	void message_arrived(mqtt::const_message_ptr msg) override {
		auto payload = msg->get_payload();
		auto data = flexbuffers::GetRoot(reinterpret_cast<const uint8_t*>(payload.data()), payload.size()).AsMap();

		fmt::print("Latitude: {}\n", data["latitude"].AsFloat());
		fmt::print("Latitude: {}\n", data["longitude"].AsFloat());
	}

	void delivery_complete(mqtt::delivery_token_ptr token) override {}

public:
	MQTT_Subscriber(const std::string& address, const std::string& topic, int qos) :
		address_(address),
		topic_(topic),
		qos_(qos),
		client_(address_, ""), // Force random clientID
		connOpts{}
	{
		connOpts.set_keep_alive_interval(20);
		connOpts.set_clean_session(true);
		client_.set_callback(*this);
		client_.connect(connOpts);
	}

	~MQTT_Subscriber()
	{
		try {
			client_.unsubscribe(topic_)->wait();
			client_.stop_consuming();
			client_.disconnect()->wait();
		}
		catch (const mqtt::exception & exc) {
			fmt::print(exc.what());
		}
	};
};

int main(void)
{
	MQTT_Subscriber sub("tcp://192.168.72.249:1883", "XP-S76-Release", 0);

	std::getchar();

	return 0;
}
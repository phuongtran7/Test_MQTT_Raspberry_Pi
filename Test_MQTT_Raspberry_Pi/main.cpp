#include "mqtt/async_client.h"
#include "fmt/format.h"

int main(void)
{
	mqtt::connect_options connOpts;
	connOpts.set_keep_alive_interval(20);
	connOpts.set_clean_session(true);

	// Pass empty string to clientID field to force random clientID
	mqtt::async_client cli("tcp://192.168.72.249:1883", "");

	try {
		cli.connect(connOpts)->wait();
		cli.start_consuming();
		cli.subscribe("MQTT", 0)->wait();

		while (true) {
			auto msg = cli.consume_message();
			if (!msg) break;
			fmt::print("{}: {}\n", msg->get_topic(), msg->to_string());
		}

		cli.unsubscribe("MQTT")->wait();
		cli.stop_consuming();
		cli.disconnect()->wait();
	}
	catch (const mqtt::exception & exc) {
		fmt::print(exc.what());
		return 1;
	}

	return 0;
}
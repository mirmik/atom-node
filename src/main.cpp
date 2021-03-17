#include <crow/tower.h>
#include <crow/address.h>
#include <crow/proto/node.h>
#include <crow/pubsub/publisher.h>
#include <crow/gates/udpgate.h>

#include <thread>
#include <chrono>
#include <string>

#include <igris/trent/trent.h>
#include <igris/trent/json.h>
#include <nos/fprint.h>


class Node : public crow::node
{
	virtual void incoming_packet(crow::packet *pack)
	{
		crow::release(pack);
	}

	virtual void undelivered_packet(crow::packet *pack)
	{
		notify_all(-1);
		crow::release(pack);
	}
};

crow::udpgate ugate;
crow::hostaddr addr;
std::string machine_name;
Node alive_sender;


int counter = 0;
void foo()
{
	alive_sender.bind(42);

	while (1)
	{
		igris::trent tr;

		tr["mnemo"] = machine_name;
		tr["count"] = counter++;

		std::string str = nos::format("{}", tr);

		alive_sender.send(42, addr, str, 0, 50, false);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
}

int main(int argc, char ** argv)
{
	ugate.open(10043);
	ugate.bind(12);

	std::string saddr = argv[1];
	machine_name = argv[2];
	addr = crow::address(saddr);

	std::thread thr(foo);

	crow::start_spin();
	crow::join_spin();
}
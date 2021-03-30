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
#include <igris/getopt/cliopts.h>

#include <nos/fprint.h>

#include <libnotify/notify.h>

NotifyNotification * N;

class Node : public crow::node
{
	virtual void incoming_packet(crow::packet *pack)
	{
		crow::node_packet_ptr ptr(pack);
		N = notify_notification_new("Atom",
		                        ptr.message().data(),
		                        "/home/mirmik/project/atom-node/icon.png");
		notify_notification_set_timeout(N, 10000); // 10 seconds
		notify_notification_show(N, 0);


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
	igris::cliopts cliopts;
	cliopts.add_option("debug", 'd');
	cliopts.parse(argc, argv);

	auto args = cliopts.get_args();
	bool debug = cliopts.get_option("debug");

	if (debug)
	{
		crow::diagnostic_setup(true, false);
	}

	if (args.size() != 3)
	{
		nos::println("Usage: atom-node CROWADDR MACHINE");
		return -1;
	}

	notify_init("Atom");
	N = notify_notification_new ("Start",
	                        "Atom node is sucessfually started",
	                        "/home/mirmik/project/atom-node/icon.png");
	notify_notification_set_timeout(N, 10000); // 10 seconds

	if (!notify_notification_show(N, 0))
	{
		std::cerr << "Notification is not worked" << std::endl;
		return -1;
	}

	ugate.open(10043);
	ugate.bind(12);

	std::string saddr = args[1];
	machine_name = args[2];
	addr = crow::address(saddr);

	std::thread thr(foo);

	crow::start_spin();
	crow::join_spin();
}
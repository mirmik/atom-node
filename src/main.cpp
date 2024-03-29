#include <crow/tower.h>
#include <crow/address.h>
#include <crow/proto/node.h>
#include <crow/gates/udpgate.h>

#include <thread>
#include <chrono>
#include <string>

#include <igris/trent/trent.h>
#include <igris/trent/json.h>
#include <igris/getopt/cliopts.h>

#include <nos/fprint.h>
#include <nos/inet/tcp_server.h>
#include <nos/inet/tcp_client.h>
#include <libnotify/notify.h>

#include <service.h>

int WITHOUT_NOTIFY = 0;
NotifyNotification * N;
service_controller srvsctr;

class Node : public crow::node
{
	virtual void incoming_packet(crow::packet *pack)
	{
		crow::node_packet_ptr ptr(pack);

		if (!WITHOUT_NOTIFY)
		{
			N = notify_notification_new("Atom",
			                            ptr.message().data(),
			                            "/home/mirmik/project/atom-node/icon.png");
			notify_notification_set_timeout(N, 10000); // 10 seconds
			notify_notification_show(N, 0);
		}

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

		alive_sender.send(42, addr, str, 0, 50);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
}

void client_handle(nos::inet::tcp_socket client)
{
	nos::println("New connection");
	while (1)
	{
		char buf[128];
		int len = client.read(buf, 128);
		if (len <= 0) break;
		buf[len] = 0;

		std::string cmd = igris::trim(std::string(buf, len));
		if (cmd == "stopall") 
		{
			srvsctr.stop_all();
		}
		else if (cmd == "startall") 
		{
			srvsctr.start_all();
		}
		else if (cmd == "statusall") 
		{
			srvsctr.status_all();
		}
	}
	nos::println("Close connection");
}

int main(int argc, char ** argv)
{
	igris::cliopts cliopts;
	cliopts.add_option("debug", 'd');
	cliopts.add_string("crowaddr", 'c', ".12.127.0.0.1.10009");
	cliopts.parse(argc, argv);

	auto args = cliopts.get_args();
	bool debug = cliopts.get_option("debug");
	std::string crowaddr = cliopts.get_string("crowaddr");

	if (debug)
	{
		crow::diagnostic_setup(false);
	}

	/*if (args.size() != 3)
	{
		nos::println("Usage: atom-node MACHINE");
		return -1;
	}*/

	//notify_init("Atom");
	/*N = notify_notification_new ("Start",
	                             "Atom node is sucessfually started",
	                             "/home/mirmik/project/atom-node/icon.png");
	notify_notification_set_timeout(N, 10000); // 10 seconds

	if (!notify_notification_show(N, 0))
	{
		std::cerr << "Notification is not worked" << std::endl;
		WITHOUT_NOTIFY = 1;
		//return -1;
	}*/

	ugate.open(10043);
	ugate.bind(12);

	machine_name = "noname";
	addr = crow::address(crowaddr);

	//std::thread thr(foo);
	srvsctr = service_controller("apps.json");

	crow::start_spin();


	nos::inet::tcp_server console("0.0.0.0", 19089);
	console.reusing(true);
	
	nos::println("Console created on tcp:19089");
	while (1)
	{
		auto client = console.accept();
		std::thread(client_handle, client).detach();
	}

	crow::join_spin();
}
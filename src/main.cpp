#include <crow/tower.h>
#include <crow/gates/udpgate.h>
#include <crow/proto/pubsub.h>
#include <crow/address.h>

#include <igris/trent/trent.h>
#include <igris/trent/json.h>
#include <igris/getopt/cliopts.h>
#include <nos/fprint.h>

#include <igris/osutil.h>
#include <igris/util/hexascii.h>

#include <thread>
#include <chrono>

#include <sys/time.h>

using namespace std::chrono_literals;

std::string unique_code;

int udpaddr;
std::string coreaddr;
std::vector<uint8_t>  crowaddr;

igris::cliopts opts;

#include <unistd.h>
#include <limits.h>
#include <pwd.h>

char hostname[HOST_NAME_MAX];
std::string username;

void thread_function()
{
	while (1)
	{
		igris::trent tr;

		tr["unique_code"] = unique_code;
		tr["hostname"] = hostname;
		tr["username"] = username;

		std::string strtr = nos::format("{}", tr);

		crow::publish(crowaddr, "atom-info", strtr, 0, 10);
		std::this_thread::sleep_for(1000ms);
	}
}

void init_creditionals()
{
	gethostname(hostname, HOST_NAME_MAX);
	
	auto uid = geteuid ();
	auto pw = getpwuid (uid);
	if (pw)
	{
		username = std::string(pw->pw_name);
	}
}

void init_unique_code()
{
	std::string atomnode_file_path = igris::osutil::expanduser("~/.atom-node");

	if (!igris::osutil::isexist(atomnode_file_path))
	{
		struct timeval time;
		gettimeofday(&time, NULL);
		srand((time.tv_sec * 1000) + (time.tv_usec / 1000));

		char code[64];

		for (int i = 0; i < 64; ++i)
		{
			code[i] = half2hex(rand() % 16);
		}

		FILE * f = fopen(atomnode_file_path.c_str(), "w");
		fwrite(code, 64, 1, f);

		unique_code = std::string(code, 64);
	}

	else
	{
		char code[64];
		FILE * f = fopen(atomnode_file_path.c_str(), "r");
		fread(code, 64, 1, f);
		unique_code = std::string(code, 64);
	}
}

int main(int argc, char ** argv)
{
	init_unique_code();
	init_creditionals();

	opts.add_integer("udp", 'u', 0);
	opts.add_string("core", 'c', ".12.109.173.108.206:10009");
	opts.parse(argc, argv);

	udpaddr = opts.get_integer("udp").value();
	coreaddr = opts.get_string("core").value();
	crowaddr = crow::address(coreaddr);

	crow::udpgate udpgate;
	udpgate.open(udpaddr);
	udpgate.bind(12);

	crow::diagnostic_enable();

	std::thread thr(thread_function);

	crow::start_spin();
	crow::join_spin();
}
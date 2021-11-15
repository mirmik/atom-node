#ifndef ATOM_NODE_SERVICE_H
#define ATOM_NODE_SERVICE_H

#include <string>
#include <igris/trent/trent.h>
#include <igris/trent/json.h>
#include <igris/sclonner.h>
#include <subprocess/subprocess.hpp>
#include <nos/fault.h>

#include <chrono>
#include <thread>

class service 
{
	std::string name;
	std::string execcmd;
	subprocess::popen subproc;

public:
	service(const std::string& name, const std::string& execcmd) : name(name), execcmd(execcmd) {}

	void start() 
	{
		nos::println("start", name, ":", execcmd);
		subproc = subprocess::popen(execcmd, std::vector<std::string>{});
	}

	void print_status() 
	{
		std::cout << subproc.stdout().rdbuf() << std::endl;
	}

	int runned() 
	{
		int stat;
		int status = waitpid(subproc.pid(), &stat, WNOHANG);
		nos::println(status, stat);
		return 0;
	}
};

class service_controller 
{
	const std::string config_path;
	igris::trent config_trent;
	std::vector<std::unique_ptr<service>> services;

public:
	service_controller() = default; 
	service_controller(const std::string & path) 
		: config_path(path)
	{
		open();
	}

	void open() 
	{
		config_trent = 
igris::json::parse(R"(
{
	"services" : 
	{
		"crowker" : 
		{
			"exec" : "/usr/local/bin/crowker"
		},

		"ls" : 
		{
			"exec" : "/usr/bin/lc"
		}
	},

	"packages" : 
	{
		"zippo" : [ "crowker", "sensors", "zippo_bluetooth_controller" ]
	}
}	
)");

		//config_trent = igris::json::parse_file(config_path);
		validate_check();

		for (auto& [key, value]: config_trent["services"].as_dict()) 
		{
			auto serv = std::make_unique<service>(key, value["exec"].as_string());
			serv->start();
			services.push_back(std::move(serv));
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		services[1]->print_status();
	}

	void validate_check() 
	{
		if (!config_trent.is_dict()) 
			nos::fault("config_trent must be a dictionary");

		for (auto & [key, value] : config_trent.as_dict()) 
		{
			if (!value.is_dict()) 
				nos::fault("nodes must be a dictionaries");
		}
	}

	void stop_all() 
	{
		for (auto & service : services) 
		{
			//service.stop();
		}
	}

	void start_all() 
	{
		for (auto & service : services) 
		{
			//service.start();
		}
	}

	void status_all() 
	{
		for (auto & service : services) 
		{
			service->runned();
		}
	}
};

#endif
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
#include <memory>

enum class ServiceStatus
{
	Runned,
	Dead,
	Terminated,
};


class service_controller;
class service
{
	std::string name;
	std::string execcmd;
	subprocess::popen subproc;
	ServiceStatus _status = ServiceStatus::Dead;

public:
	service(
	    const std::string& name,
	    const std::string& execcmd)
		:
		name(name), execcmd(execcmd)
	{}

	void start()
	{
		_status = ServiceStatus::Runned;
		subproc = subprocess::popen(execcmd, std::vector<std::string> {});
		nos::fprintln("start: {} ({}) : {}", name, subproc.pid(), execcmd);
	}

	void stop()
	{
		if (_status != ServiceStatus::Runned)
			return;

		_status = ServiceStatus::Terminated;
		subproc.terminate();
		nos::println("terminated", name);
	}

	ServiceStatus status() const
	{
		return _status;
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
	std::string config_path;
	igris::trent config_trent;
	std::vector<std::unique_ptr<service>> services;

	std::thread status_checker_thread;
	std::shared_ptr<bool> checker_runned = std::make_shared<bool>(false);

public:
	service_controller() = default;
	service_controller& operator=(service_controller &&) = default;
	service_controller(const std::string & path)
		: config_path(path)
	{
		nos::println("open");
		open();

		nos::println("create_status_checker_thread");
		run_checker();
	}

	void run_checker()
	{
		status_checker_thread = std::thread([](std::shared_ptr<bool> checker_runned)
		{
			nos::println("checker_run");
			while (1)
			{
				int sts;
				pid_t pid = wait(&sts);
				if (pid == -1)
				{
					*checker_runned = false;
					nos::println("checker_stop");
					return;
				}

				nos::println("finished: pid:", pid);
			}
		}, checker_runned);
		*checker_runned = true;
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

		"dataproxy" : 
		{
			"exec" : "/home/mirmik/project/radioline/rfmeask_dataproxy/dataproxy"
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

//		std::this_thread::sleep_for(std::chrono::milliseconds(100));
//		services[1]->print_status();
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
			service->stop();
		}
	}

	void start_all() 
	{
		for (auto & service : services) 
		{
			if (service->status() != ServiceStatus::Runned)
				service->start();
		}

		if (*checker_runned == false) 
		{
			status_checker_thread.join();
			run_checker();
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
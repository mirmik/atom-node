#ifndef ATOM_NODE_SERVICE_H
#define ATOM_NODE_SERVICE_H

#include <string>
#include <igris/trent/trent.h>
#include <igris/trent/json.h>
#include <igris/sclonner.h>
#include <nos/fault.h>

class service_controller 
{
	const std::string config_path;
	igris::trent config_trent;
	igris::sclonner clonner;

public:
	service_controller() = default; 
	service_controller(const std::string & path) 
		: config_path(path)
	{
		open();
	}

	void open() 
	{
		config_trent = igris::json::parse_file(config_path);
		validate_check();
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
};

#endif

#pragma once

class program_options {
public:
	boost::program_options::options_description desc{"options"};
	boost::program_options::variables_map vm;
	//	boost::program_options::positional_options_description positional;
	int init(int argc, char* argv[]);
	std::string get_project() { return vm.count("project") ? vm["project"].as<std::vector<std::string> >().front() : std::string();}
public:
//	program_options(int argc, char* argv[]) : desc("Options") { init(argc, argv); }
	program_options() {}// : desc("options") { }
};
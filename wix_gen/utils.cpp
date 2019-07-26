
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <assert.h>
#include <filesystem>
#include <stack>
#include <variant>
#include "tree_variant.h"
#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>

#include <xml/xml.h>
using namespace XML;

#include "utils.h"
#include "program_options.h"

// ............................................................................
void add_variable(CXML& xml, const instruct_pair& pair)
{
	pugi::xml_node temp = xml.append_child(pugi::node_pi);
	temp.set_name(("define"));
	std::stringstream str;
	str << pair.first << "='" << pair.second << '\'';
	temp.set_value(str.str().c_str());
}

// ............................................................................
void insert_xml(CXML& dest, const fs::path& path)
{
	COUT("inserting document: " << path);
	auto product_dest = XMLNODE(dest).GetElement("Wix").GetElement("Product");
	assert(product_dest);
	auto comment = product_dest.insert_child_after(pugi::node_comment, product_dest.last_child());
	std::string msg("begin insertion of: ");
	msg += path.string();
	comment.set_value(msg.c_str());
	CXML preui;
	preui.Open(path.string());
	for (const auto child : preui.get_current_node())
		product_dest.append_copy(child);
}

// ............................................................................
// ............................................................................
namespace po = boost::program_options;
int program_options::init(int argc, char* argv[])
{
	try
	{
		po::positional_options_description pos;
		pos.add("project", -1);
		desc.add_options()
			("help,h", "print usage message")
			("foo", po::value<std::string>(), "just an option")
			("project,p",po::value<std::vector<std::string>>()->
			multitoken()->zero_tokens()->composing(), "The path/filename of the project");
			;
		po::command_line_parser parser{ argc, argv };

		parser.options(desc).positional(pos).allow_unregistered();
		po::parsed_options parsed_options = parser.run();
		po::store(parsed_options, vm);

		if (vm.count("help"))
		{
			std::cout << desc << "\n";
			return 0;
		}
		po::notify(vm);
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << "\n";
		return -1;
	}
	return 0;
}

dir_tree get_components(const fs::path& path)
{
	int level = 0;
	dir_tree root{ dir_node(path) };
	dir_helper help(root);

	fs::recursive_directory_iterator it(path);
	for (; it != fs::recursive_directory_iterator(); ++it)
	{
		for (; it.depth() < level; --level)
			help.pop();

		if (fs::is_directory(it->path())) {
			help.add_node(it->path());
			++level;
		}
		else {
			help.add_child(it->path());
		}
	}
	return root;
}

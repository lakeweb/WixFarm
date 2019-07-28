
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
#include <uuid.h>
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

void write_components(const dir_tree& tree, pugi::xml_node& xmlnode)
{
	std::vector<std::string> ids;
	tree_type<fs::path> the_tree(tree);
	dir_helper help(the_tree);
	size_t index = 0;
	//files are inserted in Wix/Product, so........ hack for now
	auto target = xmlnode.append_child("DirectoryRef");
	target.append_attribute("Id").set_value("TARGETDIR");
	for (auto node = help.begin(); node != help.end(); ++node)
	{
		if (node.is_node()) {//Directory
			if (node.ipos.size() >= index) {
				(target = target.append_child("Directory"))
					.append_attribute("Name").set_value(node.iter_node().item.filename().string().c_str());
				++index;
				auto idr = HE_UUID().Generate().string();
				std::replace(idr.begin(), idr.end(), '-', '_');
				idr.insert(idr.begin(), '_');
				target.append_attribute("Id").set_value(idr.c_str());
			}
			else
				for (; index > node.ipos.size(); --index)
					target = target.parent();

			//std::cout << *node << " at: " << node.ipos.size() << " index: " << index << std::endl;
		}
		else//File
		{
			auto child = target.append_child("Component");
			auto path = std::get<fs::path>(*node.initer);
			auto val = path.filename().string();
			std::replace(val.begin(), val.end(), '-', '_');

			auto idr = HE_UUID().Generate().string();
			std::replace(idr.begin(), idr.end(), '-', '_');
			idr.insert(idr.begin(), '_');
			ids.push_back(idr);
			child.append_attribute("Id").set_value(idr.c_str());
			child.append_attribute("Guid").set_value(HE_UUID().Generate().string().c_str());
			// <File Id="file.name" Name="Pb220187.jpg" KeyPath="yes" ShortName="PB220187.JPG" DiskId="1" Source="C:\cpp\ReserveAnalyst_142\wix\dark\File\_03941D4AC92146E3A9D0C466BBA73E9C" />

			auto file = child.append_child("File");
			file.append_attribute("Id").set_value(idr.c_str());
			file.append_attribute("Name").set_value(val.c_str());
			file.append_attribute("KeyPath").set_value("yes");
			file.append_attribute("DiskId").set_value("1");
			file.append_attribute("Source").set_value(path.string().c_str());
		}
	}
	auto child = xmlnode.append_child("Feature");
	child.append_attribute("Id").set_value("MainApplication");
	child.append_attribute("Level").set_value("1");
	for (const auto& id : ids)
		child.append_child("ComponentRef").append_attribute("Id").set_value(id.c_str());
}
/*
void add_components(pugi::xml_node& node, bfs::path& path)
{
	std::vector<std::string> ids;
	int level = 0;
	bfs::recursive_directory_iterator it(path);
	for (; it != bfs::recursive_directory_iterator(); ++it)
	{
		std::cout << *it << std::endl;
		for (; it.depth() < level; --level)
			node = node.parent();

		if (bfs::is_directory(it->path())) {
			++level;
			(node = node.append_child("Directory"))
				.append_attribute("Name").set_value(it->path().filename().string().c_str());
			auto idr = HE_UUID().Generate().string();
			std::replace(idr.begin(), idr.end(), '-', '_');
			idr.insert(idr.begin(), '_');
			node.append_attribute("Id").set_value(idr.c_str());
			//ids.push_back(idr);
			//node.append_attribute("Guid").set_value(HE_UUID().Generate().string().c_str());
		}
		else {
			auto child = node.append_child("Component");
			auto val = it->path().filename().string();
			std::replace(val.begin(), val.end(), '-', '_');

			auto idr = HE_UUID().Generate().string();
			std::replace(idr.begin(), idr.end(), '-', '_');
			idr.insert(idr.begin(), '_');
			ids.push_back(idr);
			child.append_attribute("Id").set_value(idr.c_str());
			child.append_attribute("Guid").set_value(HE_UUID().Generate().string().c_str());
			// <File Id="file.name" Name="Pb220187.jpg" KeyPath="yes" ShortName="PB220187.JPG" DiskId="1" Source="C:\cpp\ReserveAnalyst_142\wix\dark\File\_03941D4AC92146E3A9D0C466BBA73E9C" />

			auto file = child.append_child("File");
			file.append_attribute("Id").set_value(idr.c_str());
			file.append_attribute("Name").set_value(it->path().filename().string().c_str());
			file.append_attribute("KeyPath").set_value("yes");
			file.append_attribute("DiskId").set_value("1");
			file.append_attribute("Source").set_value(bfs::path(it->path()).string().c_str());

		}
	}
	node = node.parent();
	auto child = node.append_child("Feature");
	child.append_attribute("Id").set_value("MainApplication");
	child.append_attribute("Level").set_value("1");
	for (const auto& id : ids)
		child.append_child("ComponentRef").append_attribute("Id").set_value(id.c_str());
}

*/
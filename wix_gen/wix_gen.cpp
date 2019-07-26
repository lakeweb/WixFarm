// wix_gen.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

//#include <windows.h>
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

//log the msi install:
//msiexec /i reserveanalyst.msi /l*vx test.log

/*
https://www.codeproject.com/Articles/21442/Creating-an-installer-using-Wix-v3-0-Votive-and-Vi
https://www.codeproject.com/Articles/21472/Creating-an-installer-using-Wix-v3-0-Votive-and-2
and:
https://stackoverflow.com/questions/471424/wix-tricks-and-tips
the video:
https://channel9.msdn.com/blogs/scobleizer/wix-team-the-most-used-piece-of-software-at-microsoft-and-its-open-source#Page=2
*/
// ............................................................................
// ............................................................................
int main(int argc, char* argv[])
{
	//SetCurrentDirectoryA("C:\cpp\ReserveAnalyst_142\WixSetup\RaProject");
	//{
	//	char buf[260];
	//	GetCurrentDirectoryA(260, buf);
	//	std::cout << "Current Directory: " << buf << std::endl;
	//}
	std::string build("Release");//from command line
	program_options prog_opts;
	if (prog_opts.init(argc, argv))
		return -1;

	if (!prog_opts.vm.count("project") || ! fs::exists(prog_opts.get_project()))
	{
		WARN("Failed to FIND project file:\n " << prog_opts.get_project() << std::endl);
		return -1;
	}

	fs::path directives_file_prime(prog_opts.get_project());//from command line
	auto directives_file = fs::absolute(directives_file_prime);
	auto directives_folder = directives_file.parent_path();
	CXML directives;
	if (!directives.Open(directives_file.string()))
	{
		WARN("Failed to OPEN project file:\n " << prog_opts.get_project() << std::endl);
		return -1;
	}
	COUT("Project file: " << directives_file);

	// gather the variables
	instruct_set i_set;
	{
		auto node = directives.first_element_by_path("root/Project");
		for (const XMLNODE& child : node)
			if(child.type() == pugi::node_element)
				i_set.push_back(instruct_pair(child.name(), child.GetAttribute("val")));

		//DOUT(i_set);
		//specialize for build type
		if (build.size())
		{
			std::string qstr("root/Build[@type='" + build + "']");
			auto nodes = directives.select_nodes(qstr.c_str());
			if (!nodes.size())
				WARN("Build type: " << build << " not found in: " << directives_file);
			else
				for (const auto nit : nodes.first().node())
				{
					std::cout << nit.name() << std::endl;
					auto it = std::find_if(i_set.begin(), i_set.end(),
						[nit](instruct_pair& it) {return it.first == nit.name(); });
					if (it != i_set.end()) {
						COUT("found Build directive: " << *it);
						it->second = nit.first_attribute().value();
					}
					else
						i_set.push_back(instruct_pair(nit.name(), nit.attribute("val").value()));
				}
		}
		//DOUT(i_set);
		DOUT(node.name());
	}
	auto files_node = XMLNODE(directives.select_nodes("root/Files").first().node());
	fs::path product_destination(files_node.GetElement("OutputProductFile").GetAttribute("path"));
	product_destination = directives_folder / product_destination;
	COUT("creating proeuct document: " << product_destination << "\n");

	CXML product; //the target project document
	product.Open(product_destination.string(), pugi::parse_declaration| pugi::parse_doctype|pugi::parse_comments|pugi::parse_embed_pcdata);
	XMLNODE dest = product.Clear("Wix");
	dest.root().remove_child("Wix");

	DOUT("adding variables set to document\n");
	for (const auto& inst : i_set) {
		//DOUT("var: " << inst);
		std::cout << "var: " << inst << std::endl;
		add_variable(product, inst);
	}

	//the product header is a fully qualified wxs file to create the body of the target wxs file
	DOUT("adding product header to document\n");
	CXML product_source;
	auto primary = files_node.select_node("/root/Files/PrimaryInputFile");
	product_source.Open(primary.node().attribute("path").value());
	auto wix_node = product_source.GetNode("/Wix", false);
	assert(wix_node);
	auto id = wix_node.GetElement("Product").GetAttribute("Id");
	dest.root().append_copy(wix_node);

	//any additions belong in 'Product'
	auto product_dest = XMLNODE(product).GetElement("Wix").GetElement("Product");

	//insert the items directly from 'Entries' without the attribute 'target'
	auto entry_nodes = directives.select_nodes("root/Entries");
	{
		for (const auto& item : entry_nodes)
		{
			auto xnode = XMLNODE(item.node());
			if (!xnode.attribute("target"))
				//product_dest.append_copy(item.node().first_child());
				for (const auto& iter : item.node()) {
					std::cout << iter.attribute("comment").value() << " Root Entries item: " << item.node().name() << " and: " << iter.name() << std::endl;
					product_dest.append_copy(iter);
				}
		}
	}

	// Insert file set
	auto second_files = directives.select_nodes("root/Files/SecondaryInputFile");
	for (const auto& file : second_files)
	{
		auto xfile = XMLNODE(file.node());
		std::cout << "Inserting: " << xfile.GetAttribute("file") << std::endl;
		insert_xml(product, file.node().attribute("path").value());
	}

	//insert the items directly from 'Entries' that have the attribute 'pos'
	for (const auto& item : entry_nodes)
	{
		std::cout << item.node().value() << std::endl;
		if (auto tnode = item.node().attribute("target"))
		{
			if (item.node().attribute("omit").as_bool()) {
				std::cout << item.node().attribute("comment").value() << std::endl;
				std::cout << "Omit: " << item.node().name() << std::endl;
				continue;
			}
			auto ptnode = product.GetNode(tnode.value());
			auto tnodes = item.node().select_nodes("*");
			for (auto iter : tnodes) {
				std::cout << item.node().attribute("comment").value() << " ptnode: " << ptnode.name() << " and: " << iter.node().name() << std::endl;
				ptnode.append_copy(iter.node());
			}
		}
	}

	//write off the product.wxs
	std::cout << "writing document to file: " << product_destination << std::endl;
	product.Close();

	fs::copy_file(R"(C:\cpp\ReserveAnalyst_142\WixSetup\RaProject\bin\Product.wxs)",
		R"(C:\cpp\ReserveAnalyst_142\wix\Product.wxs)",
		fs::copy_options::overwrite_existing);
}

/*
	<!-- testing this gets deleted !!!!!!!!!!!!! -->
	<Directory Id="TARGETDIR" Name="SourceDir">
		<Component Id="testfile" Guid="{36F6E6AA-8EF3-94E0-21D1-05D70AA87697}">
			<File Id="test_file.xml" Name="test_file.xml" Source="File1.wxs" />
		</Component>
	</Directory>
    <Feature Id="DefaultFeature" Level="1">
      <ComponentRef Id="testfile"/>
    </Feature>


const char test[] = R"(
<root>
    <Binary Id="DefBannerBitmap" SourceFile=".\images\gyro.bmp" />

	<Directory Id ="TARGETDIR" Name="SourceDir">
		<Directory Id="ProgramFilesFolder">
			<Directory Id="APPLICATIONROOTDIRECTORY" Name="Highland Electronics"/>
		</Directory>
	</Directory>
	<DirectoryRef Id="APPLICATIONROOTDIRECTORY">
		<Component Id="ars.exe" Guid="{CA479801-3BD9-4008-897B-E4AC90E0D72B}">
			<File Id="ars.exe" Source="C:\cpp\ReserveAnalyst_142\patch\ars.exe" KeyPath="yes" Checksum="yes"/>
		</Component>
	</DirectoryRef>
	<Feature Id="MainApplication" Title="Main Application" Level="1">
		<ComponentRef Id="ars.exe" />
	</Feature>
</root>
)";
//CXML testxml;
//auto loadr = testxml.load_buffer(test, sizeof(test));
//for(const auto node : testxml.root().first_child())
//	product_dest.append_copy(node);

*/
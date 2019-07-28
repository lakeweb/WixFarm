#pragma once


namespace fs = std::experimental::filesystem;
using components = std::vector<fs::path>;

using  instruct_pair = std::pair<std::string, std::string>;
using instruct_set = std::vector<instruct_pair>;

// ............................................................................
inline std::ostream& operator << (std::ostream& os, const instruct_pair& is) {
	return os << is.first << " = " << is.second;
}

//debuging output
// ............................................................................
#ifdef _DEBUG
template<typename T>
void DOUT(const T& list) {
	for (const auto& item : list)
		std::cout << item << std::endl;
};
//template<typename T>
//void DOUT(const T* os) {}
inline void DOUT(const char* val) { std::cout << val << std::endl; }
#define COUT(x) (std::cout << x << std::endl)

#else
template<typename T>
void DOUT(const T& list) {}
#define COUT(x) __noop;
#endif

#define WARN(x) (std::cout << x << std::endl)

template < typename T, typename Char = TCHAR >
class AStrTFlag
{
	typedef std::map<long, const Char*> map_type;
	typedef typename map_type::iterator It;
	typedef std::pair<long, const Char*> Pair;
	typedef std::basic_string< Char > astr_type;
	map_type map;

public:
	AStrTFlag(const Char** strTypes, long const sizeTypes)
	{
		for (long i = 0; i < sizeTypes; ++i)
			map.insert(Pair(T(i), strTypes[i]));
	}
	const Char* GetFlagsPointer(T inFlags)
	{
		It it;
		if (!inFlags)//check for null
		{
			it = map.find(inFlags);
			ASSERT(it != map.end());
			return it->second;
		}
		long mask = 1;
		for (; mask; mask = mask << 1)
		{
			if (inFlags & mask)
			{
				it = map.find(inFlags);
				ASSERT(it != map.end());
				return it->second;
			}
		}
		assert(false);
		return NULL;
	}
	T GetFlags(const Char* pstrInFlags)
	{
		astr_type strT(pstrInFlags);
		long pos = 0;
		long flags = 0;
		for (; pos != -1; )
		{
			long to = strT.find(',', pos);
			astr_type strE;
			if (to == astr_type::npos)
			{
				strE = strT.substr(pos);
				--to;
			}
			else
				strE = strT.substr(pos, to - pos);

			for (It it = map.begin(); it != map.end(); ++it)
			{
				if (strE == it->second)
				{
					flags |= it->first;
					break;
				}
			}
			pos = to + 1;
		}
		return static_cast<T>(flags);
	}
};

// ............................................................................
void add_variable(CXML& xml, const instruct_pair& pair);
void insert_xml(CXML& dest, const fs::path& path);

// ............................................................
using dir_node = tree_node<fs::path>;
using dir_tree = tree_type<fs::path>;
using dir_helper = tree_helper<fs::path>;

dir_tree get_components(const fs::path& path);
void write_components(const dir_tree& tree, pugi::xml_node& xmlnode);

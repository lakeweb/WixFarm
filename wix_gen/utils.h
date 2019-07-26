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

// ............................................................................
void add_variable(CXML& xml, const instruct_pair& pair);
void insert_xml(CXML& dest, const fs::path& path);


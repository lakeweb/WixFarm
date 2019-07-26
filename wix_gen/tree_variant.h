#pragma once

//windows namespace
namespace fs = std::experimental::filesystem;

template<typename T>
struct tree_node {
	T item;
	std::list<std::variant<tree_node<T>, T>> children;
	tree_node(const T& path) :item(path) {}
};

//the container
template<typename T>
using tree_type = std::variant<tree_node<T>, T>;

template<typename T>
std::ostream& operator <<  (std::ostream& os, const tree_type<T>& tt) {
	//if constexpr (std::is_final<T>::value) //this never resolves to true with about everything I've tried.
	//	return os << std::get<tree_node<T> >(tt).item << "**";
	//else
	//	return os << std::get<T>(tt) << "**";

	if (!tt.index())
		return os << std::get<0>(tt);
	else
		return os << std::get<1>(tt).item;
}

template<typename T>
struct tree_helper {

	using value_type = T;
	using node_type = tree_node<T>;
	using tree_type = tree_type<T>;

	tree_type& root;
	//didn't want to do this but latest try getting iterator going...
	std::stack<node_type*> pos;
	tree_helper(tree_type& ri) :root(ri) {
		pos.push(&std::get<node_type>(root));
	}
	void add_node(const T& path) {
		pos.top()->children.push_back(node_type(path));
		pos.push(&std::get<node_type>(pos.top()->children.back()));
	}
	void add_child(const T& path) {
		pos.top()->children.push_back(path);
	}
	void pop() { pos.pop(); }

	struct var_print {
		void operator()(tree_node<T>& node) { std::cout << node.item << std::endl; }
		void operator()(T& item) { std::cout << item << std::endl; }
	} visitor;

	//iterator................................
	//https://stackoverflow.com/questions/3582608/how-to-correctly-implement-custom-iterators-and-const-iterators
	//https://stackoverflow.com/questions/2150192/how-to-avoid-code-duplication-implementing-const-and-non-const-iterators
	//hmmmmmmmmm....
	//https://pabloariasal.github.io/2018/06/26/std-variant/

	template<typename ValueType>
	struct base_iterator : std::iterator< std::forward_iterator_tag, T > {

		using node_t = tree_helper::node_type;
		using tree_t = tree_helper::tree_type;

		friend std::ostream& operator <<  (std::ostream& os, const tree_t& tt) {
			std::visit(var_print(), tt);
			return os;
		}
		//private:
		using initer_t = typename std::list<tree_t>::iterator;
		initer_t initer;
		tree_type* np;
		bool it_is_dir;
		struct pos_item {
			initer_t iter;
			node_t* node;
			pos_item(node_type* ni, initer_t it) : node(ni), iter(it) {}
		};
		std::vector<pos_item> ipos;
		friend struct tree_helper;
		node_t& top_node() { return *ipos.back().node; }
		node_t& iter_node() { return std::get<node_t>(*initer); }
		bool is_node() const { return !initer->index(); }
		// .......................
		void print_stack() {
			std::cout << std::endl;
			for (auto st : ipos)
			{
				std::cout << "stack: " << st.node->item
					<< "\n     iter: ";
				std::visit(var_print(), *st.iter);
			}
			std::cout << std::endl;
		}
		// .......................
		bool next_dir() {
			for (++initer; initer != top_node().children.end() && initer->index(); ++initer)
				;
			bool notdone = initer != top_node().children.end();
			//			if (notdone)
			it_is_dir = true;
			return notdone;
		};
		bool next_type() {
			for (++initer; initer != top_node().children.end() && !initer->index(); ++initer)
				;
			return initer != top_node().children.end();
		}
		bool pop() {
			std::cout << "in pop....";
			//print_stack();
			ipos.pop_back();
			if (!ipos.size())
				return false;
			initer = ipos.back().iter;
			for (; !next_dir(); initer = ipos.back().iter) {
				ipos.pop_back();
				if (ipos.size() == 1)
					return false;
			}
			ipos.back().iter = initer;
			//std::cout << ipos.size() << "  pop: " << top_node().item << "\n     ";
			//std::visit(var_print(), *initer);
			//std::cout << std::endl;

			return true;
		}
	public:
		T& operator*() { return initer->index() ? std::get<T>(*initer) : std::get<node_type>(*initer).item; }
		friend bool operator!= (base_iterator const& lhs, base_iterator const& rhs) { return lhs.np; }//just for end()

		friend std::ostream& operator << (std::ostream& os, initer_t&) { return os; }

		void push() { //condisional
			if (!iter_node().children.empty()) {
				//std::cout << "in push....";
				//print_stack();
				auto bit = iter_node().children.begin();
				ipos.push_back(pos_item(&iter_node(), bit));

				initer = bit;
				it_is_dir = !initer->index();
				//if (it_is_dir && !next_type()) {
				//	initer = bit;//continue with node_type's
				//}
				//else
				//	it_is_dir = false;
			}
			else {
				assert(false);
				ipos.pop_back();
				//				std::cout << " empty: "; std::visit(var_print(), *initer); std::cout << std::endl;

				initer = ipos.back().iter;
				for (; next_dir(); initer = ipos.back().iter)
					;
			}
			//std::cout << ipos.size() << "  push: " << top_node().item << "\n     ";
			//std::visit(var_print(), *initer);
			//std::cout << std::endl;

		}
		friend void operator ++(base_iterator& it) {
			//			std::cout << "  ++" << it.top_node().item << "\n   "; std::visit(var_print(), *it.initer); std::cout << std::endl;
			if (it.it_is_dir)
				if (it.iter_node().children.size())
					it.push();
				else
					it.next_dir();

			else if (!it.next_type()) {
				for (it.initer = it.top_node().children.begin(); !it.next_dir() && it.initer != it.top_node().children.end(); )
					;
				if (it.initer == it.top_node().children.end())
					for (; it.ipos.size() && !it.pop(); )
						;
				if (!it.ipos.size())
					it = nullptr;
				//				std::cout << '\n' << "post pop: " << it.iter_node().item << "\n";
				it.it_is_dir = true;
			}
		}
		// etc.
	private:
		base_iterator(tree_type* ptr) : np(ptr), it_is_dir(false) {
			if (ptr) {
				initer = std::get<node_t>(*ptr).children.begin();
				ipos.push_back(pos_item(&std::get<node_t>(*ptr), initer));
			}
		}; // private constructor for begin, end
	};
	typedef base_iterator< T > iterator;
	typedef base_iterator< T const > const_iterator;

	iterator begin() { return iterator(&root); }
	iterator end() { return iterator(nullptr); }
};

// ............................................................
using dir_node = tree_node<fs::path>;
using dir_tree = tree_type<fs::path>;
using dir_helper = tree_helper<fs::path>;

dir_tree get_components(const fs::path& path);

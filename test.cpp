#include <iostream>
#include <string>
#include <unordered_map>
#include <functional>
#include <vector>
#include <fstream>
#include <stack>
#include <sstream>

template<typename T>
struct st_supported_type
{
	static const bool val = false;
};

template<>
struct st_supported_type<std::string>
{
	static const bool val = true;
	using type = std::string;
};

template<>
struct st_supported_type<const char*>
{
	static const bool val = true;
	using type = const char*;
};


template<>
struct st_supported_type<int>
{
	static const bool val = true;
	using type = int;
};

template<>
struct st_supported_type<double>
{
	static const bool val = true;
	using type = double;
};

template<typename T1, typename T2>
struct st_same_type
{
	static const bool value = false;
};
template<typename T>
struct st_same_type<T, T>
{
	static const bool value = true;
};

template<typename T1, typename T2>
constexpr bool is_same_v = st_same_type<T1, T2>::value;

template<typename T1, typename T2>
using same_t = typename st_same_type<T1, T2>::type;

namespace util
{
	class DataFile
	{
	private:
		std::vector<std::string> content_;

		std::vector<std::pair<std::string, DataFile>> vec_objs_;
		std::unordered_map<std::string, size_t> map_objs_;

	public:
		DataFile() = default;

		//SFINAE supported set function
		template<typename T, typename = st_supported_type<T>>
		void set(const T val, const size_t item = 0)
		{
			std::string str;

			//Compile time eval of type
			if constexpr (is_same_v<T, const char*> || is_same_v<T, std::string>)
			{
				str = val;
			}
			else //numeric types need to be converted to string
			{
				str = std::to_string(val);
			}

			if (item >= content_.size())
			{
				content_.resize(item + 1);
			}

			content_[item] = str;
		}

		//SFINAE supported Get function
		template<typename T, typename = st_supported_type<T>>
		T get(const size_t item = 0)
		{
			std::string str;

			if (item >= content_.size())
			{
				str =  "";
			}

			str = content_[item];

			//Compile time eval of type
			if constexpr (is_same_v<T, const char*>)
			{
				return str.c_str();
			}
			else if constexpr (is_same_v<T, std::string>)
			{
				return str;
			}
			else if constexpr (is_same_v<T, int>)
			{
				return std::stoi(str);
			}
			else if constexpr (is_same_v<T, double>)
			{
				return std::stod(str);
			}
		}

		inline size_t size() const
		{
			return content_.size();
		}

		//We can use this to exploit the array subscript operator
		inline DataFile& operator[](const std::string& name)
		{
			if (map_objs_.count(name) == 0)
			{
				//Create object for the map
				map_objs_[name] = vec_objs_.size();

				//Add blank object to vector
				vec_objs_.emplace_back(name, DataFile());
			}

			return vec_objs_[map_objs_[name]].second;
		}
	};
}

void operator_exploit()
{
	util::DataFile df;
	util::DataFile a = df["Some_Node"];

	//C++20 let's template type be inferred by parameter
	a["name"].set("Tim");
	a["age"].set(25);
	a["height"].set(1.7018);

	util::DataFile b = a["code"];

	b.set("C++", 0);
	b.set("asm", 1);
	b.set("lua", 2);

	util::DataFile c = a["pc"];

	c["processor"].set("intel");
	c["ram"].set(32);

	//Get the pc processor value
	const std::string proc = df["Some_Node"]["pc"]["processor"].get<std::string>();
}

int main()
{
	util::DataFile df;

	//Example here with type deduction in template
	df["Some_Node"]["name"].set<std::string>("Tim");
	df["Some_Node"]["age"].set<int>(25);
	df["Some_Node"]["height"].set<double>(1.88);

	df["Some_Node"]["code"].set<std::string>("C++", 0);
	df["Some_Node"]["code"].set<std::string>("vhdl", 1);
	df["Some_Node"]["code"].set<std::string>("lua", 2);

	df["Some_Node"]["pc"]["processor"].set<std::string>("intel");
	df["Some_Node"]["pc"]["ram"].set<int>(32);

	//Get the pc processor value
	std::string proc = df["Some_Node"]["pc"]["processor"].get<std::string>();
	//print proc
	std::cout << "proc: " << proc << '\n';

	return 0;
}
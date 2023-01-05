#ifndef FILE_SERIALIZER_DATAFILE_H
#define FILE_SERIALIZER_DATAFILE_H
#include <iostream>
#include <string>
#include <unordered_map>
#include <functional>
#include <vector>
#include <fstream>
#include <stack>

//SFINAE Support
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

		bool is_comment = false;

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
		T get(const size_t item = 0) const
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

		//Overload for setting objects
		template<typename T, typename = st_supported_type<T>>
		void operator()(const T val, const size_t item = 0)
		{
			this->set<T>(val, item);
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

		static bool write(const DataFile& n, const std::string& filename, const std::string& indent="\t",
						  const char list_sep=',')
		{
			//Cached Separator
			std::string separator = std::string(1, list_sep) + " ";

			size_t indent_count = 0;

			//Fully specified lambda function for recursion
			std::function<void(const DataFile&, std::ofstream&)> _write = [&](const DataFile& n, std::ofstream& file)
			{
				std::function<std::string(const std::string&, const size_t)> _indent = [&](const std::string& str, const size_t count)
			{
				std::string out;

				for (size_t n = 0; n < count; n++) //indents per tab character
				{
					out += str;
				}

				return out;
			};

				for (const std::pair<std::string, DataFile>& property: n.vec_objs_)
				{
					if (property.second.vec_objs_.empty())
					{
						file << _indent(indent, indent_count) << property.first;

						if (!property.second.is_comment) //more likely branch
						{
							file << " = ";
						}

						const size_t sz = property.second.size();
						size_t items = sz;

						for (int i = 0; i < sz; i++)
						{
							//store find first of the list separator in size_t x
							const size_t x = property.second.get<std::string>(i).find_first_of(list_sep);

							if (x != std::string::npos)
							{
								//Value contains separator to wrap in quotes
								file << "\"" << property.second.get<std::string>(i) << "\"";

								if (items > 1)
								{
									file << separator;
								}
							}
							else
							{
								//value has no separator (write it out
								file << property.second.get<std::string>(i);

								if (items > 1)
								{
									file << separator;
								}
							}

							items--;
						}

						//Property written
						file << '\n';
					}
					else
					{
						//Property has child properties
						file << "\n" << _indent(indent, indent_count) << property.first << '\n';

						//Open braces, and update indentation
						file << _indent(indent, indent_count) << "{\n";
						indent_count++;

						//Recursively write
						_write(property.second, file);

						//Close brace
						file << _indent(indent, indent_count) << "}\n\n";
					}
				}

				//finish writing the node
				if (indent_count > 0)
				{
					indent_count--;
				}
			};

			std::ofstream file(filename);
			if (file.is_open())
			{
				_write(n, file);

				file.close();
				return true;
			}

			return false;
		}

		static bool read(DataFile& n, const std::string& filename, const char list_sep=',')
		{
			//Open file
			std::ifstream file(filename);
			if (file.is_open())
			{
				std::string property_name;
				std::string property_value;

				std::stack<std::reference_wrapper<DataFile>> stk_path;
				stk_path.push(n);

				//Read file and process
				while (!file.eof())
				{
					std::string line;
					std::getline(file, line);

					std::function<void(std::string&)> _trim = [](std::string& s)
					{
						s.erase(0, s.find_first_not_of(" \t\n\r\f\v"));
						s.erase(s.find_last_not_of(" \t\n\r\f\v") + 1);
					};

					_trim(line);

					//line has content
					if (!line.empty())
					{
						if (line[0] == '#') //comment
						{
							DataFile comment;
							comment.is_comment = true;
							stk_path.top().get().vec_objs_.push_back({line, comment});
						}
						else
						{
							//section with = has properties
							size_t x = line.find_first_of('=');

							if (x != std::string::npos)
							{
								property_name = line.substr(0, x);
								_trim(property_name);

								property_value = line.substr(x + 1, line.size());
								_trim(property_value);

								bool in_quotes = false;
								std::string token;
								size_t token_count = 0;

								for (int i = 0; i < property_value.size(); i++)
								{
									if (property_value[i] == '\"')
									{
										in_quotes = !in_quotes; //toggle quote state
									}
									else
									{
										if (in_quotes)
										{
											token.append(1, property_value[i]);
										}
										else
										{
											if (property_value[i] == list_sep)
											{
												_trim(token);

												//add to vector of values for this property
												stk_path.top().get()[property_name].set<std::string>(token,token_count);

												//reset token
												token.clear();
												token_count++;
											}
											else
											{
												token.append(1, property_value[i]);
											}
										}
									}
								}

								//Any remaining character at this point is the final token
								if (!token.empty())
								{
									_trim(token);
									stk_path.top().get()[property_name].set<std::string>(token, token_count);
								}

							}
							else //no =
							{
								if (line[0] == '{')
								{
									//subsequent properties belong to new node
									stk_path.push(stk_path.top().get()[property_name]);
								}
								else if (line[0] == '}')
								{
									//close brace so pop from stack
									stk_path.pop();
								}
								else
								{
									property_name = line;
								}
							}
						}
					}
				}

				file.close();
				return true;
			}

			return false;
		}
	};
}



#endif //FILE_SERIALIZER_DATAFILE_H

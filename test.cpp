#include "Serializer/DataFile.h"

void minimalist_operator()
{
	util::DataFile df;
	util::DataFile a = df["Some_Node"];

	//C++20 let's template type be inferred by parameter
	a["name"].set("Tim");
	a["age"].set(25);
	a["height"].set(1.7018);

	util::DataFile b = a["code"];

	b.set("C++", 0);
	b.set("CUDA", 1);
	b.set("C", 2);

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
	df["Some_Node"]["code"].set<std::string>("CUDA", 1);
	df["Some_Node"]["code"].set<std::string>("C", 2);

	df["Some_Node"]["pc"]["processor"].set<std::string>("intel");
	df["Some_Node"]["pc"]["ram"].set<int>(32);
	df["Some_Node"]["pc"]["GPU"]("Nvidia", 0); //Example using () overload

	//Get the pc processor value
	std::string proc = df["Some_Node"]["pc"]["processor"].get<std::string>();
	std::cout << "proc: " << proc << '\n';

	util::DataFile::write(df, "TestOut.txt");

	util::DataFile test;

	if (util::DataFile::read(test, "TestOut.txt"))
	{
		const size_t idx = test["Some_Node"]["code"].size();
		test["Some_Node"]["code"].set<std::string>("x86", idx);

		util::DataFile::write(test, "TestOutput2.txt");
	}

	return 0;
}
# File Serialization
For Reading/Writing Serialized Files

From the Javidx9 video: https://www.youtube.com/watch?v=jlS1Y2-yKV0 (with some QOL Modifications)

My approach does things a little different with implementation mainly using SFINAE to make things more cut down than before.

Using SFINAE for set and get:

```cpp
//Set
df["Some_Node"]["name"].set<std::string>("Tim");
df["Some_Node"]["name"].set("Tim"); //Automatic Template Type Deduction
df["Some_Node"]["name"]("Tim"); //Using () overload
```

```cpp
//Get
std::string proc = df["Some_Node"]["pc"]["processor"].get<std::string>(); //Note get method cannot perform deduction here
```

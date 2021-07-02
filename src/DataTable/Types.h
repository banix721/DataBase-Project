#pragma once

#include <string>
#include <iostream> 
#include <map>
#include <typeinfo>
#include <stdexcept>
#include <vector>
#include <boost/variant.hpp>
#include <boost/type_index.hpp>
#include <boost/mpl/for_each.hpp>

using namespace std;
using namespace boost;
using namespace boost::typeindex;

typedef variant<double, float, char, string, int, uint32_t> CellTypeValue;

enum RowState { None, Changed, Inserted };

class Types
{
public:
    static void InitValue(const std::type_info& type, CellTypeValue& value)
    {
        if (type == typeid(int))
            value = int(0);
        else if (type == typeid(string))
            value = string("");
        else if (type == typeid(float))
            value = float(0.0f);
        else if (type == typeid(double))
            value = double(0.0);
        else if (type == typeid(uint32_t))
            value = uint32_t(0);
    }

    static string ToSqliteType(const type_index type)
    {
        if (type == typeid(int))
            return "INT";
        else if (type == typeid(string))
            return "TEXT";
        else if (type == typeid(char))
            return "VARCHAR(1)";
        else if (type == typeid(float))
            return "FLOAT";
        else if (type == typeid(double))
            return "DOUBLE";
        else if (type == typeid(uint32_t))
            return "INT8";
        else
            return "";
    }

    static bool SqliteTypeToTypeIndex(const string type, type_index& typeIndex)
    {
        const char* typeC = type.c_str();
        if(strcmp(typeC, "INT") == 0)
            typeIndex = typeid(int);
        else if (strcmp(typeC, "TEXT") == 0)
            typeIndex = typeid(string);
        else if (strcmp(typeC, "VARCHAR(1)") == 0)
            typeIndex = typeid(char);
        else if (strcmp(typeC, "FLOAT") == 0)
            typeIndex = typeid(float);
        else if (strcmp(typeC, "DOUBLE") == 0)
            typeIndex = typeid(double);
        else if (strcmp(typeC, "INT8") == 0)
            typeIndex = typeid(uint32_t);
        else
            return false;

        return true;
    }
};

class TypeChecker
{
public:
    TypeChecker(vector<type_index>& result) : Result(result)
    { }

    template< typename U > void operator()(U valueVariant)
    {
        Result.push_back(typeid(valueVariant));
    }

    static bool WrongType(const type_index type)
    {
        //Get all types defined in variant
        vector<type_index> types;
        boost::mpl::for_each<CellTypeValue::types>(TypeChecker(types));
        return find(types.begin(), types.end(), type) == types.end();
    }

private:
    vector<type_index>& Result;
};
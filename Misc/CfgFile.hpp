#pragma once

#include <fstream>
#include <map>
#include <regex>
#include <functional>
#include <set>

class CfgFileException
{
public:
	enum : uint8_t
	{
		Error_Unknown = 0,
		Error_FailedToOpenFile,
		Error_FailedToParseFile,
	};
	CfgFileException(uint8_t err, std::string message = "")
	{
		this->err = err;
		this->message = message;
	}

	const std::string& Message() const
	{
		return message;
	}

	const uint8_t& Error() const
	{
		return err;
	}
private:
	uint8_t err;
	std::string message;
};

class CfgFile
{
private:

	class CfgValue
	{
		const std::string& value;
	protected:
		friend CfgFile;
		CfgValue(const std::string& string) : value(string) {}
	public:

		const bool ToBool()
		{
			return value=="1" || value=="True" || value=="true";
		}

		const bool IsBool() const
		{
			return CfgFile::BoolValidator(value);
		}

		const int ToInt() const
		{
			return std::atoi(value.c_str());
		}

		const bool IsInt() const
		{
			return CfgFile::IntValidator(value);
		}

		const bool IsResolution() const
		{
			return CfgFile::ResolutionValidator(value);
		}

		const std::string& ToString()
		{
			return value;
		}

		const bool IsString()
		{
			return true;
		}

		const float ToFloat()
		{
			return std::atof(value.c_str());
		}
		const float IsFloat()
		{
			return FloatValidator(value);
		}

		operator const std::string&()
		{
			return value;
		}

	};
	std::map<std::string, std::map<std::string, std::string>> data;
public:
	const static std::function<bool(const std::string&)> BoolValidator;
	const static std::function<bool(const std::string&)> IntValidator;
	const static std::function<bool(const std::string&)> FloatValidator;
	const static std::function<bool(const std::string&)> AnyValidator;
	const static std::function<bool(const std::string&)> ResolutionValidator;
	const static std::function<bool(const std::string&)> HexValidator;
	const static std::function<std::function<bool(const std::string&)>(const std::string)> RegexValidator;
	const static std::function<std::function<bool(const std::string&)>(const std::set<int32_t>)> IntListValidator;
	const static std::function<std::function<bool(const std::string&)>(const int32_t, const int32_t)> IntRangeValidator;
	const static std::function<std::function<bool(const std::string&)>(const float, const float)> FloatRangeValidator;
	
	struct CfgValidator
	{
		std::function<bool(std::string)> validator;
		std::string default;
	};

	CfgFile()
	{
	}

	CfgValue GetValue(std::string group, std::string variable)
	{
		return CfgValue(this->data.at(group).at(variable));
	}

	const void LoadFromFile(const std::string filename, std::map<std::string,std::map<std::string,CfgValidator>> validator)
	{
		std::ifstream fs(filename);
		if (!fs.good())
		{
			throw CfgFileException(CfgFileException::Error_FailedToOpenFile,"Failed to open file");
		}

		const std::regex group("[ \\t]*\\[[ \\t]*([A-Za-z0-9_]*)[ \\t]*\\][ \\t]*");
		const std::regex value("[\\t| ]*([A-Za-z0-9_]*)[\\t| ]*=[\\t| ]*(?:([\"']?)((?:\\\\?.)*?)\\2)");

		std::string line;
		std::string errors = "";
		std::smatch matches;
		std::string currentGroup = "";

		std::map <std::string, std::map<std::string, bool>> found;

		for (auto& x : validator)
		{
			for (auto& y : x.second)
			{
				found[x.first][y.first]=false;
			}
		}

		while (std::getline(fs, line))
		{
			//Skip lines that contains only whitespace or comment
			if (std::all_of(line.begin(), line.end(), isspace) || *std::find_if_not(line.begin(), line.end(), isspace) == '#') continue;

			//Match variables
			if (!currentGroup.empty() && std::regex_match(line, matches, value) && matches.ready() && currentGroup != "")
			{
				if (validator.at(currentGroup).find(matches[1]) == validator.at(currentGroup).end())
				{
					errors+="Unknown value: "+ currentGroup + ":" + static_cast<std::string>(matches[1]) + "\n";
					continue;
				}

				if (!validator.at(currentGroup).at(matches[1]).validator(matches[3]))
				{
					errors += "Value failed validation: " + static_cast<std::string>(matches[3]) + "\n";
					continue;
				}
				found[currentGroup][matches[1]] = true;
				this->data[currentGroup][matches[1]]=matches[3];
			} //Match groups
			else if (std::regex_match(line, matches, group) && matches.ready())
			{
				if (validator.find(matches[1]) == validator.end())
				{
					errors += "Unknown group: " + static_cast<std::string>(matches[1]) + "\n";
					continue;
				}
				currentGroup = matches[1];
			}
		}

		//Fill in the empty variables
		for (auto it = validator.begin(); it != validator.end(); it++)
		{
			for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++)
			{
				if (data.find(it->first) == data.end() || data.at(it->first).find(it2->first) == data.at(it->first).end())
				{
					this->data[it->first][it2->first] = it2->second.default;
				}
			}
		}

		for (auto& x : found)
		{
			for (auto& y : x.second)
			{
				if (!y.second)
				{
					errors += "Value not found for variable: " + x.first + " : " + y.first + "\n";
				}
			}
		}

		if (errors != "")
		{
			throw CfgFileException(CfgFileException::Error_FailedToParseFile, errors);
		}
	}


	const void SaveToFile(const std::string filename)
	{
		std::ofstream fs(filename,std::ios::trunc);
		if (!fs.good())
		{
			throw CfgFileException(CfgFileException::Error_FailedToOpenFile, "Failed to open file");
		}

		for (auto it = data.begin() ;it != data.end() ;it++)
		{
			fs << "[" << it->first << "]" << std::endl ;
			for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++)
			{
				fs << it2->first << " = " << it2->second << std::endl;
			}
		}
	}
	const void LoadFromMemory(std::map<std::string, std::map<std::string, std::string>> data)
	{
		this->data = data;
	}
};

const std::function<bool(const std::string&)> CfgFile::BoolValidator = [](const std::string& value)
{
	return value == "true" || value == "True" || value == "1" || value == "0" || value == "false" || value == "False";
};

const std::function<bool(const std::string&)> CfgFile::IntValidator = [](const std::string& value)
{
	static const std::regex rx("-?[1-9][0-9]*");
	return std::regex_match(value, rx);
};

const std::function<bool(const std::string&)> CfgFile::FloatValidator = [](const std::string& value)
{
	static const std::regex rx("-?[1-9][0-9]*(.[0-9]*)?");
	return std::regex_match(value, rx);
};


const std::function<bool(const std::string&)> CfgFile::HexValidator = [](const std::string& value)
{
	static const std::regex rx("[A-Fa-f0-9]+");
	return std::regex_match(value, rx);
};
const std::function<std::function<bool(const std::string&)>(const std::string)> CfgFile::RegexValidator = [](const std::string regex)
{
	return [regex](const std::string& value)
	{
		return std::regex_match(value, std::regex(regex));
	};;
};
const std::function<std::function<bool(const std::string&)>(const std::set<int32_t>)> CfgFile::IntListValidator = [](const std::set<int32_t> data)
{
	return [data](const std::string& value) -> bool
	{
		if (!CfgFile::IntValidator(value)) return false;
		const int32_t i = std::atoi(value.c_str());
		return data.find(i) != data.end();
	};
};

const std::function<std::function<bool(const std::string&)>(const int32_t,const int32_t)> CfgFile::IntRangeValidator = [](const int32_t from,const int32_t to)
{
	return [from,to](const std::string& value) -> bool
	{
		if (!CfgFile::IntValidator(value)) return false;
		const int32_t i = std::atoi(value.c_str());
		return i >= from && i <= to;
	};;
};

const std::function<std::function<bool(const std::string&)>(const float, const float)> CfgFile::FloatRangeValidator = [](const float from, const float to)
{
	return [from, to](const std::string& value) -> bool
	{
		if (!CfgFile::FloatValidator(value)) return false;
		const float i = std::atof(value.c_str());
		return i >= from && i <= to;
	};;
};

const std::function<bool(const std::string&)> CfgFile::ResolutionValidator = [](const std::string& value)
{
	if (!RegexValidator("[1-9][0-9]*x[1-9][0-9]*")(value)) return false;
	const size_t pos = value.find_first_of('x');
	const int width = std::atoi(value.substr(0, pos).c_str());
	const int height = std::atoi(value.substr(pos+1,value.size() - pos -1).c_str());

	if (width == 800 && height == 600) return true;
	if (width == 1024 && height == 768) return true;
	if (width == 1280 && height == 720) return true;
	if (width == 1440 && height == 900) return true;
	if (width == 1920 && height == 1080) return true;

	return false;
};

const std::function<bool(const std::string&)> CfgFile::AnyValidator = [](const std::string& value)
{
	return true;
};
 
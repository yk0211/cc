#include <iostream>
#include <string>
#include <sstream>
#include <locale>
#include <codecvt>
#include <stdio.h>
#include <opencc/opencc.h>
#include <ghc/filesystem.hpp>

namespace fs = ghc::filesystem;

// 检查输入是否合法
bool CheckInputValid(fs::path &input_dir, fs::path &output_dir)
{
	fs::directory_entry input_entry(input_dir);
	fs::directory_entry output_entry(output_dir);

	if (!input_entry.is_directory())
	{
		std::cerr << "Input is not directory." << std::endl;
		return false;
	}

	if (!output_entry.is_directory())
	{
		std::cerr << "Output is not directory." << std::endl;
		return false;
	}

	if (!input_entry.exists())
	{
		std::cerr << "Input directory not exists." << std::endl;
		return false;
	}

	if (!output_entry.exists())
	{
		std::cerr << "Output directory not exists." << std::endl;
		return false;
	}

	return true;
}

// 简体转繁体
void ConvertSimple2TW(const std::wstring &in, std::wstring &out)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> convert_utf8;
	std::string in_utf8 = convert_utf8.to_bytes(in);
	const opencc::SimpleConverter converter("s2t.json");
	std::string out_utf8 = converter.Convert(in_utf8);
	out = convert_utf8.from_bytes(out_utf8);
}

// 转换输出路径
fs::path ConvertOutPath(fs::path &input_dir, fs::path &output_dir, fs::path &input_path)
{
	fs::path out_path{ "." };
	std::string input_str = input_path.u8string();
	std::string input_dir_str = input_dir.u8string();
	std::size_t len = input_dir_str.length();
	std::size_t pos = input_str.find(input_dir_str);
	if (pos != std::string::npos)
	{
		std::string input_left_str = input_str.substr(pos + len);
		std::string output_dir_str = output_dir.u8string();
		std::string out_str = output_dir_str + input_left_str;
		out_path = out_str;
	}

	return out_path;
}

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		std::cerr << "Usage like:cc input_dir output_dir." << std::endl;
		return -1;
	}

	// locale("")：调用构造函数创建一个local，其中的空字符串具有特殊含义：使用客户环境中缺省的locale
	std::locale::global(std::locale(""));
	//std::wcout.imbue(std::locale(""));

	fs::path input_dir = fs::u8path(argv[1]);
	fs::path output_dir = fs::u8path(argv[2]);

	if (!CheckInputValid(input_dir, output_dir))
	{
		return -1;
	}

	try
	{
		auto rdi = fs::recursive_directory_iterator(input_dir);
		for (auto de : rdi)
		{
			if (de.is_regular_file())
			{				
				fs::wifstream wifs;
				wifs.open(de.path());

				std::wstringstream wss;
				wss << wifs.rdbuf();
				//std::wcout << wss.str().c_str() << std::endl;

				std::wstring out;
				ConvertSimple2TW(wss.str(), out);
				//std::wcout << out.c_str() << std::endl;
				
				fs::path input_path = de.path();
				fs::path output_path = ConvertOutPath(input_dir, output_dir, input_path);
				
				fs::wofstream wofs;
				wofs.open(output_path);
				wofs << out;
				wofs.flush();
			}
			else if (de.is_directory())
			{
				fs::path input_path = de.path();
				fs::path output_path = ConvertOutPath(input_dir, output_dir, input_path);
				fs::create_directories(output_path);
			}
		}
	}
	catch (fs::filesystem_error fe)
	{
		std::cerr << "Error: " << fe.what() << std::endl;
	}   
    
    return 0;
}
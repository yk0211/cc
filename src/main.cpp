#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <opencc/opencc.h>
#include <ghc/filesystem.hpp>
#include <uchardet/uchardet.h>
#include <iconv/iconv.h>
#include <yaml-cpp/yaml.h>

namespace fs = ghc::filesystem;

bool CheckInputValid(fs::path &input_dir, fs::path &output_dir)
{
	fs::directory_entry input_entry(input_dir);
	fs::directory_entry output_entry(output_dir);

	if (!input_entry.is_directory())
	{
		std::cerr << "input is not directory." << std::endl;
		return false;
	}

	if (!output_entry.is_directory())
	{
		std::cerr << "output is not directory." << std::endl;
		return false;
	}

	if (!input_entry.exists())
	{
		std::cerr << "input is not exists." << std::endl;
		return false;
	}

	if (!output_entry.exists())
	{
		fs::create_directories(output_dir);
	}
	
	return true;
}

int ConvertCode(std::string in_charset, std::string out_charset, std::string &in, std::string &out)
{
	std::size_t in_length = in.length();
	char *in_buffer = (char *)in.data();

	std::size_t out_length = in_length * 2;
	std::size_t out_left_len = out_length;
	char *out_buffer = (char*)malloc(out_length);
	if (out_buffer == nullptr)
	{
		std::cerr << "malloc error" << std::endl;
		return -1;
	}

	char *out_left = out_buffer;

	iconv_t iconv_handle = iconv_open(out_charset.c_str(), in_charset.c_str());
	if (iconv_handle == (iconv_t)(-1))
	{
		std::cerr << "iconv_open error" << std::endl;
		free(out_buffer);
		iconv_close(iconv_handle);
		return -1;
	}

	std::size_t ret = iconv(iconv_handle, &in_buffer, &in_length, &out_left, &out_left_len);
	while (ret == -1 && errno == E2BIG)
	{		
		std::size_t increase = 12;
		out_length += increase;
		out_left_len += increase;

		char *new_out_buffer = (char*)realloc(out_buffer, out_length);
		if (new_out_buffer == nullptr)
		{
			std::cerr << "realloc error" << std::endl;
			free(out_buffer);
			iconv_close(iconv_handle);
			return -1;
		}
		
		std::size_t len = out_left - out_buffer;
		out_buffer = new_out_buffer;
		out_left = out_buffer + len;
		ret = iconv(iconv_handle, &in_buffer, &in_length, &out_left, &out_left_len);
	}

	if (ret == -1)
	{
		std::cerr << "iconv error" << std::endl;
		free(out_buffer);
		iconv_close(iconv_handle);
		return -1;
	}

	out = out_buffer;
	free(out_buffer);
	iconv_close(iconv_handle);
	return 0;
}

void Convert2Utf8(std::string &in, std::string &out)
{
	uchardet_t uchardet_handle = uchardet_new();
	int ret = uchardet_handle_data(uchardet_handle, in.data(), in.length());
	if (ret != 0)
	{
		return;
	}

	uchardet_data_end(uchardet_handle);
	std::string charset = uchardet_get_charset(uchardet_handle);
	uchardet_delete(uchardet_handle);

	std::transform(charset.begin(), charset.end(), charset.begin(), ::toupper);

	if (charset.compare("UTF-8") != 0)
	{		
		ConvertCode(charset, "UTF-8", in, out);
	}
	else
	{
		out = in;
	}
}

void ConvertSimple2Traditional(std::string in, std::string &out)
{
    std::string in_utf8;
    Convert2Utf8(in, in_utf8);
	const opencc::SimpleConverter converter("s2t.json");
	out = converter.Convert(in_utf8);
}

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
		std::string out_str = output_dir_str + std::string(1, fs::path::preferred_separator) + input_left_str;
		out_path = out_str;
	}

	return out_path;
}

int main(int argc, char* argv[])
{
	try
	{
		YAML::Node config = YAML::LoadFile("config.yaml");
		std::string input = config["cc"]["input_directory"].as<std::string>();
		std::string output = config["cc"]["output_directory"].as<std::string>();
		std::vector<std::string> exclude = config["cc"]["exclude_extension"].as<std::vector<std::string>>();

		fs::path input_dir = fs::u8path(input);
		fs::path output_dir = fs::u8path(output);

		if (!CheckInputValid(input_dir, output_dir))
		{
			return -1;
		}

		auto rdi = fs::recursive_directory_iterator(input_dir);
		for (auto de : rdi)
		{
			fs::path input_path = de.path();
			fs::path output_path = ConvertOutPath(input_dir, output_dir, input_path);

			if (de.is_regular_file())
			{			
				std::string extension = input_path.extension().u8string();
				std::vector<std::string>::iterator it = std::find_if(exclude.begin(), exclude.end(),
					[&extension](const std::string &s) {
					return extension == s;
				});

				if (it == exclude.end())
				{
					fs::ifstream ifs;
					ifs.open(de.path());

					std::stringstream ss;
					ss << ifs.rdbuf();
					ifs.close();

					std::string out;
					ConvertSimple2Traditional(ss.str(), out);

					fs::ofstream ofs;
					ofs.open(output_path);
					ofs << out;
					ofs.flush();
					ofs.close();
				}	
				else
				{
					fs::copy(input_path, output_path);
				}

				fs::path output_path_temp = output_path;
				std::string convert_output_filename;
				ConvertSimple2Traditional(output_path_temp.filename().u8string(), convert_output_filename);
				
				fs::path out_path_filename = convert_output_filename;
				output_path_temp.replace_filename(out_path_filename);
				fs::rename(output_path, output_path_temp);				
			}
			else if (de.is_directory())
			{
				fs::create_directories(output_path);
			}
		}
	}
	catch (fs::filesystem_error fe)
	{
		std::cerr << "Error: " << fe.what() << std::endl;
	}
	catch (std::exception e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}

	return 0;
}
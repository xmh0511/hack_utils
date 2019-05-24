#pragma once
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
static const std::string spliter_str = "------2b63040624f44bf0bf2e662b1a970d6b------";
static const std::string crlf = "\r\n";
class res_reader {
public:
	struct data_content
	{
		std::string name;
		std::string data;
	};
	static void add_data_to_exe(std::string const& filename, std::string const& data, std::string const& tips)
	{
		std::fstream file(filename, std::ios::in | std::ios::out | std::ios::binary);
		if (file.is_open()) {
			file.seekp(0, std::ios::end);
			file.write(spliter_str.data(), spliter_str.size());
			file.write(crlf.data(), crlf.size());
			file.write(tips.data(), tips.size());
			file.write(crlf.data(), crlf.size());
			file.write(data.data(), data.size());
			file.write(spliter_str.data(), spliter_str.size());
		}
		file.close();
	}
	template<typename Function>
	static void read_until(std::ifstream& file, std::string const& key, std::streampos max, Function&& function)
	{
		auto k = key[0];
		while (!file.eof()) {
			char c = file.get();
			if (c == k)
			{
				auto old_pos = file.tellg();
				auto befor_pos = old_pos;
				befor_pos -= 1;
				auto new_pos = old_pos;
				auto split_left_size = key.size();
				new_pos += split_left_size - 1;
				if (new_pos <= max)
				{
					std::string maybe;
					file.seekg(befor_pos, std::ios::beg);
					while (split_left_size--) {
						maybe.push_back(file.get());
					}
					if (maybe == key)
					{
						function(file);
						break;
					}
					else {
						//old_pos += 1;
						file.seekg(old_pos, std::ios::beg);
						continue;
					}
				}
			}
		}
	}
	static std::vector<data_content> read_data_from_exe(std::string const& filename)
	{
		std::ifstream file(filename, std::ios::binary);
		std::vector<data_content> data_vec;
		if (file.is_open()) {
			file.seekg(0, std::ios::end);
			auto file_max_pos = file.tellg();
			file.seekg(0, std::ios::beg);
			while (!file.eof()) {
				data_content item;
				int b = 0;
				read_until(file, spliter_str, file_max_pos, [&b](std::ifstream& file) {
					char c = file.get(); //\r
					char c1 = file.get();  //\n
					if (c == '\r' && c1 == '\n') {
						b++;
					}
				});
				if (b == 0) {
					continue;
				}
				auto pos = file.tellg();
				read_until(file, "\r\n", file_max_pos, [pos, &item, &b](std::ifstream& file) {
					auto post = file.tellg();
					post -= 2;
					file.seekg(pos);
					auto mpos = pos;
					std::string name;
					while (mpos < post) {
						name.push_back(file.get());
						mpos += 1;
					}
					item.name = std::move(name);
					post += 2;
					file.seekg(post);
					b++;
				});
				auto data_pos = file.tellg();
				read_until(file, spliter_str, file_max_pos, [data_pos, &item, &b](std::ifstream& file) {
					auto post = file.tellg();
					post -= spliter_str.size();
					file.seekg(data_pos);
					auto mdata_pos = data_pos;
					std::string data;
					while (mdata_pos < post) {
						data.push_back(file.get());
						mdata_pos += 1;
					}
					item.data = std::move(data);
					post += spliter_str.size();
					file.seekg(post);
					b++;
				});
				if (b == 3) {
					data_vec.emplace_back(std::move(item));
				}
			}
		}
		file.close();
		return data_vec;
	}
};
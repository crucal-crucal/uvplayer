#pragma once

#include <map>
#include <sstream>
#include <string>
#include <vector>

using stringList = std::vector<std::string>;
using keyValueMap = std::map<std::string, std::string>;

#define SPACE_CHARS		" \t\r\n"
#define PAIR_CHARS      "{}[]()<>\"\"\'\'``"

namespace uvstring {
	/**
	 * @note This function is used to convert a value to a string.
	 *       Low-version NDK does not provide std::to_string.
	 * @tparam T The type of the value.
	 * @param value The value to convert.
	 * @return The string representation of the value.
	 *
	 * @note 此函数用于将值转换为字符串。
	 * 低版本NDK不提供std::to_string。
	 * @tparam T 值的类型。
	 * @param value 要转换的值。
	 * @return 值的字符串表示。
	 */
	template<typename T>
	static inline std::string to_string(const T& value) {
		std::ostringstream oss;
		oss << value;
		return oss.str();
	}

	/**
	 * @note This function is used to convert a string to a value.
	 * @tparam T The type of the value.
	 * @param str The string to convert.
	 * @return The value representation of the string.
	 *
	 * @note 此函数用于将字符串转换为值。
	 * @tparam T 值的类型。
	 * @param str 要转换的字符串。
	 * @return 字符串的值表示。
	 */
	template<typename T>
	static inline T from_string(const std::string& str) {
		T t;
		std::istringstream iss(str);
		iss >> t;
		return t;
	}

	/**
	 * @note Formats a string using printf-like formatting.
	 * @param format The format string.
	 * @param ... The values to format.
	 * @return The formatted string.
	 *
	 * @note 使用 printf 样式格式化字符串。
	 * @param format 格式字符串。
	 * @param ... 要格式化的值。
	 * @return 格式化后的字符串。
	 */
	std::string asprintf(const char* format, ...);
	/**
	 * @note Splits a string by a delimiter.
	 * @param str The string to split.
	 * @param delim The delimiter character.
	 * @return A list of strings resulting from the split.
	 *
	 * @note 通过分隔符拆分字符串。
	 * @param str 要拆分的字符串。
	 * @param delim 分隔符字符。
	 * @return 拆分后得到的字符串列表。
	 */
	stringList split(const std::string& str, char delim = ',');

	/**
	 * @note Splits a string into key-value pairs.
	 * @param str The string to split.
	 * @param kv_kv The delimiter between key-value pairs.
	 * @param k_v The delimiter between keys and values.
	 * @return A map of key-value pairs.
	 *
	 * @note 将字符串拆分为键值对。
	 * @param str 要拆分的字符串。
	 * @param kv_kv 键值对之间的分隔符。
	 * @param k_v 键和值之间的分隔符。
	 * @return 键值对映射。
	 */
	keyValueMap splitKV(const std::string& str, char kv_kv = '&', char k_v = '=');

	/**
	 * @note Trims characters from both ends of a string.
	 * @param str The string to trim.
	 * @param trim_chars The characters to trim.
	 * @return The trimmed string.
	 *
	 * @note 从字符串的两端删除指定字符。
	 * @param str 要修剪的字符串。
	 * @param trim_chars 要删除的字符。
	 * @return 修剪后的字符串。
	 */
	std::string trim(const std::string& str, const std::string& trim_chars = SPACE_CHARS);

	/**
	 * @note Trims characters from the left end of a string.
	 * @param str The string to trim.
	 * @param chars The characters to trim.
	 * @return The trimmed string.
	 *
	 * @note 从字符串的左端删除指定字符。
	 * @param str 要修剪的字符串。
	 * @param chars 要删除的字符。
	 * @return 修剪后的字符串。
	 */
	std::string trimL(const std::string& str, const char* chars = SPACE_CHARS);

	/**
	 * @note Trims characters from the right end of a string.
	 * @param str The string to trim.
	 * @param chars The characters to trim.
	 * @return The trimmed string.
	 *
	 * @note 从字符串的右端删除指定字符。
	 * @param str 要修剪的字符串。
	 * @param chars 要删除的字符。
	 * @return 修剪后的字符串。
	 */
	std::string trimR(const std::string& str, const char* chars = SPACE_CHARS);

	/**
	 * @note Trims paired characters from both ends of a string.
	 * @param str The string to trim.
	 * @param pairs The paired characters to trim.
	 * @return The trimmed string.
	 *
	 * @note 从字符串的两端删除成对的字符。
	 * @param str 要修剪的字符串。
	 * @param pairs 要删除的成对字符。
	 * @return 修剪后的字符串。
	 */
	std::string trim_pairs(const std::string& str, const char* pairs = PAIR_CHARS);

	/**
	 * @note Replaces all occurrences of a substring with another substring.
	 * @param str The original string.
	 * @param from The substring to replace.
	 * @param to The substring to replace with.
	 * @return The resulting string.
	 *
	 * @note 将所有出现的子字符串替换为另一个子字符串。
	 * @param str 原始字符串。
	 * @param from 要替换的子字符串。
	 * @param to 替换后的子字符串。
	 * @return 生成的字符串。
	 */
	std::string replace(const std::string& str, const std::string& from, const std::string& to);

	/**
	 * @note Gets the base name of a file path.
	 * @param path The file path.
	 * @return The base name of the file.
	 *
	 * @note 获取文件路径的基本名称。
	 * @param path 文件路径。
	 * @return 文件的基本名称。
	 */
	std::string basename(const std::string& path);

	/**
	 * @note Gets the directory name of a file path.
	 * @param path The file path.
	 * @return The directory name.
	 *
	 * @note 获取文件路径的目录名称。
	 * @param path 文件路径。
	 * @return 目录名称。
	 */
	std::string dirname(const std::string& path);

	/**
	 * @note Gets the file name without the directory path.
	 * @param path The file path.
	 * @return The file name.
	 *
	 * @note 获取不带目录路径的文件名称。
	 * @param path 文件路径。
	 * @return 文件名称。
	 */
	std::string filename(const std::string& path);

	/**
	 * @note Gets the suffix (extension) of a file path.
	 * @param path The file path.
	 * @return The suffix (extension) of the file.
	 *
	 * @note 获取文件路径的后缀（扩展名）。
	 * @param path 文件路径。
	 * @return 文件的后缀（扩展名）。
	 */
	std::string suffixname(const std::string& path);
}

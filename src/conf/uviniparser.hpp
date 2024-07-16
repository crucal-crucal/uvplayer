#pragma once

#include <list>
#include <string>

#include "global/uvbase.hpp"

#define DEFAULT_INI_COMMENT "#"
#define DEFAULT_INI_DELIM   "="

#define LOAD_FAILED (-1)
#define LOAD_SUCCESS 0

#define UNLOAD_FAILED (-1)
#define UNLOAD_SUCCESS 0


class CUVIniNode {
public:
	enum Type {
		INI_NODE_TYPE_UNKNOWN,
		INI_NODE_TYPE_ROOT,
		INI_NODE_TYPE_SECTION,
		INI_NODE_TYPE_KEY_VALUE,
		INI_NODE_TYPE_DIV,
		INI_NODE_TYPE_SPAN,
	} type = INI_NODE_TYPE_UNKNOWN;

	std::string label; // section|key|comment
	std::string value;
	std::list<CUVIniNode*> children;

	explicit CUVIniNode();
	virtual ~CUVIniNode();

	void add(CUVIniNode* pNode);

	void del(const CUVIniNode* pNode);

	[[nodiscard]] CUVIniNode* get(const std::string& label, Type type = INI_NODE_TYPE_KEY_VALUE) const;
};

class CUVIniSection final : public CUVIniNode {
public:
	CUVIniSection() : CUVIniNode(), section(label) {
		type = INI_NODE_TYPE_SECTION;
	}

	std::string& section;
};

class CUVIniKeyValue final : public CUVIniNode {
public:
	CUVIniKeyValue() : CUVIniNode(), key(label) {
		type = INI_NODE_TYPE_KEY_VALUE;
	}

	std::string& key;
};

class CUVIniComment final : public CUVIniNode {
public:
	CUVIniComment() : CUVIniNode(), comment(label) {
	}

	std::string& comment;
};


class CUVIniParser {
public:
	explicit CUVIniParser();
	~CUVIniParser();

	int loadFromFile(const char* filepath);
	int loadFromMem(const char* data);
	int unload();
	int reload();

	std::string dumpString();
	int save();
	int saveAs(const char* filepath);

	[[nodiscard]] std::string getValue(const std::string& key, const std::string& section = "") const;
	void setValue(const std::string& key, const std::string& value, const std::string& section = "");

	template<typename T>
	T get(const std::string& key, const std::string& section = "", const T& defvalue = 0);

	template<typename T>
	void set(const std::string& key, const T& value, const std::string& section = "");

protected:
	void dumpString(CUVIniNode* node, std::string& str);

public:
	std::string _comment{};
	std::string _delim{};
	std::string _filepath{};

private:
	CUVIniNode* _root{};
};


template<>
inline float CUVIniParser::get(const std::string& key, const std::string& section, const float& defvalue) {
	const std::string str = getValue(key, section);
	return str.empty() ? defvalue : std::stof(str);
}

template<>
inline int CUVIniParser::get(const std::string& key, const std::string& section, const int& defvalue) {
	const std::string str = getValue(key, section);
	return str.empty() ? defvalue : std::stoi(str);
}

template<>
inline bool CUVIniParser::get(const std::string& key, const std::string& section, const bool& defvalue) {
	const std::string str = getValue(key, section);
	return str.empty() ? defvalue : getboolean(str.c_str());
}

template<>
inline void CUVIniParser::set(const std::string& key, const float& value, const std::string& section) {
	setValue(key, std::to_string(value), section);
}

template<>
inline void CUVIniParser::set(const std::string& key, const int& value, const std::string& section) {
	setValue(key, std::to_string(value), section);
}

template<>
inline void CUVIniParser::set(const std::string& key, const bool& value, const std::string& section) {
	setValue(key, std::to_string(value), section);
}

#include "uviniparser.hpp"

#include <sstream>

#include "uvfile.hpp"
#include "def/uvdef.hpp"
#include "def/uverr.hpp"
#include "uvstring/uvstring.hpp"

/**
 * class CUVIniNode
 */
CUVIniNode::CUVIniNode() = default;

CUVIniNode::~CUVIniNode() {
	for (const auto pNode: children) {
		delete pNode;
	}
	children.clear();
}

void CUVIniNode::add(CUVIniNode* pNode) {
	children.push_back(pNode);
}

void CUVIniNode::del(const CUVIniNode* pNode) {
	for (auto iter = children.begin(); iter != children.end(); ++iter) {
		if ((*iter) == pNode) {
			delete (*iter);
			children.erase(iter);
			return;
		}
	}
}

CUVIniNode* CUVIniNode::get(const std::string& label, const Type type) const {
	for (const auto pNode: children) {
		if (pNode->type == type && pNode->label == label) {
			return pNode;
		}
	}
	return nullptr;
}

/**
 * class CUVIniParser
 */
CUVIniParser::CUVIniParser() {
	_comment = DEFAULT_INI_COMMENT;
	_delim = DEFAULT_INI_DELIM;
	_root = nullptr;
}

CUVIniParser::~CUVIniParser() {
	unload();
}

int CUVIniParser::loadFromFile(const char* filepath) {
	_filepath = filepath;

	CUVFile file{};
	if (file.open(filepath, "r")) {
		return ERR_OPEN_FILE;
	}

	std::string str;
	file.readAll(str);
	return loadFromMem(str.c_str());
}

int CUVIniParser::loadFromMem(const char* data) {
	unload();

	_root = new CUVIniNode;
	_root->type = CUVIniNode::INI_NODE_TYPE_ROOT;

	std::stringstream ss;
	ss << data;
	std::string strLine;
	int line = 0;
	std::string::size_type pos;

	std::string content;
	std::string comment;
	std::string strDiv;
	CUVIniNode* pScopeNode = _root;
	CUVIniNode* pNewNode;
	while (std::getline(ss, strLine)) {
		++line;

		content = uvstring::trimL(strLine);
		if (content.empty()) {
			strDiv += '\n';
			continue;
		}

		comment = "";
		pos = content.find_first_of(_comment);
		if (pos != std::string::npos) {
			comment = content.substr(pos);
			content = content.substr(0, pos);
		}

		content = uvstring::trimR(content);
		if (content.empty()) {
			strDiv += strLine;
			strDiv += "\n";
			continue;
		} else if (!strDiv.empty()) {
			auto pNode = new CUVIniNode;
			pNode->type = CUVIniNode::INI_NODE_TYPE_DIV;
			pNode->label = strDiv;
			pScopeNode->add(pNode);
			strDiv = "";
		}

		if (content[0] == '[') {
			if (content[content.length() - 1] == ']') {
				// section
				content = uvstring::trim(content.substr(1, content.length() - 2));
				pNewNode = new CUVIniNode;
				pNewNode->type = CUVIniNode::INI_NODE_TYPE_SECTION;
				pNewNode->label = content;
				_root->add(pNewNode);
				pScopeNode = pNewNode;
			} else {
				continue;
			}
		} else {
			pos = content.find_first_of(_delim);
			if (pos != std::string::npos) {
				// key - value
				pNewNode = new CUVIniNode;
				pNewNode->type = CUVIniNode::INI_NODE_TYPE_KEY_VALUE;
				pNewNode->label = uvstring::trim(content.substr(0, pos));
				pNewNode->value = uvstring::trim(content.substr(pos + _delim.length()));
				pScopeNode->add(pNewNode);
			} else {
				continue;
			}
		}

		if (!comment.empty()) {
			auto pNode = new CUVIniNode;
			pNode->type = CUVIniNode::INI_NODE_TYPE_SPAN;
			pNode->label = comment;
			pNewNode->add(pNode);
			comment = ""; // NOLINT
		}
	}

	// file and comment
	if (!strDiv.empty()) {
		auto pNode = new CUVIniNode;
		pNode->type = CUVIniNode::INI_NODE_TYPE_DIV;
		pNode->label = strDiv;
		_root->add(pNode);
	}

	return 0;
}

int CUVIniParser::unload() {
	if (_root) {
		delete _root;
		_root = nullptr;
		return UNLOAD_SUCCESS;
	}
	return UNLOAD_FAILED;
}

int CUVIniParser::reload() {
	return loadFromFile(_filepath.c_str());
}

std::string CUVIniParser::dumpString() {
	std::string result;
	dumpString(_root, result);
	return result;
}

int CUVIniParser::save() {
	return saveAs(_filepath.c_str());
}

int CUVIniParser::saveAs(const char* filepath) {
	const std::string str = dumpString();
	if (str.empty()) {
		return 0;
	}

	CUVFile file;
	if (file.open(filepath, "w") != 0) {
		return ERR_SAVE_FILE;
	}
	if (file.write(static_cast<const void*>(str.c_str()), str.length()) != str.size()) {
		file.close();
		return ERR_WRITE_FILE;
	}

	file.close();

	return 0;
}

std::string CUVIniParser::getValue(const std::string& key, const std::string& section) const {
	if (!_root) {
		return "";
	}

	auto pSection = _root;
	if (!section.empty()) {
		pSection = _root->get(section, CUVIniNode::INI_NODE_TYPE_SECTION);
		if (!pSection) {
			return "";
		}
	}

	const auto pkv = pSection->get(key, CUVIniNode::INI_NODE_TYPE_KEY_VALUE);
	if (!pkv) {
		return "";
	}

	return pkv->value;
}

void CUVIniParser::setValue(const std::string& key, const std::string& value, const std::string& section) {
	if (!_root) {
		_root = new CUVIniNode;
	}

	auto pSection = _root;
	if (!section.empty()) {
		pSection = _root->get(section, CUVIniNode::INI_NODE_TYPE_SECTION);
		if (!pSection) {
			pSection = new CUVIniNode;
			pSection->type = CUVIniNode::INI_NODE_TYPE_SECTION;
			pSection->label = section;
			_root->add(pSection);
		}
	}

	auto pkv = pSection->get(key, CUVIniNode::INI_NODE_TYPE_KEY_VALUE);
	if (!pkv) {
		pkv = new CUVIniNode;
		pkv->type = CUVIniNode::INI_NODE_TYPE_KEY_VALUE;
		pkv->label = key;
		pSection->add(pkv);
	}

	pkv->value = value;
}

void CUVIniParser::dumpString(CUVIniNode* node, std::string& str) { // NOLINT
	if (!node) {
		return;
	}

	if (node->type != CUVIniNode::INI_NODE_TYPE_SPAN) {
		if (!str.empty() && str[str.length() - 1] != '\n') {
			str += '\n';
		}
	}

	switch (node->type) {
		case CUVIniNode::INI_NODE_TYPE_SECTION: {
			str += '[';
			str += node->label;
			str += ']';
			break;
		}
		case CUVIniNode::INI_NODE_TYPE_KEY_VALUE: {
			str += uvstring::asprintf("%s %s %s", node->label.c_str(), _delim.c_str(), node->value.c_str());
			break;
		}
		case CUVIniNode::INI_NODE_TYPE_DIV: {
			str += node->label;
			break;
		}
		case CUVIniNode::INI_NODE_TYPE_SPAN: {
			str += '\t';
			str += node->label;
			break;
		}

		default: break;
	}

	for (const auto& p: node->children) {
		dumpString(p, str);
	}
}

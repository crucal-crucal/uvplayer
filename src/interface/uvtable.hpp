#pragma once

#include <map>

class CUVTableCell {
public:
	explicit CUVTableCell();
	explicit CUVTableCell(int r1, int r2, int c1, int c2);

	[[nodiscard]] int rowspan() const { return r2 - r1; }

	[[nodiscard]] int colspan() const { return c2 - c1; }

	[[nodiscard]] int span() const { return rowspan() * colspan(); }

	[[nodiscard]] bool contain(const CUVTableCell& cell) const;

	int r1{}, r2{}, c1{}, c2{};
};

class CUVTable {
public:
	explicit CUVTable();

	void init(int row, int col);
	bool getTableCell(int id, CUVTableCell& cell) const;
	CUVTableCell merge(int lt, int rb);

	int row{}, col{};
	std::map<int, CUVTableCell> m_cells{}; // id => CUVTableCell
};

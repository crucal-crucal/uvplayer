#include "uvtable.hpp"

void combine(const CUVTableCell& cell1, const CUVTableCell& cell2, CUVTableCell& comb) {
	comb.r1 = std::min(cell1.r1, cell2.r1);
	comb.r2 = std::max(cell1.r2, cell2.r2);
	comb.c1 = std::min(cell1.c1, cell2.c1);
	comb.c2 = std::max(cell1.c2, cell2.c2);
}

bool overlap(const CUVTableCell& cell1, const CUVTableCell& cell2, CUVTableCell& inter, CUVTableCell& comb) {
	inter.r1 = std::max(cell1.r1, cell2.r1);
	inter.r2 = std::min(cell1.r2, cell2.r2);
	inter.c1 = std::max(cell1.c1, cell2.c1);
	inter.c2 = std::min(cell1.c2, cell2.c2);

	if (inter.r1 < inter.r2 && inter.c1 < inter.c2) {
		combine(cell1, cell2, comb);
		return true;
	}

	return false;
}

/*!
 *  \CUVTableCell
 */
CUVTableCell::CUVTableCell() {
	r1 = r2 = c1 = c2 = 0;
}

CUVTableCell::CUVTableCell(const int r1, const int r2, const int c1, const int c2) {
	this->r1 = r1;
	this->r2 = r2;
	this->c1 = c1;
	this->c2 = c2;
}

bool CUVTableCell::contain(const CUVTableCell& cell) const {
	return cell.r1 >= r1 && cell.r2 <= r2 && cell.c1 >= c1 && cell.c2 <= c2;
}

/*!
 *  \CUVTable
 */
CUVTable::CUVTable() {
	row = col = 0;
}

void CUVTable::init(const int row, const int col) {
	this->row = row;
	this->col = col;
	m_cells.clear();
	for (int r = 1; r <= row; r++) {
		for (int c = 1; c <= col; c++) {
			int id = (r - 1) * col + c;
			m_cells[id] = CUVTableCell(r, r + 1, c, c + 1);
		}
	}
}

bool CUVTable::getTableCell(const int id, CUVTableCell& cell) const {
	if (m_cells.find(id) != m_cells.end()) {
		cell = m_cells.at(id);
		return true;
	}

	return false;
}

CUVTableCell CUVTable::merge(const int lt, const int rb) {
	CUVTableCell cell_lt;
	if (CUVTableCell cell_rb; getTableCell(lt, cell_lt) && getTableCell(rb, cell_rb)) {
		CUVTableCell comb;
		CUVTableCell inter;
		combine(cell_lt, cell_rb, comb);

		auto it = m_cells.begin();
		while (it != m_cells.end()) {
			if (overlap(comb, it->second, inter, comb)) {
				it = m_cells.erase(it);
			} else {
				++it;
			}
		}

		const int id = (comb.r1 - 1) * col + comb.c1;
		m_cells.at(id) = comb;

		return comb;
	}

	return CUVTableCell();
}

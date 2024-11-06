
#include "table.h"

const Table &Table::EMPTY_TABLE() {
    static Table table; // This will be initialized only on the first call
    return table;
}

const PtrTable &PtrTable::EMPTY_TABLE() {
    static PtrTable table;
    return table;
}

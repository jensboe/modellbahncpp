
/*
 * Copyright (c) 2019, Niklas Hauser
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
// ----------------------------------------------------------------------------

#include <stddef.h>
#include <stdint.h>
#include "heap_table.hpp"

struct table_pool_t
{
	uint32_t traits;
	uint8_t *const start;
	uint8_t *const end;
} modm_packed;

extern "C" const table_pool_t __table_heap_start[];
extern "C" const table_pool_t __table_heap_end[];

namespace modm::platform
{
HeapTable::Iterator
HeapTable::begin()
{
	return Iterator(__table_heap_start);
}

HeapTable::Iterator
HeapTable::end()
{
	return Iterator(__table_heap_end);
}

HeapTable::Iterator::Iterator(const table_pool_t* table):
	table(table)
{}

HeapTable::Iterator::Type
HeapTable::Iterator::operator*() const
{
	return {MemoryTraits(table->traits), table->start, table->end, static_cast<size_t>(table->end - table->start)};
}

HeapTable::Iterator&
HeapTable::Iterator::operator++()
{
	table++;
	return *this;
}

HeapTable::Iterator
HeapTable::Iterator::operator++(int)
{
	Iterator it{table};
	++*this;
	return it;
}

bool
HeapTable::Iterator::operator==(const Iterator& other) const
{
	return table == other.table;
}

bool
HeapTable::Iterator::operator!=(const Iterator& other) const
{
	return table != other.table;
}

// Finds the largest heap with declared traits
bool
HeapTable::find_largest(const uint8_t **const start,
						const uint8_t **const end,
						const MemoryTraits trait_mask)
{
	size_t current_size = 0;
	*start = nullptr;
	*end = nullptr;

	for (const auto [ttraits, tstart, tend, tsize] : modm::platform::HeapTable())
	{
		// we only care for generic enough regions
		if (ttraits & trait_mask)
		{
			// if the start address is equal to the end address
			// we can extend the previous memory region with this one
			if (tstart == *end) {
				*end = tend;
				current_size += tsize;
			}
			else if (tsize >= current_size) {
				// otherwise we found a larger region
				*start = tstart;
				*end = tend;
				current_size = tsize;
			}
		}
	}
	return *start;
}

} // namespace modm::platform
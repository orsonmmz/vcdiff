/*
 * Copyright CERN 2016
 * @author Maciej Suminski (maciej.suminski@cern.ch)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "value.h"

#include <sstream>

using namespace std;

void Value::resize(unsigned int new_size) {
    assert(type == VECTOR);
    assert(new_size > size);

    bit_t*new_val = new bit_t[new_size];
    memcpy(new_val, data.vec, size * sizeof(bit_t));
    memset(&new_val[size], 'X', new_size - size);

    delete data.vec;
    data.vec = new_val;
}

Value&Value::operator=(const Value&other) {
    assert(type == other.type);
    assert(size == other.size);

    if(type == VECTOR) {
        memcpy(data.vec, other.data.vec, size * sizeof(bit_t));
    } else {
        data = other.data;
    }

    return *this;
}

bool Value::operator==(const Value&other) const {
    if(type != other.type)
        return false;

    if(size != other.size)
        return false;

    switch(type) {
        case BIT:
            return data.bit == other.data.bit;

        case VECTOR:
            return strncmp(data.vec, other.data.vec, size);

        case REAL:
            return data.real == other.data.real;
    }

    // Should never be executed
    assert(false);

    return false;
}

Value::operator string() const {
    switch(type) {
        case BIT:
            return string(&data.bit, 1);

        case VECTOR:
            return string(data.vec, size);

        case REAL:
        {
            stringstream s;
            s << data.real;
            return s.str();
        }
    }

    assert(false);

    return string();
}

ostream&operator<<(ostream&out, const Value&var)
{
    switch(var.type) {
        case Value::BIT:
            out << var.data.bit;

        case Value::VECTOR:
        {
            const bit_t*p = var.data.vec;

            for(unsigned int i = 0; i < var.size; ++i) {
                out << *p;
                ++p;
            }
        }

        case Value::REAL:
            out << var.data.real;
    }

    return out;
}


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

Value::Value(data_type_t data_type)
    : type(data_type), size(1) {
    switch(data_type) {
        case BIT:
            data.bit = UNINITIALIZED;
            break;

        case VECTOR:
            data.vec = new bit_t[1];
            *data.vec = UNINITIALIZED;
            break;

        case REAL:
            data.real = 0.0f;
            break;

        case UNDEFINED:
            break;

        default:
            assert(false);
            break;
    }
}

void Value::resize(unsigned int new_size) {
    assert(type == VECTOR);
    assert(new_size >= size);

    if(new_size == size)
        return;

    int size_diff = new_size - size;

    // Copy the old data and fill the new part with '?'
    bit_t*new_val = new bit_t[new_size];
    memcpy(new_val + size_diff, data.vec, size * sizeof(bit_t));
    memset(new_val, UNINITIALIZED, size_diff);

    delete[] data.vec;
    data.vec = new_val;
    size = new_size;
}

unsigned int Value::checksum() const {
    unsigned int res = 0;

    switch(type) {
        case BIT:
            res = data.bit;
            break;

        case VECTOR:
            for(unsigned int i = 0; i < size; ++i) {
                res += data.vec[i];
            }
            break;

        case REAL:
        {
            union float_int {
                float f;
                unsigned int i;
            } *u = (float_int*)&data.real;
            res = u->i;
            break;
        }

        default:
            assert(false);
            break;
    }

    return res;
}

Value&Value::operator=(const Value&other) {
    assert(type == other.type || type == UNDEFINED);
    assert(size >= other.size || type == UNDEFINED);

    if(type == UNDEFINED) {
        type = other.type;
        size = other.size;

        if(type == VECTOR)
            data.vec = new bit_t[size];
    }

    if(type == VECTOR) {
        int size_diff = size - other.size;
        // Zero the unspecified part, save right-justified new value
        memset(data.vec, '0', size_diff);
        memcpy(data.vec + size_diff, other.data.vec, other.size * sizeof(bit_t));
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

        default:
            assert(false);
            break;
    }

    return false;
}

bool Value::operator!=(const Value&other) const {
    return !(*this == other);
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

        case UNDEFINED:
            // UNDEFINED should be a temporary state, do not expect any
            // variable to print itself out when undefined
            assert(false);
            return string("<undefined>");

        default:
            assert(false);
            break;
    }

    return string();
}

ostream&operator<<(ostream&out, const Value&var)
{
    out << (string) var;

    return out;
}


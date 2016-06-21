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

#include <functional>
#include <sstream>

#include <cstring>

using namespace std;

Value::Value(const std::vector<bit_t>&val)
    : type(VECTOR), size(val.size()) {
    data.vec = new bit_t[size];
    memcpy(data.vec, &val[0], size * sizeof(bit_t));
}

Value::Value(data_type_t data_type)
    : type(data_type), size(1) {
    switch(data_type) {
        case BIT:
            data.bit = UNINITIALIZED;
            break;

        case VECTOR:
            data.vec = new bit_t[1];
            data.vec[0] = UNINITIALIZED;
            break;

        case REAL:
            data.real = 0.0f;
            break;

        case UNDEFINED:
            memset(&data, 0, sizeof(data));
            break;

        default:
            assert(false);
            break;
    }
}

Value::Value(const string&val)
    : type(VECTOR), size(val.size()) {
    data.vec = new bit_t[size];
    memcpy(data.vec, val.c_str(), size * sizeof(bit_t));
}

Value::Value(const Value&other)
    : type(other.type), size(other.size) {
    if(type == VECTOR) {
        data.vec = new bit_t[size];
        memcpy(data.vec, other.data.vec, size * sizeof(bit_t));
    } else {
        data = other.data;
    }
}

void Value::resize(unsigned int new_size) {
    assert(type == VECTOR);
    assert(new_size >= size);

    if(new_size == size)
        return;

    unsigned int size_diff = new_size - size;
    bit_t*new_vec = new bit_t[new_size];
    memset(new_vec, '0', size_diff);
    memcpy(new_vec + size_diff, data.vec, size * sizeof(bit_t));

    delete [] data.vec;
    data.vec = new_vec;
    size = new_size;
}

size_t Value::hash() const {
    size_t res = 0;

    switch(type) {
        case BIT:
            res = std::hash<bit_t>()(data.bit);
            break;

        case VECTOR:
            for(unsigned int i = 0; i < size; ++i)
                res += std::hash<bit_t>()(data.vec[i]);
            break;

        case REAL:
            res = std::hash<float>()(data.real);
            break;

        default:
            assert(false);
            break;
    }

    return res;
}


Value&Value::operator=(const Value&other) {
    assert(type == other.type || type == UNDEFINED);
    assert(size >= other.size || type == UNDEFINED);

    size = other.size;

    if(type == UNDEFINED) {
        type = other.type;

        if(type == VECTOR) {}
            data.vec = new bit_t[other.size];
    }

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
            return !strncmp(data.vec, other.data.vec, size);

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
    }

    // Fallback, should never happen
    assert(false);

    return string();
}

ostream&operator<<(ostream&out, const Value&var)
{
    out << (string) var;

    return out;
}

const bit_t Value::UNINITIALIZED = '?';


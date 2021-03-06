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

#ifndef VALUE_H
#define VALUE_H

#include <string>
#include <vector>

#include <cassert>

/// Basic bit type (possible values 0, 1, X, Z)
typedef char bit_t;

class Value {
public:
    enum data_type_t { BIT, VECTOR, REAL, UNDEFINED };

    Value()
      : type(UNDEFINED), size(0) {
    }

    Value(bit_t val)
      : type(BIT), size(1) {
        data.bit = toupper(val);
        assert(data.bit == '0' || data.bit == '1'
                || data.bit == 'X' || data.bit == 'Z'
                || data.bit == UNINITIALIZED);
    }

    Value(float val)
      : type(REAL), size(1) {
        data.real = val;
    }

    Value(const std::vector<bit_t>&val);
    Value(data_type_t data_type);
    Value(const std::string&val);
    Value(const Value&other);

    ~Value() {
        if(type == VECTOR)
            delete [] data.vec;
    }

    void resize(unsigned int new_size);

    /**
     * @brief Computes hash for quick comparison.
     */
    size_t hash() const;

    Value&operator=(const Value&other);
    bool operator==(const Value&other) const;
    bool operator!=(const Value&other) const;
    operator std::string() const;

    data_type_t type;

    union data_t {
        bit_t bit;
        bit_t*vec;
        float real;
    } data;

    unsigned int size;

    static const bit_t UNINITIALIZED;
};

std::ostream&operator<<(std::ostream&out, const Value&var);

#endif /* VALUE_H */


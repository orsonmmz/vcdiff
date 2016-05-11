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
#include <cstring>

/// Basic bit & bit vector types
typedef char bit_t;
typedef std::vector<bit_t> vec_t;

class Value {
public:
    Value(bit_t val)
      : type(BIT), size(1) {
          data.bit = toupper(val);
          assert(data.bit == '0' || data.bit == '1'
                  || data.bit == 'X' || data.bit == 'Z');
    }

    Value(float val)
      : type(REAL), size(1) {
          data.real = val;
    }

    Value(const std::vector<bit_t>&val)
      : type(VECTOR), size(val.size()) {
          assert(size > 0);

          // TODO check if it contains valid values
          data.vec = new bit_t[size];
          memcpy(data.vec, &val[0], size * sizeof(bit_t));
    }

    Value(const std::string&val)
      : type(VECTOR), size(val.size()) {
          assert(size > 0);

          // TODO check if it contains valid values?
          data.vec = new bit_t[size];
          memcpy(data.vec, &val[0], size * sizeof(bit_t));
    }

    Value(const Value&other)
      : type(other.type), size(other.size) {
        if(type == VECTOR) {
            data.vec = new bit_t[size];
            memcpy(data.vec, other.data.vec, size * sizeof(bit_t));
        } else {
            data = other.data;
        }
    }

    ~Value() {
        if(type == VECTOR)
            delete data.vec;
    }

    void resize(unsigned int new_size);

    /**
     * @brief Computes comparison checksum, used for tests only.
     */
    unsigned int checksum() const;

    Value&operator=(const Value&other);
    bool operator==(const Value&other) const;
    operator std::string() const;

    enum type_t { BIT, VECTOR, REAL } type;

    union data_t {
        bit_t bit;
        bit_t*vec;
        float real;
    } data;

    unsigned int size;

};

std::ostream&operator<<(std::ostream&out, const Value&var);

#endif /* VALUE_H */


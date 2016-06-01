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

#ifndef LINK_H
#define LINK_H

#include <ostream>

class Variable;

class Link {
public:
    Link(Variable*first, Variable*second);

    inline Variable*first() const {
        return first_;
    }

    inline Variable*second() const {
        return second_;
    }

    /*
     * @return true if the compared variables are equal.
     */
    bool compare() const;

    /*
     * Computes object's hash value, used to verify unit tests.
     */
    size_t hash() const;

private:
    Variable*first_;
    Variable*second_;
};

std::ostream&operator<<(std::ostream&out, const Link&link);

#endif /* LINK_H */


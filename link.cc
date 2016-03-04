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

#include "link.h"
#include "variable.h"

using namespace std;

Link::Link(const Variable*first, const Variable*second)
    : first_(first), second_(second) {
    assert(first && second);
    assert(first_->size() == second_->size());
}

bool Link::compare() const {
    return (first_->value_str() == second_->value_str());
}

ostream&operator<<(ostream&out, const Link&link) {
    const Variable*var1 = link.first();
    const Variable*var2 = link.second();

    out << *var1 << "\t= " << var1->value_str() << endl;
    out << *var2 << "\t= " << var2->value_str() << endl;

    return out;
}


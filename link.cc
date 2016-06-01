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
#include "options.h"

#include <iomanip>
#include <sstream>

using namespace std;

Link::Link(Variable*first, Variable*second)
    : first_(first), second_(second) {
    assert(first && second);
    assert(first_->size() == second_->size());
}

bool Link::compare() const {
    return ((first_->hash() == second_->hash())
        && (compare_states || first_->prev_hash() == second_->prev_hash()));
}

size_t Link::hash() const {
    size_t first = first_->hash() ^ first_->prev_hash();
    size_t second = second_->hash() ^ first_->prev_hash();

    return (first + 1) * (second + 1);
}

ostream&operator<<(ostream&out, const Link&link) {
    stringstream s1, s2;
    const Variable*var1 = link.first();
    const Variable*var2 = link.second();

    s1 << *var1;
    s2 << *var2;

    // Align scope names
    int len1 = s1.str().length();
    int len2 = s2.str().length();

    if(len1 > len2) {
        s2 << setw(len1 - len2) << " ";
    } else if(len2 > len1) {
        s1 << setw(len2 - len1) << " ";
    }

    s1 << "\t= ";
    s2 << "\t= ";

    if(!compare_states && var1->changed())
        s1 << var1->prev_value_str() << " -> ";

    s1 << var1->value_str();


    if(!compare_states && var2->changed())
        s2 << var2->prev_value_str() << " -> ";

    s2 << var2->value_str();

    out << s1.str() << endl;
    out << s2.str() << endl;

    return out;
}


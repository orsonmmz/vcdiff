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

#ifndef COMPARATOR_H
#define COMPARATOR_H

#include <list>

class Link;
class Scope;
class Variable;
class VcdFile;

class Comparator {
public:
    Comparator(VcdFile&file1, VcdFile&file2);
    ~Comparator();

    int compare();

private:
    void map_signals(Scope&scope1, Scope&scope2);

    void check_value_changes();

    bool compare_and_match(Variable*var1, Variable*var2);

    std::list<Link*> links_;
    VcdFile&file1_;
    VcdFile&file2_;
};

#endif /* COMPARATOR_H */


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

#ifndef SCOPE_H
#define SCOPE_H

#include "variable.h"

#include <string>
#include <map>

class Scope;

typedef std::map<std::string, Scope*> ScopeStringMap;

class Scope {
public:
    ///> Possible scope types in VCD files
    enum scope_type_t {
        BEGIN, FORK, FUNCTION, MODULE, TASK, UNKNOWN
    };

    Scope(scope_type_t scope_type, const std::string&name, Scope*parent);
    ~Scope();

    inline const std::string&name() const {
        return name_;
    }

    inline const std::string&full_name() const {
        return full_name_;
    }

    inline scope_type_t type() const {
        return scope_type_;
    }

    Scope*make_scope(scope_type_t type, const std::string&name);
    Scope*get_scope(const std::string&name);

    ScopeStringMap&scopes() {
        return scopes_;
    }

    void add_variable(Variable*var);
    Variable*get_variable(const std::string&name);

    std::map<std::string, Variable*>&variables() {
        return vars_;
    }

    inline Scope*parent() const {
        return parent_;
    }

private:
    // The unique part of the scope name
    const std::string name_;

    // Full scope name, including the scopes hierarchy
    std::string full_name_;

    // Scope kind
    const scope_type_t scope_type_;

    // Subscopes
    ScopeStringMap scopes_;

    // Variables stored in the scope
    VarStringMap vars_;

    // Parent scope, NULL if it is the root scope
    Scope*parent_;
};

#endif /* SCOPE_H */


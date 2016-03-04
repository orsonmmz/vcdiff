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

#include "scope.h"
#include "debug.h"

#include <cassert>

using namespace std;

Scope::Scope(const string&name, Scope*parent)
    : name_(name), parent_(parent)
{
}

Scope::~Scope() {
    for(ScopeStringMap::iterator it = scopes_.begin();
            it != scopes_.end(); ++it) {
        delete it->second;
    }

    for(VarStringMap::iterator it = vars_.begin();
            it != vars_.end(); ++it) {
        delete it->second;
    }
}

string Scope::full_name() {
    if(full_name_.empty()) {
        // Cache the full name
        const Scope*s = parent_;
        full_name_ = name_;

        // Concatenate scope names following the hierarchy to the root scope
        while(s) {
            full_name_ = s->name() + "." + full_name_;
            s = s->parent();
        }
    }

    return full_name_;
}

Scope*Scope::make_scope(const string&name) {
    pair<ScopeStringMap::iterator, bool> res;

    res = scopes_.insert(make_pair(name, new Scope(name, this)));
    // Be sure that scope names are unique
    assert(res.second);

    return res.first->second;
}

Scope*Scope::get_scope(const string&name) {
    ScopeStringMap::iterator res = scopes_.find(name);

    if(res != scopes_.end())
        return res->second;

    return NULL;
}

void Scope::add_variable(Variable*var) {
    assert(vars_.count(var->name()) == 0);

    DBG("added var %s\tident %s\tsize %d",
            var->full_name().c_str(), var->ident().c_str(), var->size());

    vars_[var->name()] = var;
    var->set_scope(this);
}

Variable*Scope::get_variable(const string&name) {
    VarStringMap::iterator var = vars_.find(name);

    return (var != vars_.end() ? var->second : NULL);
}


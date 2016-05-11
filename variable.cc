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

// TODO real type
// TODO ident as char[8] to speed up?

#include "variable.h"
#include "scope.h"

#include <ostream>
#include <sstream>

using namespace std;

Variable::Variable(type_t type, const string&name, const string&identifier)
    : scope_(NULL), name_(name), ident_(identifier),
        type_(type), idx_(-1), link_(NULL) {
    assert(type_ != UNKNOWN);

    // The following types are not handled at the moment
    assert(type_ != EVENT);
    assert(type_ != REAL);
}

unsigned int Variable::checksum() const {
    unsigned int res = 0;
    std::string val = value_str();
    std::string prev_val = prev_value_str();

    for(unsigned int i = 0; i < val.size(); ++i)
        res += val[i];

    for(unsigned int i = 0; i < prev_val.size(); ++i)
        res += prev_val[i];

    for(unsigned int i = 0; i < name_.size(); ++i)
        res += name_[i];

    return res;
}

Vector::Vector(type_t type, int left_idx, int right_idx,
        const string&name, const string&identifier)
    : Variable(type, name, identifier),
        left_idx_(left_idx), right_idx_(right_idx)
{
}

Vector::~Vector() {
    for(map<int, Variable*>::iterator it = val_.begin();
            it != val_.end(); ++it) {
        delete it->second;
    }
}

string Vector::full_name() const {
    stringstream s;
    s << name() << "[";

    if(left_idx_ != right_idx_)
        s << left_idx_ << ":" << right_idx_;
    else
        s << left_idx_;

    s << "]";
    return s.str();
}

void Vector::add_variable(int idx, Variable*var) {
    assert(val_.count(idx) == 0);

    // Update the dimensions if needed
    if(left_idx_ > right_idx_) {
        if(idx > left_idx_)
            left_idx_ = idx;
        else if(idx < right_idx_)
            right_idx_ = idx;
    } else { // left_idx_ < right_idx_
        if(idx > right_idx_)
            right_idx_ = idx;
        else if(idx < left_idx_)
            left_idx_ = idx;
    }

    val_[idx] = var;
    var->set_index(idx);
}

void Vector::set_value(const Value&value) {
    assert(value.size <= val_.size());
    assert(value.type == Value::VECTOR);

    int new_val_idx = value.size - 1;

    // Copy the new value and set the remaining bits to 0
    if(left_idx_ < right_idx_) {
        for(int i = left_idx_; i <= right_idx_; ++i) {
            if(new_val_idx >= 0) {
                val_[i]->set_value(value.data.vec[new_val_idx]);
                --new_val_idx;
            } else {
                val_[i]->set_value('0');
            }
        }

    } else {
        for(int i = left_idx_; i >= right_idx_; --i) {
            if(new_val_idx >= 0) {
                val_[i]->set_value(value.data.vec[new_val_idx]);
                --new_val_idx;
            } else {
                val_[i]->set_value('0');
            }
        }
    }
}

string Vector::value_str() const {
    stringstream s;

    for(map<int, Variable*>::const_iterator it = val_.begin();
            it != val_.end(); ++it) {
        s << it->second->value_str();
    }

    return s.str();
}

string Vector::prev_value_str() const {
    stringstream s;

    for(map<int, Variable*>::const_iterator it = val_.begin();
            it != val_.end(); ++it) {
        s << it->second->prev_value_str();
    }

    return s.str();
}

bool Vector::changed() const {
    for(map<int, Variable*>::const_iterator it = val_.begin();
            it != val_.end(); ++it) {
        if(it->second->changed())
            return true;
    }

    return false;
}

void Vector::clear_transition() {
    for(map<int, Variable*>::const_iterator it = val_.begin();
            it != val_.end(); ++it) {
        it->second->clear_transition();
    }
}

void Vector::fill() {
    for(int i = min_idx(); i <= max_idx(); ++i)
        add_variable(i, new Scalar(type()));
}

Scalar::Scalar(type_t type, const string&name, const string&identifier)
    : Variable(type, name, identifier), val_('?'), prev_val_('?') {
    if(type == SUPPLY0)
        val_ = 0;
    else if(type == SUPPLY1)
        val_ = 1;
}

string Scalar::full_name() const {
    if(index() == -1)
        return name();

    stringstream s;
    s << name() << "[" << index() << "]";
    return s.str();
}

string Scalar::value_str() const {
    return string(1, val_);
}

string Scalar::prev_value_str() const {
    return string(1, prev_val_);
}

Parameter::Parameter(const std::string&name, const std::string&identifier)
    : Variable(PARAMETER, name, identifier),
    value_(NULL), just_initialized_(false) {
}

Parameter::~Parameter() {
    delete value_;
}

Alias::Alias(const string&name, Variable*target)
    : Variable(target->type(), name, target->ident()), target_(target)
{
    assert(target);
}

std::ostream&operator<<(std::ostream&out, const Variable&var) {
    Scope*scope = var.scope();

    if(scope)
        out << scope->full_name() << ".";

    out << var.full_name();

    return out;
}


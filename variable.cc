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
// TODO cache full names

#include "variable.h"
#include "scope.h"

#include <ostream>
#include <sstream>

using namespace std;

Variable::Variable(var_type_t type, Value::data_type_t data_type,
        const string&name, const string&identifier)
    : scope_(NULL), name_(name), ident_(identifier),
        type_(type), data_type_(data_type), idx_(-1), link_(NULL) {
    assert(type_ != UNKNOWN);

    // The following types are not handled at the moment
    assert(type_ != EVENT);
    assert(type_ != REAL);
}

Vector::Vector(var_type_t type, int left_idx, int right_idx,
        const string&name, const string&identifier)
    : Variable(type, Value::VECTOR, name, identifier),
        left_idx_(left_idx), right_idx_(right_idx)
{
}

Vector::~Vector() {
    for(auto&var : children_)
        delete var.second;
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
    assert(children_.count(idx) == 0);

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

    var->set_index(idx);
    children_[idx] = var;
}

void Vector::set_value(const Value&value) {
    assert(value.size <= size());
    assert(value.type == Value::VECTOR);

    int new_val_idx = value.size - 1;

    // Copy the new value and set the remaining bits to 0,
    // update the children variables values
    // TODO this could be simplified
    if(!ident().empty()) {
        if(range_asc()) {
            for(int i = left_idx_; i <= right_idx_; ++i) {
                if(new_val_idx >= 0) {
                    children_[i]->set_value(value.data.vec[new_val_idx]);
                    --new_val_idx;
                } else {
                    children_[i]->set_value('0');
                }
            }

        } else {    // descending range
            for(int i = left_idx_; i >= right_idx_; --i) {
                if(new_val_idx >= 0) {
                    children_[i]->set_value(value.data.vec[new_val_idx]);
                    --new_val_idx;
                } else {
                    children_[i]->set_value('0');
                }
            }
        }
    }
}

bool Vector::changed() const {
    for(auto&var : children_) {
        if(var.second->changed())
            return true;
    }

    return false;
}

void Vector::clear_transition() {
    for(auto&var : children_)
        var.second->clear_transition();
}

unsigned long long Vector::checksum() const {
    unsigned long long res = 0;

    for(auto&var : children_)
        res ^= var.second->checksum();

    return res;
}

unsigned long long Vector::prev_checksum() const {
    unsigned long long res = 0;

    for(auto&var : children_)
        res ^= var.second->prev_checksum();

    return res;
}

string Vector::value_str() const {
    stringstream s;

    for(auto&var : children_)
        s << var.second->value_str();

    return s.str();
}

string Vector::prev_value_str() const {
    stringstream s;

    for(auto&var : children_)
        s << var.second->prev_value_str();

    return s.str();
}

void Vector::fill() {
    for(int i = min_idx(); i <= max_idx(); ++i)
        add_variable(i, new Scalar(type()));
}

Scalar::Scalar(var_type_t type, const string&name, const string&identifier)
    : Variable(type, Value::BIT, name, identifier),
        value_(Value::BIT), prev_value_(Value::BIT) {
    if(type == SUPPLY0)
        value_.data.bit = '0';
    else if(type == SUPPLY1)
        value_.data.bit = '1';
}

string Scalar::full_name() const {
    if(index() == -1)
        return name();

    stringstream s;
    s << name() << "[" << index() << "]";

    return s.str();
}

Alias::Alias(const string&name, Variable*target)
    : Variable(target->type(), target->data_type(), name,
            target->ident()), target_(target)
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


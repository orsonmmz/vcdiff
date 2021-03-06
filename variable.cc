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

// TODO ident as char[8] to speed up?

#include "variable.h"
#include "scope.h"

#include <ostream>
#include <sstream>

using namespace std;

Variable::Variable(var_type_t type, Value::data_type_t data_type,
        const string&name, const string&identifier)
    : scope_(NULL), name_(name), ident_(identifier),
        type_(type), data_type_(data_type),
        parent_(NULL), idx_(-1), link_(NULL) {
    assert(type_ != UNKNOWN);

    // The following types are not handled at the moment
    assert(type_ != EVENT);
}

void Variable::recache_var_name() {
    //assert(!name().empty());
    full_name_ = name() + full_index();
}

std::string Variable::full_index(bool last) const {
    std::stringstream s;
    const Variable*p = this;

    while((p = p->parent()))
        s << p->full_index(false);

    if(last)
        s << index_str();
    else if(idx_ >= 0)
        s << "[" << idx_ << "]";

    return s.str();
}

Vector::Vector(var_type_t type, int left_idx, int right_idx,
        const string&name, const string&identifier)
    : Variable(type, Value::VECTOR, name, identifier),
        left_idx_(left_idx), right_idx_(right_idx), reversed_range_(false)
{
}

Vector::~Vector() {
    for(auto&var : children_)
        delete var.second;
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

    var->set_index(idx, this);
    children_[idx] = var;
    recache_var_name();
}

void Vector::set_value(const Value&value) {
    assert(value.size <= size());
    assert(value.type == Value::VECTOR);

    if(ident().empty())
        return;

    // Restore original indexes for assignment
    int left_idx = reversed_range_ ? right_idx_ : left_idx_;
    int right_idx = reversed_range_ ? left_idx_ : right_idx_;
    bool asc = left_idx < right_idx;

    // Value assigned for not specified bits (if the new value has less bits
    // than the target variable)
    bit_t default_val = value.data.vec[0] == '1' ? '0' : value.data.vec[0];

    int new_idx = value.size - 1;
    int idx = right_idx;
    int i = size();

    while(i > 0) {
        // Either copy the new value or assign the default value
        if(new_idx >= 0) {
            children_[idx]->set_value(value.data.vec[new_idx]);
            --new_idx;
        } else {
            children_[idx]->set_value(default_val);
        }

        if(asc)
            --idx;
        else
            ++idx;

        --i;
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

size_t Vector::hash() const {
    size_t res = 0;

    for(auto&var : children_) {
        res ^= var.second->hash();
        res <<= 1;
    }

    return res;
}

size_t Vector::prev_hash() const {
    size_t res = 0;

    for(auto&var : children_) {
        res ^= var.second->prev_hash();
        res <<= 1;
    }

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

string Vector::index_str() const {
    stringstream s;

    if(index() >= 0)
        s << "[" << index() << "]";

    s << "[" << left_idx_;

    if(left_idx_ != right_idx_)
        s << ":" << right_idx_;

    s << "]";

    return s.str();
}


void Vector::fill() {
    for(int i = min_idx(); i <= max_idx(); ++i) {
        Scalar*s = new Scalar(type(), Value::BIT, name());
        s->set_scope(scope());
        add_variable(i, s);
    }
}

Scalar::Scalar(var_type_t type, Value::data_type_t data_type,
        const string&name, const string&identifier)
    : Variable(type, data_type, name, identifier),
        value_(data_type), prev_value_(data_type), changed_(false) {
    if(type == SUPPLY0)
        value_.data.bit = '0';
    else if(type == SUPPLY1)
        value_.data.bit = '1';
}

string Scalar::index_str() const {
    stringstream s;
    int idx = index();

    if(idx >= 0)
        s << "[" << idx << "]";

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

    if(!scope && var.parent())
        scope = var.parent()->scope();

    if(scope)
        out << scope->full_name() << ".";

    out << var.full_name();

    return out;
}


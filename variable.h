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

#ifndef VARIABLE_H
#define VARIABLE_H

#include <map>
#include <string>

#include <cassert>
#include <cmath>

#include "value.h"

class Link;
class Scope;

class Variable {
public:
    enum type_t {
        EVENT, INTEGER, PARAMETER, REAL, REG, SUPPLY0, SUPPLY1, TIME,
        TRI, TRI0, TRI1, TRIAND, TRIOR, TRIREG, WAND, WIRE, WOR, UNKNOWN
    };

    Variable(type_t type, const std::string&name = "",
            const std::string&identifier = "");
    virtual ~Variable() {}

    inline void set_scope(Scope*scope) {
        assert(scope_ == NULL || scope_ == scope);
        scope_ = scope;
    }

    inline Scope*scope() const {
        return scope_;
    }

    inline const std::string&name() const {
        return name_;
    }

    virtual std::string full_name() const {
        return name_;
    }

    inline const std::string&ident() const {
        return ident_;
    }

    inline type_t type() const {
        return type_;
    }

    inline void set_index(int index) {
        assert(index >= 0 || index == idx_);
        idx_ = index;
    }

    inline int index() const {
        return idx_;
    }

    virtual const Link*link() const {
        return link_;
    }

    inline void set_link(const Link*link) {
        assert(link_ == NULL);
        link_ = link;
    }

    /**
     * @brief Returns vector size or 1 for scalar variables.
     */
    virtual unsigned int size() const {
        return 1;
    }

    virtual bool is_vector() const {
        return false;
    }

    virtual void set_value(const Value&value) = 0;
    virtual std::string value_str() const = 0;
    virtual std::string prev_value_str() const = 0;

    virtual bool changed() const = 0;
    virtual void clear_transition() = 0;

    inline bool operator==(const Variable&other) const {
        return (value_str() == other.value_str());
    }

private:
    Scope*scope_;
    const std::string name_;
    const std::string ident_;
    type_t type_;
    int idx_;
    const Link*link_;
};

class Vector : public Variable {
public:
    Vector(type_t type, int left_idx, int right_idx,
            const std::string&name = "", const std::string&identifier = "");
    ~Vector();

    std::string full_name() const;

    unsigned int size() const {
        assert(std::abs(left_idx_ - right_idx_) + 1 == val_.size());
        return std::abs(left_idx_ - right_idx_) + 1;
    }

    inline int left_idx() const {
        return left_idx_;
    }

    inline int right_idx() const {
        return right_idx_;
    }

    inline int max_idx() const {
        return std::max(right_idx_, left_idx_);
    }

    inline int min_idx() const {
        return std::min(right_idx_, left_idx_);
    }

    void add_variable(int idx, Variable*var);

    bool is_vector() const {
        return true;
    }

    void set_value(const Value&value);

    std::string value_str() const;
    std::string prev_value_str() const;

    bool changed() const;
    void clear_transition();

    inline bool is_valid_idx(int idx) const {
        return ((left_idx_ >= idx && idx >= right_idx_) ||
                (left_idx_ <= idx && idx <= right_idx_));
    }

    /**
     * Initializes vector with scalars.
     */
    void fill();

    Variable*&operator[](int idx) {
        return val_[idx];
    }

    const Variable*operator[](int idx) const {
        return val_.at(idx);
    }

    std::ostream&operator<<(std::ostream&out) const;

private:
    int left_idx_, right_idx_;
    std::map<int, Variable*> val_;
};

class Scalar : public Variable {
public:
    Scalar(type_t type, const std::string&name = "",
            const std::string&identifier = "");

    std::string full_name() const;

    void set_value(const Value&value) {
        assert(value.type == Value::BIT);
        prev_val_ = val_;
        val_ = toupper(value.data.bit);
        assert(val_ == '0' || val_ == '1' || val_ == 'X' || val_ == 'Z');
    }

    std::string value_str() const;
    std::string prev_value_str() const;

    bool changed() const {
        return prev_val_ != val_;
    }

    void clear_transition() {
        prev_val_ = val_;
    }

private:
    bit_t val_, prev_val_;
};

class Parameter : public Variable {
public:
    Parameter(const std::string&name = "", const std::string&identifier = "");
    ~Parameter();

    void set_value(const Value&value) {
        assert(!value_);
        value_ = new Value(value);
        just_initialized_ = true;
    }

    std::string value_str() const {
        assert(value_);
        return *value_;
    }

    std::string prev_value_str() const {
        assert(value_);

        // Parameters are immutable
        return *value_;
    }

    bool changed() const {
        // Parameters are immutable, they change only during initialization
        return just_initialized_;
    }

    void clear_transition() {
        just_initialized_ = true;
    }

private:
    Value*value_;
    bool just_initialized_;
};

class Alias : public Variable {
public:
    Alias(const std::string&name, Variable*target);

    Variable*target() const {
        return target_;
    }

    std::string full_name() const {
        return target_->full_name();
    }

    const Link*link() const {
        return target_->link();
    }

    unsigned int size() const {
        return target_->size();
    }

    bool is_vector() const {
        return target_->is_vector();
    }

    void set_value(const Value&value) {
        target_->set_value(value);
    }

    std::string value_str() const {
        return target_->value_str();
    }

    std::string prev_value_str() const {
        return target_->prev_value_str();
    }

    bool changed() const {
        return target_->changed();
    }

    void clear_transition() {
        target_->clear_transition();
    }

private:
    Variable*target_;
};

typedef std::map<std::string, Variable*> VarStringMap;

std::ostream&operator<<(std::ostream&out, const Variable&var);

#endif /* VARIABLE_H */


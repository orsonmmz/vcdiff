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
    ///> Possible variable types in VCD files
    enum type_t {
        EVENT, INTEGER, PARAMETER, REAL, REG, SUPPLY0, SUPPLY1, TIME,
        TRI, TRI0, TRI1, TRIAND, TRIOR, TRIREG, WAND, WIRE, WOR, UNKNOWN
    };

    Variable(type_t type, const std::string&name = "",
            const std::string&identifier = "");
    virtual ~Variable() {}

    /**
     * @brief Assigns a scope to variable. It can be done only once for a
     * variable.
     */
    inline void set_scope(Scope*scope) {
        assert(scope_ == NULL || scope_ == scope);
        scope_ = scope;
    }

    /**
     * @brief Returns currently assigned scope or NULL if there is none.
     */
    inline Scope*scope() const {
        return scope_;
    }

    /**
     * @brief Returns short name of the variable (e.g. for 'scope.module.var'
     * returns 'var').
     */
    inline const std::string&name() const {
        return name_;
    }

    /**
     * @brief If there is a scope assigned, it returns a full name including
     * scope. Otherwise it returns the variable name.
     */
    virtual std::string full_name() const {
        return name_;
    }

    /**
     * @brief Returns identifier associated with the variable.
     */
    inline const std::string&ident() const {
        return ident_;
    }

    /**
     * @brief Returns type of the variable.
     */
    inline type_t type() const {
        return type_;
    }

    /**
     * @brief Sets an index for the variable, in case the variable is a part
     * of a vector. It can be done only once for a variable.
     */
    inline void set_index(int index) {
        assert(index >= 0 || index == idx_);
        idx_ = index;
    }

    /**
     * @brief Returns the variable index, if it is a part of a vector,
     * or -1 if it is not the case.
     */
    inline int index() const {
        return idx_;
    }

    /**
     * @brief Returns an associated Link object, representing connection with
     * a variable from other .vcd file.
     * @see Link
     */
    virtual const Link*link() const {
        return link_;
    }

    /**
     * @brief Assigns a Link object, representing connection with a variable
     * from other .vcd file. It can be done only once for a variable.
     */
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

    /**
     * @brief Returns true if the variable is a vector, false otherwise.
     */
    virtual bool is_vector() const {
        return false;
    }

    /**
     * @brief Sets a new value for the variable.
     */
    virtual void set_value(const Value&value) = 0;

    /**
     * @brief Returns current value of the variable, represented as a string.
     */
    virtual std::string value_str() const = 0;

    /**
     * @brief Returns previous value of the variable, represented as a string.
     */
    virtual std::string prev_value_str() const = 0;

    /**
     * @brief Returns true if variable has changed in the current time step.
     */
    virtual bool changed() const = 0;

    /**
     * @brief Clears modification flag for the variable. It should be called
     * at the end of each time step.
     */
    virtual void clear_transition() = 0;

    inline bool operator==(const Variable&other) const {
        return (value_str() == other.value_str());
    }

private:
    ///> Parent scope
    Scope*scope_;

    ///> Variable name
    const std::string name_;

    ///> Variable identifier
    const std::string ident_;

    ///> Variable type
    type_t type_;

    ///> Variable index, if the variable is a part of a vector
    int idx_;

    ///> Associated Link object pointing to twin variable in another VCD file
    const Link*link_;
};

class Vector : public Variable {
public:
    Vector(type_t type, int left_idx, int right_idx,
            const std::string&name = "", const std::string&identifier = "");
    ~Vector();

    std::string full_name() const;

    unsigned int size() const {
        assert((unsigned)(std::abs(left_idx_ - right_idx_)) + 1 == val_.size());
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


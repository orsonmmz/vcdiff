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

#include <functional>
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
    enum var_type_t {
        EVENT, INTEGER, PARAMETER, REAL, REG, SUPPLY0, SUPPLY1, TIME,
        TRI, TRI0, TRI1, TRIAND, TRIOR, TRIREG, WAND, WIRE, WOR, UNKNOWN
    };

    Variable(var_type_t var_type, Value::data_type_t data_type,
            const std::string&name = "", const std::string&identifier = "");
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
    inline var_type_t type() const {
        return type_;
    }

    /**
     * @brief Returns the stored data type.
     */
    inline Value::data_type_t data_type() const {
        return data_type_;
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
     * @brief Returns true if variable has changed in the current time step.
     */
    virtual bool changed() const = 0;

    /**
     * @brief Clears modification flag for the variable. It should be called
     * at the end of each time step.
     */
    virtual void clear_transition() = 0;

    /**
     * @brief Computes the current value hash for quick comparisons.
     */
    virtual size_t hash() const = 0;

    /**
     * @brief Computes the previous value hash for quick comparisons.
     */
    virtual size_t prev_hash() const = 0;

    /**
     * @brief Returns the current variable value as a string.
     */
    virtual std::string value_str() const = 0;

    /**
     * @brief Returns the previous variable value as a string.
     */
    virtual std::string prev_value_str() const = 0;

private:
    ///> Parent scope
    Scope*scope_;

    ///> Variable name
    const std::string name_;

    ///> Variable identifier
    const std::string ident_;

    ///> Variable type
    var_type_t type_;

    ///> Stored data type
    Value::data_type_t data_type_;

    ///> Variable index, if the variable is a part of a vector
    int idx_;

    ///> Associated Link object pointing to twin variable in another VCD file
    const Link*link_;
};

class Vector : public Variable {
public:
    Vector(var_type_t type, int left_idx, int right_idx,
            const std::string&name = "", const std::string&identifier = "");

    ~Vector();

    std::string full_name() const;

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

    unsigned int size() const {
        assert(vec_range_size() == children_.size());

        return children_.size();
    }

    bool is_vector() const {
        return true;
    }

    void set_value(const Value&value);

    bool changed() const;
    void clear_transition();

    size_t hash() const;
    size_t prev_hash() const;

    std::string value_str() const;
    std::string prev_value_str() const;

    ///> Checks if an index fits the vector range.
    inline bool is_valid_idx(int idx) const {
        return ((left_idx_ >= idx && idx >= right_idx_) ||
                (left_idx_ <= idx && idx <= right_idx_));
    }

    /**
     * Initializes vector with scalars.
     */
    void fill();

    Variable*operator[](int idx) {
        assert(is_valid_idx(idx));
        assert(children_.count(idx));

        return children_[idx];
    }

    const Variable*operator[](int idx) const {
        assert(is_valid_idx(idx));
        assert(children_.count(idx));

        return children_.at(idx);
    }

    std::ostream&operator<<(std::ostream&out) const;

private:
    /**
     * @brief Returns size of the declared vector range.
     */
    inline unsigned int vec_range_size() const {
        return std::abs(left_idx_ - right_idx_) + 1;
    }

    /**
     * @brief Converts a vector index to value index
     * (i.e. which array element corresponds to a specific vector index).
     */
    inline unsigned int vec_to_val_idx(int idx) const {
        return range_asc() ? idx - left_idx_ : right_idx_ - idx;
    }

    ///> Is the vector range ascending?
    inline bool range_asc() const {
        return left_idx_ < right_idx_;
    }

    ///> Is the vector range descending?
    inline bool range_desc() const {
        return left_idx_ > right_idx_;
    }

    ///> Vector left range (vector [left_idx:right_idx])
    int left_idx_;

    ///> Vector right range (vector [left_idx:right_idx])
    int right_idx_;

    ///> Variables that constitute the vector
    std::map<int, Variable*> children_;
};

class Scalar : public Variable {
public:
    Scalar(var_type_t type, Value::data_type_t data_type,
            const std::string&name = "", const std::string&identifier = "");

    std::string full_name() const;

    void set_value(const Value&value) {
        prev_value_ = value_;
        value_ = value;
    }

    bool changed() const {
        // We cannot have a bool flag to notify about changes. If scalar is a
        // part of a vector, then it might have the same value assigned.
        return value_ != prev_value_;
    }

    void clear_transition() {
        prev_value_ = value_;
    }

    size_t hash() const {
        return value_.hash();
    }

    size_t prev_hash() const {
        return prev_value_.hash();
    }

    std::string value_str() const {
        return std::string(value_);
    }

    std::string prev_value_str() const {
        return std::string(prev_value_);
    }

private:
    Value value_, prev_value_;
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

    bool changed() const {
        return target_->changed();
    }

    void clear_transition() {
        target_->clear_transition();
    }

    size_t hash() const {
        return target_->hash();
    }

    size_t prev_hash() const {
        return target_->hash();
    }

    std::string value_str() const {
        return target_->value_str();
    }

    std::string prev_value_str() const {
        return target_->prev_value_str();
    }

private:
    Variable*target_;
};

typedef std::map<std::string, Variable*> VarStringMap;

std::ostream&operator<<(std::ostream&out, const Variable&var);

#endif /* VARIABLE_H */


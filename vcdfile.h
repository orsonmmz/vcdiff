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

#ifndef VCDFILE_H
#define VCDFILE_H

#include <iostream>
#include <set>
#include <string>

#include "scope.h"
#include "tokenizer.h"
#include "variable.h"

class VcdFile {
public:
    VcdFile(const char*filename);

    inline bool valid() const {
        return tokenizer_.valid();
    }

    inline const std::string&filename() const {
        return filename_;
    }

    inline int timescale() const {
        return timescale_;
    }

    bool parse_header();

    bool next_delta(std::set<const Link*>&changes);

    inline unsigned long next_timestamp() const {
        return next_timestamp_;
    }

    inline Scope&root_scope() {
        return root_;
    }

    void show_state() const;

    int line_number() const {
        return tokenizer_.line_number();
    }

private:
    inline void push_scope(Scope::scope_type_t type, const char*scope) {
        cur_scope_ = cur_scope_->make_scope(type, scope);
    }

    inline void pop_scope() {
        cur_scope_ = cur_scope_->parent();
        assert(cur_scope_);
    }

    // Parsers for specific header sections
    bool parse_enddefinitions();
    bool parse_scope();
    bool parse_timescale();
    bool parse_upscope();
    bool parse_var();

    // Generic functions to handle particular header sections
    bool parse_not_handled(const char*section);
    bool parse_skip_to_end(const char*section);

    // Skips all tokens until the next $end token
    bool skip_to_end();

    Variable::var_type_t parse_var_type(const char*token) const;

    Scope::scope_type_t parse_scope_type(const char*token) const;

    void add_variable(const char*name, const char*ident,
                      int size, Variable::var_type_t type);

    // TODO comments
    const std::string filename_;
    Tokenizer tokenizer_;
    Scope root_;
    Scope*cur_scope_;
    int timescale_;
    unsigned long cur_timestamp_, next_timestamp_;
    VarStringMap var_idents_;

    // Flag to indicate the current scope as ignored
    bool ignore_scope_;
};

#endif /* VCDFILE_H */


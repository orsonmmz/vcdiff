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

private:
    inline void parse_error(const std::string&msg) {
        std::cerr << "Error: " << filename_ << ":"
                  << tokenizer_.line_number() << ": " << msg << std::endl;
    }

    inline void parse_warn(const std::string&msg) {
        std::cerr << "Warning: " << filename_ << ":"
                  << tokenizer_.line_number() << ": " << msg << std::endl;
    }

    inline void push_scope(const char*scope) {
        cur_scope_ = cur_scope_->make_scope(scope);
    }

    inline void pop_scope() {
        cur_scope_ = cur_scope_->parent();
        assert(cur_scope_);
    }

    bool parse_timescale();

    // Skips all tokens until the next $end token
    inline bool skip_to_end() {
        while(!tokenizer_.expect("$end"));
        return tokenizer_.valid();
    }

    Variable::type_t parse_var_type(const char*token) const;

    void add_variable(const char*name, const char*ident,
                      int size, Variable::type_t type);

    // TODO comments
    const std::string filename_;
    Tokenizer tokenizer_;
    Scope root_;
    Scope*cur_scope_;
    int timescale_;
    unsigned long cur_timestamp_, next_timestamp_;
    VarStringMap var_idents_;
};

#endif /* VCDFILE_H */


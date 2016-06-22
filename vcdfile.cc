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

#include "vcdfile.h"
#include "options.h"
#include "debug.h"

#include <list>
#include <cstring>

#define PARSE_WARN(x...)\
    { fprintf(stderr, "Warning: %s:%d: ", filename().c_str(), line_number());\
        fprintf(stderr, x); fprintf(stderr, "\n"); }

#define PARSE_ERROR(x...)\
    { fprintf(stderr, "Error: %s:%d: ", filename().c_str(), line_number());\
        fprintf(stderr, x); fprintf(stderr, "\n"); }

using namespace std;

// Converts a string to lower case in-place
static void to_lower_case(char*str) {
    while(*str) {
        *str = tolower(*str);
        ++str;
    }
}

VcdFile::VcdFile(const char*filename)
    : filename_(filename), tokenizer_(filename),
    root_(Scope::BEGIN, "(" + filename_ + ")", NULL), cur_scope_(&root_),
    timescale_(0), cur_timestamp_(0), next_timestamp_(0)
{
}

bool VcdFile::parse_header() {
    assert(tokenizer_.valid());

    char*token;
    bool result;

    while(true) {
        if(tokenizer_.get(token) == 0) {
            PARSE_ERROR("unexpected end of file");
            return false;
        }

        if(!strcmp(token, "$var")) {
            result = parse_var();

        } else if(!strcmp(token, "$scope")) {
            result = parse_scope();

        } else if(!strcmp(token, "$upscope")) {
            result = parse_upscope();

        } else if(!strcmp(token, "$enddefinitions")) {
            // Finished processing the header
            return parse_enddefinitions();

        } else if(!strcmp(token, "$timescale")) {
            result = parse_timescale();

        } else if(!strcmp(token, "$version")) {
            result = parse_skip_to_end(&token[1]);

        } else if(!strcmp(token, "$comment")) {
            result = parse_skip_to_end(&token[1]);

        } else if(!strcmp(token, "$date")) {
            result = parse_skip_to_end(&token[1]);

        } else if(!strcmp(token, "$dumpvars")) {
            // Do nothing, the values are going to be initialized anyway
            result = true;

        } else if(!strcmp(token, "$dumpon")) {
            result = parse_not_handled(&token[1]);

        } else if(!strcmp(token, "$dumpoff")) {
            result = parse_not_handled(&token[1]);

        } else if(!strcmp(token, "$dumpall")) {
            result = parse_not_handled(&token[1]);

        } else {
            // Unknown token
            if(warn_unexpected_tokens) {
                PARSE_ERROR("unexpected token: %s", token);
                result = false;
            } else {
                result = true;
            }
        }

        if(!result)
            return false;
    }

    return false;
}

bool VcdFile::next_delta(set<const Link*>&changes) {
    char*token;
    unsigned long tstamp;

    while(true) {
        if(tokenizer_.get(token) == 0) {
            DBG("file %s finished", filename_.c_str());
            return false;
        }

        Value new_value;
        string ident;
        bool assign = false;

        switch(token[0]) {
            case '#':
                if(sscanf(token, "#%lu", &tstamp) != 1) {
                    PARSE_ERROR("invalid timestamp: %s", token);
                    return false;
                }

                // Skip the initial timestamp
                if(tstamp != 0) {
                    cur_timestamp_ = next_timestamp_;
                    next_timestamp_ = tstamp;
                    DBG("%s: timestamp %lu", filename_.c_str(), cur_timestamp_);
                    return true;
                }
                break;

            case '$':
                // Some simulators (e.g. Modelsim, Icarus) put '$dumpvars'
                // right after #0 timestamp, so there is no reason to
                // display a warning
                if(warn_unexpected_tokens
                        && strcmp(&token[1], "dumpvars") && cur_timestamp_ == 0) {
                    PARSE_WARN("unexpected section token: %s", token);
                }
                break;

            case 'b':
                // Get the new vector value (skip 'b', store only the new value)
                new_value = Value(string(&token[1]));

                // Get the variable identifier
                tokenizer_.get(token);
                ident = string(token);
                assign = true;
                break;

            case 'r':
                new_value = Value((float) ::atof(&token[1]));

                // Get the variable identifier
                tokenizer_.get(token);
                ident = string(token);
                assign = true;
                break;

            case '0':
            case '1':
            case 'X':
            case 'Z':
            case 'x':
            case 'z':
            {
                // Here the expected format is: one byte value, followed by
                // a variable identifier, no spaces
                assert(strlen(token) > 1);

                new_value = Value(token[0]);
                ident = string(&token[1]);
                assign = true;
                break;
            }

            default:
                assert(false);
                PARSE_WARN("invalid entry: %s", token);
                break;
        }

        if(assign) {
            VarStringMap::iterator res = var_idents_.find(ident);
            if(res == var_idents_.end()) {
                // Some of variables are currently ignored if they
                // are stored in an unsupported scope, so we have to
                // mute the error for now
                //PARSE_ERROR("invalid variable identifier: %s", ident.c_str());
                continue;
            }

            Variable*var = res->second;
            assert(var);
            var->set_value(new_value);

            const Link*link = NULL;

            if(const Variable*parent = var->parent())
                link = parent->link();

            if(!link)
                link = var->link();

            if(link)
                changes.insert(link);

            DBG("%s: %s changed to %s", filename_.c_str(),
                    var->full_name().c_str(), string(new_value).c_str());
        }
    }

    return false;
}

void VcdFile::show_state() const {
    cout << filename_ << " @ " << cur_timestamp_ << endl;

    for(VarStringMap::const_iterator it = var_idents_.begin();
            it != var_idents_.end(); ++it) {
        Variable*var = it->second;
        cout << "    " << *var << " = " << var->value_str() << endl;
    }

    cout << endl;
}

bool VcdFile::parse_enddefinitions() {
    if(!tokenizer_.expect("$end")) {
        PARSE_ERROR("expected $end for $enddefinitions section");
        return false;
    }

    DBG("%s: header correct", filename_.c_str());
    return true;
}

bool VcdFile::parse_scope() {
    Scope::scope_type_t type;
    char*token;

    tokenizer_.get(token);
    type = parse_scope_type(token);

    // Scope name
    tokenizer_.get(token);

    if(!ignore_case)
        to_lower_case(token);

    push_scope(type, token);

    if(!tokenizer_.expect("$end")) {
        PARSE_ERROR("expected $end for $scope section");
        return false;
    }

    return true;
}

bool VcdFile::parse_timescale() {
    int timebase;
    char timeunit[3];
    char*token;

    tokenizer_.get(token);
    if(sscanf(token, "%d%2s", &timebase, timeunit) == 1) {
        // there must have been a space between timebase and timeunit
        tokenizer_.get(token);
        strncpy(timeunit, token, sizeof(timeunit));
    }

    switch(timebase) {
        case 1:     timescale_ = 0; break;
        case 10:    timescale_ = 1; break;
        case 100:   timescale_ = 2; break;
        default: PARSE_ERROR("invalid timescale base: %s", token); return false;
    }

    if(!strcmp(timeunit, "fs")) {
        timescale_ -= 15;
    } else if(!strcmp(timeunit, "ps")) {
        timescale_ -= 12;
    } else if(!strcmp(timeunit, "ns")) {
        timescale_ -= 9;
    } else if(!strcmp(timeunit, "us")) {
        timescale_ -= 6;
    } else if(!strcmp(timeunit, "ms")) {
        timescale_ -= 3;
    } else if(!strcmp(timeunit, "s")) {
        // do nothing, it is just to check if timeunits are correct
    } else {
        PARSE_ERROR("invalid timescale units: %s", token);
        return false;
    }

    if(!skip_to_end()) {
        PARSE_ERROR("expected $end token for $timescale section");
        return false;
    }

    DBG("%s\ttimescale = %d", filename_.c_str(), timescale_);

    return true;
}

bool VcdFile::parse_upscope() {
    pop_scope();

    if(!tokenizer_.expect("$end")) {
        PARSE_ERROR("expected $end for $upscope section");
        return false;
    }

    return true;
}

bool VcdFile::parse_var() {
    Variable::var_type_t type;
    int size;
    char ident[8];
    char name[128] = { 0, };
    char*token;

    tokenizer_.get(token);
    type = parse_var_type(token);

    if(type == Variable::UNKNOWN) {
        PARSE_ERROR("unknown variable type: %s", token);
        return false;
    }

    tokenizer_.get(token);
    if(sscanf(token, "%d", &size) != 1) {
        PARSE_ERROR("expected variable size, but not found");
        return false;
    }

    tokenizer_.get(token);
    strncpy(ident, token, sizeof(ident));

    // Name: concatenate strings, until $end token arrives
    tokenizer_.get(token);
    while(strcmp(token, "$end") && tokenizer_.valid()) {
        strncat(name, token, sizeof(name) - strlen(name) - 1);
        tokenizer_.get(token);
    }

    if(strlen(name) == sizeof(name))
        PARSE_WARN("too long variable name, could have been clamped (%s)", token);

    if(strlen(ident) == sizeof(ident))
        PARSE_WARN("too long variable identifier, could have been clamped (%s)", token);

    if(!ignore_case)
        to_lower_case(name);

    add_variable(name, ident, size, type);

    return true;
}

bool VcdFile::parse_not_handled(const char*section) {
    PARSE_WARN("section type '%s' is not handled", section);

    return true;
}

bool VcdFile::parse_skip_to_end(const char*section) {
    if(!skip_to_end()) {
        PARSE_ERROR("expected $end token for section '%s'", section);
        return false;
    }

    return true;
}

bool VcdFile::skip_to_end() {
    while(!tokenizer_.expect("$end")) {
        // Another section detected
        if(*tokenizer_.current() == '$')
            return false;
    }

    return tokenizer_.valid();
}

Variable::var_type_t VcdFile::parse_var_type(const char*token) const {
    if(!strcasecmp(token, "reg"))       return Variable::REG;
    if(!strcasecmp(token, "wire"))      return Variable::WIRE;
    if(!strcasecmp(token, "integer"))   return Variable::INTEGER;
    if(!strcasecmp(token, "real"))      return Variable::REAL;
    if(!strcasecmp(token, "parameter")) return Variable::PARAMETER;
    if(!strcasecmp(token, "time"))      return Variable::TIME;
    if(!strcasecmp(token, "supply0"))   return Variable::SUPPLY0;
    if(!strcasecmp(token, "supply1"))   return Variable::SUPPLY1;
    if(!strcasecmp(token, "tri"))       return Variable::TRI;
    if(!strcasecmp(token, "triand"))    return Variable::TRIAND;
    if(!strcasecmp(token, "trior"))     return Variable::TRIOR;
    if(!strcasecmp(token, "trireg"))    return Variable::TRIREG;
    if(!strcasecmp(token, "tri0"))      return Variable::TRI0;
    if(!strcasecmp(token, "tri1"))      return Variable::TRI1;
    if(!strcasecmp(token, "wand"))      return Variable::WAND;
    if(!strcasecmp(token, "wor"))       return Variable::WOR;
    if(!strcasecmp(token, "event"))     return Variable::EVENT;

    return Variable::UNKNOWN;
}

Scope::scope_type_t VcdFile::parse_scope_type(const char*token) const
{
    if(!strcasecmp(token, "module"))    return Scope::MODULE;
    if(!strcasecmp(token, "begin"))     return Scope::BEGIN;
    if(!strcasecmp(token, "function"))  return Scope::FUNCTION;
    if(!strcasecmp(token, "task"))      return Scope::TASK;
    if(!strcasecmp(token, "fork"))      return Scope::FORK;

    return Scope::UNKNOWN;
}

void VcdFile::add_variable(const char*name, const char*ident,
                    int size, Variable::var_type_t type) {
    assert(size > 0 || type == Variable::REAL || type == Variable::PARAMETER);

    string base_name;
    // Size == 0 for real type variables
    int left_idx = size > 0 ? size - 1 : 0;
    int right_idx = 0;
    list<int>idxs;
    bool has_index = false;
    //bool has_range = false;

    // Check if there is an index or a range in the name
    const char*bracket = strchr(name, '[');

    if(bracket) {
        int tmp_left, tmp_right;
        if(sscanf(bracket, "[%d:%d]", &tmp_left, &tmp_right) == 2) {
            left_idx = tmp_left;
            right_idx = tmp_right;

            assert(left_idx >= 0 && right_idx >= 0);
            assert(size == std::abs(left_idx - right_idx) + 1);

            //has_range = true;
        } else {
            // Look for multiple indexes
            int tmp_idx;
            const char*cur_bracket = bracket;

            while(cur_bracket && sscanf(cur_bracket, "[%d]", &tmp_idx) == 1) {
                cur_bracket = strchr(cur_bracket + 1, '[');
                idxs.push_back(tmp_idx);
            }

            assert(idxs.size() > 0);
            has_index = true;
        }
    }

    // Copy the name without any indexes or ranges
    if(bracket)
        base_name.append(name, (int)(bracket - name));
    else
        base_name.append(name);

    // var_name is the top level variable (e.g. a vector that stores the
    // full hierarchy), var_ident is the individual variable that contains
    // the specific bits associated with an identifier
    Variable*var_name = cur_scope_->get_variable(base_name);

    // Is it a new variable or are we extending an existing vector?
    const bool new_variable = (var_name == NULL);

    // It is possible to have two variables with the same identifier if they
    // are exactly the same signal. For consistency, keep variables with
    // the shortest signal name, otherwise variable mapping might be wrong.
    VarStringMap::iterator it = var_idents_.find(ident);
    Variable*var_ident = (it == var_idents_.end() ? NULL : it->second);

    // Is it a new identifier or the variable is an alias to an existing one?
    const bool new_ident = (var_ident == NULL);

    if(!new_ident) {
        Alias*alias = new Alias(base_name, var_ident);
        alias->set_scope(cur_scope_);

        if(warn_duplicate_vars) {
            cerr << "Info: " << filename_ << ": '" << *alias
                << "' is the same signal as '" << *var_ident
                << "', creating an alias." << endl;
        }

        var_ident = alias;
    }

    if(new_variable) {
        Value::data_type_t data_type =
            (type == Variable::PARAMETER) ? Value::REAL : Value::BIT;

        switch(type) {
            case Variable::TIME:
            case Variable::INTEGER:
            case Variable::REG:
            case Variable::WIRE:
                assert(size > 0);

            case Variable::PARAMETER:
                // Parameters are stored as numbers
                if(size == 1 && !has_index) {
                    // The simplest case: a scalar
                    if(new_ident) {
                        var_name = new Scalar(type, data_type, base_name, ident);
                        var_ident = var_name;
                    } else {
                        var_name = var_ident;
                    }

                } else if(size == 1 && has_index) {
                    // Even though it is one bit wide, it is likely to
                    // be a vector, just splitted into separate variables.
                    // Once the other signals belonging to the vector occur,
                    // they will be grouped.
                    int prev_idx = idxs.front();

                    list<int>::iterator it = idxs.begin()++;
                    Vector*cur_vec = new Vector(type, prev_idx, prev_idx,
                            base_name);

                    // This is the top vector, so store it in the name map
                    var_name = cur_vec;

                    // Create vectors for all indexes in the hierarchy,
                    // but the last one - it is going to be our scalar
                    for(unsigned int i = 0; i < idxs.size() - 1; ++i) {
                        int cur_idx = *it;
                        Vector*v = new Vector(type, cur_idx, cur_idx);
                        cur_vec->add_variable(prev_idx, v);

                        cur_vec = v;
                        prev_idx = cur_idx;
                        ++it;
                    }

                    // Now add the scalar at the bottom of the hierarchy
                    if(new_ident)
                        var_ident = new Scalar(type, data_type, base_name, ident);

                    cur_vec->add_variable(idxs.back(), var_ident);

                } else if(size > 1 && has_index) {
                    // For now we support only 2-dimensional arrays
                    assert(idxs.size() == 1);

                    // Single word of a multidimensional array
                    int idx = idxs.front();

                    // Parent vector
                    Vector*top_vec = new Vector(type, idx, idx, base_name);

                    if(new_ident) {
                        // Child vector
                        Vector*vec = new Vector(type, left_idx, right_idx,
                                base_name, ident);
                        vec->fill();

                        var_ident = vec;
                    }

                    top_vec->add_variable(idx, var_ident);
                    var_name = top_vec;

                } else if(size > 1 && !has_index) {
                    // Vector of scalars, including integers
                    assert(size == std::abs(left_idx - right_idx) + 1);

                    if(new_ident) {
                        Vector*vec = new Vector(type, left_idx, right_idx,
                                base_name, ident);
                        vec->fill();

                        var_name = vec;
                        var_ident = vec;
                    } else {
                        var_name = var_ident;
                    }

                } else if(size == 0 && type == Variable::PARAMETER) {
                    // Size == 0 indicates a parameter
                    // (at least in the Modelsim land)
                    var_ident = new Scalar(type, data_type, base_name, ident);
                    var_name = var_ident;
                    size = 1;
                } else {
                    assert(false);
                }
                break;

            default:
                PARSE_ERROR("not implemented variable type, sorry");
                assert(false);
                return;
        }

    } else {
        // There is already a variable with such base_name, so it should be
        // an indexed vector. It is another variable belonging to an
        // already existing vector.
        assert(has_index);
        assert(var_name->is_vector());
        Vector*vec = static_cast<Vector*>(var_name);

        switch(type) {
            case Variable::TIME:
            case Variable::INTEGER:
            case Variable::WIRE:
            case Variable::REG:
            case Variable::PARAMETER:
                assert(size > 0);

                if(size == 1) {
                    // Go through the vectors hierarchy, add a scalar
                    // at the end. There might be missing vectors, so we
                    // add them as needed.
                    list<int>::iterator it = idxs.begin();

                    for(unsigned int i = 0; i < idxs.size() - 1; ++i) {
                        int idx = *it;

                        if(vec->is_valid_idx(idx)) {
                            vec = static_cast<Vector*>((*vec)[idx]);
                            assert(vec);
                            ++it;
                        } else {
                            int new_idx = *++it;
                            Vector*v = new Vector(type, new_idx, new_idx);
                            vec->add_variable(idx, v);
                            vec = v;
                        }
                    }

                    if(new_ident)
                        var_ident = new Scalar(type, Value::BIT, base_name, ident);

                    vec->add_variable(idxs.back(), var_ident);

                } else {
                    assert(idxs.size() == 1);

                    Vector*new_vec = new Vector( type, left_idx, right_idx,
                            base_name, ident);
                    new_vec->fill();

                    assert(new_ident);
                    var_ident = new_vec;
                    vec->add_variable(idxs.front(), var_ident);
                }
                break;

            default:
                PARSE_ERROR("not implemented variable type, sorry");
                assert(false);
                return;
        }

        DBG("%s: extended var %s\tident %s\tsize %d\tidx %d(%lu)",
                filename_.c_str(), vec->full_name().c_str(), ident, size,
                idxs.front(), idxs.size());
    }

    if(new_variable) {
        assert(var_name);
        assert(!var_name->name().empty());
        cur_scope_->add_variable(var_name);
    }

    if(new_ident) {
        assert(var_ident);
        assert(var_ident->size() == (unsigned)size);
        assert(!var_ident->ident().empty());
        var_idents_[ident] = var_ident;
        var_ident->set_scope(cur_scope_);
    }
}


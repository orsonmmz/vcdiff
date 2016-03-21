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

#include "comparator.h"
#include "link.h"
#include "vcdfile.h"
#include "options.h"
#include "debug.h"

// TODO adapt timescales if they are different

using namespace std;

Comparator::Comparator(VcdFile&file1, VcdFile&file2)
    : file1_(file1), file2_(file2) {
}

Comparator::~Comparator() {
    for(list<Link*>::iterator it = links_.begin();
            it != links_.end(); ++it) {
        delete *it;
    }
}

int Comparator::compare() {
    if(!file1_.valid()) {
        cerr << "Error opening file " << file1_.filename() << endl;
        return 1;
    }

    if(!file2_.valid()) {
        cerr << "Error opening file " << file2_.filename() << endl;
        return 1;
    }

    if(!file1_.parse_header() || !file2_.parse_header()) {
        return 2;
    }

    if(file1_.timescale() != file2_.timescale()) {
        cerr << "Warning: Compared files use different timescales." << endl;
    }

    map_signals(file1_.root_scope(), file2_.root_scope());
    check_value_changes();

    return 0;
}

void Comparator::map_signals(Scope&scope1, Scope&scope2) {
    // Go through the scope hierarchy,
    // trying to match signals in each subscope.
    ScopeStringMap::iterator scope_it1 = scope1.scopes().begin();
    ScopeStringMap::iterator scope_it2 = scope2.scopes().begin();

    DBG("mapping %s <-> %s", scope1.full_name().c_str(),
            scope2.full_name().c_str());

    while(scope_it1 != scope1.scopes().end()
            && scope_it2 != scope2.scopes().end()) {
        // Check if the current scope names match
        int comp_name = scope_it1->first.compare(scope_it2->first);

        if(comp_name == 0) {
            // Subscope names match, go deeper
            map_signals(*scope_it1->second, *scope_it2->second);
            ++scope_it1;
            ++scope_it2;

        } else if(comp_name < 0) {
            if(warn_missing_scopes) {
                cerr << "Warning: There is no scope '"
                    << scope_it1->second->full_name()
                    << "' in " << file2_.filename() << ", skipping." << endl;
            }

            ++scope_it1;

        } else { // comp_name > 0
            if(warn_missing_scopes) {
                cerr << "Warning: There is no scope '"
                    << scope_it2->second->full_name()
                    << "' in " << file1_.filename() << ", skipping." << endl;
            }

            ++scope_it2;
        }
    }

    // Handle remainding scopes
    while(scope_it1 != scope1.scopes().end()) {
        if(warn_missing_scopes) {
            cerr << "Warning: There is no scope '"
                    << scope_it1->second->full_name()
                    << "' in " << file2_.filename() << ", skipping." << endl;
        }

        ++scope_it1;
    }

    while(scope_it2 != scope2.scopes().end()) {
        if(warn_missing_scopes) {
            cerr << "Warning: There is no scope '"
                    << scope_it2->second->full_name()
                    << "' in " << file1_.filename() << ", skipping." << endl;
        }

        ++scope_it2;
    }


    // Find matching signals in the current scope
    VarStringMap::iterator var_it1 = scope1.variables().begin();
    VarStringMap::iterator var_it2 = scope2.variables().begin();

    while(var_it1 != scope1.variables().end()
            && var_it2 != scope2.variables().end()) {
        // Check if the current variable names match
        int comp_name = var_it1->first.compare(var_it2->first);

        if(comp_name == 0) {
            // Variable names match!
            compare_and_match(var_it1->second, var_it2->second);
            ++var_it1;
            ++var_it2;

        } else if(comp_name < 0) {
            if(warn_missing_vars) {
                cerr << "Warning: There is no variable '" << *var_it1->second
                    << "' in " << file2_.filename() << "." << endl;
            }

            ++var_it1;

        } else { // comp_name > 0
            if(warn_missing_vars) {
                cerr << "Warning: There is no variable '" << *var_it2->second
                    << "' in " << file1_.filename() << "." << endl;
            }

            ++var_it2;
        }
    }

    // Handle remainding variables
    while(var_it1 != scope1.variables().end()) {
        if(warn_missing_vars) {
            cerr << "Warning: There is no variable '" << *var_it1->second
                << "' in " << file2_.filename() << "." << endl;
        }

        ++var_it1;
    }

    while(var_it2 != scope2.variables().end()) {
        if(warn_missing_vars) {
            cerr << "Warning: There is no variable '" << *var_it2->second
                    << "' in " << file1_.filename() << "." << endl;
        }

        ++var_it2;
    }
}

void Comparator::check_value_changes() {
    while(file1_.valid() && file2_.valid()) {
        unsigned long next_event1 = file1_.next_timestamp();
        unsigned long next_event2 = file2_.next_timestamp();
        unsigned long current_time;
        set<const Link*> changes;

        if(next_event1 == next_event2) {
            file1_.next_delta(changes);
            file2_.next_delta(changes);
            current_time = next_event1; // == next_event2

        } else if(next_event1 > next_event2) {
            // TODO warning
            DBG("advancing file2 %s (tstamp1 = %lu tstamp2 = %lu)",
                    file2_.filename().c_str(), next_event1, next_event2);

            file2_.next_delta(changes);
            current_time = next_event2;

        } else {    // if(next_event1 < next_event2)
            // TODO warning
            DBG("advancing file1 %s (tstamp1 = %lu tstamp2 = %lu)",
                    file1_.filename().c_str(), next_event1, next_event2);

            file1_.next_delta(changes);
            current_time = next_event1;
        }

#ifdef DEBUG
        file1_.show_state();
        file2_.show_state();
#endif

        bool emitted_diff_header = false;

        for(set<const Link*>::iterator it = changes.begin();
                it != changes.end(); ++it) {
            const Link*link = *it;

            if(!link->compare()) {
                if(!emitted_diff_header) {
                    cout << "diff #" << current_time << endl;
                    cout << "==================" << endl;
                    emitted_diff_header = true;
                }

                cout << *link << endl;
            }
        }
    }
}

bool Comparator::compare_and_match(Variable*var1, Variable*var2) {
    DBG("checking match %s <-> %s",
            var1->full_name().c_str(),
            var2->full_name().c_str());

    if(var1->size() != var2->size()) {
        DBG("different sizes");
        return false;
    }

    if(!ignore_var_type && var1->type() != var2->type()) {
        DBG("different types");
        return false;
    }

    if(!ignore_var_index) {
        if(var1->size() == 1) {
            assert(var2->size() == 1);

            if(var1->index() != var2->index()) {
                DBG("different indexes");
                return false;
            }

        } else {    // Vectors
            Vector*vec1 = static_cast<Vector*>(var1);
            Vector*vec2 = static_cast<Vector*>(var2);

            if((vec1->left_idx() != vec2->left_idx())
                    || (vec1->right_idx() != vec2->right_idx())) {
                DBG("different ranges");
                return false;
            }
        }
    }

    Link*link = new Link(var1, var2);
    var1->set_link(link);
    var2->set_link(link);
    links_.push_back(link);
    DBG("linked");

    return true;
}

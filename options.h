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

#ifndef OPTIONS_H
#define OPTIONS_H

extern bool ignore_case;
extern bool ignore_var_type;
extern bool ignore_var_index;

extern bool skip_module;
extern bool skip_function;
extern bool skip_task;

extern bool warn_missing_scopes;
extern bool warn_missing_vars;
extern bool warn_missing_tstamps;
extern bool warn_duplicate_vars;
extern bool warn_unexpected_tokens;
extern bool warn_size_mismatch;
extern bool warn_type_mismatch;

extern bool compare_states;

extern bool test_mode;

#endif /* OPTIONS_H */


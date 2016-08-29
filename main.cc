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

// VCD format description:
// http://web.archive.org/web/20120323132708/http://www.beyondttl.com/vcd.php

// TODO generating a list of unmatched signals and manual matching
// by providing a file with list of names that match
// TODO genereating a vcd file containing both signals
// TODO debug levels

#include "comparator.h"
#include "vcdfile.h"

#include <cstdlib>
#include <cstring>
#include <unistd.h>

#define VERSION "1.1"

using namespace std;

struct option_t {
    const char*name;
    bool*bool_switch;
    const char*desc;
};

// Options
bool ignore_case        = false;
bool ignore_var_type    = false;
bool ignore_var_index   = false;
option_t ignore_options[] = {
    { "case",   &ignore_case,
        "Case-insensitive variable matching (e.g. variable to VaRiAbLe)." },
    { "type",   &ignore_var_type,
        "Enable matching different, but compatible types (e.g. integer to reg[31:0])." },
    { "index",  &ignore_var_index,
        "Enable matching variables of the same size, but different index ranges (e.g. reg[3:0] to reg[4:1])." },
    { NULL, NULL }
};

bool skip_module        = false;
bool skip_function      = false;
bool skip_task          = false;
option_t skip_options[] = {
    { "module",     &skip_module,   "\tSkip module scopes." },
    { "function",   &skip_function, "Skip function scopes." },
    { "task",       &skip_task,     "\tSkip task scopes." }
};

bool warn_missing_scopes    = true;
bool warn_missing_vars      = true;
bool warn_missing_tstamps   = true;
bool warn_duplicate_vars    = true;
bool warn_unexpected_tokens = true;
bool warn_size_mismatch     = true;
bool warn_type_mismatch     = true;
option_t warn_options[] = {
    { "no-missing-scope",  &warn_missing_scopes,
        "Do not warn about scopes that do not occur in one of the files." },
    { "no-missing-var",    &warn_missing_vars,
        "\tDo not warn about variables that do not occur in one of the files." },
    { "no-missing-tstamp", &warn_missing_tstamps,
        "Do not warn about timestamps that do not occur in one of the files." },
    { "no-alias",        &warn_duplicate_vars,
        "\tDo not warn about duplicated variables (it is normal in VCD files)." },
    { "no-unexp-token",    &warn_unexpected_tokens,
        "\tDo not warn about unexpected tokens." },
    { "no-size-mismatch",  &warn_size_mismatch,
        "Do not warn about variable size mismatch." },
    { "no-type-mismatch",  &warn_type_mismatch,
        "Do not warn about variable type mismatch." },
    { NULL, NULL }
};

bool compare_states = false;
bool test_mode = false;

//bool show_unmatched_vars = false;    // TODO
//bool match_individual_scalars = false; // TODO

static void disable_all(option_t* options) {
    for(option_t*opt_ptr = options; opt_ptr->name; ++opt_ptr) {
        *opt_ptr->bool_switch = false;
    }
}

static void enable_all(option_t* options) {
    for(option_t*opt_ptr = options; opt_ptr->name; ++opt_ptr) {
        *opt_ptr->bool_switch = true;
    }
}

int main(int argc, char*argv[]) {
    option_t*opt_ptr = NULL;
    int opt;

    if(argc < 3 || !strcmp(argv[1], "--help")) {
        cerr << "vcdiff " << VERSION << " by Maciej Suminski <maciej.suminski@cern.ch>" << endl;
        cerr << "(c) CERN 2016" << endl;
        cerr << "Usage: vcdiff [options] file1.vcd file2.vcd" << endl;
        cerr << endl;

        cerr << "Options: " << endl;

        cerr << "-s\t\t\t\tCompares states instead of transitions." << endl;

        cerr << endl;
        cerr << "-r<flag>\t\t\tModifies rules when mapping variables between files, "
            "<flag> might be:" << endl;
        for(opt_ptr = ignore_options; opt_ptr->name; ++opt_ptr)
            cerr << "\t" << opt_ptr->name << "\t\t\t" << opt_ptr->desc << endl;
        cerr << "\tall\t\t\tApplies all above rules." << endl;

        cerr << endl;
        cerr << "-S<flag>\t\t\tSkips certain scopes, "
            "<flag> might be:" << endl;
        for(opt_ptr = skip_options; opt_ptr->name; ++opt_ptr)
            cerr << "\t" << opt_ptr->name << "\t\t" << opt_ptr->desc << endl;

        cerr << endl;
        cerr << "-W<flag>\t\t\tDisables certain warnings, <flag> might be:" << endl;
        for(opt_ptr = warn_options; opt_ptr->name; ++opt_ptr)
            cerr << "\t" << opt_ptr->name << "\t" << opt_ptr->desc << endl;
        cerr << "\tno-all\t\t\tDisables all warnings." << endl;

        return 0;
    }

    while((opt = getopt(argc, argv, "r:S:W:s")) != -1) {
        switch(opt) {
            case 'r':
                for(opt_ptr = ignore_options; opt_ptr->name; ++opt_ptr) {
                    if(!strcmp(optarg, opt_ptr->name)) {
                        *opt_ptr->bool_switch = true;
                        break;
                    }
                }

                // Enable all rules
                if(!strcmp(optarg, "all"))
                    enable_all(ignore_options);
                break;

            case 'S':
                for(opt_ptr = skip_options; opt_ptr->name; ++opt_ptr) {
                    if(!strcmp(optarg, opt_ptr->name)) {
                        *opt_ptr->bool_switch = true;
                        break;
                    }
                }
                break;

            case 'W':
                for(opt_ptr = warn_options; opt_ptr->name; ++opt_ptr) {
                    if(!strcmp(optarg, opt_ptr->name)) {
                        *opt_ptr->bool_switch = false;
                        break;
                    }
                }

                // Disable all warnings
                if(!strcmp(optarg, "no-all"))
                    disable_all(warn_options);
                break;

            case 's':
                compare_states = true;
                break;
        }
    }

    if(getenv("TEST_VCDIFF")) {
        disable_all(warn_options);
        test_mode = true;
    }

    VcdFile file1(argv[argc-2]);
    VcdFile file2(argv[argc-1]);

    Comparator comp(file1, file2);
    comp.compare();

    return 0;
}


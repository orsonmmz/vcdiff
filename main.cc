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

#include <cstring>
#include <unistd.h>

#define VERSION "1.0b"

using namespace std;

struct option {
    const char*name;
    bool*bool_switch;
    const char*desc;
};

// Options
bool ignore_case        = false;
bool ignore_var_type    = false;
bool ignore_var_index   = false;
option ignore_options[] = {
    { "case",   &ignore_case,
        "Case-insensitive name mapping (e.g. variable to VaRiAbLe)." },
    { "type",   &ignore_var_type,
        "Enable mapping different types (e.g. integer to reg[31:0])." },
    { "index",  &ignore_var_index,
        "Enable mapping different index ranges (e.g. reg[3:0] to reg[4:1])." },
    { NULL, NULL }
};

bool warn_missing_scopes    = true;
bool warn_missing_vars      = true;
bool warn_missing_time      = true;
bool warn_duplicate_vars    = true;
bool warn_unexpected_tokens = true;
option warn_options[] = {
    { "missing-scope",  &warn_missing_scopes,
        "Do not warn about scopes that do not occur in one of the files." },
    { "missing-var",    &warn_missing_vars,
        "Do not warn about variables that do not occur in one of the files." },
    //{ "missing-time",   &warn_missing_time,
        //"Do not warn about timestamps that do not occur in one of the files." },
    { "dup-var",        &warn_duplicate_vars,
        "\tDo not warn about duplicated variables (it is normal in VCD files)." },
    { "unexp-token",    &warn_unexpected_tokens,
        "Do not warn about unexpected tokens." },
    { NULL, NULL }
};

//bool show_unmatched_vars = false;    // TODO
//bool match_individual_scalars = false; // TODO

int main(int argc, char*argv[]) {
    option*opt_ptr = NULL;
    int opt;

    if(argc < 3 || !strcmp(argv[1], "--help")) {
        cerr << "vcdiff " << VERSION << " (c) CERN 2016" << endl;
        cerr << "Usage: vcdiff [options] file1.vcd file2.vcd" << endl;
        cerr << endl;

        cerr << "Options: " << endl;
        cerr << "-i<flag>\tModifies rules when mapping variables between files, "
            "<flag> might be:" << endl;
        for(opt_ptr = ignore_options; opt_ptr->name; ++opt_ptr)
            cerr << "\t" << opt_ptr->name << "\t" << opt_ptr->desc << endl;

        cerr << endl;
        cerr << "-W<flag>\tDisables certain warnings, <flag> might be:" << endl;
        for(opt_ptr = warn_options; opt_ptr->name; ++opt_ptr)
            cerr << "\t" << opt_ptr->name << "\t" << opt_ptr->desc << endl;

        return 0;
    }

    while((opt = getopt(argc, argv, "i:W:")) != -1) {
        bool valid = false;

        switch(opt) {
            case 'i':
                for(opt_ptr = ignore_options; opt_ptr->name; ++opt_ptr) {
                    if(!strcmp(optarg, opt_ptr->name)) {
                        *opt_ptr->bool_switch = true;
                        valid = true;
                        break;
                    }
                }
                break;

            case 'W':
                for(opt_ptr = warn_options; opt_ptr->name; ++opt_ptr) {
                    if(!strcmp(optarg, opt_ptr->name)) {
                        *opt_ptr->bool_switch = false;
                        valid = true;
                        break;
                    }
                }
                break;
        }

        if(!valid) {
            cerr << "unrecognized option -" << (char)opt;

            if(optarg)
                cerr << optarg;

            cerr << endl;
            return 0;
        }
    }

    VcdFile file1(argv[argc-2]);
    VcdFile file2(argv[argc-1]);

    Comparator comp(file1, file2);
    comp.compare();

    return 0;
}


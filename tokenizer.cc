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

#include "tokenizer.h"

using namespace std;

Tokenizer::Tokenizer(const string&filename)
    : file_(filename.c_str()), line_number_(0)
{
    // Mark buffer as empty
    buf_[0] = 0;
    buf_ptr_ = buf_;
}

Tokenizer::~Tokenizer() {
    file_.close();
}

bool Tokenizer::get(char*dest, int max_len) {
    if(!fill_if_empty())
        return false;

    skip_whitespace();

    int i = 0;
    while(i < max_len - 1 && *buf_ptr_ && !isblank(*buf_ptr_)) {
        *dest = *buf_ptr_;
        ++buf_ptr_;
        ++dest;
        ++i;
    }

    // Null-terminate
    *dest = 0;

    return valid();
}

bool Tokenizer::expect(const char*token) {
    if(!fill_if_empty())
        return false;

    skip_whitespace();

    // Here could be strcmp, but we need to move buf_ptr_ later,
    // so save some time first comparing string, and then computing its length
    while(*buf_ptr_ && *token) {
        if(*buf_ptr_ == *token) {
            ++buf_ptr_;
            ++token;
        } else {
            // Token does not match the expectation, so keep moving
            // the pointer until the token is consumed
            while(*buf_ptr_ && !isblank(*buf_ptr_)) ++buf_ptr_;
            return false;
        }
    }

    // Check if we compared all the token characters
    return (*token == 0);
}

bool Tokenizer::fill_if_empty() {
    if(*buf_ptr_ == 0 || *buf_ptr_ == '\n' || *buf_ptr_ == '\r') {
        buf_ptr_ = buf_;
        buf_[0] = 0;

        // Try until we get non-whitespace characters or the file is finished
        while(buf_ptr_[0] == 0 && file_.good()) {
            ++line_number_;
            file_.getline(buf_, sizeof(buf_));
            skip_whitespace();
        }
    }

    return file_.good();
}

void Tokenizer::skip_whitespace() {
    while(*buf_ptr_ && isblank(*buf_ptr_)) ++buf_ptr_;
}

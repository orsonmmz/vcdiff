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

#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <fstream>
#include <string>

class Tokenizer {
public:
    Tokenizer(const std::string&filename);
    ~Tokenizer();

    /*
     * @brief Gets a token (an array of characters without any whitespaces).
     * @param dest is a pointer that will be set to the obtained token or NULL
     * in case of failure.
     * @return int number of read characters.
     */
    int get(char*&dest);
    bool expect(const char*token);

    inline int line_number() const {
        return line_number_;
    }

    inline bool valid() const {
        return file_.good();
    }

private:
    // Loads another line if the buffer is empty
    bool fill_if_empty();

    // Strip whitespaces from the buffer beginning
    void skip_whitespace();

    // Handle to the processed file
    std::ifstream file_;

    // Current line number in the processed file
    int line_number_;

    // Buffer to store the currently processed line
    char buf_[1024];

    // Pointer to the next token
    char*buf_ptr_;
};

#endif /* TOKENIZER_H */


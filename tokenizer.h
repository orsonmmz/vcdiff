/*
 * Copyright CERN 2016-2017
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
#include <cassert>

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

    /*
     * @brief Reverts the last get() operation. Only the last token can be
     * reverted.
     * @return True in case of success.
     */
    bool put() {
        if(!buf_prev_)
            return false;

        buf_cur_ = buf_prev_;
        buf_prev_ = nullptr;
        return true;
    }

    /*
     * @brief Gets a token, compares with the expected one and returns
     * the comparison result (true if there is a match).
     * @param token is the expected token.
     */
    bool expect(const char*token);

    /*
     * @brief Returns the current token, without reading another one.
     */
    inline const char*peek() {
        skip_whitespace();
        return buf_cur_;
    }

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

    /*
     * @brief Doubles the buffer size, preserves the buffer contents.
     * @return Pointer to the empty memory chunk
     */
    char*inc_buffer();

    // Handle to the processed file
    std::ifstream file_;

    // Current line number in the processed file
    int line_number_;

    // Buffer to store the currently processed line
    char*buf_;

    // Pointer to the current token
    char*buf_cur_;

    // Pointer to the previous token
    char*buf_prev_;

    // Pointer to the next token
    char*buf_ptr_;

    // Current buffer size
    int buf_size_;
};

#endif /* TOKENIZER_H */


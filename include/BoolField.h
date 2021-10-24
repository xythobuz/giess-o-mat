/*
 * Copyright (c) 2021 Thomas Buck <thomas@xythobuz.de>
 *
 * This file is part of Giess-o-mat.
 *
 * Giess-o-mat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Giess-o-mat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Giess-o-mat.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef _BOOL_FIELD_H_
#define _BOOL_FIELD_H_

class BoolField {
public:
    BoolField(int size_);
    ~BoolField(void);
    
    void clear(void);
    void set(int n);
    bool isSet(int n);
    int countSet(void);
    int getFirstSet(void);
    
private:
    int size;
    bool *field;
};

#endif // _BOOL_FIELD_H_

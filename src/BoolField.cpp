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

#include "BoolField.h"

BoolField::BoolField(int size_) {
    size = size_;
    field = new bool[size];
}

BoolField::~BoolField(void) {
    delete field;
}

void BoolField::clear(void) {
    for (int i = 0; i < size; i++) {
        field[i] = false;
    }
}

void BoolField::set(int n, bool value) {
    field[n] = value;
}

bool BoolField::isSet(int n) {
    return field[n];
}

int BoolField::countSet(void) {
    int c = 0;
    for (int i = 0; i < size; i++) {
        if (field[i]) {
            c++;
        }
    }
    return c;
}

int BoolField::getFirstSet(void) {
    for (int i = 0; i < size; i++) {
        if (field[i]) {
            return i;
        }
    }
    return -1;
}

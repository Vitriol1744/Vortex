//
// Created by Vitriol1744 on 01.07.2021.
//
#pragma once

template<class T>
class NonCopyable
{
    protected:
        NonCopyable() {}

    private:
        NonCopyable(const NonCopyable& x) = delete;
        T& operator=(const T& x) = delete;
};

//
// Created by Vitriol1744 on 05.09.2022.
//
#ifndef VORTEX_NONCOPYABLE_HPP
#define VORTEX_NONCOPYABLE_HPP

namespace Vortex
{
    template<typename T>
    class NonCopyable
    {
        protected:
            NonCopyable() { }

        private:
            NonCopyable(const NonCopyable& object) = delete;
            T& operator=(const T& object) = delete;
    };
}

#endif //VORTEX_NONCOPYABLE_HPP

//
// Created by Vitriol1744 on 05.09.2022.
//
#ifndef VORTEX_SINGLETON_HPP
#define VORTEX_SINGLETON_HPP

namespace Vortex
{
    template<typename T>
    class Singleton
    {
        public:
            static T* GetInstance()
            {
                static T instance;

                return &instance;
            }

        protected:
            Singleton() = default;
            ~Singleton() = default;

        private:
            Singleton(Singleton const&) = default;
            Singleton& operator=(Singleton const&) = default;
    };
}

#endif //VORTEX_SINGLETON_HPP

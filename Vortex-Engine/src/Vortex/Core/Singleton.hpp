//
// Created by Vitriol1744 on 24.06.2021.
//
#pragma once

namespace Vortex
{
    template<class T>
    class Singleton
    {
        public:
        
            // Gets instance of an object
            static T* Instance()
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
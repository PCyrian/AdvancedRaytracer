/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

#ifndef RAYTRACER_HPP_
#define RAYTRACER_HPP_

#include "../rendering/Scene.hpp"
#include "../rendering/PostFX.hpp"
#include "Utils.hpp"

class Raytracer {
    public:
        Raytracer(int ac, char **av);
        ~Raytracer();

        
        class Error : public std::exception {
            public:
                Error(const std::string &msg = "") : _msg(msg) {};
                const char *what() const noexcept
                {
                    return this->_msg.c_str();
                }
            private:
                std::string _msg;
        };


        void run();
    protected:
    private:
        int _ac;
        char **_av;
};

#endif /* !RAYTRACER_HPP_ */

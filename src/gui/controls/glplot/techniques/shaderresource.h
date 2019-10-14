//
// Created by Jon on 14/10/2019.
//

#ifndef CLTEM_SHADERRESOURCE_H
#define CLTEM_SHADERRESOURCE_H

#include <QtOpenGL> // not the right header? (but it works)

//// Singleton to create shaders resource
class AutoShaderResource
{
public:
    AutoShaderResource() {Q_INIT_RESOURCE(shaders);};
    AutoShaderResource &operator=(AutoShaderResource const &rhs) = delete;

    AutoShaderResource(AutoShaderResource const& copy) = delete;

    ~AutoShaderResource() { Q_CLEANUP_RESOURCE(shaders); }
    inline static AutoShaderResource& GetInstance() { static AutoShaderResource instance; return instance; }
};

#endif //CLTEM_SHADERRESOURCE_H

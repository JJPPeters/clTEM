//
// Created by Jon on 03/09/2018.
//

#ifndef CLTEM_TECHNIQUE_H
#define CLTEM_TECHNIQUE_H

#include <QtOpenGL>

#include <Eigen/Dense>

namespace PGL {
    class Technique {
    public:
        explicit Technique(bool visible = true);

        virtual ~Technique() = default;

        void setVisible(bool visible) { _visible = visible; }
        bool getVisible() { return _visible; }

        virtual void render(const Eigen::Matrix4f &MV, const Eigen::Matrix4f &P, float pix_size) = 0;

        Eigen::Matrix<float, 3, 2> getLimits() {
            if (_visible)
                return _limits;
            else
                return Eigen::Matrix<float, 3, 2>::Zero();
        }

    protected:
        Eigen::Matrix<float, 3, 2> _limits;

    private:
        bool _visible;
    };
}



#endif //CLTEM_TECHNIQUE_H

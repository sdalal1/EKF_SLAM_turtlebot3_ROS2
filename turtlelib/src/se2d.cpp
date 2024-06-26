#include "turtlelib/se2d.hpp"
#include <iostream>

namespace turtlelib{
    std::ostream & operator<<(std::ostream & os, const Twist2D & tw){
        return os<<"["<<tw.omega<<" "<<tw.x<<" "<<tw.y<<"]";
    }
    std::istream & operator>>(std::istream & is, Twist2D & tw){
        char c1 = is.get();
        if (c1 == '['){
            is >>tw.omega >> tw.x >> tw.y >> c1;
        }
        else{
            is.putback(c1);
            is >>tw.omega >> tw.x >> tw.y;
        }
        return is;
    }
    Transform2D::Transform2D(){
        t.x = 0.0;
        t.y = 0.0;
        ang=0.0;
    }
    Transform2D::Transform2D(Vector2D trans){
        t.x = trans.x;
        t.y = trans.y;
        ang=0.0;
    }
    Transform2D::Transform2D(double radians){
        t.x = 0.0;
        t.y = 0.0;
        // ang=normalize_angle(radians);
        ang=radians;

    }
    Transform2D::Transform2D(Vector2D trans, double radians){
        t.x = trans.x;
        t.y = trans.y;
        // ang=normalize_angle(radians);
        ang=radians;


    }
    Point2D Transform2D::operator()(Point2D p) const{
        Point2D pt;
        pt.x=(p.x*cos(ang))-(p.y*sin(ang))+t.x;
        pt.y=(p.y*cos(ang))+(p.x*sin(ang))+t.y;
        return pt;
    }
    Vector2D Transform2D::operator()(Vector2D v) const{
        Vector2D vec;
        vec.x=(v.x*cos(ang))-(v.y*sin(ang));
        vec.y=(v.y*cos(ang))+(v.x*sin(ang));
        return vec;
    }
    Twist2D Transform2D::operator()(Twist2D v) const{
        Twist2D tw;
        tw.omega = v.omega;
        tw.x = (v.x*cos(ang))-(v.y*sin(ang))+(v.omega*t.y);
        tw.y = (v.x*sin(ang))+(v.y*cos(ang))-(v.omega*t.x);
        return Twist2D{tw.omega,tw.x,tw.y};
    }
    Transform2D Transform2D::inv() const{
        Transform2D i;
        i.t.x = (-t.x*cos(ang))-(t.y*sin(ang));
        i.t.y = (-t.y*cos(ang))+(t.x*sin(ang));
        i.ang = -ang;
        return i;
    }

    Transform2D & Transform2D::operator*=(const Transform2D & rhs){
        t.x = t.x+(rhs.t.x*cos(ang))-(rhs.t.y*sin(ang));
        t.y = t.y+(rhs.t.x*sin(ang))+(rhs.t.y*cos(ang));
        ang = ang+(rhs.ang);
        return *this;
    }

    Vector2D Transform2D::translation() const{
        return t;
    }

    double Transform2D::rotation() const{
        return ang;
    }

    Transform2D integrate_twist(Twist2D twi){
        //Entire calculation in docs/kinematics.pdf for reference
        auto w = twi.omega;
        auto x = twi.x;
        auto y = twi.y;

        if(almost_equal(w,0.0,1e-5) == true){
            Transform2D Tbbprime(Vector2D{x,y});
            return Tbbprime;
        }
        else{
            Transform2D Tsb(Vector2D{y/w,-x/w});
            Transform2D Tssprime(w);
            Transform2D Tsprimebprime(Vector2D{y/w,-x/w});
            //Using formula 7 in kinematics.pdf Begin
            auto Tbs = Tsb.inv();
            auto Tbsprime = Tbs * Tssprime;
            auto Tbbprime = Tbsprime * Tsprimebprime;
            return Tbbprime; 
        }
    }

    std::ostream & operator<<(std::ostream & os, const Transform2D & tf){
        return os<<"deg:"<<" "<<rad2deg(tf.ang)<<" "<<"x:"<<" "<<tf.t.x<<" "<<"y:"<<" "<<tf.t.y;
    }

    std::istream & operator>>(std::istream & is, Transform2D & tf){
        char c1 = is.get();
        std::string c3 ,c4, c5;
        double deg ;
        Vector2D v ;

        if (c1 == 'd'){

            is>>c3>>deg>>c4>>v.x>>c5>>v.y; ///NOTE: Not right. Need to work on this.
        }
        else{
            is.putback(c1);
            is >>deg>>v.x>>v.y;
        }
        Transform2D new_o{v,deg2rad(deg)};
        tf = new_o;
        return is;
    }

    Transform2D operator*(Transform2D lhs, const Transform2D & rhs){
        lhs *= rhs;
        return lhs;
    }
}
#ifndef LAGRANGE_ALLOCATOR_H
#define LAGRANGE_ALLOCATOR_H

#include "ros/ros.h"
#include <Eigen/Dense>
#include "geometry_msgs/Wrench.h"
#include "uranus_dp/ThrusterForces.h"

class LagrangeAllocator
{
public:
    LagrangeAllocator();
    void tauCallback(const geometry_msgs::Wrench& tauMsg);
    void setWeights(const Eigen::MatrixXd &W_new);
private:
    ros::NodeHandle nh;
    ros::Subscriber tauSub;
    ros::Publisher  uPub;

    // Move to param server
    unsigned int n; // Number of control forces (length of tau)
    unsigned int r; // Number of control inputs (length of u)

    Eigen::Matrix<double,4,1> tau; // (n) Control forces (forces and moments on the ROV)
    Eigen::Matrix<double,6,1> u;   // (r) Control inputs (forces for each thruster)

    Eigen::Matrix<double,6,6> W;            // (r*r) Control force weight matrix
    Eigen::Matrix<double,6,6> K;            // (r*r) Thrust coefficient matrix
    Eigen::Matrix<double,6,6> K_inverse;    // (r*r) Inverse of K
    Eigen::Matrix<double,4,6> T;            // (n*r) Thrust configuration matrix
    Eigen::Matrix<double,6,4> T_geninverse; // (r*n) Generalized inverse of T

    void computeGeneralizedInverse();
};

#endif

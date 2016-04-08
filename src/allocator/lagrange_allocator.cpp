// See Fossen 2011, chapter 12.3.2
#include "lagrange_allocator.h"
#include <iostream>

template<typename Derived>
inline bool is_fucked(const Eigen::MatrixBase<Derived>& x)
{
    return !((x.array() == x.array())).all() && !( (x - x).array() == (x - x).array()).all();
}

// Worlds worst print function :DDD
void printMatrix6x6(Eigen::MatrixXd m){
    for(int col = 0; col < 6; col++){

        char buffer[512];

        for(int row = 0; row < 6; row++){
            snprintf( (buffer + (row*8) ), sizeof(buffer), "  %f  ", m(col, row));
        }
        ROS_INFO("%s", buffer);
    }
}


LagrangeAllocator::LagrangeAllocator()
{
    n = 4;
    r = 6;

    tauSub = nh.subscribe("controlForces", 1, &LagrangeAllocator::tauCallback, this);
    uPub   = nh.advertise<uranus_dp::ThrusterForces>("controlInputs", 1);

    W.setIdentity(6,6); // Default to identity (i.e. no weights)
    K.setIdentity(6,6); // Scaling is done on Arduino, so this can be identity

    // Our test ROV cannot control heave and roll, so they are removed from the control objective
    T <<  0.7071 ,  0.7071 ,  0.7071 ,  0.7071 ,  1    ,  1    , // Surge
         -0.7071 ,  0.7071 , -0.7071 ,  0.7071 ,  0    ,  0    , // Sway
          0.06718,  0.06718,  0.06718,  0.06718, -0.210, -0.210, // Pitch
          0.4172 ,  0.4172 , -0.4172 , -0.4172 , -0.165,  0.165; // Yaw

    K_inverse = K.inverse();
    computeGeneralizedInverse();
}

void LagrangeAllocator::tauCallback(const geometry_msgs::Wrench& tauMsg)
{
    tau << tauMsg.force.x,
           tauMsg.force.y,
           // tauMsg.force.z,  // Not part of control objective for test rov
           // tauMsg.torque.x, // Ditto
           tauMsg.torque.y,
           tauMsg.torque.z;

    u = K_inverse * T_geninverse * tau;

    if(is_fucked(K_inverse))
    {
        ROS_WARN("K is not invertible");
    }

    uranus_dp::ThrusterForces uMsg;
    uMsg.F1 = u(0);
    uMsg.F2 = u(1);
    uMsg.F3 = u(2);
    uMsg.F4 = u(3);
    uMsg.F5 = u(4);
    uMsg.F6 = u(5);
    uPub.publish(uMsg);

    ROS_INFO_STREAM("Published: " <<u(0) << ", " << u(1) << ", " << u(2) << ", " << u(3) << ", " << u(4) << ", " << u(5));
}

void LagrangeAllocator::setWeights(const Eigen::MatrixXd &W_new)
{
    bool correctDimensions = ( W_new.rows() == r && W_new.cols() == r );
    if (!correctDimensions)
    {
        ROS_WARN_STREAM("Attempt to set weight matrix in LagrangeAllocator with wrong dimensions " << W_new.rows() << "*" << W_new.cols() << ".\n");
        return;
    }

    W = W_new; // I have checked that Eigen does a deep copy here

    // New weights mean we must recompute the generalized inverse of the  matrix
    computeGeneralizedInverse();
}

void LagrangeAllocator::computeGeneralizedInverse()
{
    T_geninverse = W.inverse()*T.transpose() * (T*W.inverse()*T.transpose()).inverse();

    if(is_fucked(T_geninverse))
    {
        ROS_WARN("T_geninverse NAN");
    }

    if(is_fucked( W.inverse() ))
    {
        ROS_WARN("W is not invertible");
    }

    if(is_fucked( (T*W.inverse()*T.transpose()).inverse() ) )
    {
        ROS_WARN("T * W_inv * T transposed is not invertible");
    }
} 

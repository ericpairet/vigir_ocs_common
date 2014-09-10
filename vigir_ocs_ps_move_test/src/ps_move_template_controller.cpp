#include "ps_move_template_controller.h"

#define _USE_MATH_DEFINES
using namespace std;

namespace vigir_ocs
{

PSMoveTemplateController::PSMoveTemplateController()
{
    // create publishers for visualization

    // get list of all templates and their poses
    template_list_sub_ = nh_.subscribe<flor_ocs_msgs::OCSTemplateList>("/template/list", 5, &PSMoveTemplateController::processTemplateList, this);

    // get object that is selected
    select_object_sub_ = nh_.subscribe<flor_ocs_msgs::OCSObjectSelection>( "/flor/ocs/object_selection", 5, &PSMoveTemplateController::processObjectSelection, this );

    // update template position
    template_update_pub_  = nh_.advertise<flor_ocs_msgs::OCSTemplateUpdate>( "/template/update", 1, false );

    received_pose_ = false;

    camera_sub_ = nh_.subscribe<flor_ocs_msgs::OCSCameraTransform>( "/flor/ocs/camera_transform",5,&PSMoveTemplateController::cameraCb,this);
}

PSMoveTemplateController::~PSMoveTemplateController()
{
}

//copy camera information and store
void PSMoveTemplateController::cameraCb(const flor_ocs_msgs::OCSCameraTransform::ConstPtr& msg)
{
    if(msg->widget_name == "MainView" && msg->view_id == 0)
    {
        camera_update_ = *msg;

        //update camera geometry
        camera_position_.setX(camera_update_.pose.position.x);
        camera_position_.setY(camera_update_.pose.position.y);
        camera_position_.setZ(camera_update_.pose.position.z);

        camera_orientation_.setX(camera_update_.pose.orientation.x);
        camera_orientation_.setY(camera_update_.pose.orientation.y);
        camera_orientation_.setZ(camera_update_.pose.orientation.z);
        camera_orientation_.setScalar(camera_update_.pose.orientation.w);
    }
}

void PSMoveTemplateController::processTemplateList(const flor_ocs_msgs::OCSTemplateList::ConstPtr &list)
{
    template_id_list_ = list->template_id_list;

    //Find the pose within the list we just got
    for(int i = 0; i < template_id_list_.size(); i++)
    {
        if((int)template_id_list_[i] == template_selected_id_)
        {
            //Get pose stamped from the array of poses given
            template_pose_ = list->pose[i];
            received_pose_ = true;
        }
    }
}

void PSMoveTemplateController::processObjectSelection(const flor_ocs_msgs::OCSObjectSelection::ConstPtr &msg)
{
    //Get id of object that is selected
    switch(msg->type)
    {
        case flor_ocs_msgs::OCSObjectSelection::TEMPLATE:
            {
            if(msg->id != template_selected_id_)
                received_pose_ = false;
            template_selected_id_ = msg->id;
            }
            break;
        // not a template
        case flor_ocs_msgs::OCSObjectSelection::FOOTSTEP:
            {

            }
            break;
        case flor_ocs_msgs::OCSObjectSelection::END_EFFECTOR:
        default:
            break;
    }
}

void quatToEuler(QQuaternion q1, float& heading, float& attitude, float& bank)
{
    double test = q1.x()*q1.y() + q1.z()*q1.scalar();
    if (test > 0.499) // singularity at north pole
    {
        heading = 2 * atan2(q1.x(),q1.scalar());
        attitude = M_PI/2;
        bank = 0;
        return;
    }
    if (test < -0.499) // singularity at south pole
    {
        heading = -2 * atan2(q1.x(),q1.scalar());
        attitude = - M_PI/2;
        bank = 0;
        return;
    }
    double sqx = q1.x()*q1.x();
    double sqy = q1.y()*q1.y();
    double sqz = q1.z()*q1.z();
    heading = 57.2957795*(atan2(2*q1.y()*q1.scalar()-2*q1.x()*q1.z() , 1 - 2*sqy - 2*sqz));
    attitude = 57.2957795*(asin(2*test));
    bank = 57.2957795*(atan2(2*q1.x()*q1.scalar()-2*q1.y()*q1.z() , 1 - 2*sqx - 2*sqz));
}

geometry_msgs::PoseStamped PSMoveTemplateController::updatePose(geometry_msgs::PoseStamped template_pose, MoveServerPacket *move_server_packet, float normalized_scale)
{
    //Calculate scale
    QVector3D cam(camera_position_);
    QVector3D obj(template_pose.pose.position.x,template_pose.pose.position.y,template_pose.pose.position.z);
    double distance = (cam - obj).length();
    ROS_ERROR("distance: %f   vfov: %f    center distance: %f", distance, camera_update_.vfov, cos(camera_update_.vfov) * distance);
    double scale = (cos(camera_update_.vfov) * distance) * 1.0 / 5.0; // 5m is scale 1.0

    float rotation_scale = normalized_scale;// * scale;
    float position_scale = 0.01 * normalized_scale * scale; // move position reported in mm, so scale it down (right now it's 10x real-world scale)

    //Translation- move relative to camera for direction but still translate in world space
    //create a vector representing the offset to be applied to position.
    QVector3D* offset = new QVector3D((move_server_packet->state[0].handle_pos[0] - old_move_position_.x()) * position_scale,
                                      (move_server_packet->state[0].handle_pos[1] - old_move_position_.y()) * position_scale,
                                      (move_server_packet->state[0].handle_pos[2] - old_move_position_.z()) * position_scale);
    //rotate the offset to be relative to camera orientation, can now be applied to position to move relative to camera
    *offset = camera_orientation_.rotatedVector(*offset);

    //Update x, y, and z values
    // NEED TO RESTORE THESE FOR POSITION CONTROL
    //template_pose.pose.position.x += offset->x();
    //template_pose.pose.position.y += offset->y();
    //template_pose.pose.position.z += offset->z();

    //Update the rotation
    QQuaternion pre;
    pre.setScalar(template_pose.pose.orientation.w);
    pre.setX(template_pose.pose.orientation.x);
    pre.setY(template_pose.pose.orientation.y);
    pre.setZ(template_pose.pose.orientation.z);

    float x,y,z;
    ROS_ERROR("pre: %.3f, %.3f, %.3f, %.3f",pre.x(),pre.y(),pre.z(),pre.scalar());

    QQuaternion difference;
    QQuaternion identity;
    //camera absolute rotation
    //QQuaternion rot(move_server_packet->state[0].quat[3],move_server_packet->state[0].quat[0],move_server_packet->state[0].quat[1],move_server_packet->state[0].quat[2]);
    // 1deg/update
    QQuaternion rot(0.9999619230641713,-0.008726535498373935,0,0);
    QQuaternion c = QQuaternion::fromAxisAndAngle(0,1,0,90);
    rot.normalize();
    camera_orientation_.normalize();

    // conversions to make it easier to read angles
    quatToEuler(rot, y, z, x);
    ROS_ERROR("rot: %.3f, %.3f, %.3f",x,y,z);
    quatToEuler(c, y, z, x);
    ROS_ERROR("cam: %.3f, %.3f, %.3f",x,y,z);
    quatToEuler(c.conjugate(), y, z, x);
    ROS_ERROR("conjugate: %.3f, %.3f, %.3f",x,y,z);

    QQuaternion res = c;
    quatToEuler(res, y, z, x);
    ROS_ERROR("res: %.3f, %.3f, %.3f",x,y,z);
    //QQuaternion rot(0.9999619230641713,0,-0.008726535498373935,0);
    //QQuaternion rot(0.9999619230641713,0,0,-0.008726535498373935);
    //ROS_ERROR("new: %.3f, %.3f, %.3f, %.3f",rot.x(),rot.y(),rot.z(),rot.scalar());
    //ROS_ERROR("old: %.3f, %.3f, %.3f, %.3f",old_move_orientation_.x(),old_move_orientation_.y(),old_move_orientation_.z(),old_move_orientation_.scalar());
    //calculate the angle offset for relative rotation
    //rot = identity.conjugate() * rot;
    //rot = camera_orientation_.conjugate() * rot;
    //quatToEuler(rot, y, x, z);
    //ROS_ERROR("res: %.3f, %.3f, %.3f",x,y,z);

    //calculate difference between camera orientation and original rotation of object
    //difference of q1 and q2 is  q` = q1^-1 * q2
    //difference = camera_orientation_.conjugate() * pre;
    //set object orientation to camera
    //pre = camera_orientation_;
    //apply desired rotation
    pre *= rot;
    //revert back change of camera rotation to leave object in newly rotated state
    //pre *= difference;

    // I have
    //   pre                    current quaternion of the object
    //   rot                    rotation to be applied, always rotates correctly, no matter where the starting point is
    //   camera_orientation_    current quaternion of the camera
    // I need to apply the rotation considering the camera quaternion
    // In order to do so, I have to eliminate the current rotation
    //   pre

    //ROS_ERROR("pos: %.3f, %.3f, %.3f, %.3f",pre.x(),pre.y(),pre.z(),pre.scalar());

    template_pose.pose.orientation.w = pre.scalar();
    template_pose.pose.orientation.x = pre.x();
    template_pose.pose.orientation.y = pre.y();
    template_pose.pose.orientation.z = pre.z();

    //ROS_ERROR("updatePose: %.3f, %.3f, %.3f, %.3f", move_server_packet->state[0].quat[0], move_server_packet->state[0].quat[1], move_server_packet->state[0].quat[2], move_server_packet->state[0].quat[3]);

    return template_pose;
}

int PSMoveTemplateController::updateSuccess(MoveServerPacket *move_server_packet)
{
    //ROS_ERROR("updateSuccess: %f, %f, %f, %d", move_server_packet->state[0].pos[0], move_server_packet->state[0].pos[1], move_server_packet->state[0].pos[2], move_server_packet->state[0].pad.analog_trigger);

    if(received_pose_)//Check if a template is selected
    {
        //Only update if trigger is pressed
        if(move_server_packet->state[0].pad.analog_trigger > 0)
        {
            flor_ocs_msgs::OCSTemplateUpdate template_update;

            //Update current pose
            geometry_msgs::PoseStamped p = template_pose_;
            p = updatePose(p, move_server_packet, /*(float)move_server_packet->state[0].pad.analog_trigger/255.0f*/1.0f);

            //Add to the template_update
            template_update.template_id = template_selected_id_;
            template_update.pose = p;

            //Publish the new pose
            template_update_pub_.publish(template_update);

            cout << "template updated and published" << "\n";
        }

        //Save position and orientation
        old_move_position_.setX(move_server_packet->state[0].handle_pos[0]);
        old_move_position_.setY(move_server_packet->state[0].handle_pos[1]);
        old_move_position_.setZ(move_server_packet->state[0].handle_pos[2]);

        old_move_orientation_.setX(move_server_packet->state[0].quat[0]);
        old_move_orientation_.setY(move_server_packet->state[0].quat[1]);
        old_move_orientation_.setZ(move_server_packet->state[0].quat[2]);
        old_move_orientation_.setScalar(move_server_packet->state[0].quat[3]);

    }

    return 0;
}


int PSMoveTemplateController::updateFailure(int error)
{
    ROS_ERROR("Error: %d", error);

    return 0;
}


int PSMoveTemplateController::updateCameraSuccess(MoveServerCameraFrameSlicePacket *move_server_camera_frame_slice_packet)
{
    ROS_ERROR("Camera Frame");

    return 0;
}


int PSMoveTemplateController::updateCameraFailure(int error)
{
    ROS_ERROR("Camera Error: %d", error);

    return 0;
}

}

/*********************************************************************
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2013-2015, Team ViGIR ( TORC Robotics LLC, TU Darmstadt, Virginia Tech, Oregon State University, Cornell University, and Leibniz University Hanover )
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of Team ViGIR, TORC Robotics, nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *********************************************************************/
//@TODO_ADD_AUTHOR_INFO
/*
 * CameraView class definition.
 *
 * Author: Felipe Bacim.CameraViewWidgetCameraViewWidget
 *
 * Based on librviz_tutorials.
 *
 * Latest changes (12/04/2012):
 */

#ifndef CAMERA_VIEW_H
#define CAMERA_VIEW_H

#include <QWidget>
#include <QSocketNotifier>

#include <ros/ros.h>

#include <std_msgs/Float64.h>
#include <trajectory_msgs/JointTrajectory.h>
#include <trajectory_msgs/JointTrajectoryPoint.h>

#include "base_3d_view.h"
#include "notification_system.h"

namespace rviz
{
class Display;
class RenderPanel;
class VisualizationManager;
class FrameManager;
class Tool;
class ViewportMouseEvent;
}

namespace Ogre
{
class Viewport;
}

namespace vigir_ocs
{
// Class "CameraView" implements the QWidget that can be added to any QT application.
class CameraView: public Base3DView
{
    Q_OBJECT
public:
    CameraView( QWidget* parent = 0, Base3DView* copy_from = 0 );
    virtual ~CameraView();

    void setCameraPitch( int );
    void setCurrentCameraPitch( int );

    virtual void processGoalPose( const geometry_msgs::PoseStamped::ConstPtr& pose );
    virtual void processHotkeyRelayMessage(const vigir_ocs_msgs::OCSHotkeyRelay::ConstPtr& msg);

    std::vector<std::string> getCameraNames();

    int getDefaultCamera();

Q_SIGNALS:
    void setFullImageResolution( int );
    void setCropImageResolution( int );
    void setCameraSpeed( int );
    void setCropCameraSpeed( int );
    void unHighlight();
    void publishCropImageRequest();
    void publishFullImageRequest();
    void setInitialized();

public Q_SLOTS:
    void changeCameraTopic( int );
    void changeFullImageResolution( int );
    void changeCropImageResolution( int );
    void changeCameraSpeed( int );
    void changeCropCameraSpeed( int );
    void applyFeedChanges();
    void applyAreaChanges();
    void requestSingleFeedImage();
    void requestSingleAreaImage();
    void disableSelection( );
    void changeAlpha(int);
    void applyAlphaDelta(float alpha_delta);
    void closeSelectedArea();
    void mouseMoved(int,int);
    void requestPointCloudROI();
    void updateImageFrame(std::string);
    void selectionToolToggle(bool);

    virtual void defineFootstepGoal();
    void mouseEnterEvent( QEvent* event );
    void mouseMoveEvent( QMouseEvent* event );
    void keyPressEvent( QKeyEvent* event );

    virtual bool eventFilter( QObject * o, QEvent * e );

private Q_SLOTS:
    void select( int, int, int, int );

protected:
    virtual void timerEvent(QTimerEvent *event);

private:
    int selected_area_[4];
    int last_selected_area_[4];
    std::string camera_frame_topic_;
    float current_pitch_;

    rviz::Display* camera_viewer_;

    rviz::Tool* selection_tool_;
    rviz::Tool* previous_tool_;

    ros::Publisher head_pitch_update_pub_;
    ros::Publisher head_pitch_update_traj_pub_;
    ros::Publisher pointcloud_request_frame_pub_;

    int feed_rate_;
    int feed_resolution_;
    int area_rate_;
    int area_resolution_;

    bool setting_pose_;

    void loadCameraTopics(std::string);
    typedef struct
    {
        std::string topic_prefix;
        std::string topic_name;
        std::string name;
        std::string frame;
        int width;
        int height;
    } Camera;
    std::vector<Camera> camera_;
    QPushButton* close_area_button_;
    bool selection_made_;
    bool initialized_;

    bool selection_tool_enabled_;
    std::string joint_name_;
};
}
#endif // CAMERA_VIEWER_H

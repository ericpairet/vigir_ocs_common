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

#include "status_window.h"
#include "ui_status_window.h"
#include "jointList.h"
#include <QtGui>
#include <QPainter>
#include <ros/ros.h>
#include <QLabel>

status_window::status_window(QWidget *parent) :
     QWidget(parent),
    ui(new Ui::status_window)
{
    ui->setupUi(this);
    this->setWindowTitle("Robot_Status");
    mode_subscriber = nh_.subscribe("/flor/controller/mode",1,&status_window::controlModeMsgReceived, this);
    stability_subscriber = nh_.subscribe("/flor/controller/stability",1,&status_window::stabilityMsgReceived, this);

    // load control modes into dropdown box from parameters
    nh_.getParam("/atlas_controller/allowed_control_modes", allowed_control_modes_);

    jntList = new jointList(NULL);
    jntList->hide();
    rbtStatus = new robotStatus(NULL);
    rbtStatus->hide();
    timerColor.start(100,this);
    oldJointStyleSheet = ui->showJointButton->styleSheet();
    oldRobotStyleSheet = ui->showRobotStatus->styleSheet();

    key_event_sub_ = nh_.subscribe( "/flor/ocs/key_event", 5, &status_window::processNewKeyEvent, this );

    timer.start(33, this);
}

void status_window::controlModeMsgReceived(const vigir_control_msgs::VigirControlModeCommand::ConstPtr modeMsg)
{
   /*

    rbtStatus->show();
    //vigir_atlas_control_msgs::VigirAtlasControlMode::LEFT_SIDE_DOWN
    switch(modeMsg->posture & 0x7F)
    {
    case vigir_atlas_control_msgs::VigirAtlasControlMode::LEFT_SIDE_DOWN:
        ui->postureLabel->setText(QString::fromStdString("Left Side Down"));
        break;
    case vigir_atlas_control_msgs::VigirAtlasControlMode::RIGHT_SIDE_DOWN:
        ui->postureLabel->setText(QString::fromStdString("Right Side Down"));
        break;
    case vigir_atlas_control_msgs::VigirAtlasControlMode::FRONT_SIDE_DOWN:
        ui->postureLabel->setText(QString::fromStdString("Front Side Down"));
        break;
    case vigir_atlas_control_msgs::VigirAtlasControlMode::BACK_SIDE_DOWN:
        ui->postureLabel->setText(QString::fromStdString("Back Side Down"));
        break;
    case vigir_atlas_control_msgs::VigirAtlasControlMode::UPRIGHT:
        ui->postureLabel->setText(QString::fromStdString("Upright"));
        break;
    case vigir_atlas_control_msgs::VigirAtlasControlMode::VERTICAL: // posture can be Upright and Vertical
    case (vigir_atlas_control_msgs::VigirAtlasControlMode::VERTICAL + vigir_atlas_control_msgs::VigirAtlasControlMode::UPRIGHT):
        ui->postureLabel->setText(QString::fromStdString("Vertical"));
        break;
    case vigir_atlas_control_msgs::VigirAtlasControlMode::HEAD_STAND:
        ui->postureLabel->setText(QString::fromStdString("Head Stand"));
        break;
    default:
        ui->postureLabel->setText(QString::fromStdString("Unknown Posture"));
    }
    if(modeMsg->posture & 0x80)
        ui->postureLabel->setStyleSheet("QLabel { color: green; }");

    ui->l_armLabel->setText(getControllerStatus(modeMsg->left_arm));
    ui->l_legLabel->setText(getControllerStatus(modeMsg->left_leg));
    ui->r_armLabel->setText(getControllerStatus(modeMsg->right_arm));
    ui->r_legLabel->setText(getControllerStatus(modeMsg->right_leg));

    QString newText;
    if (modeMsg->control_mode >= 0 && modeMsg->control_mode <  allowed_control_modes_.size())
        newText = QString::fromStdString(allowed_control_modes_[modeMsg->control_mode]);
    else
        newText = QString::fromStdString("Unknown");

    ui->behaviorLabel->setText(newText);
*/
}

QString status_window::getControllerStatus(uint8_t flag)
{
    /*
    switch(flag)
    {
    case vigir_atlas_control_msgs::VigirAtlasControlMode::IDLE:
        return QString::fromStdString("Idle");
    case vigir_atlas_control_msgs::VigirAtlasControlMode::TRAJECTORY_ACTIVE:
        return QString::fromStdString("Trajectory Active");
    case vigir_atlas_control_msgs::VigirAtlasControlMode::TRAJECTORY_HOLD:
        return QString::fromStdString("Trajectory Hold");
    case vigir_atlas_control_msgs::VigirAtlasControlMode::TRAJECTORY_WAIT:
        return QString::fromStdString("Trajectory Wait");
    case vigir_atlas_control_msgs::VigirAtlasControlMode::TRAJECTORY_INVALID:
        return QString::fromStdString("Trajectory Invalid");
    case vigir_atlas_control_msgs::VigirAtlasControlMode::TRAJECTORY_EMPTY:
        return QString::fromStdString("Trajectory Empty");
    case vigir_atlas_control_msgs::VigirAtlasControlMode::POSITION:
        return QString::fromStdString("Position");
    case vigir_atlas_control_msgs::VigirAtlasControlMode::TRACK:
        return QString::fromStdString("Track");
    case vigir_atlas_control_msgs::VigirAtlasControlMode::HOLD_PRIOR:
        return QString::fromStdString("Hold");
    case vigir_atlas_control_msgs::VigirAtlasControlMode::FORCE:
        return QString::fromStdString("Force");
    default:
        return QString::fromStdString("Unknown Controller Status");
    }
    */
}

void status_window::stabilityMsgReceived(const vigir_ocs_msgs::OCSRobotStability::ConstPtr stabilityMsg)
{
    ui->stabilityLabel->setText(QString::number(stabilityMsg->stability));
    if(stabilityMsg->stability >= 193)
        ui->stabilityLabel->setStyleSheet("QLabel { color: green; }");
    else if (stabilityMsg->stability < 92)
        ui->stabilityLabel->setStyleSheet("QLabel { color: red; }");
    else
        ui->stabilityLabel->setStyleSheet("QLabel { color: yellow; }");
    switch(stabilityMsg->stance)
    {
    case 0x01:
        ui->stanceLabel->setText(QString::fromStdString("Right Leg Stance"));
        break;
    case 0x02:
        ui->stanceLabel->setText(QString::fromStdString("Right Leg Swing"));
        break;
    case 0x10:
        ui->stanceLabel->setText(QString::fromStdString("Left Leg Stance"));
        break;rbtStatus->show();
    case 0x20:
        ui->stanceLabel->setText(QString::fromStdString("Left Leg Swing"));
        break;
    case 0x11:
        ui->stanceLabel->setText(QString::fromStdString("Double Support"));
        break;
    case 0x22:
        ui->stanceLabel->setText(QString::fromStdString("Flight"));
        break;
    default:
        ui->stanceLabel->setText(QString::fromStdString("Stance Unknown"));
    }
}

void status_window::timerEvent(QTimerEvent *event)
{
	// check if ros is still running; if not, just kill the application
    if(!ros::ok())
        qApp->quit();
    
    //Spin at beginning of Qt timer callback, so current ROS time is retrieved
    if(event->timerId() == timer.timerId())
        ros::spinOnce();
    else
        updateButtonColor();
}

void status_window::updateButtonColor()
{
    ui->showJointButton->setStyleSheet(oldJointStyleSheet);
    if(jntList->isVisible())
        ui->showJointButton->setText(QString::fromStdString("Hide Joint List"));
    else
        ui->showJointButton->setText(QString::fromStdString("Show Joint List"));
    if(jntList->getNumWarn() > 0)
    {
        ui->showJointButton->setStyleSheet("QPushButton { background-color : yellow; }");
        ui->showJointButton->setText(ui->showJointButton->text() + QString::fromStdString(" Wrn:") + QString::number(jntList->getNumWarn()));
    }
    if(jntList->getNumError() > 0)
    {
        ui->showJointButton->setStyleSheet("QPushButton { background-color : red; }");
        ui->showJointButton->setText(ui->showJointButton->text() + QString::fromStdString(" Err:") + QString::number(jntList->getNumError()));
    }

    ui->showRobotStatus->setStyleSheet(oldRobotStyleSheet);
    if(rbtStatus->isVisible())
        ui->showRobotStatus->setText(QString::fromStdString("Hide Robot Status"));
    else
        ui->showRobotStatus->setText(QString::fromStdString("Show Robot Status"));
    if(rbtStatus->getNumWarn() > 0)
    {
        ui->showRobotStatus->setStyleSheet("QPushButton { background-color : yellow; }");
        ui->showRobotStatus->setText(ui->showRobotStatus->text() + QString::fromStdString(" Wrn:") + QString::number(rbtStatus->getNumWarn()));
    }
    if(rbtStatus->getNumError() > 0)
    {
        ui->showRobotStatus->setStyleSheet("QPushButton { background-color : red; }");
        ui->showRobotStatus->setText(ui->showRobotStatus->text() + QString::fromStdString(" Err:") + QString::number(rbtStatus->getNumError()));
    }
}

status_window::~status_window()
{
    delete ui;
}


void status_window::on_showJointButton_clicked()
{
    if(jntList->isVisible())
    {
        ui->showJointButton->setText(QString::fromStdString("Show Joint List"));
        jntList->hide();
    }
    else
    {
        jntList->show();
        ui->showJointButton->setText(QString::fromStdString("Hide Joint List"));
    }
}

void status_window::on_showRobotStatus_clicked()
{
    if(rbtStatus->isVisible())
    {
        rbtStatus->hide();
        ui->showRobotStatus->setText(QString::fromStdString("Show Robot Status"));
    }
    else
    {
        rbtStatus->show();
        ui->showRobotStatus->setText(QString::fromStdString("Hide Robot Status"));
    }
}

void status_window::processNewKeyEvent(const vigir_ocs_msgs::OCSKeyEvent::ConstPtr key_event)
{
    // store key state
    if(key_event->state)
        keys_pressed_list_.push_back(key_event->keycode);
    else
        keys_pressed_list_.erase(std::remove(keys_pressed_list_.begin(), keys_pressed_list_.end(), key_event->keycode), keys_pressed_list_.end());

    // process hotkeys
    std::vector<int>::iterator key_is_pressed;

    key_is_pressed = std::find(keys_pressed_list_.begin(), keys_pressed_list_.end(), 37);
    /*if(key_event->keycode == 18 && key_event->state && key_is_pressed != keys_pressed_list_.end()) // ctrl+9
    {
        if(this->isVisible())
        {
            this->hide();
        }
        else
        {
            //this->move(QPoint(key_event->cursor_x+5, key_event->cursor_y+5));
            this->show();
        }
    }*/
}

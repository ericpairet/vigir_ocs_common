/*********************************************************************
*
* Software License Agreement (BSD License)
*
* Copyright (c) 2008, Willow Garage, Inc.
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* * Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
* * Redistributions in binary form must reproduce the above
* copyright notice, this list of conditions and the following
* disclaimer in the documentation and/or other materials provided
* with the distribution.
* * Neither the name of Willow Garage, Inc. nor the names of its
* contributors may be used to endorse or promote products derived
* from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
* ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
* Author: Eitan Marder-Eppstein
*********************************************************************/
#ifndef ACTIONLIB_COMPLEX_ACTION_SERVER_H_
#define ACTIONLIB_COMPLEX_ACTION_SERVER_H_
#include <boost/thread/condition.hpp>
#include <ros/ros.h>
#include <actionlib/server/action_server.h>
#include <actionlib/action_definition.h>
#include <queue>

namespace actionlib {
/** @class
 * Action Server that allows for the acceptance and use of multiple goals
 * Will run goals concurrently
 * Reimplemented from simple action server, will not support preempting
*/
template <class ActionSpec>
class ComplexActionServer
{
public:
    //generates typedefs that we'll use to make our lives easier
    ACTION_DEFINITION(ActionSpec);
    typedef typename ActionServer<ActionSpec>::GoalHandle GoalHandle;
    typedef boost::function<void (GoalConstPtr, GoalHandle)> ExecuteCallback;
    /**
    * @brief Constructor for a ComplexActionServer
    * @param name A name for the action server
    * @param execute_cb Optional callback that gets called in a separate thread whenever
    * a new goal is received, allowing users to have blocking callbacks.
    * Adding an execute callback also deactivates the goalCallback.
    * @param auto_start A boolean value that tells the ActionServer wheteher or not to start publishing as soon as it comes up. THIS SHOULD ALWAYS BE SET TO FALSE TO AVOID RACE CONDITIONS and start() should be called after construction of the server.
    */
    ComplexActionServer(std::string name, ExecuteCallback execute_cb, bool auto_start);
    /**
    * @brief Constructor for a ComplexActionServer
    * @param name A name for the action server
    * @param auto_start A boolean value that tells the ActionServer wheteher or not to start publishing as soon as it comes up. THIS SHOULD ALWAYS BE SET TO FALSE TO AVOID RACE CONDITIONS and start() should be called after construction of the server.
    */
    ComplexActionServer(std::string name, bool auto_start);
    /**
    * @brief DEPRECATED: Constructor for a ComplexActionServer
    * @param name A name for the action server
    * @param execute_cb Optional callback that gets called in a separate thread whenever
    * a new goal is received, allowing users to have blocking callbacks.
    * Adding an execute callback also deactivates the goalCallback.
    */
    ROS_DEPRECATED ComplexActionServer(std::string name, ExecuteCallback execute_cb = NULL);
    /**
    * @brief Constructor for a ComplexActionServer
    * @param n A NodeHandle to create a namespace under
    * @param name A name for the action server
    * @param execute_cb Optional callback that gets called in a separate thread whenever
    * a new goal is received, allowing users to have blocking callbacks.
    * Adding an execute callback also deactivates the goalCallback.
    * @param auto_start A boolean value that tells the ActionServer wheteher or not to start publishing as soon as it comes up. THIS SHOULD ALWAYS BE SET TO FALSE TO AVOID RACE CONDITIONS and start() should be called after construction of the server.
    */
    ComplexActionServer(ros::NodeHandle n, std::string name, ExecuteCallback execute_cb, bool auto_start);
    /**
    * @brief Constructor for a ComplexActionServer
    * @param n A NodeHandle to create a namespace under
    * @param name A name for the action server
    * @param auto_start A boolean value that tells the ActionServer wheteher or not to start publishing as soon as it comes up. THIS SHOULD ALWAYS BE SET TO FALSE TO AVOID RACE CONDITIONS and start() should be called after construction of the server.
    */
    ComplexActionServer(ros::NodeHandle n, std::string name, bool auto_start);
    /**
    * @brief Constructor for a ComplexActionServer
    * @param n A NodeHandle to create a namespace under
    * @param name A name for the action server
    * @param execute_cb Optional callback that gets called in a separate thread whenever
    * a new goal is received, allowing users to have blocking callbacks.
    * Adding an execute callback also deactivates the goalCallback.
    */
    ROS_DEPRECATED ComplexActionServer(ros::NodeHandle n, std::string name, ExecuteCallback execute_cb = NULL);
    ~ComplexActionServer();
    /**
    * @brief Accepts a new goal when one is available The status of this
    * goal is set to active upon acceptance, and the status of any
    * previously active goal is set to preempted. Preempts received for the
    * new goal between checking if isNewGoalAvailable or invokation of a
    * goal callback and the acceptNewGoal call will not trigger a preempt
    * callback. This means, isPreemptReqauested should be called after
    * accepting the goal even for callback-based implementations to make
    * sure the new goal does not have a pending preempt request.
    * @return A shared_ptr to the new goal.
    */
    GoalHandle acceptNewGoal();
    /**
    * @brief Allows polling implementations to query about the availability of a new goal
    * @return True if a new goal is available, false otherwise
    */
    bool isNewGoalAvailable();
    /**
    * @brief Allows polling implementations to query about preempt requests
    * @return True if a preempt is requested, false otherwise
    */
    bool isPreemptRequested();
    /**
    * @brief Allows polling implementations to query about the status of the current goal
    * @return True if a goal is active, false otherwise
    */
    bool isActive();
    /**
    * @brief Sets the status of the active goal to succeeded
    * @param result An optional result to send back to any clients of the goal
    * @param result An optional text message to send back to any clients of the goal
    */
    void setSucceeded(const Result& result = Result(), const std::string& text = std::string(""), GoalHandle goal = NULL);
    /**
    * @brief Sets the status of the active goal to aborted
    * @param result An optional result to send back to any clients of the goal
    * @param result An optional text message to send back to any clients of the goal
    */
    void setAborted(const Result& result = Result(), const std::string& text = std::string(""), GoalHandle goal = NULL);
    /**
    * @brief Publishes feedback for a given goal
    * @param feedback Shared pointer to the feedback to publish
    */
    void publishFeedback(const FeedbackConstPtr& feedback);
    /**
    * @brief Publishes feedback for a given goal
    * @param feedback The feedback to publish
    */
    void publishFeedback(const Feedback& feedback);
    /**
    * @brief Sets the status of the active goal to preempted
    * @param result An optional result to send back to any clients of the goal
    * @param result An optional text message to send back to any clients of the goal
    */
    void setPreempted(const Result& result = Result(), const std::string& text = std::string(""));
    /**
    * @brief Allows users to register a callback to be invoked when a new goal is available
    * @param cb The callback to be invoked
    */
    void registerGoalCallback(boost::function<void ()> cb);
    /**
    * @brief Allows users to register a callback to be invoked when a new preempt request is available
    * @param cb The callback to be invoked
    */
    void registerPreemptCallback(boost::function<void ()> cb);
    /**
    * @brief Explicitly start the action server, used it auto_start is set to false
    */
    void start();
    /**
    * @brief Explicitly shutdown the action server
    */
    void shutdown();
private:
    /**
    * @brief Callback for when the ActionServer receives a new goal and passes it on
    */
    void goalCallback(GoalHandle goal);
    /**
    * @brief Callback for when the ActionServer receives a new preempt and passes it on
    */
    void preemptCallback(GoalHandle preempt);
    /**
    * @brief Called from a separate thread to call blocking execute calls
    */
    void executeLoop();

    void runGoal(GoalConstPtr goal, GoalHandle goal_handle);

    ros::NodeHandle n_;
    boost::shared_ptr<ActionServer<ActionSpec> > as_;

    GoalHandle current_goal_, next_goal_;
    bool new_goal_, preempt_request_, new_goal_preempt_request_;

    boost::recursive_mutex lock_;
    boost::function<void ()> goal_callback_;
    boost::function<void ()> preempt_callback_;

    ExecuteCallback execute_callback_;
    boost::condition execute_condition_;
    boost::thread* execute_thread_;
    boost::mutex terminate_mutex_;
    bool need_to_terminate_;
    int current_goals_to_process_;
    int goal_index_;

    std::queue<GoalHandle> goal_queue_;
};
};
//include the implementation here
#include "complex_action_server_imp.h"
#endif

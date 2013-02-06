/* 
 * ImageDisplayCustom class implementation.
 * 
 * Author: Felipe Bacim.
 * 
 * Based on the rviz image display class.
 * 
 * Latest changes (12/11/2012):
 * - fixed segfault issues
 */
/*
 * Copyright (c) 2012, Willow Garage, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Willow Garage, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <boost/bind.hpp>

#include <OGRE/OgreManualObject.h>
#include <OGRE/OgreMaterialManager.h>
#include <OGRE/OgreRectangle2D.h>
#include <OGRE/OgreRenderSystem.h>
#include <OGRE/OgreRenderWindow.h>
#include <OGRE/OgreRoot.h>
#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreTextureManager.h>
#include <OGRE/OgreViewport.h>

#include <tf/transform_listener.h>

#include "rviz/display_context.h"
#include "rviz/frame_manager.h"
#include "rviz/render_panel.h"
#include "rviz/validate_floats.h"

#include "image_display_custom.h"

#include <vigir_perception_msgs/DownSampledImageRequest.h>

namespace rviz
{

ImageDisplayCustom::ImageDisplayCustom()
  : ImageDisplayBase()
  , texture_()
{
}

void ImageDisplayCustom::onInitialize()
{
  {
    static uint32_t count = 0;
    std::stringstream ss;
    ss << "ImageDisplayCustom" << count++;
    //img_scene_manager_ = Ogre::Root::getSingleton().createSceneManager(Ogre::ST_GENERIC, ss.str());
  }

  //img_scene_node_ = img_scene_manager_->getRootSceneNode()->createChildSceneNode();
  
  screen_rect_selection_ = NULL;

  {
    static int count = 0;
    std::stringstream ss;
    ss << "ImageDisplayObject" << count++;

    screen_rect_ = new Ogre::Rectangle2D(true);
    screen_rect_->setRenderQueueGroup(Ogre::RENDER_QUEUE_OVERLAY - 1);
    screen_rect_->setCorners(-1.0f, 1.0f, 1.0f, -1.0f);

    ss << "Material";
    material_ = Ogre::MaterialManager::getSingleton().create( ss.str(), Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );
    material_->setSceneBlending( Ogre::SBT_REPLACE );
    material_->setDepthWriteEnabled(false);
    material_->setReceiveShadows(false);
    material_->setDepthCheckEnabled(false);

    material_->getTechnique(0)->setLightingEnabled(false);
    Ogre::TextureUnitState* tu = material_->getTechnique(0)->getPass(0)->createTextureUnitState();
    tu->setTextureName(texture_.getTexture()->getName());
    tu->setTextureFiltering( Ogre::TFO_NONE );

    material_->setCullingMode(Ogre::CULL_NONE);
    Ogre::AxisAlignedBox aabInf;
    aabInf.setInfinite();
    screen_rect_->setBoundingBox(aabInf);
    screen_rect_->setMaterial(material_->getName());
    scene_node_->attachObject(screen_rect_);
  }
  
  //render_panel_ = new RenderPanel();
  //render_panel_->getRenderWindow()->setAutoUpdated(false);
  //render_panel_->getRenderWindow()->setActive( false );

  //render_panel_->resize( 640, 480 );
  //render_panel_->initialize(scene_manager_, context_);

  //setAssociatedWidget( render_panel_ );

  //render_panel_->setAutoRender(false);
  //render_panel_->setOverlaysEnabled(false);
  //render_panel_->getCamera()->setNearClipDistance( 0.01f );
  
  // We first subscribe to the JointState messages
	cropped_image_ = n_.subscribe<sensor_msgs::Image>( "/l_image_cropped/image_raw", 5, &ImageDisplayCustom::processCroppedImage, this );
	
	//ros::NodeHandle nh_out (nh, "camera");
  //it_out_.reset(new image_transport::ImageTransport(nh_out));
	//pub_ = it_out_->advertiseCamera("image_raw",  1, connect_cb, connect_cb, connect_cb_info, connect_cb_info);
}

ImageDisplayCustom::~ImageDisplayCustom()
{
  ImageDisplayBase::unsubscribe();
  //delete render_panel_;
  delete screen_rect_;
  scene_node_->getParentSceneNode()->removeAndDestroyChild( scene_node_->getName() );
}

void ImageDisplayCustom::onEnable()
{
  ImageDisplayBase::subscribe();
  render_panel_->getRenderWindow()->setActive(true);
}

void ImageDisplayCustom::onDisable()
{
  render_panel_->getRenderWindow()->setActive(false);
  ImageDisplayBase::unsubscribe();
  clear();
}

void ImageDisplayCustom::clear()
{
  texture_.clear();

  if( render_panel_->getCamera() )
  {
    render_panel_->getCamera()->setPosition(Ogre::Vector3(999999, 999999, 999999));
  }
}

void ImageDisplayCustom::update( float wall_dt, float ros_dt )
{
  try
  {
    texture_.update();
    texture_selection_.update();

    //make sure the aspect ratio of the image is preserved
    float win_width = render_panel_->width();
    float win_height = render_panel_->height();

    float img_width = texture_.getWidth();
    float img_height = texture_.getHeight();

    if ( img_width != 0 && img_height != 0 && win_width !=0 && win_height != 0 )
    {
      float img_aspect = img_width / img_height;
      float win_aspect = win_width / win_height;

      if ( img_aspect > win_aspect )
      {
        screen_rect_->setCorners(-1.0f, 1.0f * win_aspect/img_aspect, 1.0f, -1.0f * win_aspect/img_aspect, false);
      }
      else
      {
        screen_rect_->setCorners(-1.0f * img_aspect/win_aspect, 1.0f, 1.0f * img_aspect/win_aspect, -1.0f, false);
      }
    }
    
    if(screen_rect_selection_ != NULL)
    {
    	screen_rect_selection_->setCorners(-0.5, 0.5, 0.5, -0.5, false);
    }

    render_panel_->getRenderWindow()->update();
    
    //std::cout << "CUSTOM UPDATE LOOP" << std::endl;
  }
  catch( UnsupportedImageEncoding& e )
  {
    setStatus(StatusProperty::Error, "Image", e.what());
  }
}

void ImageDisplayCustom::reset()
{
  ImageDisplayBase::reset();
  clear();
}

/* This is called by incomingMessage(). */
void ImageDisplayCustom::processMessage(const sensor_msgs::Image::ConstPtr& msg)
{
  texture_.addMessage(msg);
   /*
  sensor_msgs::ImagePtr image_out;
  sensor_msgs::CameraInfoPtr camera_info_out;
  pub_.publish(image_out, camera_info_out);
	
	image_out = msg;
	camera_info_out = texture_
	pub_.publish(image_out, camera_info_out);*/
}

void ImageDisplayCustom::processCroppedImage(const sensor_msgs::Image::ConstPtr& msg)
{
  texture_selection_.addMessage(msg);
}

void ImageDisplayCustom::setRenderPanel( RenderPanel* rp )
{
  render_panel_ = rp;
}

void ImageDisplayCustom::selectionProcessed( int x1, int y1, int x2, int y2 )
{
	std::cout << "Select Window: " << x1 << ", " << y1 << " -> " << x2 << ", " << y2 << std::endl;

	if( screen_rect_selection_ == NULL)
	{
    static int count = 0;
    std::stringstream ss;
    ss << "ImageDisplayObject" << count++;
		screen_rect_selection_ = new Ogre::Rectangle2D(true);
		screen_rect_selection_->setRenderQueueGroup(Ogre::RENDER_QUEUE_OVERLAY - 1);
		screen_rect_selection_->setCorners(-1.0f, 1.0f, 1.0f, -1.0f);

    ss << "Material";
		material_selection_ = Ogre::MaterialManager::getSingleton().create( ss.str(), Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );
		material_selection_->setSceneBlending( Ogre::SBT_REPLACE );
		material_selection_->setDepthWriteEnabled(false);
		material_selection_->setReceiveShadows(false);
		material_selection_->setDepthCheckEnabled(false);

		material_selection_->getTechnique(0)->setLightingEnabled(false);
		Ogre::TextureUnitState* tu = material_selection_->getTechnique(0)->getPass(0)->createTextureUnitState();
		tu->setTextureName(texture_selection_.getTexture()->getName());
		tu->setTextureFiltering( Ogre::TFO_NONE );

		material_selection_->setCullingMode(Ogre::CULL_NONE);
		Ogre::AxisAlignedBox aabInf;
		aabInf.setInfinite();
		screen_rect_selection_->setBoundingBox(aabInf);
		screen_rect_selection_->setMaterial(material_selection_->getName());
		scene_node_->attachObject(screen_rect_selection_);
  }
  
	vigir_perception_msgs::DownSampledImageRequest cmd;

	// Initialize publisher
	// since creating a publisher takes time (on separate threads)
	// we need to enable latching so that the message is not lost
	pos_pub_ = n_.advertise<vigir_perception_msgs::DownSampledImageRequest>( "/l_image_cropped/image_request", 1, true );
	
	// Spin once to register advertise
	ros::spinOnce();

	// publish message
	cmd.binning_x = 4;
  cmd.binning_y = 4;
  cmd.roi.width = texture_.getWidth()/2;
  cmd.roi.height = texture_.getHeight()/2;
  cmd.roi.x_offset = texture_.getWidth()/4;
  cmd.roi.y_offset = texture_.getHeight()/4;

	pos_pub_.publish( cmd );
  
  /*float win_width = render_panel_->width();
  float win_height = render_panel_->height();

  float img_width = texture_.getWidth();
  float img_height = texture_.getHeight();
  
  std::cout << "Window dimensions: " << win_width << ", " << win_height << std::endl;
  std::cout << "Image dimensions: " << img_width << ", " << img_height << std::endl;

  if ( img_width != 0 && img_height != 0 && win_width !=0 && win_height != 0 )
  {
    float img_aspect = img_width / img_height;
    float win_aspect = win_width / win_height;

    if ( img_aspect > win_aspect )
    {
      screen_rect_selection_->setCorners(-1.0f, 1.0f * win_aspect/img_aspect, 1.0f, -1.0f * win_aspect/img_aspect, false);
    }
    else
    {
      screen_rect_selection_->setCorners(-1.0f * img_aspect/win_aspect, 1.0f, 1.0f * img_aspect/win_aspect, -1.0f, false);
    }
  }*/
}

} // namespace rviz

//#include <pluginlib/class_list_macros.h>
//PLUGINLIB_EXPORT_CLASS( rviz::ImageDisplayCustom, rviz::Display )


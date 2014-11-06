/*M///////////////////////////////////////////////////////////////////////////////////////
 //
 //  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
 //
 //  By downloading, copying, installing or using the software you agree to this license.
 //  If you do not agree to this license, do not download, install,
 //  copy or use the software.
 //
 //
 //                           License Agreement
 //                For Open Source Computer Vision Library
 //
 // Copyright (C) 2014, OpenCV Foundation, all rights reserved.
 // Third party copyrights are property of their respective owners.
 //
 // Redistribution and use in source and binary forms, with or without modification,
 // are permitted provided that the following conditions are met:
 //
 //   * Redistribution's of source code must retain the above copyright notice,
 //     this list of conditions and the following disclaimer.
 //
 //   * Redistribution's in binary form must reproduce the above copyright notice,
 //     this list of conditions and the following disclaimer in the documentation
 //     and/or other materials provided with the distribution.
 //
 //   * The name of the copyright holders may not be used to endorse or promote products
 //     derived from this software without specific prior written permission.
 //
 // This software is provided by the copyright holders and contributors "as is" and
 // any express or implied warranties, including, but not limited to, the implied
 // warranties of merchantability and fitness for a particular purpose are disclaimed.
 // In no event shall the Intel Corporation or contributors be liable for any direct,
 // indirect, incidental, special, exemplary, or consequential damages
 // (including, but not limited to, procurement of substitute goods or services;
 // loss of use, data, or profits; or business interruption) however caused
 // and on any theory of liability, whether in contract, strict liability,
 // or tort (including negligence or otherwise) arising in any way out of
 // the use of this software, even if advised of the possibility of such damage.
 //
 //M*/

#ifndef __OPENCV_SALIENCY_BASE_CLASSES_HPP__
#define __OPENCV_SALIENCY_BASE_CLASSES_HPP__

#include "opencv2/core.hpp"
#include <opencv2/core/persistence.hpp>
#include "opencv2/imgproc.hpp"
#include <iostream>
#include <sstream>
#include <complex>

namespace cv
{
namespace saliency
{

/************************************ Saliency Base Class ************************************/

class CV_EXPORTS Saliency : public virtual Algorithm
{
 public:
  /**
   * \brief Destructor
   */
  virtual ~Saliency();

  /**
   * \brief Create Saliency by saliency type.
   */
  static Ptr<Saliency> create( const String& saliencyType );

  /**
   * \brief Compute the saliency
   * \param image        The image.
   * \param saliencyMap      The computed saliency map.
   * \return true if the saliency map is computed, false otherwise
   */
  bool computeSaliency( const InputArray image, OutputArray saliencyMap );

  /**
   * \brief Get the name of the specific saliency type
   * \return The name of the tracker initializer
   */
  String getClassName() const;

 protected:

  virtual bool computeSaliencyImpl( const InputArray image, OutputArray saliencyMap ) = 0;
  String className;
};

/************************************ Static Saliency Base Class ************************************/
class CV_EXPORTS StaticSaliency : public virtual Saliency
{
 public:

  bool computeBinaryMap( const Mat& saliencyMap, Mat& binaryMap );
 protected:
  virtual bool computeSaliencyImpl( const InputArray image, OutputArray saliencyMap )=0;

};

/************************************ Motion Saliency Base Class ************************************/
class CV_EXPORTS MotionSaliency : public virtual Saliency
{

 protected:
  virtual bool computeSaliencyImpl( const InputArray image, OutputArray saliencyMap )=0;

};

/************************************ Objectness Base Class ************************************/
class CV_EXPORTS Objectness : public virtual Saliency
{

 protected:
  virtual bool computeSaliencyImpl( const InputArray image, OutputArray saliencyMap )=0;

};

} /* namespace saliency */
} /* namespace cv */

#endif
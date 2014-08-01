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
 // Copyright (C) 2013, OpenCV Foundation, all rights reserved.
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

#include "CmShow.h"
#include "opencv2/saliency/FilterTIG.h"

using namespace cv;

void FilterTIG::update( CMat &w1f )
{
  CV_Assert( w1f.cols * w1f.rows == D && w1f.type() == CV_32F && w1f.isContinuous() );
  float b[D], residuals[D];
  memcpy( residuals, w1f.data, sizeof(float) * D );
  for ( int i = 0; i < NUM_COMP; i++ )
  {
    float avg = 0;
    for ( int j = 0; j < D; j++ )
    {
      b[j] = residuals[j] >= 0.0f ? 1.0f : -1.0f;
      avg += residuals[j] * b[j];
    }
    avg /= D;
    _coeffs1[i] = avg, _coeffs2[i] = avg * 2, _coeffs4[i] = avg * 4, _coeffs8[i] = avg * 8;
    for ( int j = 0; j < D; j++ )
      residuals[j] -= avg * b[j];
    uint64_t tig = 0;
    for ( int j = 0; j < D; j++ )
      tig = ( tig << 1 ) | ( b[j] > 0 ? 1 : 0 );
    _bTIGs[i] = tig;
  }
}

void FilterTIG::reconstruct( Mat &w1f )
{
  w1f = Mat::zeros( 8, 8, CV_32F );
  float *weight = (float*) w1f.data;
  for ( int i = 0; i < NUM_COMP; i++ )
  {
    uint64_t tig = _bTIGs[i];
    for ( int j = 0; j < D; j++ )
      weight[j] += _coeffs1[i] * ( ( ( tig >> ( 63 - j ) ) & 1 ) ? 1 : -1 );
  }
}

// For a W by H gradient magnitude map, find a W-7 by H-7 CV_32F matching score map
// Please refer to my paper for definition of the variables used in this function
Mat FilterTIG::matchTemplate( const Mat &mag1u )
{
  const int H = mag1u.rows, W = mag1u.cols;
  const Size sz( W + 1, H + 1 );  // Expand original size to avoid dealing with boundary conditions
  Mat_<int64_t> Tig1 = Mat_<int64_t>::zeros( sz ), Tig2 = Mat_<int64_t>::zeros( sz );
  Mat_<int64_t> Tig4 = Mat_<int64_t>::zeros( sz ), Tig8 = Mat_<int64_t>::zeros( sz );
  Mat_<byte> Row1 = Mat_<byte>::zeros( sz ), Row2 = Mat_<byte>::zeros( sz );
  Mat_<byte> Row4 = Mat_<byte>::zeros( sz ), Row8 = Mat_<byte>::zeros( sz );
  Mat_<float> scores( sz );
  for ( int y = 1; y <= H; y++ )
  {
    const byte* G = mag1u.ptr<byte>( y - 1 );
    int64_t* T1 = Tig1.ptr<int64_t>( y );  // Binary TIG of current row
    int64_t* T2 = Tig2.ptr<int64_t>( y );
    int64_t* T4 = Tig4.ptr<int64_t>( y );
    int64_t* T8 = Tig8.ptr<int64_t>( y );
    int64_t* Tu1 = Tig1.ptr<int64_t>( y - 1 );  // Binary TIG of upper row
    int64_t* Tu2 = Tig2.ptr<int64_t>( y - 1 );
    int64_t* Tu4 = Tig4.ptr<int64_t>( y - 1 );
    int64_t* Tu8 = Tig8.ptr<int64_t>( y - 1 );
    byte* R1 = Row1.ptr<byte>( y );
    byte* R2 = Row2.ptr<byte>( y );
    byte* R4 = Row4.ptr<byte>( y );
    byte* R8 = Row8.ptr<byte>( y );
    float *s = scores.ptr<float>( y );
    for ( int x = 1; x <= W; x++ )
    {
      byte g = G[x - 1];
      R1[x] = ( R1[x - 1] << 1 ) | ( ( g >> 4 ) & 1 );
      R2[x] = ( R2[x - 1] << 1 ) | ( ( g >> 5 ) & 1 );
      R4[x] = ( R4[x - 1] << 1 ) | ( ( g >> 6 ) & 1 );
      R8[x] = ( R8[x - 1] << 1 ) | ( ( g >> 7 ) & 1 );
      T1[x] = ( Tu1[x] << 8 ) | R1[x];
      T2[x] = ( Tu2[x] << 8 ) | R2[x];
      T4[x] = ( Tu4[x] << 8 ) | R4[x];
      T8[x] = ( Tu8[x] << 8 ) | R8[x];
      s[x] = dot( T1[x], T2[x], T4[x], T8[x] );
    }
  }
  Mat matchCost1f;
  scores( Rect( 8, 8, W - 7, H - 7 ) ).copyTo( matchCost1f );
  return matchCost1f;
}

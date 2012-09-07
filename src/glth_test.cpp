#include <iostream>
#include <fstream>
#include <cmath>
//#include <tr1/random>
#include <cv.h>
#include <highgui.h>
#include "glth_xfrmr.hpp"

/******************************************************************************/
//        "Polar" version without trigonometric calls
double randn_notrig(double mu=0.0, double sigma=1.0) {
  static bool deviateAvailable=false;        //        flag
  static float storedDeviate;                        //        deviate from previous calculation
  double polar, rsquared, var1, var2;

  //        If no deviate has been stored, the polar Box-Muller transformation is
  //        performed, producing two independent normally-distributed random
  //        deviates.  One is stored for the next round, and one is returned.
  if (!deviateAvailable) {

    //        choose pairs of uniformly distributed deviates, discarding those
    //        that don't fall within the unit circle
    do {
      var1=2.0*( double(rand())/double(RAND_MAX) ) - 1.0;
      var2=2.0*( double(rand())/double(RAND_MAX) ) - 1.0;
      rsquared=var1*var1+var2*var2;
    } while ( rsquared>=1.0 || rsquared == 0.0);

    //        calculate polar tranformation for each deviate
    polar=sqrt(-2.0*log(rsquared)/rsquared);

    //        store first deviate and set flag
    storedDeviate=var1*polar;
    deviateAvailable=true;

    //        return second deviate
    return var2*polar*sigma + mu;
  }

  //        If a deviate is available from a previous call to this function, it is
  //        returned, and the flag is set to false.
  else {
    deviateAvailable=false;
    return storedDeviate*sigma + mu;
  }
}

template <typename T>
T min(T arg1, T arg2) {
  T res = (arg1 < arg2) ? arg1 : arg2;
  return res;
}

cv::Vec3b JetColour(double v,double vmax)
{
  double scale = vmax/4.0;
  unsigned char mid = 144;
  unsigned char red, blue, green;

  if( v < scale ) {
    blue = mid+(char)((255-mid)*v/vmax);
    red = green = 0;
  }
  else if( v < 2*scale ) {
    red=0;
    green=(unsigned char)(255*(v-scale)/scale);
    blue=255;
  }
  else if( v < 3*scale ) {
    red=(unsigned char)(255*(v-2*scale)/scale);
    green=255;
    blue=255-red;
  }
  else if( v < vmax ) {
    red=255;
    green=(unsigned char)(255-255*(v-3*scale)/scale);
    blue=0;
  }
  else {
    red = blue = green = 0;
  }

  return(cv::Vec3b(blue,green,red));
}

int main(const int argc, char** argv) {
  // Random number generation
  //  std::tr1::mt19937 prng(0);
  //  std::tr1::normal_distribution<double> normal;
  //  std::tr1::variate_generator<std::tr1::mt19937,std::tr1::normal_distribution<double>> randn(prng,normal);

  // Constant
  double pi = 3.14159;

  // Data
  std::size_t siglen = 4096;
  glth::signal sig(siglen);
  int fftlen = 2048;
  glth::glth_xfrmr xfm(siglen, fftlen);

  std::complex<double> **wvd = new std::complex<double>*[siglen];
  for(std::size_t i = 0; i < siglen; i++) {
    wvd[i] = new std::complex<double>[fftlen];
  }


  // By default we have succeeded.
  glth_const::exit_status res = glth_const::exit_success;

  // Make two signals and XWVD them.
  float freq_0 = 30e6;
  float pitch_angle = 90;
  float power_loss = 1.0e-15;  // Watts
  float energy_0 = 18e3; // keV
  float zmax = 1.0; // meters
  float warble = 1.0e2; // axial frequency
  float phase_v = 1.0; //299792458; // phase velocity, approx at c for now
  float gamma_0 = 1 + energy_0/511;
  //  float chirp_rate = power_loss/(gamma_0 * 8.18e-14);
  float chirp_rate = 10e4;
  float sample_rate = 250e6;
  float t = 0;

  for(int idx = 0; idx < siglen; idx++) {
    t = idx/sample_rate;
    double phase = freq_0*t*
      ((1 + 0.5*cos(pitch_angle)*cos(pitch_angle)/(sin(pitch_angle)*sin(pitch_angle))) +
       chirp_rate*t +
       zmax*warble/phase_v*cos(warble*t));
    std::complex<double> noise = std::complex<double>(randn_notrig(0.0,1.0),randn_notrig(0.0,1.0));
       sig[idx][0] += cos(2*pi*phase);
       sig[idx][1] += sin(2*pi*phase);

  }

  //   xfm.wvd(sig, wvd);

   // scan for min and max
   // double min, max;
   // for(int i = 0; i < siglen; i++) {
   //   for(int j = 0; j < fftlen; j++) {
   //     //       wvd[i][j] = log10(wvd[i][j]);
   //     if( std::norm(wvd[i][j]) > max ) max = std::norm(wvd[i][j]);
   //     else if( std::norm(wvd[i][j]) < min ) min = std::norm(wvd[i][j]);
   //   }
   // }

   // cv::Mat out_matrix(fftlen, siglen, CV_8UC3, cv::Scalar(0,0,0));
   // for(int t = 0; t < siglen; t++) {
   //   for(int f = 0; f < fftlen; f++) {
   //     out_matrix.at<cv::Vec3b>((fftlen - 1) - f,t) = JetColour(std::norm(wvd[t][f]),max);
   //   }
   // }

   // std::vector<cv::Vec4i> lines;
   // cv::Mat hgh_matrix = out_matrix.clone();
   // cv::Canny(out_matrix, hgh_matrix, 100, 200, 3);
   // cv::HoughLinesP( hgh_matrix, lines, 1, CV_PI/180, 200, 100, 20);
   // cv::Mat lin_matrix = out_matrix.clone();

   // // barf out lines
   // std::cout << "Edges found: " << lines.size() << std::endl;

   // for( std::size_t i = 0; i < lines.size(); i++ ) {
   //   cv::line( lin_matrix, cv::Point(lines[i][0], lines[i][1]),
   // 	       cv::Point(lines[i][2], lines[i][3]), cv::Scalar(255,0,255), 3, 8 );
   // }
   // cv::imwrite("glth_canny.png", hgh_matrix);
   // cv::imwrite("glth_out_lines.png", lin_matrix);
   // cv::imwrite("glth_out.png",out_matrix);

  return res;
}

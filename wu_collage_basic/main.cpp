//
//  main.cpp
//  wu_collage_basic
//
//  Created by Zhipeng Wu on 8/14/12.
//  Copyright (c) 2012 Zhipeng Wu. All rights reserved.
//

#include "wu_collage_basic.h"
#include <iostream>
#include <time.h>

int main(int argc, const char * argv[])
{

  if (argc != 2) {
    std::cout << "Error number of input arguments" << std::endl;
    return 0;
  }
  std::string image_list(argv[1]);
  int canvas_height = 0;
  while ((canvas_height < 100) || (canvas_height > 2000)) {
    std::cout << "canvas_height [100, 2000]: ";
    std::cin >> canvas_height;
  }
  float expect_alpha = 0;
  while ((expect_alpha < 0.1) || (expect_alpha > 10)) {
    std::cout << "expect_alpha: [0.1, 10]: ";
    std::cin >> expect_alpha;
  }
  
  clock_t start, end;
  CollageBasic my_collage(image_list, canvas_height);
  
  start = clock();
  //bool success = my_collage.CreateCollage();
  bool success = my_collage.CreateCollage(expect_alpha, 1.1);
  if (!success) {
    return -1;
  }
  end = clock();
  
  cv::Mat canvas = my_collage.OutputCollageImage();
  int canvas_width = my_collage.canvas_width();
  float canvas_alpha = my_collage.canvas_alpha();
  std::cout << "canvas_width: " << canvas_width << std::endl;
  std::cout << "canvas_alpha: " << canvas_alpha << std::endl;
  std::cout << "processing time: " << (end - start) * 1000000 / CLOCKS_PER_SEC
            << " us (10e-6 s)" << std::endl;
  my_collage.OutputCollageHtml("/Users/WU/result.html");
  cv::imshow("Collage", canvas);
  cv::waitKey();
  
  return 0;
}


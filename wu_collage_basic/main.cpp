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
  while (canvas_height <= 0) {
    std::cout << "canvas_height: ";
    std::cin >> canvas_height;
    std::cout << std::endl;
  }
  
  clock_t start, end;
  start = clock();
  CollageBasic my_collage(image_list, canvas_height);
  bool success = my_collage.CreateCollage();
  if (!success) {
    std::cout << "Error!" << std::endl;
    return -1;
  }
  cv::Mat canvas = my_collage.OutputCollageImage();
  end = clock();
  int canvas_width = my_collage.canvas_width();
  float canvas_alpha = my_collage.canvas_alpha();
  std::cout << "canvas_width: " << canvas_width << std::endl;
  std::cout << "canvas_alpha: " << canvas_alpha << std::endl;
  std::cout << "processing time: " << (end - start) * 1000 / CLOCKS_PER_SEC << " ms" << std::endl;
  cv::imshow("Collage", canvas);
  cv::waitKey();
  
  return 0;
}


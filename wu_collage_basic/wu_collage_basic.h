//
//  wu_collage_basic.h
//  wu_collage_basic
//
//  Created by Zhipeng Wu on 8/14/12.
//  Copyright (c) 2012 Zhipeng Wu. All rights reserved.
//

#ifndef wu_collage_basic_wu_collage_basic_h
#define wu_collage_basic_wu_collage_basic_h

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

struct TreeNode {
  char child_type_;      // Is this node left child "l" or right child "r".
  bool is_leaf_;         // Is this node a leaf node.
  double alpha_expect_;  // Expected aspect ratio value of this node.
  double alpha_;         // Actual aspect ratio of this node.
  cv::Rect position_;    // The position of the node on canvas.
  TreeNode* left_child_;
  TreeNode* right_child_;
  TreeNode* parent_;
};

// Collage with non-fixed aspect ratio
class CollageBasic {
public:
  // Constructors.
  // We need to let the user decide the canvas height.
  // Since the aspect ratio will be calculate by our program, we can compute
  // canvas width accordingly.
  CollageBasic (const std::string input_image_list, int canvas_height) {
    ReadImageList(input_image_list);
  }
  ~CollageBasic () {
    ReleaseTree();
  }
  // Create Collage.
  bool CreateCollage ();
  // Output collage into a single image.
  bool OutputCollageImage (const std::string output_image_path);
  // Output collage into a html page.
  // bool OutputCollageHtml (const std::string output_html_path);
  
private:
  // Read input images from image list.
  bool ReadImageList (std::string input_image_list);
  // Generate an initial full balanced binary tree with image_num_ leaf nodes.
  bool GenerateInitialTree ();
  // Recursively calculate aspect ratio for all the tree nodes.
  // The return value is the aspect ratio for the root node (whole canvas).
  double CalculateAspectRatio ();
  // Top-down Calculate the image positions in the colage.
  bool CalculatePositions ();
  // Clean and release the binary_tree.
  void ReleaseTree ();
  
  // Vector containing input images.
  std::vector<cv::Mat> image_vec_;
  // Number of images in the collage. (number of leaf nodes in the tree)
  int image_num_;
  // Full balanced binary for collage generation.
  TreeNode* binary_tree_;
  // Canvas height, this is decided by the user.
  int canvas_height_;
  // Canvas aspect ratio, return by CalculateAspectRatio ().
  double canvas_aspect_ratio_;
  // Canvas width, this is computed according to canvas_aspect_ratio_.
  int canvas_width_;
  
};

#endif

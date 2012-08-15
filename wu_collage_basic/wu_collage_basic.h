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

class TreeNode {
public:
  TreeNode() {
    child_type_ = 'N';
    split_type_ = 'N';
    is_leaf_ = true;
    alpha_ = 0;
    alpha_expect_ = 0;
    position_ = cv::Rect(0, 0, 0, 0);
    image_index_ = -1;
    left_child_ = NULL;
    right_child_ = NULL;
    parent_ = NULL;
  }
  char child_type_;      // Is this node left child "l" or right child "r".
  char split_type_;      // If this node is a inner node, we set 'v' or 'h', which indicate
                         // vertical cut or horizontal cut.
  bool is_leaf_;         // Is this node a leaf node or a inner node.
  float alpha_expect_;   // If this node is a leaf, we set expected aspect ratio of this node.
  float alpha_;          // If this node is a leaf, we set actual aspect ratio of this node.
  cv::Rect position_;    // The position of the node on canvas.
  int image_index_;      // If this node is a leaf, it is related with a image.
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
    canvas_height_ = canvas_height;
    canvas_alpha_ = -1;
    canvas_width_ = -1;
    image_num_ = 0;
    tree_root_ = new TreeNode();
  }
  ~CollageBasic() {
    ReleaseTree(tree_root_);
    image_vec_.clear();
    image_alpha_vec_.clear();
  }
  // Create Collage.
  bool CreateCollage();
  // Output collage into a single image.
  cv::Mat OutputCollageImage() const;
  // Output collage into a html page.
  // bool OutputCollageHtml (const std::string output_html_path);
  
  // Accessors:
  int image_num() const {
    return image_num_;
  }
  int canvas_height() const {
    return canvas_height_;
  }
  int canvas_width() const {
    return canvas_width_;
  }
  float canvas_alpha() const {
    return canvas_alpha_;
  }
  
private:
  // Read input images from image list.
  bool ReadImageList(std::string input_image_list);
  // Generate an initial full balanced binary tree with image_num_ leaf nodes.
  bool GenerateInitialTree();
  // Recursively calculate aspect ratio for all the tree nodes.
  // The return value is the aspect ratio for the node.
  float CalculateAlpha(TreeNode* node);
  // Top-down Calculate the image positions in the colage.
  bool CalculatePositions(TreeNode* node);
  // Clean and release the binary_tree.
  void ReleaseTree(TreeNode* node);
  // Random assign a 'v' (vertical cut) or 'h' (horizontal cut) for all the inner nodes.
  void RandomSplitType(TreeNode* node);
  
  // Vector containing input images.
  std::vector<cv::Mat> image_vec_;
  // Vector containing input images' aspect ratios.
  std::vector<float> image_alpha_vec_;
  // Vector containing leaf nodes of the tree.
  std::vector<TreeNode*> tree_leaves_;
  // Number of images in the collage. (number of leaf nodes in the tree)
  int image_num_;
  // Full balanced binary for collage generation.
  TreeNode* tree_root_;
  // Canvas height, this is decided by the user.
  int canvas_height_;
  // Canvas aspect ratio, return by CalculateAspectRatio ().
  float canvas_alpha_;
  // Canvas width, this is computed according to canvas_aspect_ratio_.
  int canvas_width_;
  
};

#endif

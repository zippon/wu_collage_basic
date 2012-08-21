//
//  wu_collage_basic.cpp
//  wu_collage_basic
//
//  Created by Zhipeng Wu on 8/15/12.
//  Copyright (c) 2012 Zhipeng Wu. All rights reserved.
//

#include "wu_collage_basic.h"
#include <math.h>
#include <fstream>
#include <iostream>

CollageBasic::CollageBasic(std::vector<std::string> input_image_list,
                           int canvas_width) {
  for (int i = 0; i < input_image_list.size(); ++i) {
    std::string img_path = input_image_list[i];
    cv::Mat img = cv::imread(img_path.c_str());
    image_vec_.push_back(img);
    float img_alpha = static_cast<float>(img.cols) / img.rows;
    image_alpha_vec_.push_back(img_alpha);
    image_path_vec_.push_back(img_path);
  }
  canvas_width_ = canvas_width;
  canvas_alpha_ = -1;
  canvas_height_ = -1;
  image_num_ = static_cast<int>(input_image_list.size());
  srand(static_cast<unsigned>(time(0)));
  tree_root_ = new TreeNode();
}

// Private member functions:
// Call this function after declare a CollageBasic instance.
// This function will create a non-fixed aspect ratio image collage.
bool CollageBasic::CreateCollage() {
  image_num_ = static_cast<int>(image_vec_.size());
  if (image_num_ == 0) {
    std::cout << "Error: CreateCollage 1" << std::endl;
    return false;
  }
  if (canvas_width_ <= 0) {
    std::cout << "Error: CreateCollage 2" << std::endl;
    return false;
  }
  
  // A: generate a full balanced binary tree with image_num_ leaves.
  GenerateInitialTree();
  // B: recursively calculate aspect ratio.
  canvas_alpha_ = CalculateAlpha(tree_root_);
  canvas_height_ = static_cast<int>(canvas_width_ / canvas_alpha_);
  // C: set the position for all the tile images in the collage.
  tree_root_->position_.x_ = 0;
  tree_root_->position_.y_ = 0;
  tree_root_->position_.height_ = canvas_height_;
  tree_root_->position_.width_ = canvas_width_;
  if (tree_root_->left_child_)
    CalculatePositions(tree_root_->left_child_);
  if (tree_root_->right_child_)
    CalculatePositions(tree_root_->right_child_);
  return true;
};

// If we use CreateCollage, the generated collage may have strange aspect ratio such as
// too big or too small, which seems to be difficult to be shown. We let the user to
// input their expected aspect ratio and fast adjust to make the result aspect ratio
// close to the user defined one.
// The thresh here controls the closeness between the result aspect ratio and the expect
// aspect ratio. e.g. expect_alpha is 1, thresh is 2. The result aspect ratio is around
// [1 / 2, 1 * 2] = [0.5, 2].
// We also define MAX_ITER_NUM = 100,
// If max iteration number is reached and we cannot find a good result aspect ratio,
// this function returns false.
int CollageBasic::CreateCollage(float expect_alpha, float thresh) {
  assert(thresh > 1);
  assert(expect_alpha > 0);
  tree_root_->alpha_expect_ = expect_alpha;
  float lower_bound = expect_alpha / thresh;
  float upper_bound = expect_alpha * thresh;
  int total_iter_counter = 1;
  
  // Do the initial tree generatio and calculation.
  // A: generate a full balanced binary tree with image_num_ leaves.
  GenerateInitialTree();
  // B: recursively calculate aspect ratio.
  canvas_alpha_ = CalculateAlpha(tree_root_);
  
  while ((canvas_alpha_ < lower_bound) || (canvas_alpha_ > upper_bound)) {
    GenerateInitialTree();
    canvas_alpha_ = CalculateAlpha(tree_root_);
    ++total_iter_counter;
    if (total_iter_counter > MAX_TREE_GENE_NUM) {
      std::cout << "*******************************" << std::endl;
      std::cout << "max iteration number reached..." << std::endl;
      std::cout << "*******************************" << std::endl;
      return -1;
    }
  }
  // std::cout << "Canvas generation success!" << std::endl;
  std::cout << "Total iteration number is: " << total_iter_counter << std::endl;
  // After adjustment, set the position for all the tile images.
  canvas_height_ = static_cast<int>(canvas_width_ / canvas_alpha_);
  tree_root_->position_.x_ = 0;
  tree_root_->position_.y_ = 0;
  tree_root_->position_.height_ = canvas_height_;
  tree_root_->position_.width_ = canvas_width_;
  if (tree_root_->left_child_)
    CalculatePositions(tree_root_->left_child_);
  if (tree_root_->right_child_)
    CalculatePositions(tree_root_->right_child_);
  return total_iter_counter;
}

// After calling CreateCollage() and FastAdjust(), call this function to save result
// collage to a image file specified by out_put_image_path.
cv::Mat CollageBasic::OutputCollageImage() const {
  // Traverse tree_leaves_ vector. Resize tile image and paste it on the canvas.
  assert(canvas_alpha_ != -1);
  assert(canvas_width_ != -1);
  cv::Mat canvas(cv::Size(canvas_width_, canvas_height_), image_vec_[0].type());
  assert(image_vec_[0].type() == CV_8UC3);
  for (int i = 0; i < image_num_; ++i) {
    int img_ind = tree_leaves_[i]->image_index_;
    FloatRect pos = tree_leaves_[i]->position_;
    cv::Rect pos_cv(pos.x_, pos.y_, pos.width_, pos.height_);
    cv::Mat roi(canvas, pos_cv);
    assert(image_vec_[0].type() == image_vec_[img_ind].type());
    cv::Mat resized_img(pos_cv.height, pos_cv.width, image_vec_[i].type());
    cv::resize(image_vec_[img_ind], resized_img, resized_img.size());
    resized_img.copyTo(roi);
  }
  return canvas;
}

// After calling CreateCollage(), call this function to save result
// collage to a html file specified by out_put_html_path.
bool CollageBasic::OutputCollageHtml(const std::string output_html_path) {
  assert(canvas_alpha_ != -1);
  assert(canvas_width_ != -1);
  std::ofstream output_html(output_html_path.c_str());
  if (!output_html) {
    std::cout << "Error: OutputCollageHtml" << std::endl;
  }
  
  output_html << "<!DOCTYPE html>\n";
  output_html << "<html>\n";
  output_html << "<h1 style=\"text-align:left\">\n";
  output_html << "\tImage Collage\n";
  output_html << "</h1>\n";
  output_html << "<hr //>\n";
  output_html << "\t<body>\n";
  output_html << "\t\t<div style=\"position:absolute;\">\n";
  for (int i = 0; i < image_num_; ++i) {
    int img_ind = tree_leaves_[i]->image_index_;
    output_html << "\t\t\t<a href=\"";
    output_html << image_path_vec_[img_ind];
    output_html << "\">\n";
    output_html << "\t\t\t\t<img src=\"";
    output_html << image_path_vec_[img_ind];
    output_html << "\" style=\"position:absolute; width:";
    output_html << tree_leaves_[i]->position_.width_;
    output_html << "px; height:";
    output_html << tree_leaves_[i]->position_.height_;
    output_html << "px; left:";
    output_html << tree_leaves_[i]->position_.x_;
    output_html << "px; top:";
    output_html << tree_leaves_[i]->position_.y_;
    output_html << "px;\">\n";
    output_html << "\t\t\t</a>\n";
  }
  output_html << "\t\t</div>\n";
  output_html << "\t</body>\n";
  output_html << "</html>";
  output_html.close();
  return true;
}

// Private member functions:
// The images are stored in the image list, one image path per row.
// This function reads the images into image_vec_ and their aspect
// ratios into image_alpha_vec_.
bool CollageBasic::ReadImageList(std::string input_image_list) {
  std::ifstream input_list(input_image_list.c_str());
  if (!input_list) {
    std::cout << "Error: ReadImageList()" << std::endl;
    return false;
  }
  
  while (!input_list.eof()) {
    std::string img_path;
    std::getline(input_list, img_path);
    // std::cout << img_path <<std::endl;
    cv::Mat img = cv::imread(img_path.c_str());
    image_vec_.push_back(img);
    float img_alpha = static_cast<float>(img.cols) / img.rows;
    image_alpha_vec_.push_back(img_alpha);
    image_path_vec_.push_back(img_path);
  }
  input_list.close();
  return true;
}

// Generate an initial full binary tree with image_num_ leaves.
bool CollageBasic::GenerateInitialTree() {
  tree_leaves_.clear();
  if (tree_root_ != NULL) ReleaseTree(tree_root_);
  tree_root_ = new TreeNode();
  // Step 1: create a (k-1)-depth binary tree with max nodes.
  // 2 ^ (k - 1) <= m < 2 ^ k
  int m = image_num_;
  assert(m != 0);
  int k = 0;
  while (m != 0) {
    m >>= 1;
    ++k;
  }
  std::vector<std::vector<TreeNode*>> node_queue;
  for (int i = 0; i < k; ++i) {
    std::vector<TreeNode*> node_queue_temp;
    node_queue.push_back(node_queue_temp);
  }
  node_queue[0].push_back(tree_root_);
  for (int i = 0; i < k - 1; ++i) {
    for (int j = 0; j < pow(2, i); ++j) {
      
      node_queue[i][j]->left_child_ = new TreeNode();
      node_queue[i][j]->left_child_->child_type_ = 'l';
      node_queue[i][j]->left_child_->parent_ = node_queue[i][j];
      node_queue[i + 1].push_back(node_queue[i][j]->left_child_);
      
      node_queue[i][j]->right_child_ = new TreeNode();
      node_queue[i][j]->right_child_->child_type_ = 'r';
      node_queue[i][j]->right_child_->parent_ = node_queue[i][j];
      node_queue[i + 1].push_back(node_queue[i][j]->right_child_);
      
      node_queue[i][j]->is_leaf_ = false;
    }
  }
  std::vector<TreeNode*> leaf_nodes;
  for (int i = 0; i < node_queue[k - 1].size(); ++i) {
    leaf_nodes.push_back(node_queue[k - 1][i]);
  };

  int leaf_num = pow(2, k - 1);
  bool* leaf_visited = new bool[leaf_num];
  for (int i = 0; i < leaf_num; ++i) leaf_visited[i] = false;
  assert(static_cast<int>(leaf_nodes.size()) == leaf_num);
  // Step 2: randomly select image_num_ - 2 ^ (k - 1) leaves,
  // split them with left and right children. Then, you have a
  // full balanced binary tree with image_num_ leaves.

  int left_leaves = image_num_ - leaf_num;
  int counter = 0;
  while (counter < left_leaves) {
    int rand_ind = random(left_leaves);
    if (leaf_visited[rand_ind] == true) continue;
    leaf_visited[rand_ind] = true;
    leaf_nodes[rand_ind]->is_leaf_ = false;
    leaf_nodes[rand_ind]->left_child_ = new TreeNode();
    leaf_nodes[rand_ind]->left_child_->child_type_ = 'l';
    leaf_nodes[rand_ind]->left_child_->parent_ = leaf_nodes[rand_ind];
    leaf_nodes[rand_ind]->right_child_ = new TreeNode();
    leaf_nodes[rand_ind]->right_child_->child_type_ = 'r';
    leaf_nodes[rand_ind]->right_child_->parent_ = leaf_nodes[rand_ind];
    tree_leaves_.push_back(leaf_nodes[rand_ind]->left_child_);
    tree_leaves_.push_back(leaf_nodes[rand_ind]->right_child_);
    ++counter;
  }
  for (int i = 0; i < leaf_num; ++i) {
    if (leaf_visited[i] != true) tree_leaves_.push_back(leaf_nodes[i]);
  }
  // Now we have created a binary tree with image_num_ leaves.
  // And the vector leaf_nodes_new stores all the leaf nodes.
  assert(static_cast<int>(tree_leaves_.size()) == image_num_);
  delete [] leaf_visited;
  leaf_nodes.clear();
  // Step 3: random dispatch images to leaf nodes.
  bool* img_visited = new bool[image_num_];
  for (int i = 0; i < image_num_; ++i) img_visited[i] = false;
  counter = 0;
  while (counter < image_num_) {
    int rand_img_ind = random(image_num_);
    if (img_visited[rand_img_ind] == true) continue;
    img_visited[rand_img_ind] = true;
    // Set the related image index and aspect ratio for leaf nodes.
    tree_leaves_[counter]->image_index_ = rand_img_ind;
    tree_leaves_[counter]->alpha_ = image_alpha_vec_[rand_img_ind];
    ++counter;
  }
  delete [] img_visited;
  // Step 4: assign a random 'v' or 'h' for all the inner nodes.
  RandomSplitType(tree_root_);
  return true;
}

// Recursively calculate aspect ratio for all the inner nodes.
// The return value is the aspect ratio for the node.
float CollageBasic::CalculateAlpha(TreeNode* node) {
  if (!node->is_leaf_) {
    float left_alpha = CalculateAlpha(node->left_child_);
    float right_alpha = CalculateAlpha(node->right_child_);
    if (node->split_type_ == 'v') {
      node->alpha_ = left_alpha + right_alpha;
      return node->alpha_;
    } else if (node->split_type_ == 'h') {
      node->alpha_ = (left_alpha * right_alpha) / (left_alpha + right_alpha);
      return node->alpha_;
    } else {
      std::cout << "Error: CalculateAlpha" << std::endl;
      return -1;
    }
  } else {
    // This is a leaf node, just return the image's aspect ratio.
    return node->alpha_;
  }
}

//// Top-down Calculate the image positions in the colage.
//bool CollageBasic::CalculatePositions(TreeNode* node) {
//  // Step 1: calculate height & width.
//  if (node->parent_->split_type_ == 'v') {
//    // Vertical cut, height unchanged.
//    node->position_.height = node->parent_->position_.height;
//    node->position_.width = node->position_.height * node->alpha_;
//  } else if (node->parent_->split_type_ == 'h') {
//    // Horizontal cut, width unchanged.
//    node->position_.width = node->parent_->position_.width;
//    node->position_.height = static_cast<int>
//        (node->position_.width / node->alpha_);
//  } else {
//    std::cout << "Error: CalculatePositions step 1" << std::endl;
//    return false;
//  }
//  
//  // Step 2: calculate x & y.
//  if (node->child_type_ == 'l') {
//    // If it is left child, use its parent's x & y.
//    node->position_.x = node->parent_->position_.x;
//    node->position_.y = node->parent_->position_.y;
//  } else if (node->child_type_ == 'r') {
//    if (node->parent_->split_type_ == 'v') {
//      // y (row) unchanged, x (colmn) changed.
//      node->position_.y = node->parent_->position_.y;
//      node->position_.x = node->parent_->position_.x +
//                          node->parent_->position_.width -
//                          node->position_.width;
//    } else if (node->parent_->split_type_ == 'h') {
//      // x (column) unchanged, y (row) changed.
//      node->position_.x = node->parent_->position_.x;
//      node->position_.y = node->parent_->position_.y +
//                          node->parent_->position_.height -
//                          node->position_.height;
//    } else {
//      std::cout << "Error: CalculatePositions step 2 - 1" << std::endl;
//    }
//  } else {
//    std::cout << "Error: CalculatePositions step 2 - 2" << std::endl;
//    return false;
//  }
//  
//  // Calculation for children.
//  if (node->left_child_) {
//    bool success = CalculatePositions(node->left_child_);
//    if (!success) return false;
//  }
//  if (node->right_child_) {
//    bool success = CalculatePositions(node->right_child_);
//    if (!success) return false;
//  }
//  return true;
//}

// Top-down Calculate the image positions in the colage.
bool CollageBasic::CalculatePositions(TreeNode* node) {
  // Step 1: calculate height & width.
  if (node->parent_->split_type_ == 'v') {
    // Vertical cut, height unchanged.
    node->position_.height_ = node->parent_->position_.height_;
    if (node->child_type_ == 'l') {
      node->position_.width_ = node->position_.height_ * node->alpha_;
    } else if (node->child_type_ == 'r') {
      node->position_.width_ = node->parent_->position_.width_ -
          node->parent_->left_child_->position_.width_;
    } else {
      std::cout << "Error: CalculatePositions step 0" << std::endl;
      return false;
    }
  } else if (node->parent_->split_type_ == 'h') {
    // Horizontal cut, width unchanged.
    node->position_.width_ = node->parent_->position_.width_;
    if (node->child_type_ == 'l') {
      node->position_.height_ = node->position_.width_ / node->alpha_;
    } else if (node->child_type_ == 'r') {
      node->position_.height_ = node->parent_->position_.height_ -
      node->parent_->left_child_->position_.height_;
    }
  } else {
    std::cout << "Error: CalculatePositions step 1" << std::endl;
    return false;
  }
  
  // Step 2: calculate x & y.
  if (node->child_type_ == 'l') {
    // If it is left child, use its parent's x & y.
    node->position_.x_ = node->parent_->position_.x_;
    node->position_.y_ = node->parent_->position_.y_;
  } else if (node->child_type_ == 'r') {
    if (node->parent_->split_type_ == 'v') {
      // y (row) unchanged, x (colmn) changed.
      node->position_.y_ = node->parent_->position_.y_;
      node->position_.x_ = node->parent_->position_.x_ +
      node->parent_->position_.width_ -
      node->position_.width_;
    } else if (node->parent_->split_type_ == 'h') {
      // x (column) unchanged, y (row) changed.
      node->position_.x_ = node->parent_->position_.x_;
      node->position_.y_ = node->parent_->position_.y_ +
      node->parent_->position_.height_ -
      node->position_.height_;
    } else {
      std::cout << "Error: CalculatePositions step 2 - 1" << std::endl;
    }
  } else {
    std::cout << "Error: CalculatePositions step 2 - 2" << std::endl;
    return false;
  }
  
  // Calculation for children.
  if (node->left_child_) {
    bool success = CalculatePositions(node->left_child_);
    if (!success) return false;
  }
  if (node->right_child_) {
    bool success = CalculatePositions(node->right_child_);
    if (!success) return false;
  }
  return true;
}

// Release the memory for binary tree.
void CollageBasic::ReleaseTree(TreeNode* node) {
  if (node == NULL) return;
  if (node->left_child_) ReleaseTree(node->left_child_);
  if (node->right_child_) ReleaseTree(node->right_child_);
  delete node;
}

void CollageBasic::RandomSplitType(TreeNode* node) {
  if (node == NULL) return;
  if (node->is_leaf_ == true) return;
  int v_h = random(2);
  if (v_h == 1) {
    node->split_type_ = 'v';
  } else if (v_h == 0) {
    node->split_type_ = 'h';
  } else {
    std::cout << "Error: RandomSplitType()" << std::endl;
    return;
  }
  RandomSplitType(node->left_child_);
  RandomSplitType(node->right_child_);
}

//void CollageBasic::AdjustAlpha(TreeNode *node, float thresh) {
//  assert(thresh > 1);
//  if (node->is_leaf_) return;
//  if (node == NULL) return;
//  
//  float thresh_2 = 1 + (thresh - 1) / 2;
//  
//  if (node->alpha_ > node->alpha_expect_ * thresh_2) {
//    // Too big actual aspect ratio.
//    node->split_type_ = 'h';
//    node->left_child_->alpha_expect_ = node->alpha_expect_ * 2;
//    node->right_child_->alpha_expect_ = node->alpha_expect_ * 2;
//  } else if (node->alpha_ < node->alpha_expect_ / thresh_2 ) {
//    // Too small actual aspect ratio.
//    node->split_type_ = 'v';
//    node->left_child_->alpha_expect_ = node->alpha_expect_ / 2;
//    node->right_child_->alpha_expect_ = node->alpha_expect_ / 2;
//  } else {
//    // Aspect ratio is okay.
//    if (node->split_type_ == 'h') {
//      node->left_child_->alpha_expect_ = node->alpha_expect_ * 2;
//      node->right_child_->alpha_expect_ = node->alpha_expect_ * 2;
//    } else if (node->split_type_ == 'v') {
//      node->left_child_->alpha_expect_ = node->alpha_expect_ * 2;
//      node->right_child_->alpha_expect_ = node->alpha_expect_ * 2;
//    } else {
//      std::cout << "Error: AdjustAlpha" << std::endl;
//      return;
//    }
//  }
//  AdjustAlpha(node->left_child_, thresh);
//  AdjustAlpha(node->right_child_, thresh);
//}

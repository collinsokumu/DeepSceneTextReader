#include "scene_text_reader.h"

namespace scene_text_reader{

  SceneTextReader::SceneTextReader(){

  }

  SceneTextReader::SceneTextReader(const std::string& detector_graph_filename, const string& recognizer_graph_filename,
     const std::string& detector_model, const std::string& dictionary_filename, const std::string& recognizer_model)
  {
    if(detector_model == "FasterRCNN"){
      detector = new FasterRCNNTextDetector(detector_graph_filename);
    }else{
      LOG(ERROR) <<detector_model + " not implemented yet";
    }
    if(recognizer_model == "CTC"){
      recognizer = new CTCSceneTextRecognizer(recognizer_graph_filename, dictionary_filename);
    }else{
      LOG(ERROR) <<recognizer_model + " not implemented yet";
    }
  }
  
  void SceneTextReader::extract_word_regions(cv::Mat& image, std::vector<TextBox>& boxes, std::vector<cv::Mat>& word_regions){
    int num_word = boxes.size();
    if(num_word == 0) return;

    for(int i=0; i<num_word; i++){
      cv::Mat word_region;
      float angle = get_angle(boxes[i]);
      cv::Mat cropped;
      std::vector<cv::Point> new_points;
      get_cropped_extend_image(image, boxes[i], cropped, new_points);
      
      cv::Mat rotated;
      std::vector<cv::Point> rotated_points;
      rotate_image_and_points(cropped, new_points, angle, rotated, rotated_points);
      word_regions.push_back(rotated);
    }
  }

  void SceneTextReader::read_text(cv::Mat& image, std::vector<TextBox>& res){ 
    detector->run_graph(image, res);
    std::cout<<"found "<<res.size()<<" number of text"<<std::endl; 
    std::vector<cv::Mat> word_regions;
    extract_word_regions(image, res, word_regions);
    //preprocess all the images;
    std::vector<cv::Mat> preprocessed_images = recognizer->preprocess_images(word_regions);
    std::cout<<preprocessed_images[0].rows<<" "<<preprocessed_images[0].cols<<std::endl;
    std::vector<string> output_texts = recognizer->run_graph(preprocessed_images);
    for(int i=0; i<res.size(); i++){
      res[i].set_text(output_texts[i]);
    }
  }
}

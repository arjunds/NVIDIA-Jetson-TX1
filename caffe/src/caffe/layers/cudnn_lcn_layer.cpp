#ifdef USE_CUDNN
#include <vector>

#include "caffe/filler.hpp"
#include "caffe/layer.hpp"
#include "caffe/util/im2col.hpp"
#include "caffe/util/math_functions.hpp"
#include "caffe/vision_layers.hpp"

namespace caffe {

template <typename Dtype, typename Mtype>
void CuDNNLCNLayer<Dtype,Mtype>::LayerSetUp(const vector<Blob<Dtype,Mtype>*>& bottom,
    const vector<Blob<Dtype,Mtype>*>& top) {
  LRNLayer<Dtype,Mtype>::LayerSetUp(bottom, top);

  CUDNN_CHECK(cudnnCreateLRNDescriptor(&norm_desc_));
  cudnn::createTensor4dDesc<Dtype>(&bottom_desc_);
  cudnn::createTensor4dDesc<Dtype>(&top_desc_);

  // create a LRN handle
  handles_setup_ = true;

  size_ = this->layer_param().lrn_param().local_size();
  pre_pad_ = (size_ - 1) / 2;
  alpha_ = this->layer_param().lrn_param().alpha();
  beta_ = this->layer_param().lrn_param().beta();
  k_ = this->layer_param().lrn_param().k();
}

template <typename Dtype, typename Mtype>
void CuDNNLCNLayer<Dtype,Mtype>::Reshape(const vector<Blob<Dtype,Mtype>*>& bottom,
    const vector<Blob<Dtype,Mtype>*>& top) {
  LRNLayer<Dtype,Mtype>::Reshape(bottom, top);
  cudnn::setTensor4dDesc<Dtype>(&bottom_desc_, bottom[0]->num(),
      this->channels_, this->height_, this->width_);
  cudnn::setTensor4dDesc<Dtype>(&top_desc_, bottom[0]->num(),
      this->channels_, this->height_, this->width_);
  CUDNN_CHECK(cudnnSetLRNDescriptor(norm_desc_, size_, alpha_, beta_, k_));

  // size for tempData buffers
  tempDataSize = sizeof(Dtype)*bottom[0]->num()* \
                            this->channels_*this->height_*this->width_;
}

template <typename Dtype, typename Mtype>
CuDNNLCNLayer<Dtype,Mtype>::~CuDNNLCNLayer() {
  // Check that handles have been setup before destroying.
  if (!handles_setup_) { return; }

  CUDNN_CHECK(cudnnDestroyTensorDescriptor(bottom_desc_));
  CUDNN_CHECK(cudnnDestroyTensorDescriptor(top_desc_));

  // destroy LRN handle
  CUDNN_CHECK(cudnnDestroyLRNDescriptor(norm_desc_));
}

INSTANTIATE_CLASS(CuDNNLCNLayer);

}   // namespace caffe
#endif

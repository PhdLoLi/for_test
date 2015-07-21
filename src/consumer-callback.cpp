/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014 Regents of the University of California.
 *
 * @author Lijing Wang phdloli@ucla.edu
 */

#include "video-player.hpp"
#include "consumer-callback.hpp"
#include <string>
#include <pthread.h>
#include <ctime>
#include <chrono>

// Enclosing code in ndn simplifies coding (can also use `using namespace ndn`)
namespace ndn {
// Additional nested namespace could be used to prevent/limit name contentions

 
  ConsumerCallback::ConsumerCallback()
  {
    payload_v = 0;
    payload_a = 0;
    interest_s = 0;
    interest_r = 0;
    interest_r_v = 0;
    interest_r_a = 0;
    interest_retx = 0;
    interest_expr = 0;
    data_v = 0;
    data_a = 0;
    start_frame_v = 0;
    start_frame_a = 0;
    frame_cnt_v = 0;
    frame_cnt_a = 0;
    buffers_v.resize((VIDEO_SIZE * 2), DataNode_G(0, NULL));
    buffers_a.resize((AUDIO_SIZE * 2), DataNode_G(0, NULL));
    ready_v = false;
    ready_a = false;
    playback_v = 0;
    playback_a = 0;
    framenumber_v = 0;
    framenumber_a = 0;
//    std::cout << "Construction" << std::endl;
//    player.playbin_appsrc_init();
//    std::cout << "Construction Over" << std::endl;
  }
  
  void
  ConsumerCallback::initFrameVideo() {
    frame_v = new VideoFrame();
  }

  void
  ConsumerCallback::initFrameAudio() {
    frame_a = new VideoFrame();
  }

  void
  ConsumerCallback::processDataVideo(Consumer& con, const Data& data) {
//    std::cout << "Video Data Received! Name: " << data.getName().toUri() << std::endl;
    interest_r_v_m.lock(); 
//    printf("DATA IN CNTX Name: %s  FinalBlockId: %s\n", data.getName().toUri().c_str(), data.getFinalBlockId().toUri().c_str());
    frame_v->addSegment(const_cast<Data&>(data));
    interest_r_v++;
    interest_r++;
    interest_r_v_m.unlock(); 
  }

  void
  ConsumerCallback::processDataAudio(Consumer& con, const Data& data) {
//    data.getContent();
//    std::cout << "Audio Data Received! Name: " << data.getName().toUri() << std::endl;
    frame_a->addSegment(const_cast<Data&>(data));
    interest_r_a++;
  }

  void
  ConsumerCallback::pushQueueVideo() {
    frame_cnt_v_m.lock();
    printf("pushQueue Video Frame Counter: %d \n", frame_cnt_v);
    int bufferSize;
    std::vector<uint8_t> bufferVec;
    frame_v->getData(bufferVec, bufferSize);
//    std::cout << "Video bufferSize: "  << std::dec << bufferSize << std::endl;
    if (bufferSize > 0) {
      payload_v += bufferSize;
      frame_cnt_v++;
      player.h264_appsrc_data(bufferVec.data(), bufferSize);
    } 
    frame_cnt_v_m.unlock();
  }

  void
  ConsumerCallback::pushQueueAudio() {
    //const uint8_t* buffer = new uint8_t[100000];
    int bufferSize;
    std::vector<uint8_t> bufferVec;
    frame_a->getData(bufferVec, bufferSize);
//    std::cout << "Audio bufferSize: " << std::dec << bufferSize << std::endl;
    if (bufferSize > 0) {
      payload_a += bufferSize;
      frame_cnt_a++;
      player.h264_appsrc_data_audio(bufferVec.data(), bufferSize);
    } 
  }

  void
  ConsumerCallback::processPayload(Consumer& con, const uint8_t* buffer, size_t bufferSize)
  {
//    std::cout << "video Size:" << std::dec << bufferSize <<std::endl;
//    std::cout << "video bufferSize " << bufferSize <<std::endl;
//    std::cout << "@buffer " << &buffer <<std::endl;
  
    frame_cnt_v_m.lock();
//    auto finish = std::chrono::high_resolution_clock::now();
//    printf("%llu ms\n", std::chrono::duration_cast<std::chrono::milliseconds>(finish-start_v).count());

    Name suffix;
    con.getContextOption(SUFFIX, suffix);
    int frameNumber = std::stoi(suffix.get(0).toUri());

//    printf("Video Data received! Frame_Interest: %d Frame_Counter: %d \n", frameNumber, frame_cnt_v);

    payload_v += bufferSize;
    
    frame_cnt_v++;

    if (frameNumber >= playback_v && buffers_v[frameNumber % (VIDEO_SIZE * 2)].data == NULL && buffers_v[frameNumber % (VIDEO_SIZE * 2)].length == 0) {
      uint8_t* bufferTmp = new uint8_t[bufferSize];
      memcpy(bufferTmp, buffer, bufferSize);
      buffers_v[frameNumber % (VIDEO_SIZE * 2)].data = bufferTmp;
      buffers_v[frameNumber % (VIDEO_SIZE * 2)].length = bufferSize;
    } else {
      printf("Shouldn't Happen!!!!! for VIDEO frameNumber : %d\n", frameNumber);
      while (playback_v <= frameNumber - VIDEO_SIZE * 2) {
        if (buffers_v[playback_v % (VIDEO_SIZE * 2)].data != NULL) {
          player.h264_appsrc_data(buffers_v[playback_v % (VIDEO_SIZE * 2)].data, buffers_v[playback_v % (VIDEO_SIZE * 2)].length);
          buffers_v[playback_v % (VIDEO_SIZE * 2)].data = NULL;
          buffers_v[playback_v % (VIDEO_SIZE * 2)].length = 0;
        }
        playback_v++;
      }
    }

    // playback_v move data to playingback queue
    while (playback_v <= framenumber_v && buffers_v[playback_v % (VIDEO_SIZE * 2)].data != NULL) {

      player.h264_appsrc_data(buffers_v[playback_v % (VIDEO_SIZE * 2)].data, buffers_v[playback_v % (VIDEO_SIZE * 2)].length);
      buffers_v[playback_v % (VIDEO_SIZE * 2)].data = NULL;
      buffers_v[playback_v % (VIDEO_SIZE * 2)].length = 0;
      playback_v++;

    }

//    if (framenumber_v > 0)
//      suffix = Name(std::to_string(++framenumber_v));

//    con.setContextOption(SUFFIX, Name("empty"));
//
//    mut_v.lock();
//    ready_v = true;
//    con_v.notify_all();
//    mut_v.unlock();

    frame_cnt_v_m.unlock();
//    if (framenumber_v > 0) {
//      printf("video suffix %s\n", suffix.toUri().c_str());
//      con.consume(suffix);
//      printf("video after consume suffix %s\n", suffix.toUri().c_str());
//    }

  }

  void
  ConsumerCallback::processPayloadAudio(Consumer& con, const uint8_t* buffer, size_t bufferSize)
  {

    interest_r_a_m.lock();

//    auto finish = std::chrono::high_resolution_clock::now();
//    printf("%llu ms\n", std::chrono::duration_cast<std::chrono::milliseconds>(finish-start_a).count());

    Name suffix;
    con.getContextOption(SUFFIX, suffix);
    int frameNumber = std::stoi(suffix.get(0).toUri());

//    printf("Audio Data received! Frame_Interest: %d Frame_Counter: %d \n", frameNumber, frame_cnt_a);
    payload_a += bufferSize;
//    player.h264_appsrc_data_audio(buffer, bufferSize);
    interest_r++;
    interest_r_a++;
    frame_cnt_a++;

//    con.setContextOption(SUFFIX, Name("empty"));
//
//    mut_a.lock();
//    ready_a = true;
//    con_a.notify_all();
//    mut_a.unlock();

    if (frameNumber >= playback_a && buffers_a[frameNumber % (AUDIO_SIZE * 2)].data == NULL && buffers_a[frameNumber % (AUDIO_SIZE * 2)].length == 0) {
      uint8_t* bufferTmp = new uint8_t[bufferSize];
      memcpy(bufferTmp, buffer, bufferSize);
      buffers_a[frameNumber % (AUDIO_SIZE * 2)].data = bufferTmp;
      buffers_a[frameNumber % (AUDIO_SIZE * 2)].length = bufferSize;
    } else {
      printf("Shouldn't Happen!!!!! for AUDIO frameNumber : %d\n", frameNumber);
      while (playback_a <= frameNumber - AUDIO_SIZE * 2) {
        if (buffers_a[playback_a % (AUDIO_SIZE * 2)].data != NULL) {
          player.h264_appsrc_data_audio(buffers_a[playback_a % (AUDIO_SIZE * 2)].data, buffers_a[playback_a % (AUDIO_SIZE * 2)].length);
          buffers_a[playback_a % (AUDIO_SIZE * 2)].data = NULL;
          buffers_a[playback_a % (AUDIO_SIZE * 2)].length = 0;
        }
        playback_a++;
      }
    }

    // playback_a move data to playingback queue
    while (playback_a <= framenumber_a && buffers_a[playback_a % (AUDIO_SIZE * 2)].data != NULL) {

      player.h264_appsrc_data_audio(buffers_a[playback_a % (AUDIO_SIZE * 2)].data, buffers_a[playback_a % (AUDIO_SIZE * 2)].length);
      buffers_a[playback_a % (AUDIO_SIZE * 2)].data = NULL;
      buffers_a[playback_a % (AUDIO_SIZE * 2)].length = 0;
      playback_a++;
    }

//    if (framenumber_a > 0)
//      suffix = Name(std::to_string(++framenumber_a));

    interest_r_a_m.unlock();
//    if (framenumber_a > 0) {
//      printf("audio suffix %s\n", suffix.toUri().c_str());
//      con.consume(suffix);
//      printf("audio after consume suffix %s\n", suffix.toUri().c_str());
//    }

  }
  
  void
  ConsumerCallback::processStreaminfo(Consumer& con, const uint8_t* buffer, size_t bufferSize)
  {
    std::string streaminfo((char*) buffer);
  //  long fileLength = std::stol(content);
  //  std::cout << "bufferSize " << bufferSize <<std::endl;
  //  std::cout << "buffer " << buffer <<std::endl;
 //   std::cout << "streaminfo " << streaminfo <<std::endl;
  //  std::cout << "fileLength " << fileLength <<std::endl;
    std::cout << "processStreaminfo " << streaminfo << std::endl;
    player.get_streaminfo(streaminfo);
  }

  void
  ConsumerCallback::processStreaminfoAudio(Consumer& con, const uint8_t* buffer, size_t bufferSize)
  {
    std::string streaminfo((char*) buffer);
    std::cout << "processStreaminfo_audio " << streaminfo << std::endl;
    player.get_streaminfo_audio(streaminfo);
  }

  void
  ConsumerCallback::streaminfoEnter(Consumer& con, const Data& data)
  {
    std::string start_frame_str = data.getName().get(-2).toUri();
    start_frame_v = std::stoi(start_frame_str);
    std::cout << "streaminfoEnter Name: " << data.getName() << " start_frame " << start_frame_v << std::endl;
  }

  void
  ConsumerCallback::streaminfoEnterAudio(Consumer& con, const Data& data)
  {
    std::string start_frame_str = data.getName().get(-2).toUri();
    start_frame_a = std::stoi(start_frame_str);
    std::cout << "streaminfoEnter Name: " << data.getName() << " start_frame " << start_frame_a << std::endl;
  }
  void
  ConsumerCallback::processData(Consumer& con, const Data& data)
  {
    interest_r_m.lock();
    interest_r++;
    interest_r_v++;
//    printf("DATA IN CNTX Name: %s  FinalBlockId: %s\n", data.getName().toUri().c_str(), data.getFinalBlockId().toUri().c_str());
    interest_r_m.unlock();
  }
  
  bool
  ConsumerCallback::verifyData(Consumer& con, Data& data)
  {
    return true;
  }
  
  void
  ConsumerCallback::processConstData(Consumer& con, const Data& data)
  {}
  
  void
  ConsumerCallback::processLeavingStreaminfo(Consumer& con, Interest& interest)
  {
//    std::cout << "LEAVES streaminfo name " << interest.getName() << std::endl;
  }
  void
  ConsumerCallback::processLeavingInterest(Consumer& con, Interest& interest)
  {
    interest_s_m.lock();
    interest_s ++;
//    printf("Leaving Interest Name: %s\n", interest.toUri().c_str());
    interest_s_m.unlock();
//   std::cout << "LEAVES " << interest.toUri() << std::endl;
//    std::cout << "LEAVES name " << interest.getName() << std::endl;
  }  

  void
  ConsumerCallback::onRetx(Consumer& con, Interest& interest)
  {
    interest_retx_m.lock();
    interest_retx ++;
    std::cout << "Retransmitted " << interest.getName() << std::endl;
    interest_retx_m.unlock();
  }

  void
  ConsumerCallback::onExpr(Consumer& con, Interest& interest)
  {
    interest_expr_m.lock();
    interest_expr ++;
    std::cout << "Expired " << interest.getName() << std::endl;
    interest_expr_m.unlock();
  }

  void
  ConsumerCallback::onRetx_info(Consumer& con, Interest& interest)
  {
    printf("Info Retransmission : %s\n", interest.getName().toUri().c_str());
  }

  void
  ConsumerCallback::onExpr_info(Consumer& con, Interest& interest)
  {
    printf("Info Expiration : %s\n", interest.getName().toUri().c_str());
  }
} // namespace ndn

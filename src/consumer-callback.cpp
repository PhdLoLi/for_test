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

// Enclosing code in ndn simplifies coding (can also use `using namespace ndn`)
namespace ndn {
// Additional nested namespace could be used to prevent/limit name contentions

  int times_video = 0;
  int times_audio = 0;
 
  ConsumerCallback::ConsumerCallback()
  {
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
    printf("DATA IN CNTX Name: %s  FinalBlockId: %s\n", data.getName().toUri().c_str(), data.getFinalBlockId().toUri().c_str());
    frame_v->addSegment(const_cast<Data&>(data));
    interest_r_v++;
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
    int bufferSize;
    std::vector<uint8_t> bufferVec;
    frame_v->getData(bufferVec, bufferSize);
//    std::cout << "Video bufferSize: "  << std::dec << bufferSize << std::endl;
    if (bufferSize > 0) {
      payload_v += bufferSize;
      player.h264_appsrc_data(bufferVec.data(), bufferSize);
      times_video ++;
    } 
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
      player.h264_appsrc_data_audio(bufferVec.data(), bufferSize);
      times_audio ++;
    } 
  }

  void
  ConsumerCallback::processPayload(Consumer& con, const uint8_t* buffer, size_t bufferSize)
  {
//    std::cout << "video times processPayload " << std::dec << times_video <<std::endl;
//    std::cout << "video Size:" << std::dec << bufferSize <<std::endl;
//    std::cout << "video bufferSize " << bufferSize <<std::endl;
//    std::cout << "@buffer " << &buffer <<std::endl;
    payload_v += bufferSize;
    player.h264_appsrc_data(buffer, bufferSize);
    times_video ++;
    interest_r_v++;
//    std::cout << "processPayload video over " << std::endl;
  }

  void
  ConsumerCallback::processPayloadAudio(Consumer& con, const uint8_t* buffer, size_t bufferSize)
  {
//    std::cout << "audio times processPayload " << std::dec << times_audio <<std::endl;
//    std::cout << "audio Size:" << std::dec << bufferSize <<std::endl;
//    std::cout << "audio bufferSize " << bufferSize <<std::endl;
//    std::cout << "@buffer " << &buffer <<std::endl;
    payload_a += bufferSize;
    player.h264_appsrc_data_audio(buffer, bufferSize);
    times_audio ++;
    interest_r_a++;
//    std::cout << "processPayload audio over " << std::endl;
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
    interest_r++;
    printf("DATA IN CNTX Name: %s  FinalBlockId: %s\n", data.getName().toUri().c_str(), data.getFinalBlockId().toUri().c_str());
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
    std::cout << "LEAVES streaminfo name " << interest.getName() << std::endl;
  }
  void
  ConsumerCallback::processLeavingInterest(Consumer& con, Interest& interest)
  {
    interest_s ++;
//   std::cout << "LEAVES " << interest.toUri() << std::endl;
//    std::cout << "LEAVES name " << interest.getName() << std::endl;
  }  

  void
  ConsumerCallback::onRetx(Consumer& con, Interest& interest)
  {
    interest_retx ++;
    std::cout << "Retransmitted " << interest.getName() << std::endl;
  }

  void
  ConsumerCallback::onExpr(Consumer& con, Interest& interest)
  {
    interest_expr ++;
    std::cout << "Expired " << interest.getName() << std::endl;
  }
} // namespace ndn

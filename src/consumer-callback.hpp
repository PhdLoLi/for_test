/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Regents of the University of California.
 *
 * @author Lijing Wang <phdloli@ucla.edu>
 */


#ifndef CONSUMER_CALLBACK_HPP
#define CONSUMER_CALLBACK_HPP
#include <ndn-cxx/contexts/consumer-context.hpp>
#include <fstream>
#include <iostream>
#include "video-player.hpp"
#include "video-frame.hpp"

// Enclosing code in ndn simplifies coding (can also use `using namespace ndn`)
namespace ndn {
// Additional nested namespace could be used to prevent/limit name contentions

  class ConsumerCallback
  {
      
  public:
    ConsumerCallback();
    
    void
    processPayload(Consumer& con, const uint8_t* buffer, size_t bufferSize);
    
    void
    processStreaminfo(Consumer& con, const uint8_t* buffer, size_t bufferSize);

    void
    processPayloadAudio(Consumer& con, const uint8_t* buffer, size_t bufferSize);
    
    void
    processStreaminfoAudio(Consumer& con, const uint8_t* buffer, size_t bufferSize);
    
    void
    initFrameVideo();

    void
    initFrameAudio();

    void
    pushQueueVideo();

    void
    pushQueueAudio();

    void
    processDataVideo(Consumer& con, const Data& data);
    
    void
    processDataAudio(Consumer& con, const Data& data);
    
    void
    processData(Consumer& con, const Data& data);
    
    void
    streaminfoEnter(Consumer& con, const Data& data);

    void
    streaminfoEnterAudio(Consumer& con, const Data& data);

    bool
    verifyData(Consumer& con, Data& data);
    
    void
    processConstData(Consumer& con, const Data& data);
    
    void
    processLeavingInterest(Consumer& con, Interest& interest);
    
    void
    processLeavingStreaminfo(Consumer& con, Interest& interest);

    void
    onRetx(Consumer& con, Interest& interest);

    void
    onExpr(Consumer& con, Interest& interest);

    void
    processFile(Consumer& con, const uint8_t* buffer, size_t bufferSize)
    {
      std::ofstream filestr;
    	// binary open
      
    	filestr.open ("/Users/lijing/next-ndnvideo/build/haha.mp3",  std::ios::out | std::ios::app | std::ios::binary);
      filestr.write((char *)buffer, bufferSize);
      filestr.close();
 
    }
    
    VideoFrame *frame_v;
    VideoFrame *frame_a;
    VideoPlayer player;
    gsize payload_v = 0;
    gsize payload_a = 0;
    gsize interest_s = 0;
    gsize interest_r = 0;
    gsize interest_r_v = 0;
    gsize interest_r_a = 0;
    gsize interest_retx = 0;
    gsize interest_expr = 0;
    gsize data_v = 0;
    gsize data_a = 0;
    gsize start_frame_v = 0;
    gsize start_frame_a = 0;
  };

} // namespace ndn
#endif

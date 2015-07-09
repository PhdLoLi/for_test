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
    gsize payload_v;
    gsize payload_a;
    gsize interest_s;
    gsize interest_r;
    gsize interest_r_v;
    gsize interest_r_a;
    gsize interest_retx;
    gsize interest_expr;
    gsize data_v;
    gsize data_a;
    gsize start_frame_v;
    gsize start_frame_a;
    int frame_cnt_v;
    int frame_cnt_a;

    boost::mutex frame_cnt_v_m;
    boost::mutex frame_cnt_a_m;
    boost::mutex interest_s_m;
    boost::mutex interest_r_m;
    boost::mutex interest_r_v_m;
    boost::mutex interest_r_a_m;
    boost::mutex interest_retx_m;
    boost::mutex interest_expr_m;
  };

} // namespace ndn
#endif

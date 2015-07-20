/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014 Regents of the University of California.
 *
 * @author Lijing Wang phdloli@ucla.edu
 */

#include "consumer.hpp"
#include <ndn-cxx/contexts/face-helper.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <thread>
#include <chrono>
// Enclosing code in ndn simplifies coding (can also use `using namespace ndn`)
namespace ndn {
// Additional nested namespace could be used to prevent/limit name contentions


time_t time_start;
ConsumerCallback cb_consumer;


  //when control-c detected, doing the analysis 
static void sig_int(int num)
//                    const boost::system::error_code& error,
//                    int signalNo,
//                    boost::asio::signal_set& signalSet,
//                    boost::asio::io_service& io)
  {
    time_t time_end = std::time(0);
    double seconds = difftime(time_end, time_start);
    std::cout << "  " << std::endl;
    std::cout << "｡:.ﾟヽ(*´∀`)ﾉﾟ. Test Result ヽ(✿ﾟ▽ﾟ)ノ  " << std::endl;
    std::cout << "  " << std::endl;
    std::cout << "-------- Throughput --------  " << std::endl;
    std::cout << "Video Bytes: " << std::dec << cb_consumer.payload_v << "  PayLoad_Throughput: " << cb_consumer.payload_v/seconds << " Bytes/Seconds" <<std::endl;
    std::cout << "Audio Bytes: " << cb_consumer.payload_a << "  PayLoad_Throughput: " << cb_consumer.payload_a/seconds << " Bytes/Seconds" <<std::endl;
    std::cout << "Total Bytes: " << cb_consumer.payload_v + cb_consumer.payload_a << "  PayLoad_Throughput: " << (cb_consumer.payload_v + cb_consumer.payload_a)/seconds << " Bytes/Seconds" << std::endl;

    std::cout << "Video Frames: " << std::dec << cb_consumer.frame_cnt_v << "  Frame_Throughput: " << cb_consumer.frame_cnt_v/seconds << " /Seconds" <<std::endl;
    std::cout << "Audio Frames: " << std::dec << cb_consumer.frame_cnt_a << "  Frame_Throughput: " << cb_consumer.frame_cnt_a/seconds << " /Seconds" <<std::endl;
    std::cout << "Total Frames: " << cb_consumer.frame_cnt_v + cb_consumer.frame_cnt_a << "  Frame_Throughput: " << (cb_consumer.frame_cnt_v + cb_consumer.frame_cnt_a)/seconds << " /Seconds" << std::endl;

    std::cout << "  " << std::endl;
    std::cout << "-------- Interest --------  " << std::endl;
    std::cout << "Interest_Send: " << cb_consumer.interest_s << "  Interest_Send_Speed: " <<  cb_consumer.interest_s/seconds << " /Seconds" <<  std::endl;
    std::cout << "Interest_Receive: " << cb_consumer.interest_r << "  Interest_Receive_Speed: " <<  cb_consumer.interest_r /seconds << " /Seconds" <<  std::endl;
    std::cout << "Interest_Retransmit: " << cb_consumer.interest_retx << " Interest_Retransfer_Speed: " << cb_consumer.interest_retx/seconds << " /Seconds" << std::endl;
    std::cout << "Interest_Expire: " << cb_consumer.interest_expr << " Interest_Retransfer_Speed: " << cb_consumer.interest_expr/seconds << " /Seconds" << std::endl;
    std::cout << "Interest_Useful Percentage: " << double(cb_consumer.interest_r)/double(cb_consumer.interest_s)*100 <<"%" << std::endl;
    std::cout << "  " << std::endl;
    std::cout << "---------- OVER ----------  " << seconds <<" seconds" << std::endl;

    //v_fc.scheduler.cancelEvent(v_fc.eventid);
//    io.stop();

    //signalSet.async_wait(bind(&sig_int, _1, _2, ref(signalSet), ref(io)));
    exit(num); 
  }
//
  FrameConsumer::FrameConsumer()
    : m_face(FaceHelper::getFace())
    , m_ioService(m_face->getIoService())
    , m_scheduler(m_ioService)
    , m_interval(0)
    , framenumber(0)
    , interval(0)
  {
  }

  void FrameConsumer::consume_audio_frame(Consumer* sampleConsumer, Name sampleSuffix) {
    sampleConsumer->consume(sampleSuffix);
    while (1) {
      auto start = std::chrono::high_resolution_clock::now();
      m_mut.lock();
      cb_consumer.framenumber_a++;
      Name suffix = Name(std::to_string(cb_consumer.framenumber_a));
      m_mut.unlock();
      sampleConsumer->consume(suffix);
      auto finish = std::chrono::high_resolution_clock::now();
      printf("audio_suffix: %s finish %llu ms\n", suffix.toUri().c_str(), std::chrono::duration_cast<std::chrono::milliseconds>(finish-start).count());
    }
  }
  
  void FrameConsumer::consume_video_frame(Consumer* sampleConsumer, Name sampleSuffix) {
    sampleConsumer->consume(sampleSuffix);
    while (1) {
      auto start = std::chrono::high_resolution_clock::now();
      m_mut.lock();
      cb_consumer.framenumber_v++;
      Name suffix = Name(std::to_string(cb_consumer.framenumber_v));
      m_mut.unlock();
      sampleConsumer->consume(suffix);
      auto finish = std::chrono::high_resolution_clock::now();
      printf("video_suffix: %s finish %llu ms\n", suffix.toUri().c_str(), std::chrono::duration_cast<std::chrono::milliseconds>(finish-start).count());
    }
  }

  void 
  FrameConsumer::audioConsumeFrames() {

    pool tp_audio(AUDIO_SIZE);
//    pool tp_audio(AUDIO_SIZE);
    while (cb_consumer.framenumber_a < AUDIO_SIZE) {
      cb_consumer.framenumber_a++;
//      std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(finish-start).count() << "ns\n";
//      start = finish;
      Name sampleSuffix(std::to_string(cb_consumer.framenumber_a));
      cb_consumer.start_timer_audio();

      int con_num = cb_consumer.framenumber_a % AUDIO_SIZE;
//      printf("--------- Audio Consume Frame Number: %d con_num: %d ---------- handle\n", cb_consumer.framenumber_a, con_num);
      tp_audio.schedule(boost::bind(&FrameConsumer::consume_audio_frame, this, m_consumers[con_num], sampleSuffix));
//      boost::thread(consume_audio_frame, m_consumers[con_num], sampleSuffix);
      std::this_thread::sleep_for(std::chrono::nanoseconds(m_interval));
    }
    tp_audio.wait();
  }

  void 
  FrameConsumer::videoConsumeFrames() {

    pool tp_video(VIDEO_SIZE);
//    pool tp_video(VIDEO_SIZE);
//    eventid = m_scheduler.scheduleEvent(interval, bind(&FrameConsumer::consume_video_thread, this, &cb_consumer, sampleConsumer));
    while(cb_consumer.framenumber_v < VIDEO_SIZE) {
//      start = finish;
      cb_consumer.framenumber_v++;
      Name sampleSuffix(std::to_string(cb_consumer.framenumber_v));
      cb_consumer.start_timer_video();
//      m_consumers[con_num]->consume(sampleSuffix);

//      int i;
//      for (i = 0; i < VIDEO_SIZE; i++) {
//        Name suffix;
//        m_consumers[i]->getContextOption(SUFFIX, suffix);
//        if (suffix.get(0).toUri() == "empty"){
//          break;
//        }
//      }
//
//      if (i == VIDEO_SIZE) {
//        boost::unique_lock<boost::mutex> lock(cb_consumer.mut_v);
//        cb_consumer.ready_v = false;
//        while(!cb_consumer.ready_v)
//        {
//          printf("VIDEO I'm waiting should NOT happen! cb_consumer.framenumber_v : %d \n", cb_consumer.framenumber_v);
//          cb_consumer.con_v.wait(lock);
//          printf("VIDEO waiting Over! cb_consumer.framenumber_v : %d \n", cb_consumer.framenumber_v);
//        }
//
//        for (i = 0; i < VIDEO_SIZE; i++) {
//          Name suffix;
//          m_consumers[i]->getContextOption(SUFFIX, suffix);
//          if (suffix.get(0).toUri() == "empty"){
//            break;
//          }
//        }
//      }

      int con_num = cb_consumer.framenumber_v % VIDEO_SIZE;
      printf("--------- Video Consume Frame Number: %d con_num: %d ---------- handle\n", cb_consumer.framenumber_v, con_num);
      tp_video.schedule(boost::bind(&FrameConsumer::consume_video_frame, this, m_consumers[con_num], sampleSuffix));
//      boost::thread(consume_audio_frame, m_consumers[con_num], sampleSuffix);
      std::this_thread::sleep_for(std::chrono::nanoseconds(m_interval));
    }
    tp_video.wait();
  }

  void
  FrameConsumer::setup_audio(std::string live_prefix, std::string stream_id) {

    int samplerate = 441; //(cb_consumer.player).get_audio_rate();
    interval = time::nanoseconds(1000000 * time::microseconds(10)) / samplerate;
    m_interval = 1000000 / samplerate * 10000;
    framenumber = cb_consumer.start_frame_a;

    printf("Audio start_FrameNumber: %d interval_a: %llu\n", framenumber, m_interval);

    for (int i = 0; i < AUDIO_SIZE; i++) {

      Name sampleName = Name(live_prefix).append(stream_id).append("audio").append("content");

      Consumer* sampleConsumer = new Consumer(sampleName, SDR);

      sampleConsumer->setContextOption(CONTENT_RETRIEVED, 
                            (ConsumerContentCallback)bind(&ConsumerCallback::processPayloadAudio, &cb_consumer, _1, _2, _3));

      sampleConsumer->setContextOption(MUST_BE_FRESH_S, true);
      sampleConsumer->setContextOption(INTEREST_LIFETIME, 500);
  //    sampleConsumer->setContextOption(INTEREST_RETX,5); //Retransmitted Attempted Time.
  //    sampleConsumer->setContextOption(MIN_WINDOW_SIZE, 1);
    
      sampleConsumer->setContextOption(INTEREST_LEAVE_CNTX, 
                                (ConsumerInterestCallback)bind(&ConsumerCallback::processLeavingInterest, &cb_consumer, _1, _2));

      sampleConsumer->setContextOption(INTEREST_RETRANSMIT, 
                                (ConsumerInterestCallback)bind(&ConsumerCallback::onRetx, &cb_consumer, _1, _2));

      sampleConsumer->setContextOption(INTEREST_EXPIRED, 
                                (ConsumerInterestCallback)bind(&ConsumerCallback::onExpr, &cb_consumer, _1, _2));

      sampleConsumer->setContextOption(SUFFIX, Name("empty"));
  //    sampleConsumer->setContextOption(DATA_ENTER_CNTX, 
  //                              (ConsumerDataCallback)bind(&ConsumerCallback::processData, &cb_consumer, _1, _2));

      m_consumers.push_back(sampleConsumer);
    }

    cb_consumer.start_timer_audio();
    m_consumers[0]->consume("0");

    printf("m_consumers_audio size: %lu\n", m_consumers.size());
  }

  void
  FrameConsumer::setup_video(std::string live_prefix, std::string stream_id) {

    int samplerate = 30; //(cb_consumer.player).get_audio_rate();
    interval = time::nanoseconds(1000000 * time::microseconds(1)) / samplerate;
    m_interval = 1000000 / samplerate * 1000;
    framenumber = cb_consumer.start_frame_v;

    printf("Video start_FrameNumber: %d interval_v: %llu\n", framenumber, m_interval);

    for (int i = 0; i < VIDEO_SIZE; i++) {

      Name sampleName = Name(live_prefix).append(stream_id).append("video").append("content");
      Consumer* sampleConsumer = new Consumer(sampleName, RDR);
  
      sampleConsumer->setContextOption(CONTENT_RETRIEVED, 
                            (ConsumerContentCallback)bind(&ConsumerCallback::processPayload, &cb_consumer, _1, _2, _3));
  
      sampleConsumer->setContextOption(DATA_ENTER_CNTX, 
                                      (ConsumerDataCallback)bind(&ConsumerCallback::processData, &cb_consumer, _1, _2));
  
      sampleConsumer->setContextOption(MUST_BE_FRESH_S, true);
      sampleConsumer->setContextOption(INTEREST_LIFETIME, 500);

      sampleConsumer->setContextOption(INTEREST_LEAVE_CNTX, 
                                (ConsumerInterestCallback)bind(&ConsumerCallback::processLeavingInterest, &cb_consumer, _1, _2));

      sampleConsumer->setContextOption(INTEREST_RETRANSMIT, 
                                (ConsumerInterestCallback)bind(&ConsumerCallback::onRetx, &cb_consumer, _1, _2));

      sampleConsumer->setContextOption(INTEREST_EXPIRED, 
                                (ConsumerInterestCallback)bind(&ConsumerCallback::onExpr, &cb_consumer, _1, _2));

      sampleConsumer->setContextOption(SUFFIX, Name("empty"));

      m_consumers.push_back(sampleConsumer);
    }
 
    cb_consumer.start_timer_video();
    m_consumers[0]->consume("0");
    
    printf("m_consumers_video size: %lu\n", m_consumers.size());
  }

  int
  main(int argc, char** argv)
  {
    FrameConsumer a_fc;
    FrameConsumer v_fc;
    try {
      boost::property_tree::ptree pt;
      boost::property_tree::ini_parser::read_ini("config.ini", pt);
      std::cout << "Video Prefix: " << pt.get<std::string>("ndnlive.prefix") << std::endl; 

      std::string stream_id = pt.get<std::string>("ndnlive.stream_id");
      std::string live_prefix = pt.get<std::string>("ndnlive.prefix");

      auto start_info = std::chrono::high_resolution_clock::now();

      if (argc > 1) 
        stream_id = argv[1];

      Name videoinfoName = Name(live_prefix).append(stream_id).append("video").append("streaminfo");
      Consumer* videoinfoConsumer = new Consumer(videoinfoName, RDR);
      videoinfoConsumer->setContextOption(MUST_BE_FRESH_S, true);
//      videoinfoConsumer->setContextOption(RIGHTMOST_CHILD_S, true);
//      videoinfoConsumer->setContextOption(INTEREST_LEAVE_CNTX, 
//        (ConsumerInterestCallback)bind(&ConsumerCallback::processLeavingStreaminfo, &cb_consumer, _1, _2));
      videoinfoConsumer->setContextOption(CONTENT_RETRIEVED, 
        (ConsumerContentCallback)bind(&ConsumerCallback::processStreaminfo, &cb_consumer, _1, _2, _3));
      videoinfoConsumer->setContextOption(DATA_ENTER_CNTX, 
        (ConsumerDataCallback)bind(&ConsumerCallback::streaminfoEnter, &cb_consumer, _1, _2));
      videoinfoConsumer->setContextOption(INTEREST_RETRANSMIT, 
        (ConsumerInterestCallback)bind(&ConsumerCallback::onRetx_info, &cb_consumer, _1, _2));
      videoinfoConsumer->setContextOption(INTEREST_EXPIRED, 
        (ConsumerInterestCallback)bind(&ConsumerCallback::onExpr_info, &cb_consumer, _1, _2));

      videoinfoConsumer->consume(Name("0"));

      Name audioinfoName = Name(live_prefix).append(stream_id).append("audio").append("streaminfo");
      Consumer* audioinfoConsumer = new Consumer(audioinfoName, RDR);
      audioinfoConsumer->setContextOption(MUST_BE_FRESH_S, true);
//      audioinfoConsumer->setContextOption(RIGHTMOST_CHILD_S, true);
//      audioinfoConsumer->setContextOption(INTEREST_LEAVE_CNTX, 
//        (ConsumerInterestCallback)bind(&ConsumerCallback::processLeavingStreaminfo, &cb_consumer, _1, _2));
      audioinfoConsumer->setContextOption(CONTENT_RETRIEVED, 
        (ConsumerContentCallback)bind(&ConsumerCallback::processStreaminfoAudio, &cb_consumer, _1, _2, _3));
      audioinfoConsumer->setContextOption(DATA_ENTER_CNTX, 
        (ConsumerDataCallback)bind(&ConsumerCallback::streaminfoEnterAudio, &cb_consumer, _1, _2));
      audioinfoConsumer->setContextOption(INTEREST_RETRANSMIT, 
        (ConsumerInterestCallback)bind(&ConsumerCallback::onRetx_info, &cb_consumer, _1, _2));
      audioinfoConsumer->setContextOption(INTEREST_EXPIRED, 
        (ConsumerInterestCallback)bind(&ConsumerCallback::onExpr_info, &cb_consumer, _1, _2));

      audioinfoConsumer->consume(Name("0"));

      a_fc.setup_audio(live_prefix, stream_id);      
      v_fc.setup_video(live_prefix, stream_id);      

//      std::string streaminfo_video = "video/x-h264, codec_data=(buffer)01640014ffe1001967640014acd94141fb0110000003001000000303c8f142996001000468efbcb0, stream-format=(string)avc, alignment=(string)au, level=(string)2, profile=(string)high, width=(int)320, height=(int)240, pixel-aspect-ratio=(fraction)1/1, framerate=(fraction)30/1, parsed=(boolean)true";
//      
//      std::string streaminfo_audio = "audio/mpeg, mpegversion=(int)4, channels=(int)1, rate=(int)44100, stream-format=(string)raw, level=(string)2, base-profile=(string)lc, profile=(string)lc, codec_data=(buffer)1208, framed=(boolean)true";
//
//      cb_consumer.player.get_streaminfo(streaminfo_video);
//      cb_consumer.player.get_streaminfo_audio(streaminfo_audio);

//      sleep(1); // because consume() is non-blocking

       
//      cb_consumer.wait_rate();

      auto finish = std::chrono::high_resolution_clock::now();
      printf("%llu ms\n", std::chrono::duration_cast<std::chrono::milliseconds>(finish-start_info).count());

      std::cout << "Video Rate: "<< cb_consumer.frame_rate_v << " Audio Rate: " << cb_consumer.frame_rate_a << std::endl;

      std::cout << "Playing will Start Soon~! Enjoy!" <<std::endl;
      /*      
      Verificator* verificator = new Verificator();
      Name videoName(live_prefix + "/video/content");
      */

      signal(SIGINT, sig_int);
//      boost::asio::signal_set terminationSignalSet(a_fc.m_ioService);
//      terminationSignalSet.add(SIGINT);
//      terminationSignalSet.add(SIGTERM);
//      terminationSignalSet.async_wait(bind(&sig_int, _2, _1, _2,
//                                           ref(terminationSignalSet), ref(a_fc.m_ioService)));

      time_t time_start_0 = std::time(0);

      time_start = std::time(0);
      //audio frames consumer

//      std::thread(std::bind(&FrameConsumer::audioConsumeFrames, &a_fc, live_prefix, stream_id));
      boost::thread(boost::bind(&FrameConsumer::audioConsumeFrames, &a_fc));
      boost::thread(boost::bind(&FrameConsumer::videoConsumeFrames, &v_fc));

//      a_fc.audioConsumeFrames(live_prefix, stream_id);
      //video consumer
//      v_fc.videoConsumeFrames(live_prefix, stream_id);

//      time_t time_end_0  = std::time(0);
//      std::cout << "After consume " << time_end_0 << std::endl;
//      double seconds = difftime(time_end_0, time_start_0);
//      std::cout << seconds << " seconds have passed" << std::endl;

        sleep(10000000);
//      a_fc.m_ioService.run();
    }
    catch(std::exception& e) {
      std::cerr << "ERROR: " << e.what() << std::endl;
      return 1;
    }
    return 0;
  }
} // namespace ndn

int
main(int argc, char** argv)
{
  return ndn::main(argc, argv);
}

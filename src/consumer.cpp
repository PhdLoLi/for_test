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
// Enclosing code in ndn simplifies coding (can also use `using namespace ndn`)
namespace ndn {
// Additional nested namespace could be used to prevent/limit name contentions


time_t time_start;
ConsumerCallback cb_consumer;


  //when control-c detected, doing the analysis 
static void sig_int(int num,
                    const boost::system::error_code& error,
                    int signalNo,
                    boost::asio::signal_set& signalSet,
                    boost::asio::io_service& io)
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
    io.stop();

    //signalSet.async_wait(bind(&sig_int, _1, _2, ref(signalSet), ref(io)));
    exit(num); 
  }
//
  FrameConsumer::FrameConsumer()
    : m_face(FaceHelper::getFace())
    , m_ioService(m_face->getIoService())
    , m_scheduler(m_ioService)
    , framenumber(0)
    , interval(0)
  {
  }

  void consume_audio_frame(Consumer* sampleConsumer, Name sampleSuffix) {
    sampleConsumer->consume(sampleSuffix);
  }
  
  void consume_video_frame(ConsumerCallback* cb_consumer, Consumer* sampleConsumer, Name sampleSuffix) {
      cb_consumer->initFrameVideo();
      sampleConsumer->consume(sampleSuffix);
      cb_consumer->pushQueueVideo();
  }

    void
    FrameConsumer::consume_audio_thread(Consumer* sampleConsumer)
    {

      printf("--------- Audio Consume Frame Number: %d ---------- start\n ", framenumber);
//    con->cb->initFrameAudio();
      eventid = m_scheduler.scheduleEvent(interval, bind(&FrameConsumer::consume_audio_thread, this, sampleConsumer));
//      std::cout << "Inside Audio Thread " << framenumber << std::endl;
      Name sampleSuffix(std::to_string(framenumber++));
      boost::thread(consume_audio_frame, sampleConsumer, sampleSuffix);
//      consume_audio_frame(sampleConsumer, sampleSuffix);
      printf("--------- Audio Consume Frame Number: %d ---------- over\n", framenumber - 1);
//    con->cb->initFrameAudio();
//    con->cb->pushQueueAudio();
//      std::cout << "Audio Frame " << framenumber - 1 << " Over!" << std::endl;
    }

    void
    FrameConsumer::consume_video_thread(ConsumerCallback* cb_consumer, Consumer* sampleConsumer)
    {
      printf("--------- Video Consume Frame Number: %d ---------- start\n", framenumber);

      eventid = m_scheduler.scheduleEvent(interval, bind(&FrameConsumer::consume_video_thread, this, cb_consumer, sampleConsumer));
//      std::cout << "Inside Video Thread " << framenumber << std::endl;
      Name sampleSuffix(std::to_string(framenumber++));

      boost::thread(consume_video_frame, cb_consumer, sampleConsumer, sampleSuffix);
//      consume_video_frame(cb_consumer, sampleConsumer, sampleSuffix);

      printf("--------- Video Consume Frame Number: %d ---------- over\n", framenumber - 1);
//      std::cout << "Video Frame " << framenumber - 1 << " Over!" << std::endl;
    }

  void 
  FrameConsumer::audioConsumeFrames(std::string live_prefix, std::string stream_id) {
    int samplerate = 441; //(cb_consumer.player).get_audio_rate();

    interval = time::nanoseconds(1000000 * time::microseconds(10)) / samplerate;

    std::cout << "audio interval " << interval << std::endl;
    Name sampleName = Name(live_prefix).append(stream_id).append("audio").append("content");
    framenumber = cb_consumer.start_frame_a;
    Consumer* sampleConsumer; 
    sampleConsumer = new Consumer(sampleName, SDR);
    sampleConsumer->setContextOption(CONTENT_RETRIEVED, 
                          (ConsumerContentCallback)bind(&ConsumerCallback::processPayloadAudio, &cb_consumer, _1, _2, _3));
    sampleConsumer->setContextOption(MUST_BE_FRESH_S, true);
    sampleConsumer->setContextOption(INTEREST_LIFETIME, 2000);
//    sampleConsumer->setContextOption(INTEREST_RETX,5); //Retransmitted Attempted Time.
//    sampleConsumer->setContextOption(MIN_WINDOW_SIZE, 1);
    sampleConsumer->setContextOption(INTEREST_LEAVE_CNTX, 
                              (ConsumerInterestCallback)bind(&ConsumerCallback::processLeavingInterest, &cb_consumer, _1, _2));
    sampleConsumer->setContextOption(INTEREST_RETRANSMIT, 
                              (ConsumerInterestCallback)bind(&ConsumerCallback::onRetx, &cb_consumer, _1, _2));
    sampleConsumer->setContextOption(INTEREST_EXPIRED, 
                              (ConsumerInterestCallback)bind(&ConsumerCallback::onExpr, &cb_consumer, _1, _2));
//    sampleConsumer->setContextOption(DATA_ENTER_CNTX, 
//                              (ConsumerDataCallback)bind(&ConsumerCallback::processData, &cb_consumer, _1, _2));
//      sampleConsumer->setContextOption(DATA_TO_VERIFY,consume_audio_thread
//            (DataVerificationCallback)bind(&Verificator::onPacket, verificator, _1));
    std::cout <<"Audio startFrameNum " << framenumber << std::endl;
//    m_scheduler.scheduleEvent(time::milliseconds(0),
//                              [this] () {
//                                std::cout << "OKKKKK" << std::endl;
//                              });
    eventid = m_scheduler.scheduleEvent(interval, bind(&FrameConsumer::consume_audio_thread, this, sampleConsumer));
  }

  void 
  FrameConsumer::videoConsumeFrames(std::string live_prefix, std::string stream_id) {
    int samplerate = 30; //(cb_consumer.player).get_audio_rate();
//    time::nanoseconds interval_v = time::nanoseconds(1000000*timeWindow) / nInterests; 
    interval = time::nanoseconds(1000000 * time::microseconds(1)) / samplerate;
    std::cout << "video interval " << interval << std::endl;

    Name sampleName = Name(live_prefix).append(stream_id).append("video").append("content");
    framenumber = cb_consumer.start_frame_v;
    Consumer* sampleConsumer; 
    sampleConsumer = new Consumer(sampleName, UDR);

    sampleConsumer->setContextOption(MAX_WINDOW_SIZE, 16);

    sampleConsumer->setContextOption(DATA_ENTER_CNTX, 
                                    (ConsumerDataCallback)bind(&ConsumerCallback::processDataVideo, &cb_consumer, _1, _2));
    sampleConsumer->setContextOption(MUST_BE_FRESH_S, true);
    sampleConsumer->setContextOption(INTEREST_LIFETIME, 2000);
    sampleConsumer->setContextOption(INTEREST_LEAVE_CNTX, 
                              (ConsumerInterestCallback)bind(&ConsumerCallback::processLeavingInterest, &cb_consumer, _1, _2));
    sampleConsumer->setContextOption(INTEREST_RETRANSMIT, 
                              (ConsumerInterestCallback)bind(&ConsumerCallback::onRetx, &cb_consumer, _1, _2));
    sampleConsumer->setContextOption(INTEREST_EXPIRED, 
                              (ConsumerInterestCallback)bind(&ConsumerCallback::onExpr, &cb_consumer, _1, _2));

    std::cout <<"Video startFrameNum " << framenumber << std::endl;
    eventid = m_scheduler.scheduleEvent(interval, bind(&FrameConsumer::consume_video_thread, this, &cb_consumer, sampleConsumer));
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

      if (argc > 1) 
        stream_id = argv[1];

      Name videoinfoName = Name(live_prefix).append(stream_id).append("video").append("streaminfo");
      Consumer* videoinfoConsumer = new Consumer(videoinfoName, SDR);
      videoinfoConsumer->setContextOption(MUST_BE_FRESH_S, true);
//      videoinfoConsumer->setContextOption(RIGHTMOST_CHILD_S, true);
//      videoinfoConsumer->setContextOption(INTEREST_LEAVE_CNTX, 
//        (ConsumerInterestCallback)bind(&ConsumerCallback::processLeavingStreaminfo, &cb_consumer, _1, _2));
      videoinfoConsumer->setContextOption(CONTENT_RETRIEVED, 
        (ConsumerContentCallback)bind(&ConsumerCallback::processStreaminfo, &cb_consumer, _1, _2, _3));
      videoinfoConsumer->setContextOption(DATA_ENTER_CNTX, 
        (ConsumerDataCallback)bind(&ConsumerCallback::streaminfoEnter, &cb_consumer, _1, _2));

      if(argc > 1)
        videoinfoConsumer->consume(Name(argv[1]));
      else
        videoinfoConsumer->consume(Name(""));

      Name audioinfoName = Name(live_prefix).append(stream_id).append("audio").append("streaminfo");
      Consumer* audioinfoConsumer = new Consumer(audioinfoName, SDR);
      audioinfoConsumer->setContextOption(MUST_BE_FRESH_S, true);
//      audioinfoConsumer->setContextOption(RIGHTMOST_CHILD_S, true);
//      audioinfoConsumer->setContextOption(INTEREST_LEAVE_CNTX, 
//        (ConsumerInterestCallback)bind(&ConsumerCallback::processLeavingStreaminfo, &cb_consumer, _1, _2));
      audioinfoConsumer->setContextOption(CONTENT_RETRIEVED, 
        (ConsumerContentCallback)bind(&ConsumerCallback::processStreaminfoAudio, &cb_consumer, _1, _2, _3));
      audioinfoConsumer->setContextOption(DATA_ENTER_CNTX, 
        (ConsumerDataCallback)bind(&ConsumerCallback::streaminfoEnterAudio, &cb_consumer, _1, _2));

      if(argc > 1)
        audioinfoConsumer->consume(Name(argv[1]));
      else
        audioinfoConsumer->consume(Name(""));

      sleep(1); // because consume() is non-blocking
      std::cout << "consume whole start!" <<std::endl;
/*      
      Verificator* verificator = new Verificator();
      Name videoName(live_prefix + "/video/content");
      */

      // signal(SIGINT, sig_int);
      boost::asio::signal_set terminationSignalSet(a_fc.m_ioService);
      terminationSignalSet.add(SIGINT);
      terminationSignalSet.add(SIGTERM);
      terminationSignalSet.async_wait(bind(&sig_int, _2, _1, _2,
                                           ref(terminationSignalSet), ref(a_fc.m_ioService)));

      time_t time_start_0 = std::time(0);

      time_start = std::time(0);
      //audio frames consumer
      a_fc.audioConsumeFrames(live_prefix, stream_id);
      //video consumer
      v_fc.videoConsumeFrames(live_prefix, stream_id);

//      time_t time_end_0  = std::time(0);
//      std::cout << "After consume " << time_end_0 << std::endl;
//      double seconds = difftime(time_end_0, time_start_0);
//      std::cout << seconds << " seconds have passed" << std::endl;

      a_fc.m_ioService.run();
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

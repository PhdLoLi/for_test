
/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Regents of the University of California.
 *
 * @author Lijing Wang <phdloli@ucla.edu>
 */

#ifndef CONSUMER_HPP
#define CONSUMER_HPP
#include "consumer-callback.hpp"
#include <pthread.h>
#include <ndn-cxx/security/validator.hpp>

namespace ndn{

#define IDENTITY_NAME "/Lijing/Wang"
  
  class Verificator
  {
  public:
    Verificator()
    {
      Name identity(IDENTITY_NAME);
      Name keyName = m_keyChain.getDefaultKeyNameForIdentity(identity);
      m_publicKey = m_keyChain.getPublicKey(keyName); 
    };
    
    bool
    onPacket(Data& data)
    {
    //  return true;
      if (Validator::verifySignature(data, *m_publicKey))
      {
        std::cout << "Verified Name:" << data.getName() <<  std::endl;
        return true;
      }
      else
      {
        std::cout << "NOT Verified" << data.getName() << std::endl;
        return false;
      }
    }
    
  private:
    KeyChain m_keyChain;
    shared_ptr<PublicKey> m_publicKey;
 };

  struct Consumer_Need
  {
    ConsumerCallback *cb;
  };

  class FrameConsumer
  {
   public:
    shared_ptr<Face> m_face;
    boost::asio::io_service& m_ioService;
    Scheduler m_scheduler;
    
    int framenumber;
    EventId eventid;
    int interval;
//    int v_framenumber;

    FrameConsumer();

    void
    audioConsumeFrames(std::string, std::string);
    void
    videoConsumeFrames(std::string, std::string);

   private:
    void
    consume_audio_thread(Consumer* sampleConsumer);

    void
    consume_video_thread(ConsumerCallback* cb_consumer, Consumer* sampleConsumer);
  };
}
#endif

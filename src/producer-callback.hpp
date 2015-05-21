/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014 Regents of the University of California.
 *
 * @author Lijing Wang <phdloli@ucla.edu>
 */
#ifndef PRODUCER_CALLBACK_HPP
#define PRODUCER_CALLBACK_HPP
#include <ndn-cxx/contexts/producer-context.hpp>
#include <ndn-cxx/contexts/application-nack.hpp>
namespace ndn {
// Additional nested namespace could be used to prevent/limit name contentions
  class ProducerCallback
  {
  public:
    ProducerCallback();
    
    void
    setProducer(Producer* p);
    
    void
    setSampleNumber(size_t* n);

    void
    setStreaminfoVideo(std::string video);

    void 
    setStreaminfoAudio(std::string audio);

    void
    processConstData(Producer& pro, const Data& data);
    
    void
    processStreaminfoInterest(Producer& pro, const Interest& interest);

    void
    processInterest(Producer& pro, const Interest& interest);
    
    void
    processOutgoingData(Producer& pro, const Data& data);
   
    void
    processOutgoingStreaminfo(Producer& pro, const Data& data);


    void
    processIncomingInterest(Producer& pro, const Interest& interest);
    
    bool
    verifyInterest(Interest& interest);
    
    void
    processConstInterest(Producer& pro, const Interest& interest);
  
    unsigned long interest_incoming = 0;
    unsigned long interest_outgoing = 0;
    unsigned long interest_nohit = 0;
    size_t* m_curnum;

  private:
    Producer* m_producer;
    std::string m_streaminfovideo;
    std::string m_streaminfoaudio;
  };
  
} // namespace ndn
#endif

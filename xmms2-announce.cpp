/*  XMMS2 - X Music Multiplexer System
 *  Copyright (C) 2003-2006 XMMS2 Team
 *  Copyright (C) 2013 Andreas Ots <andreasots@gmail.com>
 * 
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *                   
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  This file is a part of the XMMS2 client tutorial #8
 *  Passing extra data to signal functions.
 */

#include <xmmsclient/xmmsclient++.h>
#include <xmmsclient/xmmsclient++-glib.h>

#include <glib.h>

#include <espeak/speak_lib.h>

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>

class XmmsAnnouncer {
 public:
  XmmsAnnouncer();
  ~XmmsAnnouncer();

  void speak(const std::string&);

 private:
  bool id_cb(const std::string&, const int& id );
  bool info_cb(const std::string&, const Xmms::PropDict& propdict);
  bool status_cb(xmms_playback_status_t);
  bool error_handler(const std::string& function, const std::string& error);

  Xmms::Client _client;
  GMainLoop* _ml;
};

XmmsAnnouncer::XmmsAnnouncer()
    : _client("xmms2-announcer"), _ml(0) {
  _client.connect(std::getenv("XMMS_PATH"));
  _client.playback.broadcastCurrentID()(
      std::bind(std::mem_fn(&XmmsAnnouncer::id_cb), this,
                std::string(), std::placeholders::_1),
      std::bind(std::mem_fn(&XmmsAnnouncer::error_handler), this,
                "client.playback.broadcastCurrentID()", std::placeholders::_1));
  _client.playback.broadcastStatus()(
      std::bind(std::mem_fn(&XmmsAnnouncer::status_cb), this, std::placeholders::_1),
      std::bind(std::mem_fn(&XmmsAnnouncer::error_handler), this,
                "client.playback.broadcastStatus()", std::placeholders::_1));
  
  if(espeak_Initialize(AUDIO_OUTPUT_PLAYBACK, 0, nullptr, 0) == -1)
    throw std::runtime_error("espeak_Initialize() failed");

  _client.setMainloop(new Xmms::GMainloop(_client.getConnection()));
  _ml = g_main_loop_new(0, 0);
  g_main_loop_run(_ml);
}

XmmsAnnouncer::~XmmsAnnouncer() {
  espeak_Terminate();
}

void XmmsAnnouncer::speak(const std::string& text) {
  if(espeak_Cancel() != EE_OK)
    throw std::runtime_error("espeak_Cancel() failed");
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  std::cout << text << std::endl;
  if(espeak_Synth(text.c_str(), text.size(), 0, POS_CHARACTER, 0,
      espeakCHARS_AUTO | espeakENDPAUSE, nullptr, nullptr) != EE_OK)
    throw std::runtime_error("espeak_Syth() failed");
}

bool XmmsAnnouncer::status_cb(xmms_playback_status_t status) {
  switch(status) {
    case Xmms::Playback::PLAYING:
      _client.playback.currentID()(
        std::bind(std::mem_fn(&XmmsAnnouncer::id_cb), this,
	          std::string("Playing"), std::placeholders::_1),
        std::bind(std::mem_fn(&XmmsAnnouncer::error_handler), this,
                  "client.playback.currentID()", std::placeholders::_1));
      break;
    case Xmms::Playback::STOPPED:
      speak("Stopped");
      break;
    case Xmms::Playback::PAUSED:
      speak("Paused");
      break;
  }
  return true;
}

bool XmmsAnnouncer::id_cb(const std::string& prefix, const int& id) {
  Xmms::PropDictResult res = _client.medialib.getInfo(id);
  res.connect(std::bind(std::mem_fn(&XmmsAnnouncer::info_cb), this,
                        prefix, std::placeholders::_1));
  res.connectError(std::bind(std::mem_fn(&XmmsAnnouncer::error_handler), this,
                             "client.medialib.getInfo()",
			     std::placeholders::_1));
  res();
  return true;
}

bool XmmsAnnouncer::info_cb(const std::string& prefix, const Xmms::PropDict& propdict)
{
  std::stringstream ss;
  ss << prefix << " '" << propdict.get<std::string>("title") << "' by '";
  ss << propdict.get<std::string>("artist") << "' from album '";
  ss << propdict.get<std::string>("album") << "'";
  speak(ss.str());
  return true;
}

bool XmmsAnnouncer::error_handler(const std::string& function, const std::string& error) {
  // print function name and error message...
  std::cout << "Error in function: " << function << " - " << error << std::endl;
  return true;
}

int main(int argc, char *argv[]) {
  try {
    XmmsAnnouncer client;
  } catch(const std::exception& e) {
    std::cerr << "Caught an exception: e.what(): " << e.what() << std::endl;
  }
  return EXIT_SUCCESS;
}
